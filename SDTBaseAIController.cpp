// Fill out your copyright notice in the Description page of Project Settings.

#include "SoftDesignTraining.h"
#include "SDTBaseAIController.h"


ASDTBaseAIController::ASDTBaseAIController(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    m_ReachedTarget = false;
}

void ASDTBaseAIController::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    ChooseBehavior(deltaTime);

    if (m_ReachedTarget)
    {
        GoToBestTarget(deltaTime);
		ShowNavigationPath();
    }
    else
    {
        ShowNavigationPath();
    }
}


