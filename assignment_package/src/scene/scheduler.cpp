#include "scheduler.h"

Scheduler::Scheduler(Terrain* terr, QMutex* ptr_to_mutex): ptr_to_terrain(terr),mutex(ptr_to_mutex), shouldIRun(true)
{

}

void Scheduler::run()
{
    while(shouldIRun)
    {

        generateTerrain();//invokes the terrain generation function in the Terrain class
    }

}

void Scheduler::generateTerrain()
{

    ptr_to_terrain->drawScene(); //invokes the terrain generation function in the Terrain class

}

void Scheduler::setRunMode(bool b) {
    shouldIRun = b;
}
