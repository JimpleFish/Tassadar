#include "Tassadar.h"
#include <iostream>

using namespace BWAPI;
using namespace Filter;

void Tassadar::onStart()
{
	Broodwar->sendText("En Taro Tassadar!");

	Broodwar->enableFlag(Flag::UserInput);

	// Set the command optimization level so that common commands can be grouped
	// and reduce the bot's APM (Actions Per Minute).
	Broodwar->setCommandOptimizationLevel(2);

	Broodwar->setLocalSpeed(15);
	
	banker.SetReserved(Broodwar->self()->getRace().getCenter().mineralPrice() + 4*Broodwar->self()->getRace().getWorker().mineralPrice());
}

void Tassadar::onFrame()
{

	Broodwar->drawTextScreen(200, 0,  "FPS: %d", Broodwar->getFPS() );
	Broodwar->drawTextScreen(200, 20, "Available Mins: %d", banker.AvailableMinerals());
	Broodwar->drawTextScreen(50, 0, "Current Gateways: %d", brickie.m_gatewayAmount);
	Broodwar->drawTextScreen(50, 20, "Max Gateways: %d", brickie.calculateBestGatewayAmount());
	Broodwar->drawTextScreen(300, 0, "Worker size: %d", bossMan.massOfMasses());
	Broodwar->drawTextScreen(300, 20, "Army size: %d", general.armySize());
	
	// Prevent spamming by only running our onFrame once every number of latency frames.
	// Latency frames are the number of frames before commands are processed.
	if ( Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0 )
		return;	

	// Step 1. Pylons!
	UnitType pylon = UnitTypes::Protoss_Pylon;
	static int lastChecked = 0;

	// If we're nearly out of supply and it's been a while since we last checked
	if ( (100 * Broodwar->self()->supplyUsed()/2) / (Broodwar->self()->supplyTotal()/2) >= 80 &&
		lastChecked + pylon.buildTime() + 200 < Broodwar->getFrameCount() )
	{
		bool bSuccess = false;

		if(banker.AvailableMinerals() >= pylon.mineralPrice())
		{
			Unit probe = bossMan.GetMeAWorker(brickie.nexii.front());
			if (probe)
			{
				TilePosition targetBuildLocation = Broodwar->getBuildLocation(pylon, probe->getTilePosition());
				if (targetBuildLocation)
				{
					// Order the builder to construct the supply structure
					if(banker.RequestMinerals(pylon.mineralPrice(), false))
					{
						if(!probe->build(pylon, targetBuildLocation))
						{
							banker.ReturnMinerals(pylon.mineralPrice());
						}
						else
						{
							bSuccess = true;
							lastChecked = Broodwar->getFrameCount();
						}
					}
				}
			}
		}

		// If we fail to build a pylon when we need one, don't spend any more minerals
		//banker.blockForPylon = !bSuccess;
	}

	// Step 2. Probes!	
	for(Unitset::iterator nexus = brickie.nexii.begin(); nexus != brickie.nexii.end(); ++nexus)
	{
		if (nexus->isIdle() && banker.AvailableMinerals() >= UnitTypes::Protoss_Probe.mineralPrice() && !bossMan.MineralsSaturated())
		{
			if(banker.RequestMinerals(UnitTypes::Protoss_Probe.mineralPrice()))
			{
				if(!nexus->train(UnitTypes::Protoss_Probe))
				{
					banker.ReturnMinerals(UnitTypes::Protoss_Probe.mineralPrice());
					Position pos = nexus->getPosition();
					Error lastErr = Broodwar->getLastError();
					Broodwar->registerEvent([pos,lastErr](Game*){ Broodwar->drawTextMap(pos, "%c%s", Text::White, lastErr.c_str()); },   // action
						nullptr,    // condition
						Broodwar->getLatencyFrames());  // frames to run
				}
			}
		}
	}
	

	// Step 3. Spend your shit!
	if(banker.AvailableMinerals() >= UnitTypes::Protoss_Gateway.mineralPrice() && brickie.shouldBuildGateways() )
	{
		Unit builder = bossMan.GetMeAWorker(brickie.nexii.front());

		if ( builder )
		{
			TilePosition targetBuildLocation = Broodwar->getBuildLocation(UnitTypes::Protoss_Gateway, builder->getTilePosition());
			if ( targetBuildLocation )
			{
				// Order the builder to construct the structure
				if(banker.RequestMinerals(UnitTypes::Protoss_Gateway.mineralPrice()))
				{
					if(builder->build(UnitTypes::Protoss_Gateway, targetBuildLocation))
						++brickie.m_gatewayAmount;
					else
						banker.ReturnMinerals(UnitTypes::Protoss_Gateway.mineralPrice());
				}
			}
		}
	}
	if(banker.AvailableMinerals() >= UnitTypes::Protoss_Cybernetics_Core.mineralPrice() && !brickie.haveCyberCore )
	{
		Unit builder = bossMan.GetMeAWorker(brickie.nexii.front());

		if (builder)
		{
			TilePosition targetBuildLocation = Broodwar->getBuildLocation(UnitTypes::Protoss_Cybernetics_Core, builder->getTilePosition());
			if ( targetBuildLocation )
			{
				// Order the builder to construct the structure
				if(banker.RequestMinerals(UnitTypes::Protoss_Cybernetics_Core.mineralPrice()))
				{
					if(!builder->build(UnitTypes::Protoss_Cybernetics_Core, targetBuildLocation))
						banker.ReturnMinerals(UnitTypes::Protoss_Cybernetics_Core.mineralPrice());
				}
			}
		}
	}
	
	// Tell all of our units to do stuff
	bossMan.TickWorkers();

	Unitset myUnits = Broodwar->self()->getUnits();
	for ( Unitset::iterator u = myUnits.begin(); u != myUnits.end(); ++u )
	{
		if ( !u->exists() ||  u->isLockedDown() || u->isMaelstrommed() || u->isStasised() 
			|| u->isLoaded() || !u->isPowered() || u->isStuck() 
			|| !u->isCompleted() || u->isConstructing() )
			continue;

		if(u->getType() == UnitTypes::Protoss_Gateway)
		{
			if(u->isIdle() && banker.AvailableMinerals() >= UnitTypes::Protoss_Dragoon.mineralPrice() &&
				banker.AvailableGas() >= UnitTypes::Protoss_Dragoon.gasPrice())
			{
				if(banker.RequestMinerals(UnitTypes::Protoss_Dragoon.mineralPrice()))
				{
					if(!u->train(UnitTypes::Protoss_Dragoon))
						banker.ReturnMinerals(UnitTypes::Protoss_Dragoon.mineralPrice());
				}
			}
			if(u->isIdle() && banker.AvailableMinerals() >= UnitTypes::Protoss_Zealot.mineralPrice())
			{
				if(banker.RequestMinerals(UnitTypes::Protoss_Zealot.mineralPrice()))
				{
					if(!u->train(UnitTypes::Protoss_Zealot))
						banker.ReturnMinerals(UnitTypes::Protoss_Zealot.mineralPrice());
				}
			}			
		}				
	}

	if(general.armySize() == 10 || general.armySize() == 30 || general.armySize() == 60 || general.armySize() > 100)
	{
		Broodwar->sendText("ATTTAAAAAACCCCKK!!!");
		if (brickie.nexii.front() && brickie.nexii.front()->getTop() < 700)
		{
			// We are in the top half of the map
			general.allOutAttack(Position(1381,5757));
		}
		else
		{
			// We are in the bottom half of the map
			general.allOutAttack(Position(633,472));
		}
	}
}

