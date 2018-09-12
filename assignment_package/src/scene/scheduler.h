#ifndef SCHEDULER_H
#define SCHEDULER_H
#include<QRunnable>
#include<QMutex>
#include<QThreadPool>
#include<scene/terrain.h>
#include<player.h>


class Scheduler:public QRunnable //This is the class whose instances are used to create threads
{
public:
    Scheduler(Terrain*, QMutex*);
    Terrain* ptr_to_terrain;
    QMutex* mutex;

    void run() override;
    void generateTerrain();
    void setRunMode(bool b);

private:
    bool shouldIRun;

};

#endif // SCHEDULER_H
