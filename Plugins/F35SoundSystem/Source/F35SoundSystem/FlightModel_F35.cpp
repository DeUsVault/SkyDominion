// Copyright SOUNDFX STUDIO © 2022


#include "FlightModel_F35.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Get Plane Altitude
int32 UFlightModel_F35::GetAltitude() { return (int32)(owner->GetComponentLocation().Z / 100.f); }

// Is Plane On Land
bool UFlightModel_F35::IsPlaneOnLand() const { return bIsOnLand; }

// Is Engine Running
bool UFlightModel_F35::IsEngineRunning() const { return bIsEngineRunning; }

void UFlightModel_F35::SwitchbIsEngineRunning() { bIsEngineRunning = !bIsEngineRunning; }

// Get Plane Velocity
FVector UFlightModel_F35::GetVel_l() const { return v_l; }

// Get Pitch Rate
float UFlightModel_F35::GetPitchRate() { return wp; }

// Get Roll Rate
float UFlightModel_F35::GetRollRate() const { return wr; }

// Get RPM
float UFlightModel_F35::GetRPM() { return rpm; }

// Get Speed
float UFlightModel_F35::GetSpeed() { return speed; }


// Get Scale for Camera Shake
float UFlightModel_F35::GetScaleForCameraShake() { return FMath::Abs(wp) / Pitch_Rate; }

// Sets default values for this component's properties
UFlightModel_F35::UFlightModel_F35()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UFlightModel_F35::BeginPlay()
{
	Super::BeginPlay();

	owner = CastChecked<UStaticMeshComponent>(GetOwner()->GetRootComponent());

	if (bInitOnRunway)
	{
		InitOnRunway();
		if (bIsReadyToTakeoff)
		{
			bIsEngineRunning = true;
		}
	}
	else
	{
		speed = Speed_Nom;
		rpm = rpmr = 0.9f;
		v_ort_l = owner->GetForwardVector();
		v_l = speed * v_ort_l;

		// Start position and rotation for plane in the sky
		pos_w = FVector(FMath::RandRange(-180000.f, 220000.f), 0.f, FMath::RandRange(400000.f, 500000.f));	// (-220000.f, 0.f, 40000.f) for Landing, (-200000.f, 0.f, 450000.f) for Flying
		rot_w = FQuat(owner->GetComponentRotation());
	}
	
}


// Called every frame
void UFlightModel_F35::TickComponent(float dt, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(dt, TickType, ThisTickFunction);
}