void Tassadar::onUnitCreate(BWAPI::Unit unit)
{
	if(unit->getPlayer() == Broodwar->self())
	{
		banker.ReturnMinerals(unit->getType().mineralPrice());
		
		using namespace UnitTypes::Enum;
		switch(unit->getType())
		{
		case Protoss_Probe:
			bossMan.WorkerCreated(unit);
			break;
		case Protoss_Nexus:
			brickie.nexii.push_back(unit);
			break;
		case Protoss_Cybernetics_Core:
			brickie.haveCyberCore = true;
			break;		
		}
	}
}

void Tassadar::onUnitComplete(BWAPI::Unit unit)
{
	if(unit->getPlayer() == Broodwar->self())
	{
		using namespace UnitTypes::Enum;
		switch(unit->getType())
		{
			case Protoss_Zealot:
			case Protoss_Dragoon:
				general.warriorCreated(unit);
				break;
		}
	}
}

#pragma region Unused methods
void Tassadar::onEnd(bool isWinner)
{
}

void Tassadar::onSendText(std::string text)
{
	Broodwar->sendText("%s", text.c_str());
}

void Tassadar::onReceiveText(BWAPI::Player player, std::string text)
{
}

void Tassadar::onPlayerLeft(BWAPI::Player player)
{
}

void Tassadar::onNukeDetect(BWAPI::Position target)
{
}

void Tassadar::onUnitDiscover(BWAPI::Unit unit)
{
}

void Tassadar::onUnitEvade(BWAPI::Unit unit)
{
}

void Tassadar::onUnitShow(BWAPI::Unit unit)
{
}

void Tassadar::onUnitHide(BWAPI::Unit unit)
{
}

void Tassadar::onUnitDestroy(BWAPI::Unit unit)
{
	if(unit->getPlayer() == Broodwar->self())
	{
		using namespace UnitTypes::Enum;
		switch(unit->getType())
		{
			case Protoss_Zealot:
			case Protoss_Dragoon:
				general.warriorFallen(unit);
				break;
		}
	}
}

void Tassadar::onUnitMorph(BWAPI::Unit unit)
{
}

void Tassadar::onUnitRenegade(BWAPI::Unit unit)
{
}

void Tassadar::onSaveGame(std::string gameName)
{
}
#pragma endregion 
