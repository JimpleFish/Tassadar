#include "WorkerManager.h"

WorkerManager::WorkerManager(void)
{
}


WorkerManager::~WorkerManager(void)
{
}

void WorkerManager::WorkerCreated(BWAPI::Unit &proleteriat)
{
}

void WorkerManager::TickWorker(BWAPI::Unit &proleteriat)
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