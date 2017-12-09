#include "gamestate.h"
#include <QDateTime>
#include <QKeyEvent>

#include <set>

#include <iostream>

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
      mp_progLambert(new ShaderProgram(in_context)), mp_progFlat(new ShaderProgram(in_context)), mp_lavavision(new ShaderProgram(in_context)),
      mp_shadowmap(new ShaderProgram(in_context)),
      mp_camera(new Camera()), mp_lightcamera(new Camera()),
      mp_terrain(new Terrain(in_context, mp_camera, &mutex)), mp_crosshairs(new CrossHairs(in_context)),
      mp_player(new Player(mp_camera, mp_terrain)), underwater(false), underlava(false), underground(false), time(0), dt(0),
      start_time(QDateTime::currentMSecsSinceEpoch()),
      skyColor(glm::vec4(0.37f, 0.74f, 1.0f, 1)),
      scheduler(new Scheduler(mp_terrain, &mutex)), mp_quad(new Quad(in_context)),
      music(new QMediaPlayer()),water(new QMediaPlayer()), m_frameBuffer(-1), m_renderedTexture(-1),
      m_depthRenderBuffer(-1)
{

    //Create the instance of Cube
    //mp_geomCube->create();
    mp_worldAxes->create();
    mp_quad->create();

    // Create and set up the diffuse shader
    mp_progLambert->create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    mp_progLambert->addTexture(":/textures/minecraft_textures_all.png");
    mp_progLambert->bindTexture(0);
    // Create and set up the flat lighting shader
    mp_progFlat->create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    mp_lavavision->create(":/glsl/lavavision.vert.glsl", ":/glsl/lavavision.frag.glsl");
    mp_lavavision->addTexture(":/textures/fire1.png");
    mp_lavavision->addTexture(":/textures/water_overlay.png");
    mp_lavavision->bindTexture(0);

    mp_shadowmap->create(":/glsl/shadow.vert.glsl", ":/glsl/shadow.frag.glsl");

    mp_camera->eye = glm::vec3(mp_terrain->dimensions.x-10.f, mp_terrain->dimensions.y * 0.60, mp_terrain->dimensions.z-10.f);
    resizeWindow(context->width(), context->height());
    mp_lightcamera->ref = glm::vec3(0, 0, 0);
    mp_lightcamera->eye = glm::vec3(1, 1, 1);
    mp_lightcamera->near_clip = 30;
    mp_lightcamera->far_clip = 100;
    mp_lightcamera->RecomputeAttributes();

    mp_terrain->setTerrainType(new Highland);

    mp_terrain->createRivers();
    mp_terrain->excavateCave();
    mp_terrain->createForest();
    mp_terrain->drawScene();


    for(int i = 0; i < mp_terrain->chunks2Add.size(); ++i)
    {
        mp_terrain->chunk_map[mp_terrain->chunks2Add[i].key] = mp_terrain->chunks2Add[i].ch;

    }
    mp_terrain->chunks2Add.clear();

    QThreadPool::globalInstance()->start(scheduler);
    mp_player->keeptime = 5.f;

    // music->setPlaylist(playlist);
     music->setMedia(QUrl("qrc:/music/Minecraft_Loop.mp3"));
     water->setMedia(QUrl("qrc:/music/Water_scapes.mp3"));
     music->setVolume(50);
     water->setVolume(50);
     music->play();
     musicflag = true;

     /*QSound music("qrc:/music/Minecraft_Loop.mp3");
     music.setLoops(20);
     music.play(); */

    context->glClearColor(skyColor.r,skyColor.g,skyColor.b,skyColor.a);

    createRenderBuffers();
}

PlayState::~PlayState() {
    scheduler->setRunMode(false);
    QThreadPool::globalInstance()->waitForDone();

    mp_worldAxes->destroy();
    mp_crosshairs->destroy();

    delete mp_player;
    delete mp_worldAxes;
    delete mp_progLambert;
    delete mp_progFlat;
    //delete scheduler; // This line throws a seg fault for some reason...
    delete mp_camera;
    delete mp_terrain;
    delete mp_crosshairs;
}

void PlayState::keyPress(QKeyEvent *e) {
    if(e->key()==Qt::Key_K)
    {
        musicStop();
    }
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

    for(int i = 0; i < mp_terrain->chunks2Add.size(); ++i)
    {
        mp_terrain->chunk_map[mp_terrain->chunks2Add[i].key] = mp_terrain->chunks2Add[i].ch;

    }
    mp_terrain->chunks2Add.clear();

    musicCheck();

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
    if (mp_camera->eye[1] < 128){
        underground = true;
    }
    else{
        underground = false;
    }

    time = QDateTime::currentMSecsSinceEpoch();
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
    renderLightCamera();
    //renderFinalScene();
}

void PlayState::GLDrawScene(bool shadow)
{
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
                    if (shadow) {
                        mp_shadowmap->draw(*ch);
                    } else {
                        mp_progLambert->draw(*ch);
                    }
                }
            }
        }
    }

}

