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
	bool RequestMinerals(int, bool);
	void ReturnMinerals(int);
	int AvailableMinerals();
	int AvailableGas();
	bool blockForPylon;

private:
	int reservedMinerals;
};

