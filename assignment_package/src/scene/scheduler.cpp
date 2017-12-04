#include "scheduler.h"

Scheduler::Scheduler(Terrain* terr, QMutex* ptr_to_mutex): ptr_to_terrain(terr),mutex(ptr_to_mutex)
{

}

void Scheduler::run()
{
    while(true)
    {

        generateTerrain();
    }
}

void Scheduler::generateTerrain()
{
    mutex->lock();
    ptr_to_terrain->drawScene();
    mutex->unlock();


}
