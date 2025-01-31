// Copyright SOUNDFX STUDIO © 2022

#include "Sounds_F35.h"
#include "Sound/SoundClass.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundNode.h"
#include "Sound/SoundNodeAttenuation.h"
#include "Sound/SoundNodeSoundClass.h"
#include "Sound/SoundNodeDelay.h"
#include "Sound/SoundNodeWavePlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// GET PLANE PARAMETERS 
void USounds_F35::SetRPM(float rpm_) { SoundParams.rpm = rpm_; }
void USounds_F35::SetSpeed(float speed_) { SoundParams.speed = speed_ / 100.f; }
void USounds_F35::SetPitchRate(float pitchRate_) { SoundParams.pitchRate = pitchRate_; }
void USounds_F35::SetIsEngineRunning(bool bIsEngineRunning_) { SoundParams.bIsEngineRunning = bIsEngineRunning_; }
void USounds_F35::SetIsPlaneOnLand(bool bIsPlaneOnLand_) { bIsPlaneOnLand = bIsPlaneOnLand_; }

// Sets default values for this component's properties
USounds_F35::USounds_F35()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USounds_F35::SetStartupOptions(bool bInitOnRunway_, bool bIsReadyToTakeoff_, float maximumPlaneSpeed_)
{
	bInitOnRunway = bInitOnRunway_;
	bIsReadyToTakeoff = bIsReadyToTakeoff_;
	maximumPlaneSpeed = maximumPlaneSpeed_;
}


// Called every frame
void USounds_F35::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// For Local player
	if (!bIsCockpitMixerOn)
	{
		if (GetOwner()->GetLocalRole() >= ENetRole::ROLE_AutonomousProxy)
		{
			CalculateVolumePitchMultipliers(CalculateDotV(), SoundParams.speed, CalculateDistanceToCamera(), SoundParams.dopplerPitchMultiplier, SoundParams.frontVolumeMultiplier, SoundParams.backVolumeMultiplier, SoundParams.flybyMultiplier, SoundParams.flybyLPFilter);
			
			if (!bIsFlybyCamera)
			{
				SoundParams.dopplerPitchMultiplier = 1.f;
				SoundParams.flybyLPFilter = 44100.f;
				SoundParams.flybyMultiplier = 1.f;
			}
		}
	}
	else
	{
		SoundParams.frontVolumeMultiplier = 1.f;
		SoundParams.backVolumeMultiplier = 1.f;
		SoundParams.dopplerPitchMultiplier = 1.f;
		SoundParams.flybyLPFilter = 44100.f;
		SoundParams.flybyMultiplier = 1.f;
	}
	//UE_LOG(LogTemp, Error, TEXT("%s  BACK = %f   FRONT = %f   LPF = %f   FLYBY = %f"), *GetOwner()->GetName(), SoundParams.backVolumeMultiplier, SoundParams.frontVolumeMultiplier, SoundParams.flybyLPFilter, SoundParams.flybyMultiplier);

	// Delayed Music Start
	if (bPlayMusic)
		PlayMusic(DeltaTime);


	if (bIsAfterbOn)
		afterbWorkingTime += DeltaTime;
}


