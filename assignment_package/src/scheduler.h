#ifndef SCHEDULER_H
#define SCHEDULER_H
#include<QThreadPool>
#include<QRunnable>
#include<QMutex>
#include<mygl.h>


class Scheduler : public QRunnable
{
public:
    OpenGLContext* context;

    Scheduler(OpenGLContext*);
    void run() override;
    void generateTerrain();


};

#endif // SCHEDULER_H
