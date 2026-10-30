// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "ZeroHour/HUD/ShooterHUD.h"
#include "ZeroHour/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "ZeroHour/Character/ShooterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "ZeroHour/GameMode/ShooterGameMode.h"
#include "ZeroHour/PlayerState/ShooterPlayerState.h"
#include "ZeroHour/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "ZeroHour/ShooterComponents/CombatComponent.h"
#include "ZeroHour/GameState/ShooterGameState.h"
#include "Components/Image.h"
#include "ZeroHour/HUD/ReturnToMainMenu.h"
#include "ZeroHour/ShooterTypes/Announcement.h"

void AShooterPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void AShooterPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
		if (ShooterHUD)
		{
			if (Attacker == Self && Victim != Self)
			{
				ShooterHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self)
			{
				ShooterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "You");
				return;
			}
			if (Attacker == Victim && Attacker == Self)
			{
				ShooterHUD->AddElimAnnouncement("You", "Yourself");
				return;
			}
			if (Attacker == Victim && Attacker != Self)
			{
				ShooterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "Themselves");
				return;
			}
			ShooterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ShooterHUD = Cast<AShooterHUD>(GetHUD());
	ServerCheckMatchState();
}

void AShooterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterPlayerController, MatchState);
	DOREPLIFETIME(AShooterPlayerController, bShowTeamScores);
}

void AShooterPlayerController::HideTeamScores()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->RedTeamScore &&
		ShooterHUD->CharacterOverlay->BlueTeamScore &&
		ShooterHUD->CharacterOverlay->ScoreSpacerText;
	if (bHUDValid)
	{
		ShooterHUD->CharacterOverlay->RedTeamScore->SetText(FText());
		ShooterHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
		ShooterHUD->CharacterOverlay->ScoreSpacerText->SetText(FText());
	}
}

void AShooterPlayerController::InitTeamScores()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->RedTeamScore &&
		ShooterHUD->CharacterOverlay->BlueTeamScore &&
		ShooterHUD->CharacterOverlay->ScoreSpacerText;
	if (bHUDValid)
	{
		FString Zero("0");
		FString Spacer("|");
		ShooterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(Zero));
		ShooterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
		ShooterHUD->CharacterOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
	}
}

void AShooterPlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->RedTeamScore;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		ShooterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void AShooterPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->BlueTeamScore;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		ShooterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void AShooterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
	CheckPing(DeltaTime);
}

void AShooterPlayerController::CheckPing(float DeltaTime)
{
	if (HasAuthority()) return;
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		if (!PlayerState) PlayerState = GetPlayerState<APlayerState>();
		if (PlayerState)
		{
			if (PlayerState->GetPingInMilliseconds() > HighPingThreshold) 
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	bool bHighPingAnimationPlaying =
		ShooterHUD && ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->HighPingAnimation &&
		ShooterHUD->CharacterOverlay->IsAnimationPlaying(ShooterHUD->CharacterOverlay->HighPingAnimation);
	if (bHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

void AShooterPlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidget == nullptr) return;
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void AShooterPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

// Is the ping too high?
void AShooterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void AShooterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void AShooterPlayerController::HighPingWarning()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->HighPingImage &&
		ShooterHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		ShooterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		ShooterHUD->CharacterOverlay->PlayAnimation(
			ShooterHUD->CharacterOverlay->HighPingAnimation,
			0.f,
			5);
	}
}

