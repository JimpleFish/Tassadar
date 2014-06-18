#include "MoneyManager.h"

MoneyManager::MoneyManager(void)
	: reservedMinerals(0)
	, blockForPylon(false)
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

bool MoneyManager::RequestMinerals(int mins, bool block)
{
	if(AvailableMinerals() >= mins && !block)
	{
		reservedMinerals += mins;
		return true;
	}
	return false;
}

bool MoneyManager::RequestMinerals(int mins)
{
	return RequestMinerals(mins, blockForPylon);
}

void MoneyManager::ReturnMinerals(int mins)
{
	reservedMinerals -= mins;
}

int MoneyManager::AvailableMinerals()
{
	return Broodwar->self()->minerals() - reservedMinerals;
}

int MoneyManager::AvailableGas()
{
	return 0;
}