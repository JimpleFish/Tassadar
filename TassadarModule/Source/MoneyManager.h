#pragma once
#include "BWAPI.h"

using namespace BWAPI;

class MoneyManager
{
public:
	MoneyManager(void);
	~MoneyManager(void);
	void SetReserved(int mins);
	bool RequestMinerals(int);
	void ReturnMinerals(int);
	int AvailableMinerals();

private:
	int reservedMinerals;
};

