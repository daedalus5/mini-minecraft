#include "camera.h"

#include <la.h>
#include <iostream>
#include<math.h>


Camera::Camera():
    Camera(400, 400)
{

    look = glm::vec3(0,0,-1);
    up = glm::vec3(0,1,0);
    right = glm::vec3(1,0,0);
    phi = 0;

}

Camera::Camera(unsigned int w, unsigned int h):
    Camera(w, h, glm::vec3(0,0,10), glm::vec3(0,10,0), glm::vec3(0,1,0))
{}

Camera::Camera(unsigned int w, unsigned int h, const glm::vec3 &e, const glm::vec3 &r, const glm::vec3 &worldUp):
    fovy(45),
    width(w),
    height(h),
    near_clip(0.1f),
    far_clip(1000),
    eye(e),
    ref(r),
    world_up(worldUp)

{
    phi = 0;
    RecomputeAttributes();
}

Camera::Camera(const Camera &c):
    fovy(c.fovy),
    width(c.width),
    height(c.height),
    near_clip(c.near_clip),
    far_clip(c.far_clip),
    aspect(c.aspect),
    eye(c.eye),
    ref(c.ref),
    look(c.look),
    up(c.up),
    right(c.right),
    world_up(c.world_up),
    V(c.V),
    H(c.H)
{}


void Camera::RecomputeAttributes()
{
    look = glm::normalize(ref - eye);
    right = glm::normalize(glm::cross(look, world_up));
    up = glm::cross(right, look);

    float tan_fovy = tan(glm::radians(fovy/2));
    float len = glm::length(ref - eye);
    aspect = width/height;
    V = up*len*tan_fovy;
    H = right*len*aspect*tan_fovy;
    straight = glm::cross(world_up,right);
    straight = glm::normalize(straight);




}

glm::mat4 Camera::getViewProj()
{
    return glm::perspective(glm::radians(fovy), width / (float)height, near_clip, far_clip) * glm::lookAt(eye, ref, up);
}

void Camera::RotateAboutUp(float deg)
{
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(deg), world_up);

    ref = ref - eye;
    ref = glm::vec3(rotation * glm::vec4(ref, 1));
    ref = ref + eye;
    RecomputeAttributes();
}
void Camera::RotateAboutRight(float deg)
{
    if((phi+deg<75)&&(phi+deg>-75))
    {
    phi = phi+deg;



   glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(deg), right);


    ref = ref - eye;
    ref = glm::vec3(rotation * glm::vec4(ref, 1));
    ref = ref + eye;

    RecomputeAttributes();
    }

}

void Camera::TranslateAlongLook(float amt)
{
    glm::vec3 translation = look * amt;
    eye += translation;
    ref += translation;
}

void Camera::TranslateAlongRight(float amt)
{
    glm::vec3 translation = right * amt;
    eye += translation;
    ref += translation;
}
void Camera::TranslateAlongUp(float amt)
{
    glm::vec3 translation = up * amt;
    eye += translation;
    ref += translation;
}

static const int CH_IDX_COUNT = 4;
static const int CH_VERT_COUNT = 4;

//////////////////////////////////

GLenum CrossHairs::drawMode()
{
    return GL_LINES;
}

void CrossHairs::create()
{
    GLuint ch_idx[CH_IDX_COUNT];
    glm::vec4 ch_vert_pos[CH_VERT_COUNT];
    glm::vec4 ch_vert_col[CH_VERT_COUNT];

    float xOffset = 0.1f;
    float yOffset = 0.1f;;

    if (aspect < 1){
        xOffset = yOffset * aspect;
    }
    else{
        yOffset = xOffset * aspect;
    }

    glm::vec4 posA0 = glm::vec4(-xOffset, 0.0f, 0.0f, 1.0f);
    glm::vec4 posA1 = glm::vec4(xOffset, 0.0f, 0.0f, 1.0f);
    glm::vec4 posB0 = glm::vec4(0.0f, -yOffset, 0.0f, 1.0f);
    glm::vec4 posB1 = glm::vec4(0.0f, yOffset, 0.0f, 1.0f);
    ch_vert_pos[0] = posA0;
    ch_vert_pos[1] = posA1;
    ch_vert_pos[2] = posB0;
    ch_vert_pos[3] = posB1;

    glm::vec4 grey = glm::vec4(0.75f, 0.75f, 0.75f, 1.0f);
    for(int i = 0; i < CH_VERT_COUNT; ++i){
        ch_vert_col[i] = grey;
    }

    ch_idx[0] = 0;
    ch_idx[1] = 1;
    ch_idx[2] = 2;
    ch_idx[3] = 3;

    count = CH_IDX_COUNT;

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, CH_IDX_COUNT * sizeof(GLuint), ch_idx, GL_STATIC_DRAW);

    // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generatePos();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    context->glBufferData(GL_ARRAY_BUFFER, CH_VERT_COUNT * sizeof(glm::vec4), ch_vert_pos, GL_STATIC_DRAW);

    generateCol();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    context->glBufferData(GL_ARRAY_BUFFER, CH_VERT_COUNT * sizeof(glm::vec4), ch_vert_col, GL_STATIC_DRAW);
}
