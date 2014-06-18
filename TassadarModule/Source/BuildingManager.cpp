#include "BuildingManager.h"

BuildingManager::BuildingManager(void)
	: m_gatewayAmount(0)
	, haveCyberCore(false)
{
}

BuildingManager::~BuildingManager(void)
{
}

int BuildingManager::calculateBestGatewayAmount()
{
	int supplyUsed = Broodwar->self()->supplyUsed()/2;
	
	return (supplyUsed >= 50) ? 6 : (supplyUsed / 10) + 2;
}

bool BuildingManager::shouldBuildGateways()
{
	return m_gatewayAmount < calculateBestGatewayAmount();
}