#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>

#include <set>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      mp_geomCube(new Cube(this)), mp_worldAxes(new WorldAxes(this)),
      mp_progLambert(new ShaderProgram(this)), mp_progFlat(new ShaderProgram(this)),
      mp_camera(new Camera()), mp_terrain(new Terrain(this)), mp_crosshairs(new CrossHairs(this))
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    // Tell the timer to redraw 60 times per second
    timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    mp_geomCube->destroy();

    delete mp_geomCube;
    delete mp_worldAxes;
    delete mp_progLambert;
    delete mp_progFlat;
    delete mp_camera;
    delete mp_terrain;
    delete mp_crosshairs;
}


void MyGL::MoveMouseToCenter()
{
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of Cube
    mp_geomCube->create();
    mp_worldAxes->create();

    // Create and set up the diffuse shader
    mp_progLambert->create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    mp_progFlat->create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    // Set a color with which to draw geometry since you won't have one
    // defined until you implement the Node classes.
    // This makes your geometry render green.
    //mp_progLambert->setGeometryColor(glm::vec4(0,1,0,1));

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
//    vao.bind();
    glBindVertexArray(vao);


    //mp_terrain->CreateHighland(-16 * 5, 16 * 5, -16 * 5, 16 * 5);

    //mp_terrain->CreateTestScene();

    //mp_terrain->updateAllVBO();
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    *mp_camera = Camera(w, h, glm::vec3(mp_terrain->dimensions.x, mp_terrain->dimensions.y * 0.75, mp_terrain->dimensions.z),
                       glm::vec3(mp_terrain->dimensions.x / 2, mp_terrain->dimensions.y / 2, mp_terrain->dimensions.z / 2), glm::vec3(0,1,0));
    glm::mat4 viewproj = mp_camera->getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    mp_progLambert->setViewProjMatrix(viewproj);
    mp_progFlat->setViewProjMatrix(viewproj);

    mp_crosshairs->aspect = mp_camera->height / float(mp_camera->width);
    mp_crosshairs->create();

    printGLErrorLog();
}


// MyGL's constructor links timerUpdate() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to use timerUpdate
void MyGL::timerUpdate()
{   
    // Add new chunk if needed?

    update();
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mp_progFlat->setViewProjMatrix(mp_camera->getViewProj());
    mp_progLambert->setViewProjMatrix(mp_camera->getViewProj());

    GLDrawScene();

    glDisable(GL_DEPTH_TEST);
    mp_progFlat->setModelMatrix(glm::mat4());
    mp_progFlat->draw(*mp_worldAxes);

    mp_progFlat->setViewProjMatrix(glm::mat4());
    mp_progFlat->draw(*mp_crosshairs);
    glEnable(GL_DEPTH_TEST);
}

void MyGL::GLDrawScene()
{
    mp_progLambert->setModelMatrix(glm::mat4());
    int chunkX = mp_terrain->getChunkPosition1D(mp_camera->eye[0]);
    int chunkZ = mp_terrain->getChunkPosition1D(mp_camera->eye[2]);

    std::vector<Chunk*> chunks2Draw = std::vector<Chunk*>();
    std::set<uint64_t> chunks2Update = std::set<uint64_t>();

    int num = 5;
    int x, z;

    Chunk* ch;
    for (int i = -num; i < num; i++) {
        for (int k = -num; k < num; k++) {
            x = chunkX + i;
            z = chunkZ + k;
            ch = mp_terrain->getChunk(x, z);
            if (ch != nullptr) {
                chunks2Draw.push_back(ch);
            } else {
                chunks2Draw.push_back(mp_terrain->CreateHighland(x, z));

                chunks2Update.insert(mp_terrain->convertToInt(x, z));

                chunks2Update.insert(mp_terrain->convertToInt(x + 1, z));
                chunks2Update.insert(mp_terrain->convertToInt(x - 1, z));
                chunks2Update.insert(mp_terrain->convertToInt(x, z + 1));
                chunks2Update.insert(mp_terrain->convertToInt(x, z - 1));
            }
        }
    }

    int tempX, tempZ;
    for (uint64_t i : chunks2Update) {
        mp_terrain->splitInt(i, &tempX, &tempZ);
        mp_terrain->updateChunkVBO(tempX, tempZ);
    }

    for (Chunk* ch : chunks2Draw) {
        mp_progLambert->draw(*ch);
    }

}


