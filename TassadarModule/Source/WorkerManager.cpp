#include "WorkerManager.h"

WorkerManager::WorkerManager(void)
{
}


WorkerManager::~WorkerManager(void)
{
}

void WorkerManager::WorkerCreated(Unit &proleteriat)
{
	unwashedMasses.push_back(proleteriat);
}

void WorkerManager::TickWorkers()
{
	for(auto proleteriat = unwashedMasses.begin(); proleteriat != unwashedMasses.end(); ++proleteriat)
	{
		if ( proleteriat->isIdle() )
		{
			if ( proleteriat->isCarryingGas() || proleteriat->isCarryingMinerals() )
			{
				proleteriat->returnCargo();
			}
			else if ( !proleteriat->getPowerUp() )
			{
				proleteriat->gather(proleteriat->getClosestUnit( IsMineralField || IsRefinery ));
			}
		}
	}
}

int WorkerManager::massOfMasses()
{
	return unwashedMasses.size();
}

Unit WorkerManager::GetMeAWorker(Unit nexus)
{
	Unit worker;

	if(nexus)
		worker = nexus->getClosestUnit(GetType == UnitTypes::Protoss_Probe && (IsIdle || IsGatheringMinerals) && IsOwned);
	return worker;
}

bool WorkerManager::MineralsSaturated()
{
	return unwashedMasses.size() >= 3*9;
}