#pragma once
#include <BWAPI.h>

using namespace BWAPI;
using namespace Filter;

class WorkerManager
{
	public:
		WorkerManager(void);
		~WorkerManager(void);
		void WorkerCreated(BWAPI::Unit &proleteriat);
		void TickWorkers();
		int massOfMasses();
		Unit GetMeAWorker(Unit nexus);
		bool MineralsSaturated();

	private:
		Unitset unwashedMasses;
};

