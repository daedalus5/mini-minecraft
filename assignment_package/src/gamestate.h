#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "openglcontext.h"
#include "utils.h"
#include "shaderprogram.h"
#include "scene/worldaxes.h"
#include "camera.h"
#include "scene/terrain.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QThreadPool>
#include <QRunnable>
#include <QMutex>
#include "player.h"
#include "scene/quad.h"
#include "mygl.h"
#include "scene/scheduler.h"

// Forward declaration
class MyGL;
class Scheduler;

class GameState
{
public:
    GameState(OpenGLContext* in_context);
    virtual ~GameState();

    virtual void update() = 0;
    virtual void resizeWindow(int w, int h) = 0;
    virtual void paint() = 0;
    virtual void keyPress(QKeyEvent *e);
    virtual void keyRelease(QKeyEvent *r);
    virtual void mouseMove(QMouseEvent *m);
    virtual void mouseRelease(QMouseEvent *mr);
    virtual void mousePress(QMouseEvent *e);

protected:
    OpenGLContext* context;
};

struct ray{
    glm::vec3 orig;
    glm::vec3 dir;
};

class PlayState : public GameState {
public:
    PlayState(OpenGLContext* in_context);
    ~PlayState();
    void update() override;
    void resizeWindow(int w, int h) override;
    void paint() override;
    void keyPress(QKeyEvent *e) override;
    void keyRelease(QKeyEvent *r) override;
    void mouseMove(QMouseEvent *m) override;
    void mouseRelease(QMouseEvent *mr) override;
    void mousePress(QMouseEvent *e) override;

private:
    WorldAxes* mp_worldAxes; // A wireframe representation of the world axes. It is hard-coded to sit centered at (0, 128, 0).
    ShaderProgram* mp_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram* mp_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    ShaderProgram* mp_lavavision; // Red overlay for under Lava

    Camera* mp_camera;
    Terrain* mp_terrain;
    CrossHairs* mp_crosshairs;
    Player* mp_player; // Instance of Player

    bool underwater;
    bool underlava;

    quint64 time; // this holds the milliseconds value of Time since Epoch, i.e. time since January 1, 1970
    quint64 dt; // time elapsed since last timerUpdate()
    const quint64 start_time; // time when the game started in milliseconds since Epoch

    void destroyBlock();    // destroys a block within a unit distance of the player
    void createBlock();     // creates a block adjacent to the face of a block the player is pointing at
    float rayBoxIntersect(const glm::ivec3 cubeMin, const ray r) const;   // tests for intersection between a box and a ray
                                                                          // returns t_near, -1 if no intersection
                                                                          // Kay and Kayjia algorithm
    void GLDrawScene();

    glm::vec4 skyColor;
    QMutex mutex;
    Scheduler* scheduler;
    Quad* mp_quad;
};

class MenuState : public GameState {
public:
    MenuState(MyGL* in_mygl);
    ~MenuState();
    void update() override;
    void resizeWindow(int w, int h) override;
    void paint() override;
    void keyPress(QKeyEvent *e) override;
    void keyRelease(QKeyEvent *r) override;

private:
    MyGL* mygl;
    ShaderProgram* mp_progMenu;
    Quad* mp_quad;
};

#endif // GAMESTATE_H
