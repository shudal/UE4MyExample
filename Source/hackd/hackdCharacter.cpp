// Copyright Epic Games, Inc. All Rights Reserved.

#include "hackdCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h" 
#include "CableComponent.h"
#include "Kismet/KismetMathLibrary.h" 
#include "Kismet/GameplayStatics.h" 
#include "Components/LineBatchComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "SwingEmulator.h"
//////////////////////////////////////////////////////////////////////////
// AhackdCharacter

AhackdCharacter::AhackdCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	 
	
	myrope = CreateDefaultSubobject<UCableComponent>(TEXT("MySilkRope"));
	myrope->SetupAttachment(RootComponent);  

	follow_camera_normal_rel_loc = FVector(0, -40, 30);
	follow_camera_const_rel_loc = FVector(-250, 0, 100);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AhackdCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("ShootSilk", IE_Released, this, &AhackdCharacter::ShootSilk);
	PlayerInputComponent->BindAction("ConstraintSwing", IE_Released, this, &AhackdCharacter::SetConstraintSwing);
	
	PlayerInputComponent->BindAction("ReleaseConstraintSwing", IE_Released, this, &AhackdCharacter::ReleaseSilk);

	PlayerInputComponent->BindAxis("MoveForward", this, &AhackdCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AhackdCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AhackdCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AhackdCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AhackdCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AhackdCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AhackdCharacter::OnResetVR);
}


void AhackdCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AhackdCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AhackdCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AhackdCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AhackdCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AhackdCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AhackdCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


void AhackdCharacter::SetConstraintSwing() { 
	if (bIsSilking == false) {
		return;
	}

	if (bIsConstraintSwing == false) {
		ConstraintSwing();
	}
	else {
		RelaseConstraintSwing();
	} 
}

void AhackdCharacter::ReleaseSilk() {
	RelaseConstraintSwing();

	bIsSilking = false;
	HideRope();
}
void AhackdCharacter::RelaseConstraintSwing() {
	bIsConstraintSwing = false;
	if (swingemu && swingemu->IsPendingKill() == false) {
		swingemu->Destroy();
		swingemu = nullptr;
	}
	FollowCamera->SetRelativeLocation(follow_camera_normal_rel_loc);
}
void AhackdCharacter::ConstraintSwing() {
	bIsConstraintSwing = true; 

	FollowCamera->SetRelativeLocation(follow_camera_const_rel_loc);

	if (GetWorld() && SwingEmuClass) {
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		FVector SpawnLoc;
		FRotator SpawnRot = FRotator::ZeroRotator;

		SpawnLoc = silk_target_pos;
		swingemu = GetWorld()->SpawnActor<ASwingEmulator>(SwingEmuClass, SpawnLoc, SpawnRot, SpawnParams);
		
		verifyf(swingemu != nullptr, TEXT("swingemu can not null"));
		swingemu->MyEmulate(silk_target_pos, GetActorLocation());
		
		//this->LaunchCharacter(FVector::ZeroVector,true,true);
		//this->AttachToComponent(swingemu->GetSwingComponent(),FAttachmentTransformRules(EAttachmentRule::SnapToTarget,true));
	
	}

}
void AhackdCharacter::ShootSilk() {  
	
	FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, this);
	RV_TraceParams.bTraceComplex = false;
	RV_TraceParams.bReturnPhysicalMaterial = true;

	//Re-initialize hit info
	//FHitResult RV_Hit(ForceInit);

	FCollisionObjectQueryParams coqp;
	coqp.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
	coqp.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);
	coqp.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
 
	FVector CamLoc;
	FRotator CamRot;
	GetController()->GetPlayerViewPoint(CamLoc, CamRot);

	FVector Start = CamLoc;// = this->GetMesh()->GetSocketLocation("hand_r");
	//Start = GetActorLocation();
	auto Rot = CamRot;
	FVector End;// = GetActorLocation() + UKismetMathLibrary::GetForwardVector(Rot) * 1500;

	//Rot = UKismetMathLibrary::FindLookAtRotation(FollowCamera->GetComponentLocation(), GetActorLocation());
	//End = GetActorLocation() + UKismetMathLibrary::GetForwardVector(Rot) * 1500;
	End = GetActorLocation() + Rot.Vector() * silk_shoot_dis;
	if (GetWorld()) {
		//GetWorld()->PersistentLineBatcher->DrawLine(Start, End, FLinearColor::Red,1,10,5);
	}
	/*
	auto startp = Start, endp = End;
	myrope->SetWorldLocation(startp);
	myrope->EndLocation = endp - startp;
	myrope->CableLength = (endp-startp).Size();
	*/
	/*
	auto startp = Start, endp = End;
	myrope->SetWorldLocation(startp);
	myrope->EndLocation = endp - startp;
	*/
	//End = GetActorLocation() + Rot.Vector() * 1500;
	UE_LOG(LogClass, Log, TEXT("actor loc:%s"), *(GetActorLocation().ToString()));

	//auto tmps = End.ToString();
	UE_LOG(LogClass, Log, TEXT("start:%s,end:%s"),*(Start.ToString()),*(End.ToString()));
	TArray<FHitResult> HitResults;
	bool everHit = GetWorld()->LineTraceMultiByObjectType(
		HitResults,
		Start,
		End,
		coqp,
		RV_TraceParams
	);
	
	ProcessHitResult(HitResults); 
}