// Initialise All Sounds
void USounds_F35::InitialiseAllSounds()
{
	UGameplayStatics::SetBaseSoundMix(this, soundMixes[SoundMixes_F35::SM_Base]);

	SoundParams.vol_MasterExternal = volume_MasterExternal;
	SoundParams.vol_MasterCockpit = volume_MasterCockpit;

	// Duration of startup sound in sec
	startupDuration = CuesStartup[0]->GetDuration();
	// Duration of shutdown sound in sec
	shutdownDuration = CuesShutdown[0]->GetDuration();

	// Create Audio Components
	for (USoundCue* cue : CuesStartup)
	{
		UAudioComponent* ac = CreateAudioComponent(cue, false);
		acsStartup.Add(ac);
		if (!startupAudioComponent)
			startupAudioComponent = ac;
	}
	acStartup_Cockpit = CreateAudioComponent(CueStartup_Cockpit, false);

	for (USoundCue* cue : CuesShutdown)
	{
		UAudioComponent* ac = CreateAudioComponent(cue, false);
		acsShutdown.Add(ac);
		if (!shutdownAudioComponent)
			shutdownAudioComponent = ac;
	}
	acShutdown_Cockpit = CreateAudioComponent(CueShutdown_Cockpit, false);

	for (USoundCue* cue : CuesLooped)
	{
		acsLooped.Add(CreateAudioComponent(cue, true));
	}
	for (USoundCue* cue : CuesLooped_Cockpit)
	{
		acsLooped_Cockpit.Add(CreateAudioComponent(cue, true));
	}
	acAvionics = CreateAudioComponent(CueAvionics, true);
	
	if (!bInitOnRunway || (bInitOnRunway && bIsReadyToTakeoff))
	{
		afterbWorkingTime = afterbFadeInTime;
		PlayLoopedSounds();
		bStartAvionicsSnd = true;
		if (acAvionics)
			acAvionics->Play();
	}

	//UE_LOG(LogTemp, Warning, TEXT("%s"), *UEnum::GetValueAsString(GetOwner()->GetLocalRole()));
	if (GetOwner()->GetLocalRole() >= ENetRole::ROLE_AutonomousProxy)
		SwitchCockpitSnd();
}


// Check if ready for Startup
int32 USounds_F35::CheckIsReadyForStartup()
{
	if (bShutdownFinished)
		return 3;
	else if (SoundParams.bIsEngineRunning)
		return 2;
	else if (!bStartupFinished)
		return -1;
	else
		return 99;
}


// Startup
void USounds_F35::Startup()
{	
	startupTimer = GetRealTime();
	bStartupFinished = true;
	PlayStartupSounds();
}


// Startup Finished
void USounds_F35::StartupFinished(USoundWave* soundRef, float startupPlayingPercent)
{
	if (bStartupFinished && startupPlayingPercent >= 0.99f)
	{
		PlayLoopedSounds();
		StopStartupSounds();
		bStartupFinished = false;
	}
}

// Check if ready for Shutdown
int32 USounds_F35::CheckIsReadyForShutdown()
{
	if (bShutdownFinished)	// already shutdowned
		return 99;
	else if (!bIsPlaneOnLand)
		return 0;
	else if (SoundParams.speed != 0)
		return 4;
	else if (!SoundParams.bIsEngineRunning)
		return 1;
	else return -1;
}

// Shutdown
void USounds_F35::Shutdown()
{
	shutdownTimer = GetRealTime();
	bShutdownFinished = true;
	PlayShutdownSounds();
	StopLoopedSounds();
}

// Shutdown Finished
void USounds_F35::ShutdownFinished()
{
	bShutdownFinished = false;
}

// Play Startup Sounds
void USounds_F35::PlayStartupSounds(bool bPlayCockpitSounds)
{
	for (UAudioComponent* ac : acsStartup)
	{
		ac->Play();
	}

	if (bPlayCockpitSounds)
	{
		acStartup_Cockpit->Play();
		if (acAvionics)
			acAvionics->Play();
		avionicsTimer = GetRealTime();
		bStartAvionicsSnd = true;
	}
}


void USounds_F35::StopStartupSounds(bool bPlayCockpitSounds)
{
	for (UAudioComponent* ac : acsStartup)
	{
		ac->FadeOut(fadeTime, 0.f, EAudioFaderCurve::Sin);
	}

	if (bPlayCockpitSounds)
		acStartup_Cockpit->FadeOut(fadeTime, 0.f, EAudioFaderCurve::Sin);
}

void USounds_F35::PlayShutdownSounds(bool bPlayCockpitSounds)
{
	for (UAudioComponent* ac : acsShutdown)
	{
		ac->FadeIn(0.5f * fadeTime, 1.f, 0.f, EAudioFaderCurve::Sin);
	}

	if (bPlayCockpitSounds)
	{
		acShutdown_Cockpit->FadeIn(0.5f * fadeTime, 1.f, 0.f, EAudioFaderCurve::Sin);
		avionicsTimer = GetRealTime();
		bStartAvionicsSnd = false;
	}
}


