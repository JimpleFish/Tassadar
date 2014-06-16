#include "Tassadar.h"
#include <iostream>

using namespace BWAPI;
using namespace Filter;

void Tassadar::onStart()
{
	// Send text sends it to the other players
	Broodwar->sendText("En Taro Tassadar!");

	// This writes it to the in-game console
	Broodwar << "The map is " << Broodwar->mapName() << "!" << std::endl;
	

	// Enable the UserInput flag, which allows us to control the bot and type messages.
	Broodwar->enableFlag(Flag::UserInput);

	// Set the command optimization level so that common commands can be grouped

	// and reduce the bot's APM (Actions Per Minute).
	Broodwar->setCommandOptimizationLevel(2);
	
	// Retrieve you and your enemy's races. enemy() will just return the first enemy.
	// If you wish to deal with multiple enemies then you must use enemies().
	if ( Broodwar->enemy() ) // First make sure there is an enemy
		Broodwar << "The matchup is " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << std::endl;

	banker.SetReserved(Broodwar->self()->getRace().getCenter().mineralPrice() + 4*Broodwar->self()->getRace().getWorker().mineralPrice());
}

void Tassadar::onEnd(bool isWinner)
{
	// Called when the game ends
	if ( isWinner )
	{
		// Log your win here!
	}
}

// Called once every game frame
void Tassadar::onFrame()
{
	// Display the game frame rate as text in the upper left area of the screen
	Broodwar->drawTextScreen(200, 0,  "FPS: %d", Broodwar->getFPS() );
	//Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS() );
	Broodwar->drawTextScreen(200, 20, "Available Mins: %d", banker.AvailableMinerals());
	Broodwar->drawTextScreen(200, 40, "Supply: %d / %d", Broodwar->self()->supplyUsed()/2, Broodwar->self()->supplyTotal()/2);
	Broodwar->drawTextScreen(200, 60, "%d%%", (100 * Broodwar->self()->supplyUsed()/2) / (Broodwar->self()->supplyTotal()/2));
	

	// Return if the game is a replay or is paused
	if ( Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self() )
		return;

	// Prevent spamming by only running our onFrame once every number of latency frames.
	// Latency frames are the number of frames before commands are processed.
	if ( Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0 )
		return;

	// Iterate through all the units that we own
	Unitset myUnits = Broodwar->self()->getUnits();
	for ( Unitset::iterator u = myUnits.begin(); u != myUnits.end(); ++u )
	{
		// Ignore the unit if it no longer exists
		// Make sure to include this block when handling any Unit pointer!
		if ( !u->exists() )
			continue;

		// Ignore the unit if it has one of the following status ailments
		if ( u->isLockedDown() || u->isMaelstrommed() || u->isStasised() )
			continue;

		// Ignore the unit if it is in one of the following states
		if ( u->isLoaded() || !u->isPowered() || u->isStuck() )
			continue;

		// Ignore the unit if it is incomplete or busy constructing
		if ( !u->isCompleted() || u->isConstructing() )
			continue;


		// Finally make the unit do some stuff!


		// If the unit is a worker unit
		if ( u->getType().isWorker() )
		{
			bossMan.TickWorker(*u);
		}
		else if ( u->getType().isResourceDepot() ) // A resource depot is a Command Center, Nexus, or Hatchery
		{
			// Retrieve the supply provider type in the case that we have run out of supplies
			UnitType supplyProviderType = u->getType().getRace().getSupplyProvider();
			static int lastChecked = 0;

			// If we're nearly out of supply
			if ( (100 * Broodwar->self()->supplyUsed()/2) / (Broodwar->self()->supplyTotal()/2)  >= 80 &&
				lastChecked + supplyProviderType.buildTime() + 200 < Broodwar->getFrameCount() )
			{
				if(banker.AvailableMinerals() >= 100)
				{
					Broodwar->sendText("Building pylon"); 
					Broodwar->sendText("My supply usage is at %d%%", 100 * (Broodwar->self()->supplyUsed()/2) / (Broodwar->self()->supplyTotal()/2));

					// Retrieve a unit that is capable of constructing the supply needed
					Unit supplyBuilder = u->getClosestUnit(  GetType == supplyProviderType.whatBuilds().first &&
						(IsIdle || IsGatheringMinerals) &&
						IsOwned);
					// If a unit was found
					if ( supplyBuilder )
					{
						TilePosition targetBuildLocation = Broodwar->getBuildLocation(supplyProviderType, supplyBuilder->getTilePosition());
						if ( targetBuildLocation )
						{
							// Order the builder to construct the supply structure
							banker.RequestMinerals(supplyProviderType.mineralPrice());
							if(!supplyBuilder->build( supplyProviderType, targetBuildLocation ))
								banker.ReturnMinerals(supplyProviderType.mineralPrice());
							lastChecked = Broodwar->getFrameCount();
						}
					} // closure: supplyBuilder is valid
				}
			} // closure: insufficient supply
			// Order the depot to construct more workers! But only when it is idle.
			else if ( u->isIdle() && banker.AvailableMinerals() >= UnitTypes::Protoss_Probe.mineralPrice() )
			{
				if(banker.RequestMinerals(UnitTypes::Protoss_Probe.mineralPrice()))
				{
					if(!u->train(UnitTypes::Protoss_Probe))
					{
						banker.ReturnMinerals(UnitTypes::Protoss_Probe.mineralPrice());
						Position pos = u->getPosition();
						Error lastErr = Broodwar->getLastError();
						Broodwar->registerEvent([pos,lastErr](Game*){ Broodwar->drawTextMap(pos, "%c%s", Text::White, lastErr.c_str()); },   // action
							nullptr,    // condition
							Broodwar->getLatencyFrames());  // frames to run
					}
				}
			}

			if(banker.AvailableMinerals() >= UnitTypes::Protoss_Gateway.mineralPrice())
			{
				Unit builder = u->getClosestUnit(GetType == UnitTypes::Protoss_Probe &&
				(IsIdle || IsGatheringMinerals) &&
				IsOwned);
				// If a unit was found
				if ( builder )
				{
					TilePosition targetBuildLocation = Broodwar->getBuildLocation(UnitTypes::Protoss_Gateway, builder->getTilePosition());
					if ( targetBuildLocation )
					{
						// Order the builder to construct the structure
						if(banker.RequestMinerals(UnitTypes::Protoss_Gateway.mineralPrice()))
							builder->build(UnitTypes::Protoss_Gateway, targetBuildLocation);
					}
				}
			}
		}
		else if(u->getType() == UnitTypes::Protoss_Gateway)
		{
			if(u->isIdle() && banker.AvailableMinerals() >= UnitTypes::Protoss_Zealot.mineralPrice())
			{
				if(u->train(UnitTypes::Protoss_Zealot))
					banker.RequestMinerals(UnitTypes::Protoss_Zealot.mineralPrice());
			}
		}
	}
}