void AShooterPlayerController::StopHighPingWarning()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->HighPingImage &&
		ShooterHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		ShooterHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		if (ShooterHUD->CharacterOverlay->IsAnimationPlaying(ShooterHUD->CharacterOverlay->HighPingAnimation))
		{
			ShooterHUD->CharacterOverlay->StopAnimation(ShooterHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

void AShooterPlayerController::ServerCheckMatchState_Implementation()
{
	AShooterGameMode* GameMode = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

void AShooterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (ShooterHUD && MatchState == MatchState::WaitingToStart)
	{
		ShooterHUD->AddAnnouncement();
	}
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn);
	if (ShooterCharacter)
	{
		SetHUDHealth(ShooterCharacter->GetHealth(), ShooterCharacter->GetMaxHealth());
	}
}

void AShooterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay && 
		ShooterHUD->CharacterOverlay->HealthBar && 
		ShooterHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		ShooterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		ShooterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AShooterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->ShieldBar &&
		ShooterHUD->CharacterOverlay->ShieldText;
	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		ShooterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		ShooterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void AShooterPlayerController::SetHUDScore(float Score)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		ShooterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void AShooterPlayerController::SetHUDDefeats(int32 Defeats)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		ShooterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void AShooterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		ShooterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void AShooterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		ShooterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void AShooterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			ShooterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = FMath::FloorToInt(CountdownTime) % 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		ShooterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void AShooterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->Announcement &&
		ShooterHUD->Announcement->WarmupTime;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			ShooterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		ShooterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void AShooterPlayerController::SetHUDGrenades(int32 Grenades)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD &&
		ShooterHUD->CharacterOverlay &&
		ShooterHUD->CharacterOverlay->GrenadesText;
	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		ShooterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

void AShooterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;
}

void AShooterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (ShooterHUD && ShooterHUD->CharacterOverlay)
		{
			CharacterOverlay = ShooterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);

				AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetPawn());
				if (ShooterCharacter && ShooterCharacter->GetCombat())
				{
					if (bInitializeGrenades) SetHUDGrenades(ShooterCharacter->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;

	InputComponent->BindAction("Quit", IE_Pressed, this, &AShooterPlayerController::ShowReturnToMainMenu);

}

void AShooterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReciept = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReciept);
}

void AShooterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float AShooterPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AShooterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AShooterPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch) 
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AShooterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AShooterPlayerController::HandleMatchHasStarted(bool bTeamsMatch) 
{
	if (HasAuthority()) bShowTeamScores = bTeamsMatch;
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	if (ShooterHUD)
	{
		if (ShooterHUD->CharacterOverlay == nullptr) ShooterHUD->AddCharacterOverlay();
		if (ShooterHUD->Announcement)
		{
			ShooterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		if (!HasAuthority()) return;
		if (bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

void AShooterPlayerController::HandleCooldown()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	if (ShooterHUD)
	{
		ShooterHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid = ShooterHUD->Announcement &&
			ShooterHUD->Announcement->AnnouncementText &&
			ShooterHUD->Announcement->InfoText;

		if (bHUDValid)
		{
			ShooterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			ShooterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
			AShooterPlayerState* ShooterPlayerState = GetPlayerState<AShooterPlayerState>();
			if (ShooterGameState && ShooterPlayerState)
			{
				TArray<AShooterPlayerState*> TopPlayers = ShooterGameState->TopScoringPlayers;
				FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(ShooterGameState) : GetInfoText(TopPlayers);

				ShooterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetPawn());
	if (ShooterCharacter && ShooterCharacter->GetCombat())
	{
		ShooterCharacter->bDisableGameplay = true;
		ShooterCharacter->GetCombat()->FireButtonPressed(false);
	}
}

FString AShooterPlayerController::GetInfoText(const TArray<class AShooterPlayerState*>& Players)
{
	AShooterPlayerState* ShooterPlayerState = GetPlayerState<AShooterPlayerState>();
	if (ShooterPlayerState == nullptr) return FString();
	FString InfoTextString;
	if (Players.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == ShooterPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		InfoTextString = Announcement::PlayersTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}

	return InfoTextString;
}

FString AShooterPlayerController::GetTeamsInfoText(AShooterGameState* ShooterGameState)
{
	if (ShooterGameState == nullptr) return FString();
	FString InfoTextString;

	const int32 RedTeamScore = ShooterGameState->RedTeamScore;
	const int32 BlueTeamScore = ShooterGameState->BlueTeamScore;

	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if (BlueTeamScore > RedTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
	}

	return InfoTextString;
}