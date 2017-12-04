#include "lsystem.h"
#define GLM_FORCE_RADIANS
#include <glm/gtx/transform.hpp>
#include <iostream>

// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// LSYSTEM START

LSystem::LSystem(glm::vec2 pos, glm::vec2 heading) :
    path(""), activeTurtle(Turtle(pos, heading)), turtleStack(), ruleSet(), charToDrawingOperation(), branchProb(1.0f) {}

LSystem::~LSystem(){}

void LSystem::addRule(QChar chr, QString str){
    ruleSet[chr] = str;
}

void LSystem::generatePath(int n, QString seed){
    // add generative rules
    addRule('X', "[-FX]+FX");
    // string generation
    QString s = seed;
    for(int i = 0; i < n; i++){
        QHashIterator<QChar, QString> j(ruleSet);
        while (j.hasNext()) {
            j.next();
            s.replace(QRegularExpression(QString(j.key())), j.value());
        }
    }
    path = s;
}

void LSystem::populateOps(){
    Rule leftBracketPtr = leftBracket;
    Rule rightBracketPtr = rightBracket;
    Rule FPtr = F;
    Rule minusSignPtr = minusSign;
    Rule plusSignPtr = plusSign;
    Rule XPtr = X;

    charToDrawingOperation['['] = leftBracketPtr;
    charToDrawingOperation[']'] = rightBracketPtr;
    charToDrawingOperation['F'] = FPtr;
    charToDrawingOperation['-'] = minusSignPtr;
    charToDrawingOperation['+'] = plusSignPtr;
    charToDrawingOperation['X'] = XPtr;
}

void LSystem::leftBracket(){
    activeTurtle.increaseDepth();
    Turtle turtleCopy = Turtle(activeTurtle);
    turtleStack.push(turtleCopy);
}

void LSystem::rightBracket(){
    activeTurtle = turtleStack.pop();
}

void LSystem::F(){
    activeTurtle.moveForward();
}

void LSystem::minusSign(){
    activeTurtle.turnLeft();
}

void LSystem::plusSign(){
    activeTurtle.turnRight();
}

void LSystem::X(){}

void LSystem::printPath(){
    std::cout << path.toUtf8().constData() << std::endl;
}

float LSystem::rand01(){
    float n = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    return n;
}

// LSYSTEM END
// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// RIVERDELTA START

RiverDelta::RiverDelta(glm::vec2 pos, glm::vec2 heading) :
    LSystem(pos, heading){
    this->branchProb = 0.05f;
}

RiverDelta::~RiverDelta(){}

void RiverDelta::generatePath(int n, QString seed){
    // add generative rules
    addRule('X', "[-FX]+FX");
    // string generation
    QString s = seed;
    float r;
    for(int i = 0; i < n; i++){
        QHashIterator<QChar, QString> j(ruleSet);
        while (j.hasNext()) {
            j.next();

            std::vector<int> indices;
            for (int k = 0; k < s.length(); ++k){
                if (s[k] == j.key()){
                    indices.push_back(k);
                }
            }
            int deltaA = j.value().length();
            int deltaB = 6;
            int multiplierA = 0;
            int multiplierB = 0;
            for (unsigned int k = 0; k < indices.size(); ++k){
                r = rand01();
                if (r < this->branchProb){
                    s.replace(indices[k] + multiplierA*(deltaA - 1) + multiplierB*(deltaB - 1), 1, j.value());
                    multiplierA++;
                }
                else{
                    s.replace(indices[k] + multiplierA*(deltaA - 1) + multiplierB*(deltaB - 1), 1, "+--+FX");
                    multiplierB++;
                }
            }
        }
    }

    path = s;
}


void RiverDelta::minusSign(){
    activeTurtle.turnLeft();
    if (activeTurtle.orientation[1] < 0) {  // keep the turtle moving North
        activeTurtle.orientation = glm::vec2(-1.0f,0);
    }
}
void RiverDelta::plusSign(){
    activeTurtle.turnRight();
    if (activeTurtle.orientation[1] < 0) {  // keep the turtle moving North
        activeTurtle.orientation = glm::vec2(1.0f,0);
    }
}

// RIVERDELTA END
// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// RIVER START

River::River(glm::vec2 pos, glm::vec2 heading) :
    LSystem(pos, heading){
    this->branchProb = 0.01f;
}

River::~River(){}

