// Fill out your copyright notice in the Description page of Project Settings.
#include "MyGameInstanceSubsystem.h"
#include "SQLiteDatabase.h"



UMyGameInstanceSubsystem::UMyGameInstanceSubsystem() {

	FieldName.Username = "Username";
	FieldName.Password = "Password";
	FieldName.PlayerName = "PlayerName";
	FieldName.PlayerScore = "PlayerScore";

}


FString UMyGameInstanceSubsystem::EncryptionPass(const FText& Password) {
	//TODO Implement some encryption algorithm
	return Password.ToString();
	
}

bool UMyGameInstanceSubsystem::CreateUser(const FText& Username, const FText& Password, const FText& PlayerName) const {
	
	UE_LOG(LogTemp, Warning, TEXT("DEBUG:: CREATE USER Username = %s, PlayerName = %s, Password = %s"), *Username.ToString(),
		*PlayerName.ToString(), *Password.ToString());
	 
	if (Username.IsEmpty() || Password.IsEmpty() || PlayerName.IsEmpty()) {
		UE_LOG(LogTemp, Warning, TEXT("CreateUser:: One or more parameters are empty"));
		return false; 
	}

	FSQLiteDatabase Database; 
	if (!Database.Open(DatabasePath, ESQLiteDatabaseOpenMode::ReadWrite)) {
		UE_LOG(LogTemp, Warning, TEXT("CreateUser:: Failed to open db %s"), *DatabasePath);
		return false;
	}

	FSQLitePreparedStatement PreparedStatement;
	const FString Querry = "INSERT INTO " + TableName + " (" + FieldName.Username + ", " + FieldName.PlayerName + ", "
		+ FieldName.PlayerScore + ", " + FieldName.Password + ") VALUES (?, ?, ?, ?);";

	UE_LOG(LogTemp, Warning, TEXT("DEBUG CREATE USER Querry = %s"), *Querry);

	if (!PreparedStatement.Create(Database, *Querry)) {
		UE_LOG(LogTemp, Warning, TEXT("CreateUser:: Failed to prepare statement"));
		Database.Close();
		return false;
	}

	const FString PasswordEncrypted = EncryptionPass(Password); // not really :D 

	int32 BindingIndex = 1; 
	if (!PreparedStatement.SetBindingValueByIndex(BindingIndex++ , Username.ToString()) || 
		!PreparedStatement.SetBindingValueByIndex(BindingIndex++, PlayerName.ToString()) ||
		!PreparedStatement.SetBindingValueByIndex(BindingIndex++, 0) ||
		!PreparedStatement.SetBindingValueByIndex(BindingIndex, Password.ToString())) {

		UE_LOG(LogTemp, Error, TEXT("CreateUser:: Failed to execute statement"));
		PreparedStatement.Destroy();
		Database.Close();
		return false;

	}
	
	if (!PreparedStatement.Execute()) {
		UE_LOG(LogTemp, Error, TEXT("CreateUser::Failed to execute statement"));
		PreparedStatement.Destroy();
		Database.Close();
		return false; 
	}

	PreparedStatement.Destroy();
	Database.Close();
	return true;
}

void UMyGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collections) {

	Super::Initialize(Collections);

	FSQLiteDatabase Database;

	if (Database.Open(DatabasePath, ESQLiteDatabaseOpenMode::ReadWriteCreate) == false) {
		
		UE_LOG(LogTemp, Error, TEXT("Initialize:: Failed to open database: %s"), *DatabasePath);
		return;

	}

	const FString TableQuerry = "CREATE TABLE IF NOT EXISTS " + TableName + " (" + FieldName.Username + " TEXT PRIMARY KEY, " + FieldName.PlayerName +
		" TEXT UNIQUE NOT NULL, " + FieldName.PlayerScore + " INTEGER, " + FieldName.Password + " TEXT);";

	if (Database.Execute(*TableQuerry) == false) {
		UE_LOG(LogTemp, Error, TEXT("Initialize:: Failed to create table "));
	}

	Database.Close();

}

FString UMyGameInstanceSubsystem::LoginPlayer(const FText& Username, const FText& Password) const {

	if (Username.IsEmpty() || Password.IsEmpty()) {

		return FString();

	}

	FSQLiteDatabase Database;
	if (!Database.Open(DatabasePath, ESQLiteDatabaseOpenMode::ReadOnly)) {

		UE_LOG(LogTemp, Error, TEXT("Login:: Failed to open database: %s"), *DatabasePath);
		return FString();

	}

	FSQLitePreparedStatement PreparedStatement; 
	const FString Querry = "SELECT " + FieldName.PlayerName + ", " + FieldName.Password + " FROM " + TableName + " WHERE Username = ?;";

	if (!PreparedStatement.Create(Database, *Querry)) {
		UE_LOG(LogTemp, Error, TEXT("Login:: Failed to prepare statement"));
		Database.Close();
		return FString();
	
	}

	if (!PreparedStatement.SetBindingValueByIndex(1, Username.ToString())) {
		UE_LOG(LogTemp, Error, TEXT("Login:: Failed to bind username parameter"));
		PreparedStatement.Destroy();
		Database.Close();
		return FString();

	}

	if (!PreparedStatement.Execute()) {
		UE_LOG(LogTemp, Error, TEXT("Login:: Failed to execute statement"));
		PreparedStatement.Destroy();
		Database.Close();
		return FString();

	}

	if (PreparedStatement.Step() == ESQLitePreparedStatementStepResult::Row) {
		
		FString PlayerNameStr;
		FString StorePassword;

		if (!PreparedStatement.GetColumnValueByName(*FieldName.PlayerName, PlayerNameStr) ||
			!PreparedStatement.GetColumnValueByName(*FieldName.Password, StorePassword)) {
			
			UE_LOG(LogTemp, Error, TEXT("Login:: Failed to get column values"));
			PreparedStatement.Destroy();
			Database.Close();
			return FString();

		}

		const FString SetPassword = EncryptionPass(Password);

		if (SetPassword == StorePassword) {
			PreparedStatement.Destroy();
			Database.Close();
			return PlayerNameStr;
		}

	}

	PreparedStatement.Destroy();
	Database.Close();
	return FString();

}

