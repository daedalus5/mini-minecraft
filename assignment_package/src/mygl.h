#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include <scene/cube.h>
#include <scene/worldaxes.h>
#include "camera.h"
#include <scene/terrain.h>

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include<player.h>
#include<QDateTime>
#include<QThreadPool>
#include<QRunnable>
#include<QMutex>
#include<scene/quad.h>
#include<scene/scheduler.h>
#include<QSound>
#include<QMediaPlayer>



class Scheduler;


struct ray{
    glm::vec3 orig;
    glm::vec3 dir;
};

class MyGL : public OpenGLContext
{
    Q_OBJECT
private:
    Cube* mp_geomCube;// The instance of a unit cube we can use to render any cube. Should NOT be used in final version of your project.
    WorldAxes* mp_worldAxes; // A wireframe representation of the world axes. It is hard-coded to sit centered at (32, 128, 32).
    ShaderProgram* mp_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram* mp_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    ShaderProgram* mp_lavavision; // Red overlay for under Lava



    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    Camera* mp_camera;
    Terrain* mp_terrain;
    CrossHairs* mp_crosshairs;
    Player* mp_player; // Instance of Player
    bool underwater;
    bool underlava;
    bool underground;

    quint64 time; // this holds the milliseconds value of Time since Epoch, i.e. time since January 1, 1970
    quint64 dt; // time elapsed since last timerUpdate()
    const quint64 start_time; // time when the game started in milliseconds since Epoch

    bool isSandbox;
    Quad* m_geomQuad;
    glm::vec4 skyColor;
    QMutex mutex;
    Scheduler* scheduler;
     /// Timer linked to timerUpdate(). Fires approx. 60 times per second
    QTimer timer;
    QMediaPlayer* music;
    QMediaPlayer* water;
    bool musicflag;


    void MoveMouseToCenter(); // Forces the mouse position to the screen's center. You should call this
                              // from within a mouse move event after reading the mouse movement so that
                              // your mouse stays within the screen bounds and is always read.

public:
    explicit MyGL(QWidget *parent = 0);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void GLDrawScene();


protected:
    void keyPressEvent(QKeyEvent *e);

    //void mousePressEvent(QMouseEvent *e);

    void keyReleaseEvent(QKeyEvent *r);
    void mouseMoveEvent(QMouseEvent *m);
    void mousePressEvent(QMouseEvent *mp);
    void mouseReleaseEvent(QMouseEvent *event);


    void destroyBlock();    // destroys a block within a unit distance of the player
    void createBlock();     // creates a block adjacent to the face of a block the player is pointing at
    float rayBoxIntersect(const glm::ivec3 cubeMin, const ray r) const;   // tests for intersection between a box and a ray
                                                                          // returns t_near, -1 if no intersection
                                                                          // Kay and Kayjia algorithm
    void musicCheck();
    void musicStop();
private slots:
    /// Slot that gets called ~60 times per second
    void timerUpdate();

};


#endif // MYGL_H
