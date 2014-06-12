#include "WorkerManager.h"

WorkerManager::WorkerManager(void)
{
}


WorkerManager::~WorkerManager(void)
{
}

void WorkerManager::WorkerCreated(BWAPI::Unit &proleteriat)
{
	Broodwar->sendText("Built probe at %d", Broodwar->getFrameCount());
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