// Copyright SOUNDFX STUDIO © 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Animation/WidgetAnimation.h"
#include "MyUI_F35.generated.h"


/**
 * USER INTERFACE
 */
UCLASS()
class F35SOUNDSYSTEM_API UMyUI_F35 : public UUserWidget
{
	GENERATED_BODY()

public:

	// Switch UI CockpitSND
	UFUNCTION(BlueprintCallable, Category = "MyUI_F35 Functions")
		void UI_SwitchCockpitSnd();

	// Switch On-Off Startup and Shutdown buttons
	UFUNCTION(BlueprintCallable, Category = "MyUI_F35 Functions")
		void UI_StartupOn();
	UFUNCTION(BlueprintCallable, Category = "MyUI_F35 Functions")
		void UI_ShutdownOn();
	UFUNCTION(BlueprintCallable, Category = "MyUI_F35 Functions")
		void UI_StartupOff();
	UFUNCTION(BlueprintCallable, Category = "MyUI_F35 Functions")
		void UI_ShutdownOff();

	// Error Messages on the screen
	UFUNCTION(BlueprintCallable, Category = "MyUI_F35 Functions")
		void ErrorMessages(int32 n);
	
protected:

	// ref to Animation for UI Error Text
	UPROPERTY(BlueprintReadWrite, Category = "MyUI_F35 Properties")
		UWidgetAnimation* animTxtErrorsRef;


private:

	UPROPERTY(meta = (BindWidget))
		UBorder* btn_gameCam;
	UPROPERTY(meta = (BindWidget))
		UBorder* btn_freeCam;
	UPROPERTY(meta = (BindWidget))
		UBorder* btn_flybyCam;
	UPROPERTY(meta = (BindWidget))
		UBorder* btn_Startup;
	UPROPERTY(meta = (BindWidget))
		UBorder* btn_Shutdown;
	UPROPERTY(meta = (BindWidget))
		UBorder* btn_CockpitSnd;
	UPROPERTY(meta = (BindWidget))
		UTextBlock* txt_errors;
	UPROPERTY(meta = (BindWidget))
		UImage* img_zoomCam;

	// Switch UI Camera Buttons
	UFUNCTION(BlueprintCallable, Category = "MyUI_F35 Functions")
		void UI_SwitchCamera(int32 camIndex);

	// Switch UI Buttons On/Off
	void SetBtnOnOff(UBorder* btn, bool on = true);
	
	// Colors for button's borders
	FLinearColor btn_on_color = FLinearColor(1.f, 1.f, 1.f, 1.f);
	FLinearColor btn_off_color = FLinearColor(1.f, 1.f, 1.f, 0.4f);

	// Is Cockpit Snd On
	bool bIsCockpitSnd = false;
};
