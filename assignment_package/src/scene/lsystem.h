#ifndef LSYSTEM_H
#define LSYSTEM_H

#include <la.h>
#include <QString>
#include <QChar>
#include <QHash>
#include <QStack>
#include <QRegularExpression>
#include <time.h>

const float PI_4 = 0.78539816;
const float PI_6 = 0.52359877;
const float PI_8 = 0.39269908;
const int MAX_DEPTH_DELTA = 60; // number of steps to generate river delta
const int MAX_DEPTH_RIVER = 60; // number of steps to generate main river
const int TREE_DEPTH = 5 ;      // number of steps to generate tree

class LSystem;

typedef void (LSystem::*Rule)(void);

class Turtle
{
public:
    glm::vec2 position;     // starting position of turtle
    glm::vec2 orientation;  // staring orientation of turtle
    int depth;              // recursion depth
    float fDistance;        // distance turtle moves on 'F'

    Turtle();
    Turtle(glm::vec2 pos, glm::vec2 heading, float fDistance);
    Turtle(const Turtle& t);
    ~Turtle();

    void turnLeft();
    void turnRight();
    void moveForward();
    void increaseDepth();
    void decreaseDepth();

    void printCoordinates() const;  // prints turtle's position
    void printOrientation() const;  // prints turtle's orientation vector
private:
    float randAngle() const;        // returns an angle between 0 and PI/4 radians
};

class LSystem
{
public:
    QString path;
    Turtle activeTurtle;
    QStack<Turtle> turtleStack;
    QHash<QChar, QString> ruleSet;              // char -> string map replacement rules for generating turtle path instructions
    QHash<QChar, Rule> charToDrawingOperation;  // maps characters to LSystem functions controlling this turtle
    float branchProb;                           // probability of branch generation

    LSystem(glm::vec2 pos, glm::vec2 heading, float fDistance);
    virtual ~LSystem();

    virtual void generatePath(int n, QString seed); // generates path to be traversed by turtle (n branching events)
    virtual void populateOps();                     // populates charToDrawingOperation hash
    void printPath();                               // prints path string
    float rand01();                                 // returns a random number between 0 and 1
protected:
    void addRule(QChar chr, QString str);           // add a replacement rule for path string generation

    virtual void leftBracket();
    virtual void rightBracket();
    virtual void F();
    virtual void minusSign();
    virtual void plusSign();
    virtual void X();
};

class River : public LSystem{                       // An LSystem River
                                                    // FX -> [-FY]+--+FX
                                                    // FY -> [-FX]+--+FY
public:
    River(glm::vec2 pos, glm::vec2 heading, float fDistance);
    virtual ~River();

    void generatePath(int n, QString seed) override;
    void leftBracket() override;
    void minusSign() override;
    void plusSign() override;
};

class RiverDelta : public LSystem{                  // An LSystem River Delta
                                                    // FX -> [-FX]+FX
public:
    RiverDelta(glm::vec2 pos, glm::vec2 heading, float fDistance);
    virtual ~RiverDelta();

    void generatePath(int n, QString seed) override;
    void minusSign() override;
    void plusSign() override;
};

class LineSegment2D{            // river is drawn by testing intersections with 2D line segments traced by turtle movement
public:
    glm::vec2 start;
    glm::vec2 end;
    float slope;

    LineSegment2D(glm::vec2& start, glm::vec2& end);
    ~LineSegment2D();

    bool intersectAt(const int row, float* xIntersect); // line-row intersection test
};

class Tree : public LSystem{                        // An LSystem Tree
public:
    Tree(glm::vec2 pos, float fDistance);
    virtual ~Tree();
};






#endif // LSYSTEM_H
