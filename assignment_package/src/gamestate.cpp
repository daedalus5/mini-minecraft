#include "gamestate.h"
#include <QDateTime>
#include <QKeyEvent>

#include <set>

GameState::GameState(OpenGLContext* in_context)
    : context(in_context) {

}

GameState::~GameState() {

}

void GameState::keyPress(QKeyEvent *e) {
    // default no-op
}

void GameState::keyRelease(QKeyEvent *r) {
    // default no-op
}

void GameState::mouseMove(QMouseEvent *m) {
    // default no-op
}

void GameState::mouseRelease(QMouseEvent *mr) {
    // default no-op
}

void GameState::mousePress(QMouseEvent *e) {
    // default no-op
}

PlayState::PlayState(OpenGLContext* in_context)
    : GameState(in_context),
      mp_worldAxes(new WorldAxes(in_context)),
      mp_progLambert(new ShaderProgram(in_context)), mp_progFlat(new ShaderProgram(in_context)),
      mp_camera(new Camera()), mp_terrain(new Terrain(in_context, mp_camera, &mutex)), mp_crosshairs(new CrossHairs(in_context)),
      mp_player(new Player(mp_camera, mp_terrain)), underwater(false), underlava(false), time(0), dt(0),
      start_time(QDateTime::currentMSecsSinceEpoch()),
      skyColor(glm::vec4(0.37f, 0.74f, 1.0f, 1)),
      scheduler(new Scheduler(mp_terrain, &mutex))
{

    //Create the instance of Cube
    //mp_geomCube->create();
    mp_worldAxes->create();

    // Create and set up the diffuse shader
    mp_progLambert->create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    mp_progLambert->setTexture(":/textures/minecraft_textures_all.png");
    // Create and set up the flat lighting shader
    mp_progFlat->create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    // mp_lavavision->create(":/glsl/lavavision.vert.glsl", ":/glsl/lavavision.frag.glsl");

    mp_terrain->setTerrainType(new Highland);
    mp_terrain->createRivers();

    //mp_terrain->updateAllVBO();

    mp_camera->eye = glm::vec3(mp_terrain->dimensions.x-10.f, mp_terrain->dimensions.y * 0.60, mp_terrain->dimensions.z-10.f);
    resizeWindow(context->width(), context->height());
    //mp_player = new Player(mp_camera, mp_terrain);

    QThreadPool::globalInstance()->start(scheduler);
    mp_player->keeptime = 5.f;

    context->glClearColor(skyColor.r,skyColor.g,skyColor.b,skyColor.a);
}

PlayState::~PlayState() {
    mp_worldAxes->destroy();
    mp_crosshairs->destroy();

    delete mp_player;
    delete mp_worldAxes;
    delete mp_progLambert;
    delete mp_progFlat;
    delete scheduler;
    delete mp_camera;
    delete mp_terrain;
    delete mp_crosshairs;
}

void PlayState::keyPress(QKeyEvent *e) {
    mp_player->keyPressState(e);
}

void PlayState::keyRelease(QKeyEvent *r) {
    mp_player->keyReleaseState(r);
}

void PlayState::mouseMove(QMouseEvent *m) {
    mp_player->mouseMoveState(m);
}

void PlayState::mouseRelease(QMouseEvent *mr) {
    mp_player->mouseReleaseState(mr);
}

void PlayState::mousePress(QMouseEvent *e) {
    if (e->buttons() == Qt::LeftButton){
        destroyBlock();
    } else if (e->buttons() == Qt::RightButton){
        createBlock();
    }
    mp_player->mousePressState(e);
}

void PlayState::update() {
    // Initialize time if it hasn't been initialized yet
    // Cannot be done in constructor because the time passed between
    // constructor and first update could be so much
    // that player falls through ground
    if (time == 0) {
        time = QDateTime::currentMSecsSinceEpoch();
    }
    //obtains number of milliseconds elapsed since January 1, 1970
    dt = QDateTime::currentMSecsSinceEpoch() - time; //calculates dt, the change in time since the last timerUpdate
    if(!mp_player->isSandbox)
    {
        mp_player->gravityCheck();
    }

    if(mp_player->controllerState == true || mp_player->mouseState==true) // reads if the player is recieving input from the controller, then proceeds to pass it dt and cause it to change
                                           // its attributes like position, velocity, etc.
    {

        mp_player->updateTime(dt); //updates player's valuue of dt, converts to seconds
        mp_player->updateCameraOrientation();
        mp_player->updateAttributes(); // updates player/camera position, velocity, etc
        mp_player->playerGeometry(); // updates player bounding box and limits

    }
    BlockType b1 = mp_player->checkSubmerged();
    if(b1==LAVA)
    {
        underwater = false;
        underlava = true;
    }
    else if(b1==WATER)
    {
        underlava = false;
        underwater = true;

    }
    else
    {
        underlava= false;
        underwater = false;
    }

    time = QDateTime::currentMSecsSinceEpoch();
    for(int i=0;i<mp_terrain->chunksGonnaDraw.size();i++)
    {
        mp_terrain->chunk_map[mp_terrain->keysGonnaDraw[i]] = mp_terrain->chunksGonnaDraw[i];

    }
    mp_terrain->keysGonnaDraw.clear();
    mp_terrain->chunksGonnaDraw.clear();
}