void UFlightModel_F35::Movement(float dt, FVector& out_pos, FQuat& out_rot)
{
	bIsOnLand = GetAltitude() <= 0;

	FVector angles = owner->GetComponentRotation().Euler();

	roll = -angles.X;
	pitch = angles.Y;
	yaw = angles.Z < 0 ? angles.Z + 360 : angles.Z;

	if (stick_roll > -0.001f && stick_roll < 0.001f)
		stick_roll = 0.f;
	if (stick_pitch > -0.001f && stick_pitch < 0.001f)
		stick_pitch = 0.f;
	if (rudder > -0.001f && rudder < 0.001f)
		rudder = 0.f;

	float speedr = speed > Speed_Min ? Speed_Nom : speed < 700 ? 0 : 800;
	float dvMin = 0.02f + 0.1f * speed / Speed_Max;
	float dvMax = 0.1f;

	if (throttle == 1.f)
	{
		speedr = Speed_Max;
		dvMax *= 2;

		rpmr += dt * 0.5f;
		if (rpmr > 1.f)
			rpmr = 1.f;
	}
	else if (throttle == -1.f)
	{
		speedr = bIsOnLand ? 0 : Speed_Min;

		dvMin *= GetAltitude() > 40.f ? 2 : -0.45f * GetAltitude() + 20.f;

		rpmr -= dt * 0.5f;
		if (rpmr < 0.f)
			rpmr = 0.f;
	}

	rpm += (rpmr - rpm) * dt * 1.2f;

	FVector forwardVector = owner->GetForwardVector();

	speedr -= 0.3f * 90 * forwardVector.Z;
	if (pos_w.Z > 2000 && forwardVector.Z > -0.1)
		speedr -= 0.02f * (pos_w.Z - 2000);
	if (speedr < 0)
		speedr = 0;

	float dv_wp = 0.1f * FMath::Abs(wp) / Pitch_Rate + 0.0005f * 90 * forwardVector.Z;
	float dspeedReq = FMath::Clamp(1.5f * (speedr - speed), -(dvMin + dv_wp) * Speed_Nom, (dvMax - dv_wp) * Speed_Nom);

	stick_pitch = FMath::Clamp(stick_pitch, -1.0f, 1.0f);
	stick_roll = FMath::Clamp(stick_roll, -1.0f, 1.0f);

	float stick_pitch_ = stick_pitch < 0 ? 0.5f * stick_pitch : stick_pitch;

	float speed_k = v_speedvim < Speed_Nom ? FMath::Max(0.0f, (v_speedvim - 3500) / (Speed_Nom - 3500)) : Speed_Nom / v_speedvim;

	float stick_wp = stick_pitch_ * speed_k;
	float stick_wr = stick_roll * speed_k;
	float rudder_wy = rudder * speed_k;

	float wr_req = Roll_Rate * stick_wr;
	float wp_req = Pitch_Rate * stick_wp;
	float wy_req = Yaw_Rate * rudder_wy;

	if (FMath::Abs(roll) > 45 && FMath::Abs(roll) < 135)
		wy_req -= 5 * FMath::Sin(2 * (roll - (roll > 0 ? 45 : -45)) * FMath::DegreesToRadians(1)) * FMath::Cos(pitch * FMath::DegreesToRadians(1));

	dspeed += 2.5f * (dspeedReq - dspeed) * dt;

	float land_k = 1.f;
	if (throttle == -1 && bIsOnLand && speed < 3000.f)
		land_k = -5E-04f * speed + 2.5f;
	speed += land_k * Speed_Rate * dspeed * dt;
	if ((speed < 50.f && throttle != 1) || (speed < 0))
		speed = 0.f;

	wr += 5.5f * (wr_req - wr) * dt;
	wp += 3.5f * (wp_req - wp) * dt;
	wy += 5.5f * (wy_req - wy) * dt;

	if (bIsOnLand && dspeed < 0 && speed < 3 && speed > 0.1f)
		wp -= 15 * (speed - 0.3f) * dt;

	for (int32 i = 0; i < 3; i++)
	{
		v_ort_l[i] += (forwardVector[i] - v_ort_l[i]) * dt;
	}

	v_w = speed * v_ort_l;

	if (v_speedvim < Speed_Nom)
		v_w.Z -= 15 * (Speed_Nom - v_speedvim) / (Speed_Nom - Speed_Min);

	if (bIsOnLand && v_w.Z < 0)
		v_w.Z = 0;

	v_l = UKismetMathLibrary::InverseTransformDirection(GetOwner()->GetTransform(), v_w);

	v_speedvim = v_l.X;

	float rot_P = wp - 9 * (v_l.Z + FMath::Min(v_l.Y, 0.f)) / (speed + 1.0f);
	float rot_R = wr;
	float rot_Y = wy + 10 * v_l.Y / (speed + 1.0f);

	if (GetAltitude() < 200.f && throttle != 1.f)
	{
		rot_P += 0.005f * (200.f - GetAltitude()) * (rot_P - owner->GetComponentRotation().Pitch) * dt * 15.f;
		rot_R += 0.005f * (200.f - GetAltitude()) * (rot_R - owner->GetComponentRotation().Roll) * dt * 15.f;
	}

	//owner->AddLocalRotation(FQuat::MakeFromEuler(FVector(rot_R, rot_P, rot_Y) * dt));
	out_rot = FQuat::MakeFromEuler(FVector(rot_R, rot_P, rot_Y) * dt);
	pos_w += v_w * dt;

	//owner->SetWorldLocation(pos_w);
	out_pos = pos_w;
}

// Set Parameters for Initialization on Runway
void UFlightModel_F35::InitOnRunway()
{
	bIsOnLand = true;
	bIsEngineRunning = false;
	v_l = FVector(0);
	v_ort_l = owner->GetForwardVector();
	speed = dspeed = rpm = rpmr = 0;
	wy = wp = wr = 0;

	// Start position and rotation for plane on Runway
	pos_w = owner->GetComponentLocation();//FVector(-30000.f, 0.f, 15.2f);
	//owner->SetWorldLocation(pos_w);
	rot_w = FQuat(owner->GetComponentRotation());
}