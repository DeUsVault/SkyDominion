// Copyright SOUNDFX STUDIO © 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sounds_F35.generated.h"

// Forward Declaration
class UAudioComponent;
class USoundCue;
class USoundAttenuation;
class USoundClass;


// List of Sound Mixes
UENUM(BlueprintType)
enum SoundMixes_F35 {
	SM_Base = 0,
	SM_MainGamePlay,
	SM_Cockpit,
	//
	SoundMixes_Max
};


USTRUCT(BlueprintType)
struct FSoundParams_F35
{
	GENERATED_BODY()

	// For Multiplayer: replicate cockpit sounds or not
	UPROPERTY(BlueprintReadWrite, Category = SoundParams)
		bool bPlayCockpitSounds = true;
	UPROPERTY(BlueprintReadWrite, Category = SoundParams)
		bool bIsEngineRunning = false;
	UPROPERTY(BlueprintReadWrite, Category = SoundParams)
		float rpm = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = SoundParams)
		float speed = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = SoundParams)
		float pitchRate = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = SoundParams)
		float frontVolumeMultiplier = 1.f;
	UPROPERTY(BlueprintReadWrite, Category = SoundParams)
		float backVolumeMultiplier = 1.f;
	UPROPERTY(BlueprintReadWrite, Category = SoundParams)
		float dopplerPitchMultiplier = 1.f;
	UPROPERTY(BlueprintReadWrite, Category = SoundParams)
		float flybyMultiplier = 1.f;
	UPROPERTY(BlueprintReadWrite, Category = SoundParams)
		float flybyLPFilter = 44.100f;
	UPROPERTY(BlueprintReadWrite, Category = SoundParams)
		float vol_MasterExternal = 1.f;
	UPROPERTY(BlueprintReadWrite, Category = SoundParams)
		float vol_MasterCockpit = 1.f;
};


UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class F35SOUNDSYSTEM_API USounds_F35 : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USounds_F35();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Sounds adjustable settings
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = SoundSettings)
		bool bPlayMusic = true;

	// SOUND VOLUMES

	// Master Cockpit Volume
	UPROPERTY(EditAnywhere, Category = MasterSoundVolumes, meta = (UIMin = "0.0", UIMax = "1.0"))
		float volume_MasterCockpit = 0.2f;
	// Master Cockpit Volume
	UPROPERTY(EditAnywhere, Category = MasterSoundVolumes, meta = (UIMin = "0.0", UIMax = "1.0"))
		float volume_MasterExternal = 0.5f;
	// Master Flybys Volume
	UPROPERTY(EditAnywhere, Category = MasterSoundVolumes, meta = (UIMin = "0.0", UIMax = "1.0"))
		float volume_MasterFlybys = 2.f;


	// Sound CUEs
	// Music
	UPROPERTY(EditAnywhere, Category = Sounds)
		USoundCue* music;
	// Looped sounds
	UPROPERTY(EditAnywhere, Category = Sounds)
		TArray<USoundCue*> CuesLooped;
	UPROPERTY(EditAnywhere, Category = Sounds)
		TArray<USoundCue*> CuesLooped_Cockpit;
	// Startup sounds
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Sounds)
		TArray<USoundCue*> CuesStartup;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Sounds)
		USoundCue* CueStartup_Cockpit;
	// Shutdown sounds
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Sounds)
		TArray<USoundCue*> CuesShutdown;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Sounds)
		USoundCue* CueShutdown_Cockpit;
	UPROPERTY(EditAnywhere, Category = Sounds)
		USoundCue* CueAvionics;


	// Sound Attenuations
	// For External sounds
	UPROPERTY(EditAnywhere, Category = SoundAttenuations)
		USoundAttenuation* ExternalAttenuation;
	// For Distance sounds
	UPROPERTY(EditAnywhere, Category = SoundAttenuations)
		USoundAttenuation* DistantAttenuation;
	// For Cockpit sounds
	UPROPERTY(EditAnywhere, Category = SoundAttenuations)
		USoundAttenuation* CockpitAttenuation;

	// Sound Classes
	UPROPERTY(EditAnywhere, Category = SoundClasses)
		USoundClass* ExtAround_soundClass;
	UPROPERTY(EditAnywhere, Category = SoundClasses)
		USoundClass* ExtFront_soundClass;
	UPROPERTY(EditAnywhere, Category = SoundClasses)
		USoundClass* ExtBack_soundClass;
	UPROPERTY(EditAnywhere, Category = SoundClasses)
		USoundClass* Cockpit_soundClass;
	UPROPERTY(EditAnywhere, Category = SoundClasses)
		USoundClass* External_soundClass;
	UPROPERTY(EditAnywhere, Category = SoundClasses)
		USoundClass* Master_soundClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = SoundMixes)
		TArray<USoundMix*> soundMixes;

	// Change Audio Listener
	UFUNCTION(BlueprintCallable, Category = SoundFunctions)
		void ChangeAudioListener(USceneComponent* currentCamera);

	// Initialise All Sounds
	UFUNCTION(BlueprintCallable, Category = SoundFunctions)
		void InitialiseAllSounds();

	// Switch Cockpit-External sounds
	UFUNCTION(BlueprintCallable, Category = SoundFunctions)
		void SwitchCockpitSnd();

	// Check if ready for Startup
	UFUNCTION(BlueprintCallable, Category = SoundFunctions)
		int32 CheckIsReadyForStartup();
	// Startup
	UFUNCTION(BlueprintCallable, Category = SoundFunctions)
		void Startup();
	// Startup Finished
	UFUNCTION(BlueprintCallable, Category = SoundFunctions)
		void StartupFinished(USoundWave* soundRef, float startupPlayingPercent);

	// Check if ready for Shutdown
	UFUNCTION(BlueprintCallable, Category = SoundFunctions)
		int32 CheckIsReadyForShutdown();
	// Shutdown
	UFUNCTION(BlueprintCallable, Category = SoundFunctions)
		void Shutdown();
	// Shutdown Finished
	UFUNCTION(BlueprintCallable, Category = SoundFunctions)
		void ShutdownFinished();

	// Shutdown
	UFUNCTION(BlueprintCallable, Category = SoundFunctions)
		void SetStartupOptions(bool bInitOnRunway_, bool bIsReadyToTakeoff_, float maximumPlaneSpeed_);

	// SET PLANE PARAMETERS 
	UFUNCTION(BlueprintCallable, Category = "Set FM Params")
		void SetRPM(float rpm_);

	UFUNCTION(BlueprintCallable, Category = "Set FM Params")
		void SetSpeed(float speed_);

	UFUNCTION(BlueprintCallable, Category = "Set FM Params")
		void SetPitchRate(float pitchrate_);

	UFUNCTION(BlueprintCallable, Category = "Set FM Params")
		void SetIsEngineRunning(bool bIsEngineRunning_);
	
	UFUNCTION(BlueprintCallable, Category = "Set FM Params")
		void SetIsPlaneOnLand(bool bIsPlaneOnLand_);

	UPROPERTY(BlueprintReadWrite, Category = SoundProperties)
		UAudioComponent* startupAudioComponent = nullptr;
	UPROPERTY(BlueprintReadWrite, Category = SoundProperties)
		UAudioComponent* shutdownAudioComponent = nullptr;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	// Change Sound Mix
	UFUNCTION(BlueprintCallable, Category = SoundFunctions)
		void SetSoundMix(USoundMix* soundMix, bool applySoundMix);

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Plane Sound Parameters")
		FSoundParams_F35 SoundParams;

	// Update plane sounds
	UFUNCTION(BlueprintCallable, Category = SoundFunctions)
		void UpdatePlaneSounds(FSoundParams_F35 soundParams_);

	// Update volume/pitch Multipliers
	UFUNCTION(BlueprintCallable, Category = SoundFunctions)
		void UpdateVolumeMultipliers(UAudioComponent* ac, FSoundParams_F35 soundParams_);