// Play Idle Sounds
void USounds_F35::PlayLoopedSounds(bool bPlayCockpitSounds)
{
	bUpdateSounds = true;

	for (UAudioComponent* ac : acsLooped)
	{
		ac->FadeIn(fadeTime, 1.f, 0.f, EAudioFaderCurve::Sin);
	}

	if (bPlayCockpitSounds)
	{
		for (UAudioComponent* ac : acsLooped_Cockpit)
		{
			ac->FadeIn(fadeTime, 1.f, 0.f/*FMath::RandRange(0.f, 5.f)*/, EAudioFaderCurve::Sin);
		}
	}
}


void USounds_F35::StopLoopedSounds(bool bPlayCockpitSounds)
{
	for (UAudioComponent* ac : acsLooped)
	{
		ac->FadeOut(0.5f * fadeTime, 0.f, EAudioFaderCurve::Sin);
	}

	if (bPlayCockpitSounds)
	{
		for (UAudioComponent* ac : acsLooped_Cockpit)
		{
			ac->FadeOut(0.5f * fadeTime, 0.f, EAudioFaderCurve::Sin);
		}
	}
}

// Link Volume and Pitch params from one AudioComponent to Another
void USounds_F35::LinkParams(UAudioComponent* ac, TArray<UAudioComponent*> acArray, FString waveName)
{
	for (UAudioComponent* audioComponent : acArray)
	{
		FString acName = *(audioComponent->GetName());

		if (acName == waveName)
		{			
			ac->SetVolumeMultiplier(audioComponent->VolumeMultiplier);
			ac->SetPitchMultiplier(audioComponent->PitchMultiplier);
			break;
		}
	}
}


