#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include<QDateTime>
#include <time.h>


#include <set>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent), mp_gamestate(nullptr),
      m_frameBuffer(-1),
      m_renderedTexture(-1),
      m_depthRenderBuffer(-1)
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));

    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible


}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    delete mp_gamestate;
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);



    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);
    createRenderBuffers();

    // Set a color with which to draw geometry since you won't have one
    // defined until you implement the Node classes.
    // This makes your geometry render green.
    //mp_progLambert->setGeometryColor(glm::vec4(0,1,0,1));

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
//    vao.bind();
    glBindVertexArray(vao);

    mp_gamestate = new MenuState(this);
    // Tell the timer to redraw 60 times per second
    timer.start(16);
}

void MyGL::resizeGL(int w, int h)
{

    printGLErrorLog();
    mp_gamestate->resizeWindow(w, h);
}

// MyGL's constructor links timerUpdate() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to use timerUpdate
void MyGL::timerUpdate()
{

    mp_gamestate->update();

    update();

    MoveMouseToCenter();

}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mp_gamestate->paint();
}

void MyGL::keyPressEvent(QKeyEvent *e) // triggered when key is pressed
{
    if(e->key()==Qt::Key_Escape)
    {
        QApplication::quit();
    }
    mp_gamestate->keyPress(e);
}

void MyGL::keyReleaseEvent(QKeyEvent *r) // triggered when key is released
{
    mp_gamestate->keyRelease(r);

}

void MyGL::mouseMoveEvent(QMouseEvent *m) // triggered at mouse movement
{
    QRect s = geometry();
    int x = m->globalX();
    int y = m->globalY();

    bool reset = false;

    if(m->x()<0)
    {
        x -=  m->x();
        reset = true;
    }
    else if(m->x() >=s.width())
    {
        x += s.width() - m->x() - 1;
        reset = true;
    }

    if(m->y()<0)
    {
        y -= m->y();
        reset = true;
    }

    if(m->y() >=s.height())
    {
        y += s.height() - m->y() - 1;
        reset = true;
    }

    if (reset)
    {
        QCursor::setPos(x,y);
    }

    mp_gamestate->mouseMove(m);

}

void MyGL::mouseReleaseEvent(QMouseEvent *mr)// triggered when mousebutton is released
{
    mp_gamestate->mouseRelease(mr);
}

void MyGL::mousePressEvent(QMouseEvent *e)
{
    mp_gamestate->mousePress(e);
}

void MyGL::set2PlayState() {
    //timer.stop();
    GameState* temp_new = new PlayState(this);
    GameState* temp_old = mp_gamestate;
    //delete mp_gamestate;
    mp_gamestate = temp_new;
    delete temp_old;
    //timer.start(16);
}

void MyGL::createRenderBuffers()
{
//    // Initialize the frame buffers and render textures
//    glGenFramebuffers(1, &m_frameBuffer);
//    glGenTextures(1, &m_renderedTexture);
//    glGenRenderbuffers(1, &m_depthRenderBuffer);

//    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
//    // Bind our texture so that all functions that deal with textures will interact with this one
//    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);
//    // Give an empty image to OpenGL ( the last "0" )
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width(), this->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)0);

//    // Set the render settings for the texture we've just created.
//    // Essentially zero filtering on the "texture" so it appears exactly as rendered
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    // Clamp the colors at the edge of our texture
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

//    // Initialize our depth buffer
//    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width(), this->height());
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);

//    // Set m_renderedTexture as the color output of our frame buffer
//    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_renderedTexture, 0);

//    // Sets the color output of the fragment shader to be stored in GL_COLOR_ATTACHMENT0, which we previously set to m_renderedTextures[i]
//    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
//    glDrawBuffers(1, drawBuffers); // "1" is the size of drawBuffers

     glGenFramebuffers(1, &m_frameBuffer);
     glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

     // Depth texture. Slower than a depth buffer, but you can sample it later in your shader
     glGenTextures(1, &m_renderedTexture);
     glBindTexture(GL_TEXTURE_2D, m_renderedTexture);
     glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, 2048, 2048, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

     glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_renderedTexture, 0);

     glDrawBuffer(GL_NONE); // No color buffer is drawn to.

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Frame buffer did not initialize correctly..." << std::endl;
        printGLErrorLog();
    }
}

void MyGL::bindDefaultFrameBufferAfterShadow() {
    // Tell OpenGL to render to the viewport's frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glViewport(0,0,this->width(),this->height());
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);
}

void MyGL::bindFrameBufferBeforeShadow() {
    // Render the 3D scene to our frame buffer

    // Render to our framebuffer rather than the viewport
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glViewport(0, 0, 2048, 2048);
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
