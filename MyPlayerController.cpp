// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "MyCharacter.h"

void AMyPlayerController::SavePlayerInfo() {

	AMyCharacter* MyCharacter = Cast<AMyCharacter>(GetPawn());
	MyCharacter->SaveSQLite();
}