void MyGL::keyPressEvent(QKeyEvent *e)
{

    float amount = 1.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    if (e->key() == Qt::Key_Escape) {
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        mp_camera->RotateAboutUp(-amount);
    } else if (e->key() == Qt::Key_Left) {
        mp_camera->RotateAboutUp(amount);
    } else if (e->key() == Qt::Key_Up) {
        mp_camera->RotateAboutRight(-amount);
    } else if (e->key() == Qt::Key_Down) {
        mp_camera->RotateAboutRight(amount);
    } else if (e->key() == Qt::Key_1) {
        mp_camera->fovy += amount;
    } else if (e->key() == Qt::Key_2) {
        mp_camera->fovy -= amount;
    } else if (e->key() == Qt::Key_W) {
        mp_camera->TranslateAlongLook(amount);
    } else if (e->key() == Qt::Key_S) {
        mp_camera->TranslateAlongLook(-amount);
    } else if (e->key() == Qt::Key_D) {
        mp_camera->TranslateAlongRight(amount);
    } else if (e->key() == Qt::Key_A) {
        mp_camera->TranslateAlongRight(-amount);
    } else if (e->key() == Qt::Key_Q) {
        mp_camera->TranslateAlongUp(-amount);
    } else if (e->key() == Qt::Key_E) {
        mp_camera->TranslateAlongUp(amount);
    } else if (e->key() == Qt::Key_R) {
        *mp_camera = Camera(this->width(), this->height());
    }
    mp_camera->RecomputeAttributes();
}

void MyGL::mousePressEvent(QMouseEvent *e){
    if (e->buttons() == Qt::LeftButton){
        destroyBlock();
    } else if (e->buttons() == Qt::RightButton){
        createBlock();
    }
}

void MyGL::destroyBlock(){
    glm::vec3 eye = mp_camera->eye;
    glm::vec3 forward = mp_camera->look;

    ray look;
    look.orig = eye;
    look.dir = forward;

    // cube at eye position
    glm::ivec3 eyeCube = glm::ivec3(floor(eye[0]), floor(eye[1]), floor(eye[2]));

    // store all cubes look ray intersects
    QMap<float, glm::ivec3> intersections;
    // going to want to delete the cube that's nearest and intersects our look vector
    float t_nearest = 1E6;

    // consider surrounding 26 cubes
    for(int i = -1; i < 2; ++i){
        for(int j = -1; j < 2; ++j){
            for(int k = -1; k < 2; ++k){
                if(i == 0 && j == 0 && k == 0){ // don't consider cube at eye position
                    continue;
                }
                glm::ivec3 cube = glm::ivec3(eyeCube[0] + i, eyeCube[1] + j, eyeCube[2] + k);

                // make sure cube is within the bounds of the terrain
//                if (cube[0] < 0 || cube[0] > mp_terrain->dimensions.x){
//                    return;
//                }
//                if (cube[1] < 0 || cube[1] > mp_terrain->dimensions.y){
//                    return;
//                }
//                if (cube[2] < 0 || cube[2] > mp_terrain->dimensions.z){
//                    return;
//                }

                float t_near = rayBoxIntersect(cube, look);
                BlockType b = mp_terrain->getBlockAt(cube[0], cube[1], cube[2]);
                if (t_near > 0.0f && b != EMPTY){
                    intersections[t_near] = cube;
                    if(t_near < t_nearest){
                        t_nearest = t_near;
                    }
                }
            }
        }
    }
    if (intersections.isEmpty() == false){
        glm::ivec3 closestCube = intersections.value(t_nearest);
        // destroys closestCube by setting to empty
        mp_terrain->addBlockAt(closestCube[0], closestCube[1], closestCube[2], EMPTY);
    }
}

