// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "hackdCharacter.generated.h"

UCLASS(config=Game)
class AhackdCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AhackdCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }


private:
	void ProcessHitResult(const TArray<FHitResult>& HitResults);
	

	void HideRope();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UCableComponent* myrope = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsConstraintSwing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float tmpmin = 200;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UPhysicsConstraintComponent* myphycon = nullptr;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class ASwingEmulator* swingemu = nullptr;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<class ASwingEmulator> SwingEmuClass = nullptr;
	virtual void BeginPlay() override;


	UFUNCTION(BlueprintCallable)
		void ConstraintSwing();

	UFUNCTION(BlueprintCallable)
		void RelaseConstraintSwing();

	UFUNCTION(BlueprintCallable)
		void ReleaseSilk();

	UFUNCTION(BlueprintCallable)
		void SetConstraintSwing();
public:
	void ShootSilk();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsSilking = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector silk_target_pos = FVector::ZeroVector;

	void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float tan_huang_foc = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float tan_huang_k = 0.5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float silk_mindis_to_care = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector follow_camera_normal_rel_loc;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector follow_camera_const_rel_loc;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float silk_shoot_dis = 3500;

};

