#pragma once
#include <BWAPI.h>

using namespace BWAPI;

class ArmyManager
{
public:
	ArmyManager(void);
	~ArmyManager(void);
	void warriorCreated(Unit);
	void warriorFallen(Unit);
	void allOutAttack(Position target);
	int armySize();

private:
	Unitset armedForces;
};