// Calculate Sound Parameters
void USounds_F35::UpdatePlaneSounds(FSoundParams_F35 soundParams_)
{
	float rpm = soundParams_.rpm;
	float speed = soundParams_.speed;
	float pitchRate = soundParams_.pitchRate;
	float dopplerPitchMultiplier = soundParams_.dopplerPitchMultiplier;
	bool bIsEngineRunning = soundParams_.bIsEngineRunning;
	bool bPlayCockpitSounds = soundParams_.bPlayCockpitSounds;

	float rpm2 = rpm*rpm;

	// for Cockpit Afterburner FadeIn
	if (rpm > 0.95f && !bIsAfterbOn)
	{
		bIsAfterbOn = true;
		afterbWorkingTime = 0.f;
	}
	else if (rpm <= 0.95f)
		bIsAfterbOn = false;

	// Adjust pitch and gain for Looped sounds
	if (bUpdateSounds || bStartupFinished || bShutdownFinished)
	{
		for (UAudioComponent* ac : acsLooped)
		{
			FString acName = *(ac->GetName());			

			if (acName == "Idle_Cue")
			{
				ac->SetVolumeMultiplier(rpm > 0.2f ? FMath::Max(0.f, -3.33f * rpm + 1.67f) : -5.42f * rpm2 + 1.83f * rpm + 0.85f);
				float pitch = (1.21f * rpm + 1) / 1.105f;
				ac->SetPitchMultiplier(pitch * dopplerPitchMultiplier);
			}
			else if (acName == "Idle2_Cue")
			{
				ac->SetVolumeMultiplier(-2.4f * rpm2 + 1.7f * rpm + 0.7f);
				ac->SetPitchMultiplier(dopplerPitchMultiplier);
			}
			else if (acName == "Idle_Front_Cue")
			{
				ac->SetVolumeMultiplier(1.f);
				float pitch = 0.5f * rpm + 1.f;
				ac->SetPitchMultiplier(pitch * dopplerPitchMultiplier);
			}
			else if (acName == "Idle_Back_Cue")
			{
				ac->SetVolumeMultiplier(rpm > 0.4f ? 1.27f - 0.67f * rpm : 1.f);
				ac->SetPitchMultiplier(dopplerPitchMultiplier);
			}
			else if (acName == "Maximum_Cue")
			{
				ac->SetVolumeMultiplier(rpm > 0.9f ? 60.46f * rpm2 - 120.87f * rpm + 60.81f : FMath::Max(0.f, -2.02f * rpm2 + 3.65f * rpm - 0.65f));
				float pitch = 0.55f * rpm + 0.45;
				ac->SetPitchMultiplier(pitch * dopplerPitchMultiplier);
			}
			else if (acName == "Maximum2_Cue")
			{
				float vol = FMath::Max(0.f, 1.43f * rpm - 0.29f);
				vol *= FMath::Max(0.f, 1.f - speed / 80.f);
				ac->SetVolumeMultiplier(vol);
				ac->SetPitchMultiplier(dopplerPitchMultiplier);
			}
			else if (acName == "Maximum_Back_Cue")
			{
				ac->SetVolumeMultiplier(FMath::Min(0.9f, rpm2));
				ac->SetPitchMultiplier(dopplerPitchMultiplier);
			}
			else if (acName == "MoreThrust_Cue")
			{				
				float k1 = 0.7f;
				float volume = FMath::Min(1.f, FMath::Max(0.f, 5.f * rpm - 0.5f));
				if (volume > 0)
				{
					if (moreThrustWorkingTime < moreThrustFadeInTime)
					{
						k1 = moreThrustWorkingTime < 0.22 ? 34.53f * moreThrustWorkingTime * moreThrustWorkingTime + 6.04f * moreThrustWorkingTime : - 1.22f * moreThrustWorkingTime + 3.27f;
						moreThrustWorkingTime += GetWorld()->GetDeltaSeconds();
					}
				}
				else
					moreThrustWorkingTime = 0.f;

				ac->SetVolumeMultiplier(k1*volume);
				ac->SetPitchMultiplier(dopplerPitchMultiplier);
			}
			else if (acName == "Afterburner_Back_Cue" || acName == "Distant_Afterburner_Back_Cue")
			{
				ac->SetVolumeMultiplier(FMath::Max(0.f, 20.f * (rpm - 0.95f)));
				ac->SetPitchMultiplier(dopplerPitchMultiplier);
			}
			else if (acName == "Distant_Back_Cue" || acName == "Distant_Cue")
			{
				ac->SetVolumeMultiplier(0.7f * rpm + 0.3f);
				ac->SetPitchMultiplier(dopplerPitchMultiplier);
			}
			else if (acName == "Distant_Front_Cue")
			{
				LinkParams(ac, acsLooped, "Distant_Back_Cue");
				float vol = ac->VolumeMultiplier * FMath::Min(1.f, (speed / 30.f));
				ac->SetVolumeMultiplier(vol);
				ac->SetPitchMultiplier(dopplerPitchMultiplier);
			}
			else if (acName == "Maximum_Front_Cue")
			{
				float vol = rpm * FMath::Min(1.f, (speed / 30.f));
				ac->SetVolumeMultiplier(vol);
			}
			else if (acName == "Flyby_Cue")
			{
				float vol = rpm * FMath::Min(1.f, (speed / 80.f));
				ac->SetVolumeMultiplier(vol);
				ac->SetPitchMultiplier(dopplerPitchMultiplier);
			}

			UpdateVolumeMultipliers(ac, soundParams_);
		}

		if (bPlayCockpitSounds)
		{
			// COCKPIT SOUNDS
			for (UAudioComponent* ac : acsLooped_Cockpit)
			{
				FString acName = *(ac->GetName());

				if (acName == "Idle_cockpit_Cue")
				{
					ac->SetVolumeMultiplier(rpm > 0.2f ? -0.75f * rpm + 1.15f : -5.42f * rpm2 + 1.83f * rpm + 0.85f);
					ac->SetPitchMultiplier((1.21f * rpm + 1) / 1.105f);
				}
				else if (acName == "Afterburner_cockpit_Cue")
				{
					float k1 = 1.f;
					if (afterbWorkingTime < afterbFadeInTime)
						k1 = FMath::Max(1.f, 1.f + FMath::Sin(afterbWorkingTime));
					ac->SetVolumeMultiplier(k1 * FMath::Max(0.f, 20.f * (rpm - 0.95f)));
				}
				else if (acName == "Maximum_cockpit_Cue")
				{
					ac->SetVolumeMultiplier(rpm);
				}
				else if (acName == "Wind_cockpit_Cue")
				{
					ac->SetVolumeMultiplier(FMath::Min(1.f, speed / (0.01f * maximumPlaneSpeed)));
				}
				else if (acName == "Turbulence_cockpit_Cue")
				{
					float AoA = FMath::Min(1.f, (FMath::Abs(pitchRate) / 40.f));
					ac->SetVolumeMultiplier(AoA);
					ac->SetPitchMultiplier(0.9f + 0.25f * AoA);
				}

				UpdateVolumeMultipliers(ac, soundParams_);
			}

			// Only Avionics sound is playing relative to startup and shutdown sounds 
			if (acAvionics)
			{
				if (acAvionics->IsPlaying())
				{
					float volume;
					float avionicsPlayingTimer = GetRealTime() - avionicsTimer;

					if (bStartAvionicsSnd)
					{
						float clipLength = startupDuration;
						float timeStartToFadeIn = 0.14f;

						if (avionicsPlayingTimer < clipLength * timeStartToFadeIn)
							volume = 0.f;
						else
							volume = FMath::GetMappedRangeValueClamped(FVector2D(timeStartToFadeIn * clipLength, clipLength), FVector2D(0.f, 1.f), avionicsPlayingTimer);
					}
					else
					{
						float clipLength = shutdownDuration;
						float timeToStop = 0.2f;

						if (avionicsPlayingTimer < clipLength * timeToStop)
							volume = FMath::GetMappedRangeValueClamped(FVector2D(0.f, timeToStop * clipLength), FVector2D(1.f, 0.f), avionicsPlayingTimer);
						else
						{
							volume = 0.f;
						}
					}
					acAvionics->SetVolumeMultiplier(volume);
					UpdateVolumeMultipliers(acAvionics, soundParams_);
				}
			}
		}
	}

	// Adjust volume multipliers for Non-Looped sounds
	for (UAudioComponent* ac : acsStartup)
	{
		if (ac->IsPlaying())
		{
			ac->SetVolumeMultiplier(1.f);
			UpdateVolumeMultipliers(ac, soundParams_);
		}
	}
	for (UAudioComponent* ac : acsShutdown)
	{
		if (ac->IsPlaying())
		{
			ac->SetVolumeMultiplier(1.f);
			UpdateVolumeMultipliers(ac, soundParams_);
		}
	}
}


