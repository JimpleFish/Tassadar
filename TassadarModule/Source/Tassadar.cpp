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

	if(Broodwar->self()->getRace() != Races::Protoss)
		Broodwar << "I wanted to be Protoss, but all I got was crappy " << Broodwar->self()->getRace() << std::endl;
	
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
	Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS() );

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
			// if our worker is idle
			if ( u->isIdle() )
			{
				// Order workers carrying a resource to return them to the center,
				// otherwise find a mineral patch to harvest.
				if ( u->isCarryingGas() || u->isCarryingMinerals() )
				{
					u->returnCargo();
				}
				else if ( !u->getPowerUp() )  // The worker cannot harvest anything if it
				{                             // is carrying a powerup such as a flag
					// Harvest from the nearest mineral patch or gas refinery
					if ( !u->gather( u->getClosestUnit( IsMineralField || IsRefinery )) )
					{
						// If the call fails, then print the last error message
						Broodwar << Broodwar->getLastError() << std::endl;
					}

				} // closure: has no powerup
			} // closure: if idle

		}
		else if ( u->getType().isResourceDepot() ) // A resource depot is a Command Center, Nexus, or Hatchery
		{
			// Retrieve the supply provider type in the case that we have run out of supplies
			UnitType supplyProviderType = u->getType().getRace().getSupplyProvider();
			static int lastChecked = 0;

			// If we're nearly out of supply
			if ( Broodwar->self()->supplyTotal()/2 - Broodwar->self()->supplyUsed()/2 <= 2 &&
				lastChecked + 600 < Broodwar->getFrameCount() )
			{
				if(Broodwar->self()->minerals() >= 120)
				{
					Broodwar->sendText("Building pylon"); 
					Broodwar->sendText("My supply is %d/%d", Broodwar->self()->supplyUsed()/2, Broodwar->self()->supplyTotal()/2);

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
							supplyBuilder->build( supplyProviderType, targetBuildLocation );							
							lastChecked = Broodwar->getFrameCount();
						}
					} // closure: supplyBuilder is valid
				}
			} // closure: insufficient supply
			// Order the depot to construct more workers! But only when it is idle.
			else if ( u->isIdle() && !u->train(u->getType().getRace().getWorker()) )
			{
				// If that fails, draw the error at the location so that you can visibly see what went wrong!
				// However, drawing the error once will only appear for a single frame
				// so create an event that keeps it on the screen for some frames
				Position pos = u->getPosition();
				Error lastErr = Broodwar->getLastError();
				Broodwar->registerEvent([pos,lastErr](Game*){ Broodwar->drawTextMap(pos, "%c%s", Text::White, lastErr.c_str()); },   // action
					nullptr,    // condition
					Broodwar->getLatencyFrames());  // frames to run
			}// closure: failed to train idle unit
		}
	} // closure: unit iterator
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
