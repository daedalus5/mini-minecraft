#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include<QDateTime>


#include <set>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent), mp_gamestate(nullptr)
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

    //float amount = 1.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        //amount = 10.0f;
    }

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
    timer.stop();
    GameState* temp_new = new PlayState(this);
    GameState* temp_old = mp_gamestate;
    //delete mp_gamestate;
    mp_gamestate = temp_new;
    delete temp_old;
    timer.start(16);
}
