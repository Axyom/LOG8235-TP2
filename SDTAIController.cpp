// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SDTCollectible.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "UnrealMathUtility.h"
#include "SDTUtils.h"
#include "EngineUtils.h"
#include "AI/Navigation/NavigationPath.h"

ASDTAIController::ASDTAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
}

void ASDTAIController::GoToBestTarget(float deltaTime)
{
    //Move to target depending on current behavior
	APawn* selfPawn = GetPawn();

	MoveToLocation(m_target);
}

void ASDTAIController::OnMoveToTarget()
{
    m_ReachedTarget = false;
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    m_ReachedTarget = true;
}

void ASDTAIController::ShowNavigationPath()
{
    //Show current navigation path DrawDebugLine and DrawDebugSphere

	APawn* selfPawn = GetPawn();

	// on trace la ligne entre le personnage et la cible
	FColor debugColor = FColor();
	debugColor.InitFromString("R=255,G=0,B=0,A=255");

	UNavigationPath *tpath;
	UNavigationSystem* NavSys = UNavigationSystem::GetCurrent(GetWorld());

	tpath = NavSys->FindPathToLocationSynchronously(GetWorld(), selfPawn->GetActorLocation(), m_target);

	if (tpath != NULL)
	{
		for (int pointiter = 0; pointiter < tpath->PathPoints.Num(); pointiter++)
		{
			DrawDebugSphere(GetWorld(), tpath->PathPoints[pointiter], 10.0f, 12, FColor(255, 0, 0));
			if (pointiter + 1 < tpath->PathPoints.Num())
				DrawDebugLine(GetWorld(), tpath->PathPoints[pointiter], tpath->PathPoints[pointiter + 1], debugColor);
		}
	}
}

void ASDTAIController::ChooseBehavior(float deltaTime)
{
    UpdatePlayerInteraction(deltaTime);
}

void ASDTAIController::UpdatePlayerInteraction(float deltaTime)
{
    //finish jump before updating AI state
    if (AtJumpSegment)
        return;

    APawn* selfPawn = GetPawn();
    if (!selfPawn)
        return;

    ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
    FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;

    TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_COLLECTIBLE));
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));

    TArray<FHitResult> allDetectionHits;
    GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

    FHitResult detectionHit;
	if (GetHightestPriorityDetectionHit(allDetectionHits, detectionHit))
	{
		//on garde la cible en memoire
		m_target = detectionHit.GetActor()->GetActorLocation();
	}

    //Set behavior based on hit

    DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
}

bool ASDTAIController::GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit)
{
	if (hits.Num() > 0)
	{
		for (const FHitResult& hit : hits)
		{
			if (UPrimitiveComponent* component = hit.GetComponent())
			{
				if (component->GetCollisionObjectType() == COLLISION_PLAYER)
				{
					//we can't get more important than the player
					outDetectionHit = hit;
					return true;
				}
				else if (component->GetCollisionObjectType() == COLLISION_COLLECTIBLE)
				{
					outDetectionHit = hit;
				}
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

void ASDTAIController::AIStateInterrupted()
{
    StopMovement();
    m_ReachedTarget = true;
}