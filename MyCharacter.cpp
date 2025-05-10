// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "MyPlayerController.h"
#include "SQLiteDatabase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyGameInstance.h"
#include "MyGameInstanceSubsystem.h"

void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();
    EnableInput(GetWorld()->GetFirstPlayerController());

}

void AMyCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    PlayerController = Cast<AMyPlayerController>(NewController);
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("SetupPlayerInputComponent called"));  //Debug

    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Bind actions
    PlayerInputComponent->BindAction("Right_Dash", IE_Pressed, this, &AMyCharacter::RightDash);
    PlayerInputComponent->BindAction("Left_Dash", IE_Pressed, this, &AMyCharacter::LeftDash);

}

void AMyCharacter::LeftDash() {

    if (bIsDashing) return; 

    if (ConsumeEnergy(EnergyCost_Dash)) {
        
        bIsDashing = true;

        FVector LaunchVelocity = GetActorRightVector() * DashVelocity * (-1);
        LaunchCharacter(LaunchVelocity, true, true);

        GetWorldTimerManager().SetTimer(DashResetTimerHandle,
            this,
            &AMyCharacter::ResetDash,
            DashDelay,
            false
        );
    }

}

void AMyCharacter::RightDash()
{
    // Early return to prevent spamming this action
    if (bIsDashing) return;

    // Ensure the player has the energy for this action
    if (ConsumeEnergy(EnergyCost_Dash))
    {
        // We are now dashing
        bIsDashing = true;

        // Launch character
        FVector LaunchVelocity = GetActorRightVector() * DashVelocity;
        LaunchCharacter(LaunchVelocity, true, true);

        // Start a timer to reset bIsDashing
        GetWorldTimerManager().SetTimer(
            DashResetTimerHandle,
            this,
            &AMyCharacter::ResetDash,  // Function to call
            DashDelay,                // Time delay in seconds
            false                     // Do not loop
        );
    }
}

bool AMyCharacter::ConsumeEnergy(const float Energy)
{
    if (Energy <= CurrentEnergy)
    {
        CurrentEnergy -= Energy;
        return true;
    }

    return false;
}

void AMyCharacter::ResetDash()
{
    bIsDashing = false;
    GetCharacterMovement()->StopMovementImmediately();
}

// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    //ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>("ProjectileSpawnPoint");
}

void AMyCharacter::SetPlayerName(const FText& Name) {

    PlayerController->SetPlayerName(Name);

}

FText AMyCharacter::GetPlayerName() {

    return PlayerController ? PlayerController->GetPlayerName() : FText();

}

void AMyCharacter::SaveSQLite() const
{
    // Get the player name from our custom game instance
    const FString PlayerName = Cast<UMyGameInstance>(GetGameInstance())->PlayerName;

    // Log debug message
    UE_LOG(LogTemp, Log, TEXT("AMyCharacter::SaveSQLite() PlayerName: %s, Score: %d"), *PlayerName, PlayerScore);

    UMyGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UMyGameInstanceSubsystem>();
    Subsystem->UpdatePlayerScore(PlayerName, PlayerScore);
}

/**void AMyCharacter::Fire() {

    GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "Fire");

    if (!IsValid(ProjectileClass)) {
        GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "Invalid Projectile class.");
        return; 
    }

    const FVector SpawnLocation = ProjectileSpawnPoint->GetComponentLocation();
    
    const FRotator SpawnRotation = FRotationMatrix::MakeFromX(ProjectileSpawnPoint->GetForwardVector()).Rotator();

    FActorSpawnParameters SpawnParameters;
    
    AActor* Projectile = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParameters);
    if (!IsValid(Projectile)) {
        GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, "Invalid Projectile");
    }
}

*/



