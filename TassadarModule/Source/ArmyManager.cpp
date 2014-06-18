#include "ArmyManager.h"


ArmyManager::ArmyManager(void)
{
}


ArmyManager::~ArmyManager(void)
{
}

void ArmyManager::warriorCreated(Unit warrior)
{
	armedForces.push_back(warrior);
}

void ArmyManager::warriorFallen(Unit warrior)
{
	armedForces.remove(warrior);
}

void ArmyManager::allOutAttack(Position target)
{
	armedForces.attack(target);
}

int ArmyManager::armySize()
{
	return armedForces.size();
}