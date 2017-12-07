#pragma once
#include <QList>
#include <la.h>

#include "drawable.h"
#include <unordered_map>

#include<camera.h>
#include<set>
#include<QMutex>

#include "lsystem.h"
#include <time.h>


// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.

class TerrainType;

enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, LAVA, WATER, WOOD, LEAF, BEDROCK, ICE, SAND
};

class Chunk : public Drawable
{
public:
    Chunk(OpenGLContext* context);
    BlockType getBlockType(int x, int y, int z) const;
    BlockType& getBlockType(int x, int y, int z);

    void create() override;
    GLenum drawMode() override;

    // Flag for whether this Chunk's data is in GPU
    bool isCreated;
    // Flag for whether this Chunk's data has been populated
    bool hasData;

    // Passes the VBO data to the GPU
    void createVBO();

    // VBO data
    // everything is the interleaved data of positions, normals, and UVs
    std::vector<glm::vec4> everything ;
    // indices is the indices info
    std::vector<GLuint> indices;



private:
    BlockType block_array[65536]; // 16 x 256 x 16 (x by y by z)
};

class Terrain
{
public:
    Terrain(OpenGLContext* context,Camera*,QMutex*);
    ~Terrain();
    QMutex* mutex;

    TerrainType* terrainType;                           // pointer to terrain type set in initialzeGL
    LSystem* lsys;                                      // pointer to LSystem for river generation
    void setTerrainType(TerrainType* t);
    void setLSystem(LSystem* l);
    Chunk* createScene(int x, int z);

    Camera* mp_camera;

    // Data of Chunks that need to be added to map
    // They are added in MyGL's timerUpdate
    // Need to make more robust so multiple threads work with it
    // Array of Chunks that need to be added to map
    std::vector<Chunk*> chunksGonnaDraw;
    // Array of keys for the Chunks that need to be added to map
    std::vector<uint64_t> keysGonnaDraw;

    // river stuff
    void createRivers();                                // create the rivers in this terrain
    // tree stuff
    void createForest();                                // populates the world with randomly distributed trees
    // cave stuff
    void excavateCave();

    glm::ivec3 dimensions;
    glm::ivec3 chunk_dimensions;

    // Maps Chunk Position to the Chunk
    // Chunk Position obtained through getChunkPosition
    std::unordered_map<uint64_t, Chunk*> chunk_map;

    BlockType getBlockAt(int x, int y, int z) const;       // Given a world-space coordinate (which may have negative
                                                           // values) return the block stored at that point in space.
    void setBlockAt(int x, int y, int z, BlockType t);     // Given a world-space coordinate (which may have negative
                                                           // values) set the block at that point in space to the
                                                           // given type.

    Chunk* getChunk(int x, int z) const;

    // Converts a world coordinate to Chunk coordinate
    int getChunkPosition1D(int x) const;

    // Uses camera position to loop through visible Chunks to player
    // Updates their VBO data if needed
    // Invoked by thread
    void drawScene();

    // Does same as setBlockAt
    // Originally it also updated VBO, but that is handled elsewhere now
    void addBlockAt(int x, int y, int z, BlockType t);

    // Updates a single Chunk's VBO
    // x and z are Chunk coordinates
    void updateChunkVBO(int x, int z);

    // Loops through all Chunks and updates their VBOs
    // Used during game initialization
    //void updateAllVBO();

    uint64_t convertToInt(int x, int z) const;
    void splitInt(uint64_t in, int* x, int* z) const;

private:
    OpenGLContext* context; // To pass on to Chunks

    std::unordered_map<char, glm::vec4> color_map; // Map of char (BlockType) to a color
    std::unordered_map<char, glm::vec4> normal_vec_map; // Map of char (direction) to its normal vector
    std::unordered_map<char, glm::vec4> draw_start_map; // Map of char (direction) to a starting point for procedural square generation
    std::unordered_map<char, std::vector<glm::vec4>> block_uv_map; // Map of char (BlockType) to a list of UVs

    // Const variables that are used over and over again in VBO creation
    const glm::vec3 offset;

    int getChunkLocalPosition1D(int x) const;

    // Adds a square to the VBOs
    void addSquare(glm::vec3 *pos,
                   std::vector<glm::vec4>* everything,
                   std::vector<GLuint>* indices,
                   char direction, BlockType block);

    // Converts global x, z to which Chunk those coordinates are in
    glm::ivec2 getChunkPosition(int x, int z) const;

    // Converts global x, y, z coordinates to a local position within Chunk
    // Does not find which Chunk this position belongs to
    glm::ivec3 getChunkLocalPosition(int x, int y, int z) const;

    // river stuff
    void traceRiverPath(const std::vector<int>& depths);// sets river cubes in scene
    // forest stuff
    void drawTree(glm::ivec2 pos);                      // draws a tree at position
    // cave stuff

    // gets height of terrain at (x, z) pos
    int getHeightAt(glm::ivec2 pos);
};

class TerrainType{
public:
    TerrainType(int octaves, float persistance, float resolution, float dampen);
    virtual ~TerrainType();

    int getOctaves() const;
    float getPersistance() const;
    float getResolution() const;
    float getDampen() const;

    virtual float fbm(const float x,
              const float z,
              const float persistance,
              const int octaves) const;             // returns a pseudorandom number between 0 and 1 for 2D FBM noise
    virtual float fbm3D(const float x,
              const float y,
              const float z,
              const float persistance,
              const int octaves) const;             // returns a pseudorandom number between 0 and 1 for 3D FBM noise
    virtual int mapToHeight(const float val) const; // maps [0, 1] -> [128, 255]

protected:
    int octaves;
    float persistance;
    float resolution;
    float dampen;

    // For procedural terrain
    float rand(const glm::vec2 n) const;        // pseudorandom number generator for 2D FBM noise
                                                // returns a pseudorandom value between -1 and 1
    float rand3D(const glm::vec3 n) const;      // pseudorandom number generator for 3D FBM noise
                                                // returns a pseudorandom value between -1 and 1
    float interpNoise2D(const float x,
                        const float z) const;   // 2D noise interpolation function for smooth FBM noise
    float interpNoise3D(const float x,
                        const float y,
                        const float z) const;   // 3D noise interpolation function for smooth FBM noise
};

class Highland : public TerrainType{    // generates a "highland" terrain
public:
    Highland();
    virtual ~Highland();
};

class Foothills : public TerrainType{   // generates "foothill" terrain
public:
    Foothills();
    virtual ~Foothills();
};

class Cave : public TerrainType{
public:
    Cave(glm::ivec3 pos);
    virtual ~Cave();
    glm::ivec3 pos;

    void step();                        // moves the point around which the cave is excavated by one step

private:
    float mapToAngle(float num);        // maps [-1, 1] -> [0, 360]
    glm::ivec2 mapToXZOffset(float angle);
    int mapToYOffset(float angle);
};



