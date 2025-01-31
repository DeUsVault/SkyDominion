// Copyright SOUNDFX STUDIO © 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlightModel_F35.generated.h"


/**
 * PLANE FLIGHT MODEL
 */
 
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class F35SOUNDSYSTEM_API UFlightModel_F35 : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFlightModel_F35();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Plane adjustable settings
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PlaneSettings)
		float Speed_Max = 34300.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PlaneSettings)
		float Speed_Min = 6000.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PlaneSettings)
		float Speed_Nom = 12500.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PlaneSettings)
		float Speed_Rate = 0.2f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PlaneSettings)
		float Roll_Rate = 250.f;//650.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PlaneSettings)
		float Pitch_Rate = 45.f;//310.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PlaneSettings)
		float Yaw_Rate = 10.f;//40.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PlaneSettings)
		bool bInitOnRunway = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = PlaneSettings)
		bool bIsReadyToTakeoff = true;

	// Get Plane Altitude
	UFUNCTION(BlueprintPure, Category = "Get FM Params")
		int32 GetAltitude();

	// Is Plane On Land
	UFUNCTION(BlueprintPure, Category = "Get FM Params")
		bool IsPlaneOnLand() const;

	// Is Engine Running
	UFUNCTION(BlueprintPure, Category = "Get FM Params")
		bool IsEngineRunning() const;

	// Set bIsEngineRunning
	UFUNCTION(BlueprintCallable, Category = "FM Switch Engine")
		void SwitchbIsEngineRunning();

	// Get Plane Velocity
	FVector GetVel_l() const;

	// Get Pitch Rate
	UFUNCTION(BlueprintPure, Category = "Get FM Params")
	float GetPitchRate();

	// Get Roll Rate
	float GetRollRate() const;

	// Get RPM
	UFUNCTION(BlueprintPure, Category = "Get FM Params")
		float GetRPM();

	// Get Speed
	UFUNCTION(BlueprintPure, Category = "Get FM Params")
		float GetSpeed();


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// ROLL
	UPROPERTY(BlueprintReadWrite, Category = "FM Control")
		float stick_roll = 0.f;
	// PITCH
	UPROPERTY(BlueprintReadWrite, Category = "FM Control")
		float stick_pitch = 0.f;
	// RUDDER
	UPROPERTY(BlueprintReadWrite, Category = "FM Control")
		float rudder = 0.f;
	// THROTTLE
	UPROPERTY(BlueprintReadWrite, Category = "FM Control")
		float throttle = 0.f;

	// Get Scale for Camera Shake
	UFUNCTION(BlueprintPure, Category = "Get FM Params")
		float GetScaleForCameraShake();

private:

	float speed, rpm;
	float dspeed = 0, v_speedvim = 0, rpmr = 0;
	float wy = 0.0f, wp = 0.0f, wr = 0.0f;
	float yaw = 0, pitch = 0, roll = 0;

	FVector v_ort_l = FVector(1, 0, 0);
	FVector v_l, v_w;
	FVector pos_w;

	FQuat rot_w;

	bool bIsOnLand = false, bIsEngineRunning = true;

	// Set Parameters for Initialization on Runway
	void InitOnRunway();

public: 

	UFUNCTION(BlueprintCallable, Category = SoundFunctionsForMultiplayer)
		void Movement(float dt, FVector& out_pos, FQuat& out_rot);

protected:

	UPROPERTY()
	UStaticMeshComponent* owner;
};