float MyGL::rayBoxIntersect(const glm::ivec3 cubeMin, const ray r) const{
    float t_near = -1E6;
    float t_far = 1E6;
    glm::ivec3 cubeMax = glm::ivec3(cubeMin[0] + 1, cubeMin[1] + 1, cubeMin[2] + 1);

    for(int i = 0; i < 3; ++i){
        int xl = cubeMin[i];
        int xr = cubeMax[i];
        float xd = r.dir[i];
        float xo = r.orig[i];
        float t1 = (xl - xo) / xd;
        float t2 = (xr - xo) / xd;

        if (t1 > t2){       // swap
            const float t3 = t2;
            t2 = t1;
            t1 = t3;
        }
        if (t1 > t_near){
            t_near = t1;    // want largest t_near
        }
        if (t2 < t_far){
            t_far = t2;     // want smallest t_far
        }
        if (t_near > t_far){
            return -1.0f;
        }
    }
    return t_near;
}

void MyGL::createBlock(){
    glm::vec3 eye = mp_camera->eye;
    glm::vec3 forward = mp_camera->look;

    ray look;
    look.orig = eye;
    look.dir = forward;

    // get cube two forward vectors away
    glm::ivec3 lookCube = glm::ivec3(floor(eye[0] + 2*look.dir[0]), floor(eye[1] + 2*look.dir[1]), floor(eye[2] + 2*look.dir[2]));
    BlockType b = mp_terrain->getBlockAt(lookCube[0], lookCube[1], lookCube[2]);

    // if there is an existing cube in place, we can build off it
    if (b != EMPTY){
        float t_near = rayBoxIntersect(lookCube, look);
        // get the point on the surface of the cube we're looking at in world coordinates
        glm::vec3 pos = look.orig + t_near * look.dir;
        glm::ivec3 insertPos;

        // considering the difference between the cube's "origin"
        // and the point we're considering will tell us which face of the cube to build off
        glm::vec3 diff = pos - glm::vec3(lookCube);

        if (diff[0] < 1E-6 && diff[0] > -1E-6){ // -x face
            insertPos = glm::ivec3(lookCube[0] - 1, lookCube[1], lookCube[2]);
        }
        else if (diff[0] < 1.0f + 1E-6 && diff[0] > 1.0f - 1E-6){ // +x face
            insertPos = glm::ivec3(lookCube[0] + 1, lookCube[1], lookCube[2]);
        }
        if (diff[1] < 1E-6 && diff[1] > -1E-6){ // -y face
            insertPos = glm::ivec3(lookCube[0], lookCube[1] - 1, lookCube[2]);
        }
        else if (diff[1] < 1.0f + 1E-6 && diff[1] > 1.0f - 1E-6){ // +y face
            insertPos = glm::ivec3(lookCube[0], lookCube[1] + 1, lookCube[2]);
        }
        if (diff[2] < 1E-6 && diff[2] > -1E-6){ // -z face
            insertPos = glm::ivec3(lookCube[0], lookCube[1], lookCube[2] - 1);
        }
        else if (diff[2] < 1.0f + 1E-6 && diff[2] > 1.0f - 1E-6){ // +z face
            insertPos = glm::ivec3(lookCube[0], lookCube[1], lookCube[2] + 1);
        }

        BlockType b2 = mp_terrain->getBlockAt(insertPos[0], insertPos[1], insertPos[2]);
        // only build cube if there's an open space to place it
        if (b2 == EMPTY){
            mp_terrain->addBlockAt(insertPos[0], insertPos[1], insertPos[2], LAVA);
        }
    }
}