void USounds_F35::UpdateVolumeMultipliers(UAudioComponent* ac, FSoundParams_F35 soundParams_)
{
	FString acName = *(ac->GetName());

	// cockpit sounds
	if (acName.Contains("cockpit"))
	{
		ac->SetVolumeMultiplier(ac->VolumeMultiplier * soundParams_.vol_MasterCockpit);
	}
	else  // external sounds
	{
		if (acName.Contains("Back"))
		{
			ac->SetVolumeMultiplier(ac->VolumeMultiplier * soundParams_.backVolumeMultiplier);
		}
		else if (acName.Contains("Front"))
		{
			ac->SetVolumeMultiplier(ac->VolumeMultiplier * soundParams_.frontVolumeMultiplier);
			ac->SetLowPassFilterEnabled(true);
			ac->SetLowPassFilterFrequency(soundParams_.flybyLPFilter);
		}

		ac->SetVolumeMultiplier(ac->VolumeMultiplier * soundParams_.flybyMultiplier * soundParams_.vol_MasterExternal);
	}
}


void USounds_F35::CalculateVolumePitchMultipliers(float dotV, float speed, int32 distanceToCamera, float& out_dopplerPitchMultiplier, float& out_frontVolumeMultiplier, float& out_backVolumeMultiplier, float& out_flybyMultiplier, float& out_flybyLPFilter)
{
	float dotV_2 = dotV * dotV;

	// Calculate Doppler multiplier
	float dot2Pitch = -0.4f * dotV_2 + 1.2f * dotV + 0.5f;
	out_dopplerPitchMultiplier = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 1.f), FVector2D(1.f, dot2Pitch), speed / 200.f);

	// Calculate Front/Back sounds volume multiplier
	out_frontVolumeMultiplier = FMath::Max(0.001f, 0.82f * dotV + 0.18f);  // Can't set to 0 , because UE will stop playing all these Audio Components
	out_backVolumeMultiplier = FMath::Max(0.001f, dotV_2 - 2.f * dotV + 1.f);

	// Calculate LowPass Filter for Front sounds
	if (dotV > 0.5f)
		out_flybyLPFilter = FMath::GetMappedRangeValueClamped(FVector2D(1.f, 0.5f), FVector2D(1500.f, 44100.f), dotV);
	else
		out_flybyLPFilter = 44100.f;

	// Calculate volume multiplier for flybys
	if (distanceToCamera < 50000)
	{
		float dist = distanceToCamera / 50000.f;

		out_flybyMultiplier = dotV > 0.9f ? FMath::GetMappedRangeValueClamped(FVector2D(0.85f, 1.f), FVector2D(2.3f, 0.1f), dotV) :
			dotV > 0.6f ? FMath::GetMappedRangeValueClamped(FVector2D(0.6f, 0.85f), FVector2D(4.f, 2.3f), dotV) :
			dotV > 0.4f ? FMath::GetMappedRangeValueClamped(FVector2D(0.4f, 0.6f), FVector2D(6.f, 4.f), dotV) :
			dotV > 0.1f ? FMath::GetMappedRangeValueClamped(FVector2D(0.1f, 0.4f), FVector2D(6.f, 5.f), dotV)
			: FMath::GetMappedRangeValueClamped(FVector2D(0.f, 0.1f), FVector2D(3.f, 5.f), dotV);

		out_flybyMultiplier = FMath::Lerp(out_flybyMultiplier, 1.f, dist);
	}
	else
	{
		float front_reduce = dotV > 0.15f ? -2.375f * dotV_2 * dotV + 6.159f * dotV_2 - 5.415f * dotV + 1.682f : 1.f;
		float dist = FMath::Max(0.15f, 1.f - (distanceToCamera - 50000) / 150000.f);
		out_flybyMultiplier = FMath::Lerp(front_reduce, 1.f, dist);
	}

	out_flybyMultiplier = FMath::Lerp(1.f, out_flybyMultiplier, speed / 200.f);
	out_flybyMultiplier *= 0.25f * volume_MasterFlybys;
}


