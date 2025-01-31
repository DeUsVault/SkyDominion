// Copyright SOUNDFX STUDIO © 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FlightModel_F35.h"
#include "Net/UnrealNetwork.h"
#include "MyPlane_F35.generated.h"

UCLASS()
class F35SOUNDSYSTEM_API AMyPlane_F35 : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMyPlane_F35();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY()
	UFlightModel_F35* fm;

	UFUNCTION(BlueprintCallable, Category = SoundFunctionsForMultiplayer)
		void Movement(FVector pos, FQuat rot);

	// Movement function
	UFUNCTION(BlueprintCallable, Category = SoundFunctionsForMultiplayer)
		void Movement_ForMultiplayer(FVector pos, FQuat rot);

protected:
	
	UPROPERTY()
		FVector plane_pos;
	UPROPERTY()
		FQuat plane_rot;

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_Movement(FVector pos, FQuat rot);
	void Server_Movement_Implementation(FVector pos, FQuat rot);
	bool Server_Movement_Validate(FVector pos, FQuat rot);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void Multi_Movement(FVector pos, FQuat rot);
	void Multi_Movement_Implementation(FVector pos, FQuat rot);
	bool Multi_Movement_Validate(FVector pos, FQuat rot);

};