void River::generatePath(int n, QString seed){
    // add generative rules
    addRule('X', "[+FX]+--+FY");
    addRule('Y', "[-FY]+--+FX");
    // string generation
    QString s = seed;
    float r;
    for(int i = 0; i < n; i++){
        QHashIterator<QChar, QString> j(ruleSet);
        while (j.hasNext()) {
            j.next();

            std::vector<int> indices;
            for (int k = 0; k < s.length(); ++k){
                if (s[k] == j.key()){
                    indices.push_back(k);
                }
            }
            int deltaA = j.value().length();
            int deltaB = 6;
            int multiplierA = 0;
            int multiplierB = 0;
            for (unsigned int k = 0; k < indices.size(); ++k){
                r = rand01();
                if (r < this->branchProb){
                    s.replace(indices[k] + multiplierA*(deltaA - 1) + multiplierB*(deltaB - 1), 1, j.value());
                    multiplierA++;
                }
                else{
                    s.replace(indices[k] + multiplierA*(deltaA - 1) + multiplierB*(deltaB - 1), 1, "+--+FX");
                    multiplierB++;
                }
            }
        }
    }

    path = s;
}

void River::leftBracket(){
    Turtle turtleCopy = Turtle(activeTurtle);
    turtleStack.push(turtleCopy);
    activeTurtle.increaseDepth();
}
void River::minusSign(){    // keep the turtle moving South
    activeTurtle.turnLeft();
    if (activeTurtle.orientation[1] > 0) {
        activeTurtle.orientation = glm::vec2(1.0f,0);
    }
}
void River::plusSign(){     // keep the turtle moving South
    activeTurtle.turnRight();
    if (activeTurtle.orientation[1] > 0) {
        activeTurtle.orientation = glm::vec2(-1.0f,0);
    }
}

// RIVER END
// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// TURTLE START

Turtle::Turtle() :
    position(glm::vec2(0,0)), orientation(glm::vec2(0,1)), depth(1) {}

Turtle::Turtle(glm::vec2 pos, glm::vec2 heading) :
    position(pos), orientation(heading), depth(1)
{}

Turtle::Turtle(const Turtle& t) :
    position(t.position), orientation(t.orientation), depth(t.depth)
{}

Turtle::~Turtle(){}

void Turtle::turnLeft(){
    float angle = PI_4 - randAngle();
    orientation = glm::vec2(orientation[0] * cosf(angle) - orientation[1] * sinf(angle),
                            orientation[0] * sinf(angle) + orientation[1] * cosf(angle));
}

void Turtle::turnRight(){
    float angle = -PI_4 + randAngle();
    orientation = glm::vec2(orientation[0] * cosf(angle) - orientation[1] * sinf(angle),
                            orientation[0] * sinf(angle) + orientation[1] * cosf(angle));
}

void Turtle::moveForward(){
    glm::vec2 change = orientation * F_DISTANCE;
    position += change;
}

void Turtle::increaseDepth(){
    depth++;
}

void Turtle::decreaseDepth(){
    depth--;
}

void Turtle::printCoordinates() const{
    std::cout << "(" << position[0] << ", " << position[1] << ")" << std::endl;
}

void Turtle::printOrientation() const{
    std::cout << "(" << orientation[0] << ", " << orientation[1] << ")" << std::endl;
}

float Turtle::randAngle() const{
    float n = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    return n * PI_4;
}

// TURTLE END
// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// LINESEGMENT2D START

LineSegment2D::LineSegment2D(glm::vec2& start, glm::vec2& end)
    : start(start), end(end), slope((end[1] - start[1]) / (end[0] - start[0])) {}

LineSegment2D::~LineSegment2D() {}

bool LineSegment2D::intersectAt(const int row, float* xIntersect){
    float y = (float) row;
    // no x intersection if y is outside the vertical bounds of the segment
    float yUpperBound = end[1] > start[1] ? end[1] : start[1];
    float yLowerBound = end[1] > start[1] ? start[1] : end[1];
    if (y > yUpperBound || y < yLowerBound){
        return false;
    }
    // no x intersection if edge is horizontal
    if(slope == 0){
        return false;
    }
    // if the edge is vertical, the x-intersection is known
    if(start[0] == end[0]){
        *xIntersect = start[0];
        return true;
    }
    // doesn't matter which vertex the intersection point is calculated from
    float x = (y - start[1]) / slope + start[0];
    *xIntersect = x;
    return true;
}
