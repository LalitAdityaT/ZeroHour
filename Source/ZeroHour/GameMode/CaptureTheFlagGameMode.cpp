// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagGameMode.h"
#include "ZeroHour/Weapon/Flag.h"
#include "ZeroHour/CaptureTheFlag/FlagZone.h"
#include "ZeroHour/GameState/ShooterGameState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(class AShooterCharacter* ElimmedCharacter, class AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController)
{
	AShooterGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	AShooterGameState* BGameState = Cast<AShooterGameState>(GameState);
	if (BGameState)
	{
		if (Zone->Team == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
		if (Zone->Team == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
	}
}