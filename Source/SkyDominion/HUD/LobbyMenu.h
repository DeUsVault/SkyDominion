// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyMenu.generated.h"

class UButton;
class UTextBlock;
class UMultiplayerSessionsSubsystem;

/**
 * 
 */
UCLASS()
class SKYDOMINION_API ULobbyMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetUserNameTextBlock();

	UFUNCTION(NetMulticast, Reliable)
	void UpdatePlayersList();

protected:
	virtual bool Initialize() override;

	virtual void NativeConstruct() override;

    //virtual void NativeDestruct() override;

	void UpdatePlayerListLocal();

private:
	UPROPERTY(meta = (BindWidget))
	UButton* Bttn_StartGame;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_StarttGameBttn;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_UserName;

	UPROPERTY(meta = (BindWidget))
	UButton* Bttn_MainMenu;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Red_1;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Red_2;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Blue_1;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text_Blue_2;

	UFUNCTION()
	void MainMenuBttnClicked();

	UFUNCTION()
	void StartGameBttnClicked();

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
};