float USounds_F35::CalculateDotV()
{
	FVector from = GetOwner()->GetActorForwardVector();
	FVector listenerCameraLocation = UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager->GetCameraLocation();
	FVector to = (GetOwner()->GetActorLocation() - listenerCameraLocation).GetSafeNormal();

	float dotV = 0.5f * FMath::Abs((FVector::DotProduct(from, to) - 1.f));  // 1 - Front, 0 - Back
	return dotV;
}


int32 USounds_F35::CalculateDistanceToCamera()
{
	FVector listenerCameraLocation = UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager->GetCameraLocation();
	return FVector::Dist(GetOwner()->GetActorLocation(), listenerCameraLocation);
}


// Change Audio Listener
void USounds_F35::ChangeAudioListener(USceneComponent* currentCamera)
{
	if (!currentCamera)
		return;

	curCam = currentCamera;

	bool isFlybyCamera = curCam->ComponentHasTag(FName("FlybyCamera"));
	if (bIsFlybyCamera != isFlybyCamera)
	{
		bIsFlybyCamera = isFlybyCamera;
	}

	if (!bIsCockpitMixerOn)
		GetWorld()->GetFirstPlayerController()->SetAudioListenerOverride(curCam, FVector(0), FRotator(0));
}


// Switch Cockpit-External sounds
void USounds_F35::SwitchCockpitSnd()
{
	bIsCockpitMixerOn = !bIsCockpitMixerOn;

	if (!soundMixes[SoundMixes_F35::SM_MainGamePlay] || !soundMixes[SoundMixes_F35::SM_Cockpit])
		return;

	SetSoundMix(soundMixes[SoundMixes_F35::SM_Cockpit], bIsCockpitMixerOn);
	SetSoundMix(soundMixes[SoundMixes_F35::SM_MainGamePlay], !bIsCockpitMixerOn);
	
	if (bIsCockpitMixerOn)
		GetWorld()->GetFirstPlayerController()->SetAudioListenerOverride(GetOwner()->FindComponentByClass<USceneComponent>(), FVector(0), FRotator(0));
	else
		ChangeAudioListener(curCam);
}


