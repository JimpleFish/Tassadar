#pragma once
#include "BWAPI.h"

using namespace BWAPI;

class BuildingManager
{
public:
	BuildingManager(void);
	~BuildingManager(void);
	int calculateBestGatewayAmount();
	bool shouldBuildGateways();

public:
	int m_gatewayAmount;
	Unitset nexii;
	bool haveCyberCore;
};