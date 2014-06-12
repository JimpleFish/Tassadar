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
		void TickWorker(BWAPI::Unit &proleteriat);
};

