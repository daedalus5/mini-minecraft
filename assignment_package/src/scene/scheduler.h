#ifndef SCHEDULER_H
#define SCHEDULER_H
#include<QRunnable>
#include<QMutex>
#include<QThreadPool>
#include<scene/terrain.h>
#include<player.h>


class Scheduler:public QRunnable
{
public:
    Scheduler(Terrain*, QMutex*);
    Terrain* ptr_to_terrain;
    QMutex* mutex;

    void run() override;
    void generateTerrain();


};

#endif // SCHEDULER_H
