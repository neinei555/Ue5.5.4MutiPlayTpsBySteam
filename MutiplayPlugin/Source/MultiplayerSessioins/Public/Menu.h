// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Components/Button.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIOINS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BluePrintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4,FString TypeOfMatch = "FreeForAll",FString LobbyPath=FString(TEXT("/Game/Maps/Lobby")));
	UPROPERTY(BlueprintReadWrite)
	bool bCreateSessionOnDestroy = false;
protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	//
	//Callbacks for the custom delegate on the MultiplayerSessionSubsystem
	//
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
private:
	UPROPERTY(meta=(BindWidget))
	UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown();

	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 NumPublicConnections = 4;
	UPROPERTY(BlueprintReadWrite,meta =(AllowPrivateAccess="true"))
	FString MatchType = TEXT("FreeForAll");
	FString PathToLobby{ TEXT("/Game/Maps/Lobby?listen") };
	
};