bool UMyGameInstanceSubsystem::PlayerNameExists(const FText& PlayerName) const {

	if (PlayerName.IsEmpty()) {
		UE_LOG(LogTemp, Error, TEXT("PlayerNameExists:: Player name is Empty"));
		return false;
	}

	FSQLiteDatabase Database;
	if (!Database.Open(DatabasePath, ESQLiteDatabaseOpenMode::ReadOnly)) {
		UE_LOG(LogTemp, Error, TEXT("PlayerNameExists:: Failed to open database: %s"), *DatabasePath);
		return false;
	}

	FSQLitePreparedStatement PrepareStatement;
	const FString Querry = "SELECT 1 FROM " + TableName + " WHERE " + FieldName.PlayerName + " = ? LIMIT 1;";
	if (!PrepareStatement.Create(Database, *Querry)) {
		UE_LOG(LogTemp, Error, TEXT("PlayerNameExists:: Failed to prepare statement"));
		Database.Close();
		return false;
	}

	if (!PrepareStatement.SetBindingValueByIndex(1, PlayerName.ToString())) {
		UE_LOG(LogTemp, Error, TEXT("PlayerNameExists:: Failed to bind player name parameter"));
		PrepareStatement.Destroy();
		Database.Close();
		return false;
	}

	bool bExists = false; 
	if (PrepareStatement.Execute() && PrepareStatement.Step() == ESQLitePreparedStatementStepResult::Row) {
		bExists = true;
	}

	PrepareStatement.Destroy();
	Database.Close();
	return bExists;

}

bool UMyGameInstanceSubsystem::UpdatePlayerScore(const FString& PlayerName, const int32 PlayerScore) const {
	
	UE_LOG(LogTemp, Warning, TEXT("(DEBUG)::UpdatePlayerScore - Begin"));
	
	if (PlayerName.IsEmpty()) {
		UE_LOG(LogTemp, Error, TEXT("UpdatePlayerScore:: PlayerName is empty"));
		return false;
	}

	FSQLiteDatabase Database; 
	if (!Database.Open(DatabasePath, ESQLiteDatabaseOpenMode::ReadWrite)) {
		UE_LOG(LogTemp, Error, TEXT("UpdatePlayerScore:: Failed to open database: %s "), *DatabasePath);
		return false;
	}

	FSQLitePreparedStatement PreparedStatement; 
	const FString Querry = "UPDATE " + TableName + " SET " + FieldName.PlayerScore + " = " + FieldName.PlayerScore + " + ? WHERE " +
		FieldName.PlayerName + " = ?;";

	if (!PreparedStatement.Create(Database, *Querry)) {
		UE_LOG(LogTemp, Error, TEXT("UpdatePlayerScore:: Failed to prepare statement"));
		Database.Close();
		return false;
	}

	if (!PreparedStatement.SetBindingValueByIndex(1, PlayerScore) || !PreparedStatement.SetBindingValueByIndex(2, PlayerName)){	
		UE_LOG(LogTemp, Error, TEXT("UpdatePlayerScore:: Failed to bind paramaters"));
		PreparedStatement.Destroy();
		Database.Close();
		return false; 
	}

	if (!PreparedStatement.Execute()) {
		UE_LOG(LogTemp, Error, TEXT("UpdatePlayerScore:: Failed to execute statement."));
		PreparedStatement.Destroy();
		Database.Close();
		return false;
	}

	PreparedStatement.Destroy();
	Database.Close();
	return true;

}

bool UMyGameInstanceSubsystem::UsernameExists(const FText& Username) const {

	if (Username.IsEmpty()) {
		UE_LOG(LogTemp, Error, TEXT("UsernameExists:: Username is Empty"));
		return false;
	}

	FSQLiteDatabase Database;
	if (!Database.Open(DatabasePath, ESQLiteDatabaseOpenMode::ReadOnly)) {
		UE_LOG(LogTemp, Error, TEXT("UsernameExists:: Failed to Open data base: %s"), *DatabasePath);
		return false; 
	}

	FSQLitePreparedStatement PreparedStatement;

	const FString Querry = "SELECT 1 FROM " + TableName + " WHERE " + FieldName.Username + " = ? LIMIT 1;";

	if (!PreparedStatement.Create(Database, *Querry)) {

		UE_LOG(LogTemp, Error, TEXT("UsernameExists:: Failed to prepare statement"));
		Database.Close();
		return false;
	}

	if (!PreparedStatement.SetBindingValueByIndex(1, Username.ToString())) {
		UE_LOG(LogTemp, Error, TEXT("UsernameExists:: Failed to bind username parameter"));
		PreparedStatement.Destroy();
		Database.Close();
		return false;
	}

	bool bExists = false; 
	if (PreparedStatement.Execute() && PreparedStatement.Step() == ESQLitePreparedStatementStepResult::Row) {
		bExists = true; 
	}

	PreparedStatement.Destroy();
	Database.Close();
	return bExists;

}