// Play Music
void USounds_F35::PlayMusic(float DeltaTime)
{
	// Delay Music Playing for 2 seconds
	if (musicStartTimer > 2.f)
	{
		UAudioComponent* musicAudioComponent = NewObject<UAudioComponent>(this, FName("Music"));

		if (!musicAudioComponent)
			return;

		musicAudioComponent->SetSound(music);
		musicAudioComponent->RegisterComponent();

		bPlayMusic = false;
	}
	else
	{
		musicStartTimer += DeltaTime;
	}
}


UAudioComponent* USounds_F35::CreateAudioComponent(USoundCue* soundCue, bool bIsLooped)
{
	auto owner = GetOwner();
	if (!soundCue || !owner)
		return nullptr;

	UAudioComponent* audioComponent = NewObject<UAudioComponent>(this, *soundCue->GetName());

	audioComponent->SetWorldLocation(owner->GetActorLocation());
	audioComponent->SetWorldRotation(owner->GetActorRotation());
	audioComponent->AttachToComponent(owner->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform, NAME_None);
	//audioComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform, NAME_None);
	audioComponent->bAutoActivate = false;

	FString cueName = soundCue->GetName();

	// Get Sound Class for cue
	USoundClass* soundClass = nullptr;
	if (cueName.Contains("Back"))
		soundClass = ExtBack_soundClass;
	else if (cueName.Contains("Front"))
		soundClass = ExtFront_soundClass;
	else if (cueName.Contains("cockpit"))
		soundClass = Cockpit_soundClass;
	else
		soundClass = ExtAround_soundClass;

	// Get Sound Attenuation for cue
	USoundAttenuation* soundAttenuation = nullptr;
	if (cueName.Contains("Distant"))
		soundAttenuation = DistantAttenuation;
	else if (cueName.Contains("cockpit"))
		soundAttenuation = CockpitAttenuation;
	else
		soundAttenuation = ExternalAttenuation;


	// SETUP SOUND NODES
	TArray<USoundNode*> allCueNodes;
	soundCue->FirstNode->GetAllNodes(allCueNodes);
	for (int32 i = 0; i < allCueNodes.Num(); i++)
	{
		FName nodeName = *((allCueNodes[i])->GetClass()->GetName());
		
		// Setup Attenuation Node
		if (nodeName == "SoundNodeAttenuation")
		{
			USoundNodeAttenuation* attenuation = Cast<USoundNodeAttenuation>(allCueNodes[i]);
			if (attenuation)
				attenuation->AttenuationSettings = soundAttenuation;
		}
		// Setup Sound Class Node
		else if (nodeName == "SoundNodeSoundClass")
		{
			USoundNodeSoundClass* sclass = Cast<USoundNodeSoundClass>(allCueNodes[i]);
			if (sclass)
				sclass->SoundClassOverride = soundClass;
		}
		else if (bIsLooped)
		{
			// Setup WavePlayer Node
			if (nodeName == "SoundNodeWavePlayer")
			{
				USoundNodeWavePlayer* wavePlayer = Cast<USoundNodeWavePlayer>(allCueNodes[i]);
				if (wavePlayer)
					wavePlayer->bLooping = true;
			}
		}
	}

	if (soundClass != Cockpit_soundClass)
		audioComponent->SetVolumeMultiplier(SoundParams.vol_MasterExternal);
	else
		audioComponent->SetVolumeMultiplier(SoundParams.vol_MasterCockpit);

	audioComponent->SetSound(soundCue);
	audioComponent->RegisterComponent();

	return audioComponent;
}


// Get Real Time since Startup in seconds
float USounds_F35::GetRealTime()
{
	int32 timeInt = 0;
	float timeFloat = 0.f;
	UGameplayStatics::GetAccurateRealTime(timeInt, timeFloat);
	return timeInt + timeFloat;
}


void USounds_F35::SetSoundMix(USoundMix* soundMix, bool applySoundMix)
{
	if (applySoundMix)
		UGameplayStatics::PushSoundMixModifier(this, soundMix);
	else
		UGameplayStatics::PopSoundMixModifier(this, soundMix);
}


////////////////////////////////////
// FOR MULTIPLAYER
// Replicate Variables
void USounds_F35::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USounds_F35, SoundParams);
}