// Copied from hw 05
void PlayState::renderLightCamera()
{
    // Render to our framebuffer rather than the viewport
    //context->glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    context->glBindFramebuffer(GL_FRAMEBUFFER, context->defaultFramebufferObject());
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    context->glViewport(0,0,context->width(),context->height());
    // Clear the screen so that we only see newly drawn images
    context->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mp_lightcamera->ref = mp_camera->eye;
    mp_lightcamera->eye = mp_lightcamera->ref + glm::vec3(50, 50, 0);
    mp_lightcamera->RecomputeAttributes();

    mp_shadowmap->setModelMatrix(glm::mat4());
    mp_shadowmap->setViewProjMatrix(mp_lightcamera->getViewProj());

    GLDrawScene(true);
}

// Copied from hw 05
void PlayState::renderFinalScene()
{
    // Render the frame buffer as a texture on a screen-size quad
    if (m_frameBuffer == -1) {
        return;
    }

    // Tell OpenGL to render to the viewport's frame buffer
    context->glBindFramebuffer(GL_FRAMEBUFFER, context->defaultFramebufferObject());
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    context->glViewport(0,0,context->width(),context->height());
    // Clear the screen
    context->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Bind our texture in Texture Unit 0
    context->glActiveTexture(GL_TEXTURE0 + 1);
    context->glBindTexture(GL_TEXTURE_2D, m_renderedTexture);
    mp_progLambert->bindShadowTexture(1);

    mp_progFlat->setViewProjMatrix(mp_camera->getViewProj());
    mp_progLambert->setViewProjMatrix(mp_camera->getViewProj());
    mp_progLambert->setEyePos(glm::vec4(mp_camera->eye, 1.f));
    mp_progLambert->setTime((time - start_time) /1000.f); // convert time to seconds
    mp_progLambert->setShadowViewProjMatrix(mp_lightcamera->getViewProj());
    mp_progLambert->setModelMatrix(glm::mat4());

    GLDrawScene(false);

    glDisable(GL_DEPTH_TEST);
    mp_progFlat->setModelMatrix(glm::mat4());
    //mp_progFlat->draw(*mp_worldAxes);

    mp_progFlat->setViewProjMatrix(glm::mat4());
    mp_progFlat->draw(*mp_crosshairs);
    glEnable(GL_DEPTH_TEST);

    if(underlava==true) // add overlay if player is under lava
    {
        mp_lavavision->setTime((time - start_time) /1000.f); // convert time to seconds
        mp_lavavision->bindTexture(0);
        mp_lavavision->draw(*mp_quad);

    }
    else if(underwater==true) // add overlay if player is underwater
    {
        mp_lavavision->setTime((time - start_time) /1000.f); // convert time to seconds
        mp_lavavision->bindTexture(1);
        mp_lavavision->draw(*mp_quad);
    }

    if (underground == true){
        mp_progLambert->setUnderground(1);
    }
    else{
        mp_progLambert->setUnderground(0);
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

void PlayState::musicCheck()
{
    if(musicflag==false)
    {
        music->stop();
        water->stop();
    }
    else if(musicflag==true)
    {
        if(underwater)
        {
            music->stop();
            water->play();
        }
        else if((!underwater)&&(water->state()==1))
        {
            water->stop();
            music->play();
        }
        else if((!underwater)&&(water->state()==0))
        {
            music->play();
        }
    }

}

void PlayState::musicStop()
{
    musicflag = false;
}

void PlayState::createRenderBuffers()
{
    // Initialize the frame buffers and render textures
    context->glGenFramebuffers(1, &m_frameBuffer);
    //context->glGenTextures(1, &m_renderedTexture);
    context->glGenRenderbuffers(1, &m_depthRenderBuffer);

    context->glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    // Bind our texture so that all functions that deal with textures will interact with this one
    context->glBindTexture(GL_TEXTURE_2D, m_renderedTexture);
    // Give an empty image to OpenGL ( the last "0" )
    context->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, context->width(), context->height(), 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);

    // Set the render settings for the texture we've just created.
    // Essentially zero filtering on the "texture" so it appears exactly as rendered
    context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // Clamp the colors at the edge of our texture
    context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    context->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Initialize our depth buffer
    context->glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
    context->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, context->width(), context->height());
    context->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);

    // Set m_renderedTexture as the color output of our frame buffer
    context->glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_renderedTexture, 0);

    // Sets the color output of the fragment shader to be stored in GL_COLOR_ATTACHMENT0, which we previously set to m_renderedTextures[i]
    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    context->glDrawBuffers(1, drawBuffers); // "1" is the size of drawBuffers
    //context->glDrawBuffer(GL_NONE);

    if(context->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Frame buffer did not initialize correctly..." << std::endl;
        context->printGLErrorLog();
    }

}



MenuState::MenuState(MyGL* in_mygl)
    : GameState(in_mygl), mygl(in_mygl),
      mp_quad(new Quad(in_mygl)),
      mp_progMenu(new ShaderProgram(in_mygl))
{
    mp_progMenu->create(":/glsl/menu.vert.glsl", ":/glsl/menu.frag.glsl");
    mp_progMenu->addTexture(":/textures/title.png");
    mp_progMenu->addTexture(":/textures/minecraft_textures_all.png");
    mp_progMenu->bindTexture(0);
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
    mp_progMenu->bindTexture(1);
    mygl->set2PlayState();
}

void MenuState::keyRelease(QKeyEvent *r) {

}