void AhackdCharacter::ProcessHitResult(const TArray<FHitResult>& HitResults) {
	bool EverHited = false; 

	FVector hand_r_pos = this->GetMesh()->GetSocketLocation("hand_r");
	FVector hit_loc = hand_r_pos;
	for (int i = 0; i < HitResults.Num() && EverHited == false; i++) {
		auto HitRe = HitResults[i];

		bool bBlockingHit, bInitialOverlap;
		float Time, Distance;
		FVector Location, ImpactPoint, Normal, ImpactNormal;
		UPhysicalMaterial* PhysMat;
		AActor* HitActor;
		UPrimitiveComponent* HitComponent;
		FName HitBoneName;
		int32 HitItem, FaceIndex;
		FVector TraceStart, TraceEnd;
		UGameplayStatics::BreakHitResult(HitRe, bBlockingHit, bInitialOverlap, Time, Distance, Location, ImpactPoint, Normal, ImpactNormal
			,
			PhysMat,
			HitActor,
			HitComponent,
			HitBoneName,
			HitItem,
			FaceIndex,
			TraceStart,
			TraceEnd
		); 
		  
		if (HitActor->GetUniqueID() == this->GetUniqueID()) {
			continue;
		}
		hand_r_pos = this->GetMesh()->GetSocketLocation("hand_r");
		hit_loc = Location;


		
		UE_LOG(LogClass, Log, TEXT("have hit"));
		break;
	}

	verifyf(myrope != nullptr, TEXT("rope is null"));
	myrope->bAttachStart = true;
	myrope->bAttachEnd = true;

	float silklen = (hand_r_pos - hit_loc).Size();
	if (silklen > 5) {
		silk_target_pos = hit_loc;
		auto rota = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), silk_target_pos);
		FRotator rott; rott.Roll = 0; rott.Pitch = 0; rott.Yaw = rota.Yaw;
		this->SetActorRotation(rott);
		 
		bIsSilking = true; 
		 
		RelaseConstraintSwing();
		//ConstraintSwing();
	}
	else {
		UE_LOG(LogClass, Log, TEXT("silkelen < 0"));
		HideRope();
		bIsSilking = false;
	}


}

void  AhackdCharacter::HideRope() {
	FVector hand_r_relative_pos = this->GetMesh()->GetSocketTransform("hand_r", ERelativeTransformSpace::RTS_Actor).GetLocation();
	FVector hand_r_world_loc = this->GetMesh()->GetSocketLocation("hand_r");

	myrope->SetWorldLocation(hand_r_world_loc);
	myrope->EndLocation = hand_r_relative_pos;
	myrope->CableLength = 0;
}
void  AhackdCharacter::BeginPlay() {
	Super::BeginPlay();
	HideRope();
}
void AhackdCharacter::Tick(float DeltaTime) { 
	Super::Tick(DeltaTime);



	if (bIsSilking) {

		if (bIsConstraintSwing) {
			auto b1 = GetMovementComponent()->IsFalling();
			if (b1 == false) {
				//RelaseConstraintSwing();
			}
		}

		FVector hand_r_relative_pos = this->GetMesh()->GetSocketTransform("hand_r", ERelativeTransformSpace::RTS_Actor).GetLocation();
		FVector hand_r_world_loc = this->GetMesh()->GetSocketLocation("hand_r");
		FVector Forc = silk_target_pos - GetActorLocation();
		Forc.Normalize();  
		auto silklen = (silk_target_pos - GetActorLocation()).Size();
		Forc *= tan_huang_foc;
		Forc *=  (silklen * tan_huang_k); 

		if (bIsConstraintSwing == false) { 
			this->LaunchCharacter(Forc, true, true);
		}
		else {
			//this->AttachToComponent(swingemu->GetSwingComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			 //FAttachmentTransformRules::
		}
		auto rota = UKismetMathLibrary::FindLookAtRotation(hand_r_world_loc, silk_target_pos);
		myrope->SetWorldLocationAndRotation(silk_target_pos, rota); 
		myrope->EndLocation = hand_r_relative_pos;
		myrope->CableLength = silklen;
		if (silklen < silk_mindis_to_care) {
			//ConstraintSwing();
			ReleaseSilk();
		}
	}
}