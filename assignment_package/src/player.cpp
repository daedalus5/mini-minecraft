#include "player.h"
#include<QKeyEvent>
#include<iostream>

Player::Player(Camera* cam, Terrain* terr):ptr_to_cam(cam),ptr_to_terrain(terr),isWpressed(false),isApressed(false),isSpressed(false),isDpressed(false),isSpacepressed(false),isQpressed(false),isEpressed(false),isLMBpressed(false),isRMBpressed(false),mouseOrientFlag(false),controllerState(false),mouseState(false),isSandbox(false){

    pos= ptr_to_cam->eye;



    vertexpositions.push_back(glm::vec3(pos.x+0.5,pos.y+0.5,pos.z+0.5));// Constructs Bounding Box for Player
    vertexpositions.push_back(glm::vec3(pos.x+0.5,pos.y+0.5,pos.z-0.5));
    vertexpositions.push_back(glm::vec3(pos.x-0.5,pos.y+0.5,pos.z-0.5));
    vertexpositions.push_back(glm::vec3(pos.x-0.5,pos.y+0.5,pos.z+0.5));

    vertexpositions.push_back(glm::vec3(pos.x+0.5,pos.y-0.5,pos.z+0.5));
    vertexpositions.push_back(glm::vec3(pos.x+0.5,pos.y-0.5,pos.z-0.5));
    vertexpositions.push_back(glm::vec3(pos.x-0.5,pos.y-0.5,pos.z-0.5));
    vertexpositions.push_back(glm::vec3(pos.x-0.5,pos.y-0.5,pos.z+0.5));

    vertexpositions.push_back(glm::vec3(pos.x+0.5,pos.y-1.5,pos.z+0.5));
    vertexpositions.push_back(glm::vec3(pos.x+0.5,pos.y-1.5,pos.z-0.5));
    vertexpositions.push_back(glm::vec3(pos.x-0.5,pos.y-1.5,pos.z-0.5));
    vertexpositions.push_back(glm::vec3(pos.x-0.5,pos.y-1.5,pos.z+0.5));

}
void Player::updateTime(quint64 dtfromTimer) //player's copy of dt is updated by myGL's timerUpdate()
{
    dt = (float)dtfromTimer/1000.f; //convert dt from milliseconds to seconds

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
    if(e->key()==Qt::Key_Q)
    {
        isQpressed = true;
    }
    if(e->key()==Qt::Key_E)
    {
        isEpressed = true;
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
        keeptime = 0.5f;
    }
    if(e->key()==Qt::Key_Q)
    {
        isQpressed = false;
    }
    if(e->key()==Qt::Key_E)
    {
        isEpressed = false;
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

bool Player::boundingBoxcheck(glm::vec3 box1min, glm::vec3 box1max, glm::vec3 box2min, glm::vec3 box2max ) // function for testing box-box intersection
{

    if((box1min.x<box2max.x)&&(box2min.x<box1max.x)&&(box1min.y<box2max.y)&&(box2min.y<box1max.y)&&(box1min.z<box2max.z)&&(box2min.z<box1max.z))
    {
        return true;
    }
    else
        return false;

}

bool Player::collisionDetect() // returns true if any of the player vertices faces a potential collision in the next timerUpdate()
{
    if(isSandbox) //disable collisions if game is being run in Sandbox Mode
    {
        return false;
    }
    else
    {
        int flag = 0;
    int velocityFlag = 0;

    std::vector<glm::vec3>nextpositions;

    for(int i=0;i<vertexpositions.size();i++)// calculate nextpositions for every player vertex
    {
        nextpositions.push_back(glm::vec3(vertexpositions[i] + velocity * 2.f * dt));


    }
    for(int i=0;i<nextpositions.size();i++)
    {
        BlockType nextblock = ptr_to_terrain->getBlockAt(nextpositions[i].x,nextpositions[i].y,nextpositions[i].z);
        BlockType currentblock = ptr_to_terrain->getBlockAt(vertexpositions[i].x, vertexpositions[i].y, vertexpositions[i].z);
        if((ptr_to_terrain->getBlockAt(nextpositions[i].x,nextpositions[i].y,nextpositions[i].z) != EMPTY)&&(ptr_to_terrain->getBlockAt(nextpositions[i].x,nextpositions[i].y,nextpositions[i].z) != WATER)&&(ptr_to_terrain->getBlockAt(nextpositions[i].x,nextpositions[i].y,nextpositions[i].z) != LAVA))//check if nextposition has a non-empty block
        {
            flag = 1;
            velocity.x = 0.f;
            if(velocity.y>0)
            {
                velocity.y= 0.f;
            }
            return true;
           /*glm::vec3 intersectBoxMin;
            glm::vec3 intersectBoxMax;
            intersectBoxMin.x = floor(nextpositions[i].x);
            intersectBoxMin.y = floor(nextpositions[i].y);
            intersectBoxMin.z = floor(nextpositions[i].z);
            intersectBoxMax = intersectBoxMin + glm::vec3(1.0,1.0,1.0);
            bool a = boundingBoxcheck(box1min,box1max,intersectBoxMin,intersectBoxMax);// perform box-box intersection test for player box1
            bool b = boundingBoxcheck(box2min, box2max, intersectBoxMin, intersectBoxMax);// perform box-box intersection test for player box 2
            bool c = a||b;
            flag = 1;
            return c;*/
        }
        if((currentblock==WATER)||(currentblock==LAVA))
        {
           velocityFlag = 1;

            }
        }
    if(velocityFlag==1)
    {
        velocity = velocity*0.2323f;
    }
   if(flag==0)
    {
        return false;
    }
    nextpositions.clear();
    }

 }
void Player::gravityCheck() // implements gravity
{

        std::vector<glm::vec3>bottompositions;
    for(int i=8;i<12;i++)
    {
         bottompositions.push_back(glm::vec3(vertexpositions[i] + velocity * 2.f * dt));
    }

    BlockType a = ptr_to_terrain->getBlockAt(bottompositions[0].x,bottompositions[0].y,bottompositions[0].z); //checks below the bottom vertices of the player's bounding box for ground
    BlockType b = ptr_to_terrain->getBlockAt(bottompositions[1].x,bottompositions[1].y,bottompositions[1].z);
    BlockType c = ptr_to_terrain->getBlockAt(bottompositions[2].x,bottompositions[2].y,bottompositions[2].z);
    BlockType d = ptr_to_terrain->getBlockAt(bottompositions[3].x,bottompositions[3].y,bottompositions[3].z);
    if((a==EMPTY)&&(b==EMPTY)&&(c==EMPTY)&&(d==EMPTY))
    {

        pos = ptr_to_cam->eye;
        glm::vec3 prevpos = pos;

        if(velocity.y>((-1.0f)*50.f))
        {
            velocity.y = velocity.y - 1.5f;

        }
         // downward velocity to simulate gravity

        bool cldetect = collisionDetect();
        if(cldetect==false)
         {
            pos = pos + velocity*(float)dt;
         }
        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye+translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;

    }
    else if((a==LAVA)&&(b==LAVA)&&(c==LAVA)&&(d==LAVA))
    {
        //disable collision detection for LAVA blocks, and dampen gravity to simulate a sinking effect

        pos = ptr_to_cam->eye;
        glm::vec3 prevpos = pos;

        velocity.y = velocity.y - 0.5f; // downward velocity to simulate gravity

        bool cldetect = collisionDetect();
        if(cldetect==false)
         {
            pos = pos + velocity*(float)dt;
         }
        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye+translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;
    }
    else if((a==WATER)&&(b==WATER)&&(c==WATER)&&(d==WATER))
    {
        //disable collision detection for WATER blocks, and dampen gravity to simulate a sinking effect

        pos = ptr_to_cam->eye;
        glm::vec3 prevpos = pos;
        velocity.y = velocity.y - 0.9f; // downward velocity to simulate gravity
        bool cldetect = collisionDetect();
        if(cldetect==false)
         {
            pos = pos + velocity*(float)dt;
         }
        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye+translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;
    }
    else if(((a==WATER)||(a==LAVA)||(a==EMPTY))&&((b==WATER)||(b==LAVA)||(b==EMPTY))&&((c==WATER)||(c==LAVA)||(c==EMPTY))&&((d==WATER)||(d==LAVA)||(d==EMPTY)))
    {
        //if player has any combination of lava, water, and empty blocks beneath it, add sinking effect and disable collisions
        pos = ptr_to_cam->eye;
        glm::vec3 prevpos = pos;

        velocity.y = velocity.y - 0.5f; // downward velocity to simulate gravity

        bool cldetect = collisionDetect();
        if(cldetect==false)
         {
            pos = pos + velocity*(float)dt;
         }
        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye+translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;

    }
}

void Player::updateAttributes()// invoked by myGL's timerUpdate(). Player updates its position and velocity based on dt passed to updateTime().
{

    if(isWpressed == true)
    {
        pos = ptr_to_cam->eye;
        glm::vec3 prevpos = pos;
        velocity = 5.f*glm::vec3(glm::normalize(glm::vec4(ptr_to_cam->look,0))); //velocity along the camera's look vector
        velocity.y=0; //prevents flight
        bool cldetect = collisionDetect();
        if(cldetect==false) // check for collision detection. If there is a potential collision, don't move
        {
            pos = pos + velocity*(float)dt;
        }
        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye + translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;
    }

    if(isApressed)
    {
        pos = ptr_to_cam->eye;
        glm::vec3 prevpos = pos;
        velocity = -5.f*glm::vec3(glm::normalize(glm::vec4(ptr_to_cam->right,0)));
        velocity.y=0;
        bool cldetect = collisionDetect();
        if(cldetect==false)
        {
            pos = pos + velocity*(float)dt;

        }
        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye+translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;

    }
    if(isSpressed)
    {
        pos = ptr_to_cam->eye;
        glm::vec3 prevpos = pos;
        velocity = -5.f*glm::vec3(glm::normalize(glm::vec4(ptr_to_cam->look,0)));
        velocity.y=0;
        bool cldetect = collisionDetect();
        if(cldetect==false)
        {
            pos = pos + velocity*(float)dt;
        }

        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye+translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;
     }
    if(isDpressed)
    {
        pos = ptr_to_cam->eye;
        glm::vec3 prevpos = pos;
        velocity = 5.f*glm::vec3(glm::normalize(glm::vec4(ptr_to_cam->right,0)));
        velocity.y=0;
         bool cldetect = collisionDetect();
         if(cldetect==false)
          {
            pos = pos + velocity * (float)dt;
          }

        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye+translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;
    }
    if(isSpacepressed)
    {
        if((checkSubmerged()!=LAVA)&&(checkSubmerged()!=WATER))
        {
            keeptime = keeptime - dt;
        }
        if(keeptime > 0.f)
        {

        pos = ptr_to_cam->eye;
        glm::vec3 prevpos = pos;
        velocity = 10.f*glm::vec3(glm::normalize(glm::vec4(ptr_to_cam->world_up,0)));
         bool cldetect = collisionDetect();
         if(cldetect==false)
         {
            pos = pos + velocity*(float)dt;
         }
        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye+translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;
        }
        else if(keeptime<0.f)
        {
            isSpacepressed = false;
        }


    }

    if(isQpressed)
    {
        if(isSandbox) //only work if in sandbox mode
        {
        pos = ptr_to_cam->eye;
        glm::vec3 prevpos = pos;
        velocity = -5.f*glm::vec3(glm::normalize(glm::vec4(ptr_to_cam->world_up,0)));
        bool cldetect = collisionDetect();
        if(cldetect==false)
         {
            pos = pos + velocity*(float)dt;
         }
        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye+translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;
        }
    }

    if(isEpressed)
    {
        if(isSandbox) //only work if in sandbox mode
        {
        pos = ptr_to_cam->eye;
        glm::vec3 prevpos = pos;
        velocity = 5.f*glm::vec3(glm::normalize(glm::vec4(ptr_to_cam->world_up,0)));
        bool cldetect = collisionDetect();
        if(cldetect==false)
         {
            pos = pos + velocity*(float)dt;
         }
        glm::vec3 translation1 = pos-prevpos;
        ptr_to_cam->eye = ptr_to_cam->eye+translation1;
        ptr_to_cam->ref = ptr_to_cam->ref + translation1;
        }


    }
}
void Player::mouseMoveState(QMouseEvent *m) //detects change in cursor position. Rotates Camera accordingly
{
    mouseOrientFlag = true;
    float currMouseX = m->x();
    dx = currMouseX - (ptr_to_cam->width / 2.f);
    rotation.x = rotation.x + (float)dx*0.35f;
    float currMouseY = m->y();
    dy = currMouseY - (ptr_to_cam->height / 2.f);
    rotation.y = rotation.y + (float)dy*0.35f;

   /*if((m->x()!=mouseX)||(m->y()!=mouseY))
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
        mouseOrientFlag = false;*/
}

void Player::mousePressState(QMouseEvent *m)
{
    mouseState = true;

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

    vertexpositions[0] = glm::vec3(pos.x+0.5,pos.y+0.5,pos.z+0.5);
    vertexpositions[1] = glm::vec3(pos.x+0.5,pos.y+0.5,pos.z-0.5);
    vertexpositions[2] = glm::vec3(pos.x-0.5,pos.y+0.5,pos.z-0.5);
    vertexpositions[3] = glm::vec3(pos.x-0.5,pos.y+0.5,pos.z+0.5);

    vertexpositions[4] = glm::vec3(pos.x+0.5,pos.y-0.5,pos.z+0.5);
    vertexpositions[5] = glm::vec3(pos.x+0.5,pos.y-0.5,pos.z-0.5);
    vertexpositions[6] = glm::vec3(pos.x-0.5,pos.y-0.5,pos.z-0.5);
    vertexpositions[7] = glm::vec3(pos.x-0.5,pos.y-0.5,pos.z+0.5);

    vertexpositions[8] = glm::vec3(pos.x+0.5,pos.y-1.5,pos.z+0.5);
    vertexpositions[9] = glm::vec3(pos.x+0.5,pos.y-1.5,pos.z-0.5);
    vertexpositions[10] = glm::vec3(pos.x-0.5,pos.y-1.5,pos.z-0.5);
    vertexpositions[11] = glm::vec3(pos.x-0.5,pos.y-1.5,pos.z+0.5);
    box1min = vertexpositions[10]; //sets limits for box intersection testing
    box1max = vertexpositions[4];
    box2min = vertexpositions[6];
    box2max = vertexpositions[0];
    playerMin = box1min;
    playerMax = box2max;
}

BlockType Player::checkSubmerged()
{
    if(ptr_to_terrain->getBlockAt(pos.x, pos.y, pos.z) == LAVA)
    {
        return LAVA;

    }
    else if(ptr_to_terrain->getBlockAt(pos.x, pos.y, pos.z) == WATER)
    {
        return WATER;
    }
    else
        return EMPTY;
}


