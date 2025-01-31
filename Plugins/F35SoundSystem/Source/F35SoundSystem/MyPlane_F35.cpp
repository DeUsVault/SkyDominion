// Copyright SOUNDFX STUDIO © 2022

#include "MyPlane_F35.h"


// Sets default values
AMyPlane_F35::AMyPlane_F35()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts
void AMyPlane_F35::BeginPlay()
{
	Super::BeginPlay();

	fm = GetComponentByClass<UFlightModel_F35>();
}


void AMyPlane_F35::Movement(FVector pos, FQuat rot)
{	
	AddActorLocalRotation(rot);
	SetActorLocation(pos);
}
// Called every frame
void AMyPlane_F35::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (fm && fm->IsEngineRunning())
	{
		fm->Movement(DeltaTime, plane_pos, plane_rot);

		if (GetLocalRole() == ROLE_AutonomousProxy)
			Movement_ForMultiplayer(plane_pos, plane_rot);
		else if (GetLocalRole() == ROLE_Authority)
			Movement(plane_pos, plane_rot);
	}
}


void AMyPlane_F35::Movement_ForMultiplayer(FVector pos, FQuat rot)
{
	Movement(pos, rot);

	if (!HasAuthority())
		Server_Movement(pos, rot);
}


void AMyPlane_F35::Server_Movement_Implementation(FVector pos, FQuat rot)
{
	Multi_Movement(pos, rot);
}
bool AMyPlane_F35::Server_Movement_Validate(FVector pos, FQuat rot) { return true; }

void AMyPlane_F35::Multi_Movement_Implementation(FVector pos, FQuat rot)
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
		Movement(pos, rot);
}
bool AMyPlane_F35::Multi_Movement_Validate(FVector pos, FQuat rot) { return true; }
