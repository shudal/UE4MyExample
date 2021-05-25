// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwingEmulator.generated.h"

UCLASS()
class HACKD_API ASwingEmulator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASwingEmulator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* smori = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UStaticMeshComponent* smswing = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UPhysicsConstraintComponent* myphycon = nullptr;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
		void MyEmulate(FVector target_pos, FVector player_pos);
	

	UFUNCTION(BlueprintCallable)
		class USceneComponent* GetSwingComponent();
};
