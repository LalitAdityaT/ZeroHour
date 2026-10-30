// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerState.h"
#include "ZeroHour/Character/ShooterCharacter.h"
#include "ZeroHour/PlayerController/ShooterPlayerController.h"
#include "Net/UnrealNetwork.h"

void AShooterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterPlayerState, Defeats);
	DOREPLIFETIME(AShooterPlayerState, Team);
}

void AShooterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore (GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->GetController()) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(Score);
		}
	}
}

void AShooterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	
	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->GetController()) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AShooterPlayerState::AddToDefeats(int32 DefeatAmount)
{
	Defeats += DefeatAmount;
	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->GetController()) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void AShooterPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void AShooterPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;

	AShooterCharacter* BCharacter = Cast <AShooterCharacter>(GetPawn());
	if (BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}

void AShooterPlayerState::OnRep_Team()
{
	AShooterCharacter* BCharacter = Cast <AShooterCharacter>(GetPawn());
	if (BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}