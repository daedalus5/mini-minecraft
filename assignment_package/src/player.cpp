#include "player.h"
#include<QKeyEvent>

Player::Player(Camera* cam)
    : ptr_to_cam(cam), controllerState(false), aerialState(true)
{
    ptr_to_cam->ref = ptr_to_cam->eye + glm::vec3(10,0,10);
    ptr_to_cam->look = ptr_to_cam->ref-ptr_to_cam->eye;
    pos= ptr_to_cam->eye;

}
void Player::updateTime(quint64 dtfromTimer) //player's copy of dt is updated by myGL's timerUpdate()
{
    dt = (float)dtfromTimer/1000.f;

}

void Player::keyPressState(QKeyEvent *e) // invoked by keyPressEvent. sets key flags
{
    controllerState = true;
    if(e->key()==Qt::Key_W)
    {
        isWpressed = true;
    }
    if(e->key()==Qt::Key_A)
    {
        isApressed = true;
    }
    if(e->key()==Qt::Key_S)
    {
        isSpressed = true;
    }
    if(e->key()==Qt::Key_D)
    {
        isDpressed = true;
    }
    if(e->key()==Qt::Key_Space)
    {
        isSpacepressed = true;
    }
    if(e->key()==Qt::Key_Z)
    {
        isZpressed = true;
    }
}
void Player::keyReleaseState(QKeyEvent *e)// invoked by keyReleaseEvent. sets key flags.
{
    controllerState = true;
    if(e->key()==Qt::Key_W)
    {
        isWpressed = false;
    }
    if(e->key()==Qt::Key_A)
    {
        isApressed = false;
    }
    if(e->key()==Qt::Key_S)
    {
        isSpressed = false;
    }
    if(e->key()==Qt::Key_D)
    {
        isDpressed = false;
    }
    if(e->key()==Qt::Key_Space)
    {
        isSpacepressed = false;
    }
    if(e->key()==Qt::Key_Z)
    {
        isZpressed = false;
    }

}

void Player::updateCameraOrientation() //updates Camera Orientation according to cursor movement
{

    if(mouseOrientFlag==true)
    {


    ptr_to_cam->RotateAboutUp(-rotation.x);
    rotation.x=0;



    ptr_to_cam->RotateAboutRight(rotation.y);
    rotation.y=0;

    }





}

void Player::updateAttributes()// invoked by myGL's timerUpdate(). Player updates its position and velocity based on dt passed to updateTime().
{
    if(isWpressed == true)
    {
        glm::vec3 prevpos = pos;


        velocity = 5.f*glm::vec3(glm::normalize(glm::vec4(ptr_to_cam->look,0)));
        velocity.y=0;
        pos = pos + velocity*(float)dt;
        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye+translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;


     }

    if(isApressed)
    {
        glm::vec3 prevpos = pos;


        velocity = -5.f*glm::vec3(glm::normalize(glm::vec4(ptr_to_cam->right,0)));
        velocity.y=0;
        pos = pos + velocity*(float)dt;
        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye+translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;

    }
    if(isSpressed)
    {
        glm::vec3 prevpos = pos;


        velocity = -5.f*glm::vec3(glm::normalize(glm::vec4(ptr_to_cam->look,0)));
        velocity.y=0;
        pos = pos + velocity*(float)dt;
        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye+translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;


    }
    if(isDpressed)
    {
        glm::vec3 prevpos = pos;


        velocity = 5.f*glm::vec3(glm::normalize(glm::vec4(ptr_to_cam->right,0)));
        velocity.y=0;
        pos = pos + velocity*(float)dt;
        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye+translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;
    }
    if(isSpacepressed)
    {
        glm::vec3 prevpos = pos;


        velocity = 10.f*glm::vec3(glm::normalize(glm::vec4(ptr_to_cam->world_up,0)));

        pos = pos + velocity*(float)dt;
        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye+translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;
    }

    if(isZpressed)
    {
        glm::vec3 prevpos = pos;


        velocity = -50.f*glm::vec3(glm::normalize(glm::vec4(ptr_to_cam->world_up,0)));

        pos = pos + velocity*(float)dt;
        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye+translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;

    }
    if(aerialState==true)
    {
       // velocity.y*=0.075;
    }



}
void Player::mouseMoveState(QMouseEvent *m)
{
    if((m->x()!=mouseX)||(m->y()!=mouseY))
    {
        mouseOrientFlag = true;



    float currMouseX = m->x();
    dx = currMouseX - mouseX;
    rotation.x = (float)dx*0.35f;


    float currMouseY = m->y();
    dy = currMouseY - mouseY;
    rotation.y = (float)dy*0.35f;
    mouseX = m->x();
    mouseY = m->y();


}
    else
        mouseOrientFlag = false;
}

void Player::mousePressState(QMouseEvent *m)
{
    controllerState = true;

    if(m->buttons() & Qt::LeftButton)
    {
        isLMBpressed = true;
    }
    else if(m->buttons() & Qt::RightButton)
    {
        isRMBpressed = true;
    }




}

void Player::mouseReleaseState(QMouseEvent *mr)
{
    //controllerState = false;

    if(mr->button() & Qt::LeftButton)
    {
        isLMBpressed = false;
    }
    else if(mr->button() & Qt::RightButton)
    {
        isRMBpressed = false;
    }

}

void Player::playerGeometry() // constructs bounding box for player
{
    if(vertexpositions.empty()==false)
    {
        vertexpositions.clear();

    }
    vertexpositions.push_back(glm::vec3(pos.x,pos.y,pos.z+0.5));
    vertexpositions.push_back(glm::vec3(pos.x,pos.y,pos.z-0.5));
    vertexpositions.push_back(glm::vec3(pos.x-1,pos.y,pos.z-0.5));
    vertexpositions.push_back(glm::vec3(pos.x-1,pos.y,pos.z+0.5));

    vertexpositions.push_back(glm::vec3(pos.x,pos.y-1,pos.z+0.5));
    vertexpositions.push_back(glm::vec3(pos.x,pos.y-1,pos.z-0.5));
    vertexpositions.push_back(glm::vec3(pos.x-1,pos.y-1,pos.z-0.5));
    vertexpositions.push_back(glm::vec3(pos.x-1,pos.y-1,pos.z+0.5));

    vertexpositions.push_back(glm::vec3(pos.x,pos.y-2,pos.z+0.5));
    vertexpositions.push_back(glm::vec3(pos.x,pos.y-2,pos.z-0.5));
    vertexpositions.push_back(glm::vec3(pos.x-1,pos.y-2,pos.z-0.5));
    vertexpositions.push_back(glm::vec3(pos.x-1,pos.y-2,pos.z+0.5));
}