void USounds_F35::Startup_ForMultiplayer()
{
	startupTimer = GetRealTime();
	bStartupFinished = true;
	PlayStartupSounds();

	auto owner = GetOwner();
	if (owner == nullptr)
		return;

	if (!owner->HasAuthority())
		Server_Startup();
}

void USounds_F35::Server_Startup_Implementation()
{
	Multi_Startup();
}
bool USounds_F35::Server_Startup_Validate() { return true; }

void USounds_F35::Multi_Startup_Implementation()
{
	auto owner = GetOwner();
	if (owner == nullptr)
		return;

	if (owner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		PlayStartupSounds(false);
	}
}
bool USounds_F35::Multi_Startup_Validate() { return true; }


void USounds_F35::Shutdown_ForMultiplayer()
{
	shutdownTimer = GetRealTime();
	bShutdownFinished = true;

	PlayShutdownSounds();
	StopLoopedSounds();

	auto owner = GetOwner();
	if (owner == nullptr)
		return;
	
	if (!owner->HasAuthority())
		Server_Shutdown();
}

void USounds_F35::Server_Shutdown_Implementation()
{
	Multi_Shutdown();
}
bool USounds_F35::Server_Shutdown_Validate() { return true; }

void USounds_F35::Multi_Shutdown_Implementation()
{
	auto owner = GetOwner();
	if (owner == nullptr)
		return;

	if (owner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		PlayShutdownSounds(false);
		StopLoopedSounds(false);
	}
}
bool USounds_F35::Multi_Shutdown_Validate() { return true; }


void USounds_F35::StartupFinished_ForMultiplayer(USoundWave* soundRef, float startupPlayingPercent)
{
	if (bStartupFinished && startupPlayingPercent >= 0.99f)
	{
		PlayLoopedSounds();
		StopStartupSounds();
		bStartupFinished = false;


		auto owner = GetOwner();
		if (owner == nullptr)
			return;

		if (!owner->HasAuthority())
			Server_StartupFinished();
	}
}

void USounds_F35::Server_StartupFinished_Implementation()
{
	Multi_StartupFinished();
}
bool USounds_F35::Server_StartupFinished_Validate() { return true; }

void USounds_F35::Multi_StartupFinished_Implementation()
{
	auto owner = GetOwner();
	if (owner == nullptr)
		return;

	if (owner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		PlayLoopedSounds(false);
		StopStartupSounds(false);
	}
}
bool USounds_F35::Multi_StartupFinished_Validate() { return true; }


void USounds_F35::UpdatePlaneSounds_ForMultiplayer(FSoundParams_F35 soundParams_)
{
	auto owner = GetOwner();
	if (owner == nullptr)
		return;

	UpdatePlaneSounds(soundParams_);

	if (!owner->HasAuthority())
		Server_UpdatePlaneSounds(soundParams_);
}

void USounds_F35::Server_UpdatePlaneSounds_Implementation(FSoundParams_F35 soundParams_)
{
	Multi_UpdatePlaneSounds(soundParams_);
}
bool USounds_F35::Server_UpdatePlaneSounds_Validate(FSoundParams_F35 soundParams_) { return true; }

void USounds_F35::Multi_UpdatePlaneSounds_Implementation(FSoundParams_F35 soundParams_)
{
	auto owner = GetOwner();
	if (owner == nullptr)
		return;

	if (owner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		// For Other player
		soundParams_.bPlayCockpitSounds = false;
		CalculateVolumePitchMultipliers(CalculateDotV(), soundParams_.speed, CalculateDistanceToCamera(), soundParams_.dopplerPitchMultiplier, soundParams_.frontVolumeMultiplier, soundParams_.backVolumeMultiplier, soundParams_.flybyMultiplier, soundParams_.flybyLPFilter);
		//UE_LOG(LogTemp, Error, TEXT("%s  DotV = %f  Front = %f  Back = %f  LPF = %f"), *GetOwner()->GetName(), CalculateDotV(), soundParams_.frontVolumeMultiplier, soundParams_.backVolumeMultiplier, soundParams_.flybyLPFilter);
		UpdatePlaneSounds(soundParams_);
	}
}
bool USounds_F35::Multi_UpdatePlaneSounds_Validate(FSoundParams_F35 soundParams_) { return true; }