void PlayState::resizeWindow(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    glm::vec3 prev_eye = mp_camera->eye;
    *mp_camera = Camera(w, h, prev_eye,
                       glm::vec3(mp_terrain->dimensions.x / 2, mp_terrain->dimensions.y*0.60, mp_terrain->dimensions.z / 2), glm::vec3(0,1,0));
    glm::mat4 viewproj = mp_camera->getViewProj();
    //mp_player = new Player(mp_camera, mp_terrain);
    mp_player->playerGeometry();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    mp_progLambert->setViewProjMatrix(viewproj);
    mp_progFlat->setViewProjMatrix(viewproj);
    mp_progLambert->setEyePos(glm::vec4(mp_camera->eye, 1.f));

    mp_crosshairs->aspect = mp_camera->height / float(mp_camera->width);
    mp_crosshairs->create();
}

void PlayState::paint() {
    mp_progFlat->setViewProjMatrix(mp_camera->getViewProj());
    mp_progLambert->setViewProjMatrix(mp_camera->getViewProj());
    mp_progLambert->setEyePos(glm::vec4(mp_camera->eye, 1.f));
    mp_progLambert->setTime((time - start_time) /1000.f); // convert time to seconds

    GLDrawScene();

    glDisable(GL_DEPTH_TEST);
    mp_progFlat->setModelMatrix(glm::mat4());
    //mp_progFlat->draw(*mp_worldAxes);

    mp_progFlat->setViewProjMatrix(glm::mat4());
    mp_progFlat->draw(*mp_crosshairs);
    glEnable(GL_DEPTH_TEST);
    if(underlava==true) // change sky color if player is under lava
    {
        glm::vec4 lavaColor= glm::vec4(1.0,0.60,0.5,0.3);
        skyColor = glm::mix(lavaColor,skyColor,0.3);
        context->glClearColor(skyColor.r,skyColor.g,skyColor.b,skyColor.a);
        glm::vec2 r = glm::vec2(0.0,1.0);
        mp_progLambert->setSubmerged(r);
    }

    if(underwater==true) // change sky color if player is underwater
    {
        glm::vec4 waterColor = glm::vec4(0.20,0.50,1.0,0.3);
        skyColor = glm::mix(waterColor,skyColor,0.3);
        skyColor.a = 1.0;
        context->glClearColor(skyColor.r,skyColor.g,skyColor.b,skyColor.a);
        glm::vec2 r = glm::vec2(1.0,0.0);
        mp_progLambert->setSubmerged(r);
    }
    if((underlava==false)&&(underwater==false)) // disable sky color change if player is not submerged
    {
        skyColor = glm::vec4(0.37f, 0.74f, 1.0f, 1);

        context->glClearColor(skyColor.r,skyColor.g,skyColor.b,skyColor.a);
        glm::vec2 r = glm::vec2(0.0,0.0);
        mp_progLambert->setSubmerged(r);


    }
}

void PlayState::GLDrawScene()
{
    mp_progLambert->setModelMatrix(glm::mat4());

    int chunkX = mp_terrain->getChunkPosition1D(mp_camera->eye[0]);
    int chunkZ = mp_terrain->getChunkPosition1D(mp_camera->eye[2]);

    int num = 10;
    int x, z;

    Chunk* ch;

    for (int i = -num; i < num; i++) {
        for (int k = -num; k < num; k++) {
            x = chunkX + i;
            z = chunkZ + k;
            ch = mp_terrain->getChunk(x, z);
            if (ch != nullptr) {
                // If Chunk data has not been passed to the GPU
                // But it has VBO data ready to be passed
                // Then pass the data
                if (!ch->isCreated && ch->hasData) {
                    ch->createVBO();
                }

                // If Chunk has VBO data in GPU,
                // draw it
                if (ch->isCreated) {
                    mp_progLambert->draw(*ch);
                }

            }
        }
    }

}

void PlayState::destroyBlock(){
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
        for(int j = -2; j < 2; ++j){
            for(int k = -1; k < 2; ++k){
                if(i == 0 && j == 0 && k == 0){ // don't consider cube at eye position
                    continue;
                }
                glm::ivec3 cube = glm::ivec3(eyeCube[0] + i, eyeCube[1] + j, eyeCube[2] + k);

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

float PlayState::rayBoxIntersect(const glm::ivec3 cubeMin, const ray r) const{
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

void PlayState::createBlock(){
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


MenuState::MenuState(MyGL* in_mygl)
    : GameState(in_mygl), mygl(in_mygl),
      mp_quad(new Quad(in_mygl)),
      mp_progMenu(new ShaderProgram(in_mygl))
{
    mp_progMenu->create(":/glsl/menu.vert.glsl", ":/glsl/menu.frag.glsl");
    mp_progMenu->setTexture(":/textures/title.png");
    mp_quad->create();
}

MenuState::~MenuState() {
   delete mp_quad;
}

void MenuState::update() {

}

void MenuState::resizeWindow(int w, int h) {

}

void MenuState::paint() {
    mp_progMenu->draw(*mp_quad);
}

void MenuState::keyPress(QKeyEvent *e) {
    mygl->set2PlayState();
}

void MenuState::keyRelease(QKeyEvent *r) {

}

