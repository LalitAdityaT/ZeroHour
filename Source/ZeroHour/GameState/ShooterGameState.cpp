// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameState.h"
#include "Net/UnrealNetwork.h"
#include "ZeroHour/PlayerState/ShooterPlayerState.h"
#include "ZeroHour/PlayerController/ShooterPlayerController.h"

void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterGameState, TopScoringPlayers);
	DOREPLIFETIME(AShooterGameState, RedTeamScore);
	DOREPLIFETIME(AShooterGameState, BlueTeamScore);
}

void AShooterGameState::UpdateTopScore(class AShooterPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void AShooterGameState::RedTeamScores()
{
	++RedTeamScore;
	AShooterPlayerController* BPlayer = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AShooterGameState::BlueTeamScores()
{
	++BlueTeamScore;
	AShooterPlayerController* BPlayer = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void AShooterGameState::OnRep_RedTeamScore()
{
	AShooterPlayerController* BPlayer = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void AShooterGameState::OnRep_BlueTeamScore()
{
	AShooterPlayerController* BPlayer = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}