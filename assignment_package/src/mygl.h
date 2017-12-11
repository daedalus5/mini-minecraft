#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>

#include <gamestate.h>

// Forward declaration
class GameState;

class MyGL : public OpenGLContext
{
    Q_OBJECT
private:

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.


    /// Timer linked to timerUpdate(). Fires approx. 60 times per second
    QTimer timer;

    GameState* mp_gamestate;

    void MoveMouseToCenter(); // Forces the mouse position to the screen's center. You should call this
                              // from within a mouse move event after reading the mouse movement so that
                              // your mouse stays within the screen bounds and is always read.

    // A collection of handles to the five frame buffers we've given
    // ourselves to perform render passes. The 0th frame buffer is always
    // written to by the render pass that uses the currently bound surface shader.
    GLuint m_frameBuffer;
    // A collection of handles to the textures used by the frame buffers.
    // m_frameBuffers[i] writes to m_renderedTextures[i].
    GLuint m_renderedTexture;
    // A collection of handles to the depth buffers used by our frame buffers.
    // m_frameBuffers[i] writes to m_depthRenderBuffers[i].
    GLuint m_depthRenderBuffer;

public:
    explicit MyGL(QWidget *parent = 0);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void set2PlayState();
    void createRenderBuffers();
    void bindDefaultFrameBufferAfterShadow();
    void bindFrameBufferBeforeShadow();


protected:
    void keyPressEvent(QKeyEvent *e);

    //void mousePressEvent(QMouseEvent *e);

    void keyReleaseEvent(QKeyEvent *r);
    void mouseMoveEvent(QMouseEvent *m);
    void mousePressEvent(QMouseEvent *mp);
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    /// Slot that gets called ~60 times per second
    void timerUpdate();

};


#endif // MYGL_H
