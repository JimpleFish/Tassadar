#include "MoneyManager.h"

MoneyManager::MoneyManager(void)
{
	reservedMinerals = 0;
}


MoneyManager::~MoneyManager(void)
{
}

void MoneyManager::SetReserved(int mins)
{
	reservedMinerals = mins;
}

bool MoneyManager::RequestMinerals(int mins)
{
	if(AvailableMinerals() >= mins)
	{
		reservedMinerals += mins;
		return true;
	}
	return false;
}

void MoneyManager::ReturnMinerals(int mins)
{
	reservedMinerals -= mins;
}

int MoneyManager::AvailableMinerals()
{
	return Broodwar->self()->minerals() - reservedMinerals;
}