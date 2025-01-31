// Copyright SOUNDFX STUDIO © 2022

#include "MyUI_F35.h"


void UMyUI_F35::UI_SwitchCamera(int32 camIndex)
{
	if (!(btn_gameCam || btn_freeCam || btn_flybyCam || img_zoomCam))
		return;

	switch (camIndex)
	{
		case 0:	// Free camera selected
		default:
		{
			SetBtnOnOff(btn_freeCam);
			SetBtnOnOff(btn_gameCam, false);
			SetBtnOnOff(btn_flybyCam, false);
			img_zoomCam->SetVisibility(ESlateVisibility::Hidden);
			break;
		}		
		case 1:	// Game camera selected
		{
			SetBtnOnOff(btn_gameCam);
			SetBtnOnOff(btn_freeCam, false);
			SetBtnOnOff(btn_flybyCam, false);
			img_zoomCam->SetVisibility(ESlateVisibility::Hidden);
			break;
		}
		case 2:	// Flyby camera selected
		{
			SetBtnOnOff(btn_flybyCam);
			SetBtnOnOff(btn_freeCam, false);
			SetBtnOnOff(btn_gameCam, false);
			img_zoomCam->SetVisibility(ESlateVisibility::Visible);
			break;
		}
	}

}


// Switch UI Startup Button
void UMyUI_F35::UI_StartupOn()
{
	SetBtnOnOff(btn_Startup);
}
void UMyUI_F35::UI_StartupOff()
{
	SetBtnOnOff(btn_Startup, false);
}


// Switch UI Shutdown Button
void UMyUI_F35::UI_ShutdownOn()
{
	SetBtnOnOff(btn_Shutdown);
}
void UMyUI_F35::UI_ShutdownOff()
{
	SetBtnOnOff(btn_Shutdown, false);
}

void UMyUI_F35::ErrorMessages(int32 n)
{
	if (!txt_errors)
		return;

	switch (n)
	{
	case 0:
		txt_errors->SetText(FText::FromString("LAND YOUR PLANE"));
		break;
	case 1:
		txt_errors->SetText(FText::FromString("ENGINE IS NOT RUNNING"));
		break;
	case 2:
		txt_errors->SetText(FText::FromString("ENGINE IS ALREADY RUNNING"));
		break;
	case 3:
		txt_errors->SetText(FText::FromString("ENGINE IS STILL RUNNING"));
		break;
	case 4:
		txt_errors->SetText(FText::FromString("CHECK YOUR SPEED"));
		break;
	case 99:
	default:
		// DUMMY
		break;
	}

	PlayAnimation(animTxtErrorsRef);
}

void UMyUI_F35::UI_SwitchCockpitSnd()
{
	bIsCockpitSnd = !bIsCockpitSnd;
	SetBtnOnOff(btn_CockpitSnd, bIsCockpitSnd);
}

// Switch UI Buttons On/Off
void UMyUI_F35::SetBtnOnOff(UBorder* btn, bool on /*= true*/)
{
	if (!btn)
		return;

	FLinearColor setColor = on ? btn_on_color : btn_off_color;

	btn->SetContentColorAndOpacity(setColor);
	btn->SetBrushColor(setColor);
}