protected:
	
	// Play Music
	void PlayMusic(float DeltaTime);

	// Create Audio Component
	UAudioComponent* CreateAudioComponent(USoundCue* soundCue, bool IsLooped);

	// Play Looped Sounds
	void PlayLoopedSounds(bool bPlayCockpitSounds = true);

	// Stop Looped Sounds
	void StopLoopedSounds(bool bPlayCockpitSounds = true);

	// Play Startup Sounds
	void PlayStartupSounds(bool bPlayCockpitSounds = true);

	// Stop Startup Sounds
	void StopStartupSounds(bool bPlayCockpitSounds = true);

	// Play Shutdown Sounds
	void PlayShutdownSounds(bool bPlayCockpitSounds = true);

	// Link Volume and Pitch params from one AudioComponent to Another
	void LinkParams(UAudioComponent* ac, TArray<UAudioComponent*> acArray, FString acLinkName);

	// Calculate volume/pitch Multipliers
	void CalculateVolumePitchMultipliers(float dotV, float speed, int32 distanceToCamera, float& out_dopplerPitchMultiplier, float& out_frontVolumeMultiplier, float& out_backVolumeMultiplier, float& out_flybyMultiplier, float& out_flybyLPFilter);

	// Calculate dotV
	float CalculateDotV();

	// Calculate distance to listener's camera
	int32 CalculateDistanceToCamera();

	// Get Real Time since Startup in seconds
	float GetRealTime();

	//Timers
	float musicStartTimer = 0.f;
	float startupTimer = 0.f;
	float shutdownTimer = 0.f;
	float avionicsTimer = -99.f;


	// Current Camera
	USceneComponent* curCam = nullptr;


	// Array of External Sounds Audio Components
	TArray<UAudioComponent*> acsLooped;
	TArray<UAudioComponent*> acsLooped_Cockpit;
	TArray<UAudioComponent*> acsStartup;
	UAudioComponent* acStartup_Cockpit;
	TArray<UAudioComponent*> acsShutdown;
	UAudioComponent* acShutdown_Cockpit;
	UAudioComponent* acAvionics = nullptr;

	// Duration of startup sound in sec
	float startupDuration = 0.f;
	// Duration of shutdown sound in sec
	float shutdownDuration = 0.f;

	float fadeTime = 0.8f;

	float afterbWorkingTime = 0.f;
	float afterbFadeInTime = 3.2f;

	float moreThrustWorkingTime = 0.f;
	float moreThrustFadeInTime = 2.1f;

	bool bIsAfterbOn = false;

	// Flag for Startup Finished
	bool bStartupFinished = false;
	// Flag for Shutdown Finished
	bool bShutdownFinished = false;

	bool bIsCockpitMixerOn = true;
	bool bIsFlybyCamera = false;
	bool bIsCueCockpitSnd = false;

	bool bStartAvionicsSnd = false;

	//float dopplerPitchMultiplier = 1.f;

	bool bInitOnRunway = true;
	bool bIsReadyToTakeoff = true;
	float maximumPlaneSpeed = 34300.f;

	bool bIsPlaneOnLand = false;

	bool bUpdateSounds = false;


public:

	// Startup and Shutdown functions
	UFUNCTION(BlueprintCallable, Category = SoundFunctionsForMultiplayer)
		void Startup_ForMultiplayer();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_Startup();
	void Server_Startup_Implementation();
	bool Server_Startup_Validate();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void Multi_Startup();
	void Multi_Startup_Implementation();
	bool Multi_Startup_Validate();

	UFUNCTION(BlueprintCallable, Category = SoundFunctionsForMultiplayer)
		void Shutdown_ForMultiplayer();

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_Shutdown();
	void Server_Shutdown_Implementation();
	bool Server_Shutdown_Validate();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void Multi_Shutdown();
	void Multi_Shutdown_Implementation();
	bool Multi_Shutdown_Validate();

	UFUNCTION(BlueprintCallable, Category = SoundFunctionsForMultiplayer)
		void StartupFinished_ForMultiplayer(USoundWave* soundRef, float startupPlayingPercent);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_StartupFinished();
	void Server_StartupFinished_Implementation();
	bool Server_StartupFinished_Validate();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void Multi_StartupFinished();
	void Multi_StartupFinished_Implementation();
	bool Multi_StartupFinished_Validate();

	// Calculate Sound Parameters
	UFUNCTION(BlueprintCallable, Category = SoundFunctionsForMultiplayer)
		void UpdatePlaneSounds_ForMultiplayer(FSoundParams_F35 soundParams_);

	UFUNCTION(Server, Reliable, WithValidation)
		void Server_UpdatePlaneSounds(FSoundParams_F35 soundParams_);
	void Server_UpdatePlaneSounds_Implementation(FSoundParams_F35 soundParams_);
	bool Server_UpdatePlaneSounds_Validate(FSoundParams_F35 soundParams_);

	UFUNCTION(NetMulticast, Reliable, WithValidation)
		void Multi_UpdatePlaneSounds(FSoundParams_F35 soundParams_);
	void Multi_UpdatePlaneSounds_Implementation(FSoundParams_F35 soundParams_);
	bool Multi_UpdatePlaneSounds_Validate(FSoundParams_F35 soundParams_);
};