void Tassadar::onSendText(std::string text)
{
	// Send the text to the game if it is not being processed.
	Broodwar->sendText("%s", text.c_str());


	// Make sure to use %s and pass the text as a parameter,
	// otherwise you may run into problems when you use the %(percent) character!
}

void Tassadar::onReceiveText(BWAPI::Player player, std::string text)
{
}

void Tassadar::onPlayerLeft(BWAPI::Player player)
{
	// Interact verbally with the other players in the game by
	// announcing that the other player has left.
	Broodwar->sendText("Goodbye %s!", player->getName().c_str());
}

void Tassadar::onNukeDetect(BWAPI::Position target)
{

	// Check if the target is a valid position
	if ( target )
	{
		// if so, print the location of the nuclear strike target
		Broodwar << "Nuclear Launch Detected at " << target << std::endl;
	}
	else 
	{
		// Otherwise, ask other players where the nuke is!
		Broodwar->sendText("Where's the nuke?");
	}

	// You can also retrieve all the nuclear missile targets using Broodwar->getNukeDots()!
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

void Tassadar::onUnitCreate(BWAPI::Unit unit)
{
	if(unit->getPlayer() == Broodwar->self())
	{
		//Broodwar << "Available b4 unit: " << banker.AvailableMinerals() << std::endl;
		banker.ReturnMinerals(unit->getType().mineralPrice());
		//Broodwar << "Available after unit: " << banker.AvailableMinerals() << std::endl;
		Broodwar->sendText("I have built a %s", unit->getType().c_str());
	}
	if(unit->getType().isWorker())
	{
		bossMan.WorkerCreated(unit);
	}
}

void Tassadar::onUnitDestroy(BWAPI::Unit unit)
{
}

void Tassadar::onUnitMorph(BWAPI::Unit unit)
{
}

void Tassadar::onUnitRenegade(BWAPI::Unit unit)
{
}

void Tassadar::onSaveGame(std::string gameName)
{
	Broodwar << "The game was saved to \"" << gameName << "\"" << std::endl;
}

void Tassadar::onUnitComplete(BWAPI::Unit unit)
{
}
