#ifndef PLAYER_H
#define PLAYER_H
#include<la.h>
#include<camera.h>
#include <openglcontext.h>
#include<scene/terrain.h>
#include<vector>



class Player
{
private:
    glm::vec3 pos;
    float dt;

    glm::vec3 velocity;
    std::vector<glm::vec3> vertexpositions;
    Camera* ptr_to_cam;
    Terrain* ptr_to_terrain;
    glm::vec3 playerMin;
    glm::vec3 playerMax;


    int mouseX;
    int dx; // tracks change in cursor's x position
    int mouseY;
    int dy;// tracks change in cursor's y position
    bool isLMBpressed;
    bool isRMBpressed;
    bool mouseOrientFlag;


    bool isWpressed;//checks if WASD and Space keys are pressed
    bool isApressed;
    bool isSpressed;
    bool isDpressed;
    bool isSpacepressed;

    bool isZpressed; //remove upon implementing collision detection

    bool aerialState; //Determines if the player is grounded or aerial. If aerial, gravity will pull him down
    Qt::MouseButton mouseButtonPressed;
    bool collision;
    glm::vec3 box1min;
    glm::vec3 box1max;
    glm::vec3 box2min;
    glm::vec3 box2max;








public:
    Player(Camera*, Terrain*);
    glm::vec2 rotation;
    bool controllerState;// true if player is recieving input from the controller
    bool mouseState;

    void updateTime(quint64);// updates player's dt
    void keyPressState(QKeyEvent *e);// Listener function for WASD and Space presses. sets key flags
    void keyReleaseState(QKeyEvent *e);// Listener function for key releases

    void mousePressState(QMouseEvent *m);
    void mouseReleaseState(QMouseEvent*);
    void updateAttributes();// updates position and velocity. Invoked by myGL's timerUpdate().
    void updateCameraOrientation(); // adjusts camera orientation
    void playerGeometry();

    void mouseMoveState(QMouseEvent*);
    bool collisionDetect();
    bool boundingBoxcheck(glm::vec3, glm::vec3, glm::vec3, glm::vec3);
    void gravityCheck();









};

#endif // PLAYER_H