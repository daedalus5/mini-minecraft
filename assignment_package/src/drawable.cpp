#include "drawable.h"
#include <la.h>

Drawable::Drawable(OpenGLContext* context)
    : bufIdx(), bufPos(), bufNor(), bufCol(), bufEve(),
      idxBound(false), posBound(false), norBound(false), colBound(false),
      eveBound(false), uvBound(false), bufUV(),
      context(context)
{}

Drawable::~Drawable()
{}


void Drawable::destroy()
{
    context->glDeleteBuffers(1, &bufIdx);
    context->glDeleteBuffers(1, &bufPos);
    context->glDeleteBuffers(1, &bufNor);
    context->glDeleteBuffers(1, &bufCol);
    context->glDeleteBuffers(1, &bufUV);
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount()
{
    return count;
}

void Drawable::generateIdx()
{
    if (!idxBound) {
        idxBound = true;
        // Create a VBO on our GPU and store its handle in bufIdx
        context->glGenBuffers(1, &bufIdx);
    }
}

void Drawable::generatePos()
{
    if (!posBound) {
        posBound = true;
        // Create a VBO on our GPU and store its handle in bufPos
        context->glGenBuffers(1, &bufPos);
    }
}

void Drawable::generateNor()
{
    if (!norBound) {
        norBound = true;
        // Create a VBO on our GPU and store its handle in bufNor
        context->glGenBuffers(1, &bufNor);
    }
}

void Drawable::generateCol()
{
    if (!colBound) {
        colBound = true;
        // Create a VBO on our GPU and store its handle in bufCol
        context->glGenBuffers(1, &bufCol);
    }
}

void Drawable::generateUV()
{
    if (!uvBound) {
        uvBound = true;
        // Create a VBO on our GPU and store its handle in bufCol
        context->glGenBuffers(1, &bufUV);
    }
}

void Drawable::generateEve() {
    if (!eveBound) {
        eveBound = true;
        context->glGenBuffers(1, &bufEve);
    }
}

bool Drawable::bindIdx()
{
    if(idxBound) {
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    }
    return idxBound;
}

bool Drawable::bindPos()
{
    if(posBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    }
    return posBound;
}

bool Drawable::bindNor()
{
    if(norBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    }
    return norBound;
}

bool Drawable::bindCol()
{
    if(colBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    }
    return colBound;
}

bool Drawable::bindEve() {
    if (eveBound) {
        context->glBindBuffer(GL_ARRAY_BUFFER, bufEve);
    }
    return eveBound;
}

bool Drawable::bindUV()
{
    if(uvBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufUV);
    }
    return uvBound;
}
