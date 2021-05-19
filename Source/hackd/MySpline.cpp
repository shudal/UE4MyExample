// Fill out your copyright notice in the Description page of Project Settings.


#include "MySpline.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AMySpline::AMySpline()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	mSplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("RoadSpline"));
	mSplineComponent->SetupAttachment(RootComponent);
	{

		static ConstructorHelpers::FObjectFinder<UStaticMesh> ObjMesh(TEXT("StaticMesh'/Game/Geometry/Meshes/1M_Cube.1M_Cube'"));
		spMesh = ObjMesh.Object;
	}
}

// Called when the game starts or when spawned
void AMySpline::BeginPlay()
{
	Super::BeginPlay();
	auto pCount = mSplineComponent->GetNumberOfSplinePoints();
	auto lastt = mSplineComponent->GetTransformAtSplinePoint(0, ESplineCoordinateSpace::World);
	float maxlen = mSplineComponent->GetSplineLength();

	//mSplineComponent->Add
	for (float i = 0; i < maxlen; i += deltaaddi) {
		USceneComponent* createdComp = NewObject<USceneComponent>(this, USplineMeshComponent::StaticClass(), FName(FString::Printf(TEXT("splinemesh%f"), i)));
		if (createdComp)
		{
			createdComp->RegisterComponent();
			createdComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			//createdComp->SetupAttachment(GetRootComponent());
			USplineMeshComponent* smc = (USplineMeshComponent*)createdComp;
			if (smc) {
				smc->SetStaticMesh(spMesh);
				//auto nowt = mSplineComponent->GetTransformAtSplinePoint(i, ESplineCoordinateSpace::World);
				auto nowt = mSplineComponent->GetTransformAtDistanceAlongSpline(i, ESplineCoordinateSpace::World);

				smc->SetStartAndEnd(lastt.GetLocation(), lastt.GetRotation().Vector(), nowt.GetLocation(), nowt.GetRotation().Vector());

				smc->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				lastt = nowt;
			}
		}
	}
}

// Called every frame
void AMySpline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

