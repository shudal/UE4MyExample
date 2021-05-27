// Fill out your copyright notice in the Description page of Project Settings.


#include "SwingEmulator.h"
#include "Components/StaticMeshComponent.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"
// Sets default values
ASwingEmulator::ASwingEmulator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	smori = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("mysmori"));
	smswing = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("mysmswing"));
	 
}

// Called when the game starts or when spawned
void ASwingEmulator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASwingEmulator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASwingEmulator::MyEmulate(FVector target_pos, FVector player_pos) {
	smori->SetWorldLocation(target_pos);
	smswing->SetWorldLocation(player_pos);
	myphycon = (UPhysicsConstraintComponent*)NewObject<USceneComponent>(this, UPhysicsConstraintComponent::StaticClass(), FName(FString::Printf(TEXT("myphycon"))));
	if (myphycon) {
		myphycon->RegisterComponent();
		myphycon->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		myphycon->SetConstrainedComponents(smori, "None", smswing,"None");
		smswing->SetSimulatePhysics(true);
		
		/*
		myphycon->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Free, 90);
		myphycon->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Free, 90);
		myphycon->SetLinearXLimit(ELinearConstraintMotion::LCM_Free, 0);
		myphycon->SetLinearYLimit(ELinearConstraintMotion::LCM_Free, 0);
		myphycon->SetLinearZLimit(ELinearConstraintMotion::LCM_Free, 0);
		*/
	}
}

USceneComponent* ASwingEmulator::GetSwingComponent() {
	return smswing;
}