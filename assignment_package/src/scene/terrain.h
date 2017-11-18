#pragma once
#include <QList>
#include <la.h>

#include "drawable.h"
#include <unordered_map>

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, LAVA
};

// Copied from
// https://stackoverflow.com/questions/9047612/glmivec2-as-key-in-unordered-map
// Used for hashing glm::ivec2
struct KeyFuncs
{
    size_t operator()(const glm::ivec2& k) const
    {
        return std::hash<int>()(k.x) ^ std::hash<int>()(k.y);
    }

    bool operator()(const glm::ivec2& a, const glm::ivec2& b) const
    {
            return a.x == b.x && a.y == b.y;
    }
};

class Chunk : public Drawable
{
public:
    Chunk(OpenGLContext* context);
    BlockType getBlockType(int x, int y, int z) const;
    BlockType& getBlockType(int x, int y, int z);

    void create() override;
    GLenum drawMode() override;

    void createVBO(std::vector<glm::vec4> &everything,
                      std::vector<GLuint> &indices);

private:
    BlockType block_array[65536]; // 16 x 256 x 16 (x by y by z)
};

class Terrain
{
public:
    Terrain(OpenGLContext* context);
    ~Terrain();
    //BlockType m_blocks[64][256][64];                    // A 3D list of the blocks in the world.
                                                           // You'll need to replace this with a far more
                                                           // efficient system of storing terrain.

    void CreateTestScene();
    // Returns whether a Chunk was made
    Chunk* CreateHighland(int x, int z);      // a Scottish "highland"

    glm::ivec3 dimensions;
    glm::ivec3 chunk_dimensions;

    BlockType getBlockAt(int x, int y, int z) const;       // Given a world-space coordinate (which may have negative
                                                           // values) return the block stored at that point in space.
    void setBlockAt(int x, int y, int z, BlockType t);     // Given a world-space coordinate (which may have negative
                                                           // values) set the block at that point in space to the
                                                           // given type.

    Chunk* getChunk(int x, int z) const;
    int getChunkPosition1D(int x) const;

    // Does almost same as setBlockAt, but also updates VBO
    void addBlockAt(int x, int y, int z, BlockType t);
    //void destroyBlockAt(int x, int y, int z);

    // Updates a single Chunk's VBO
    // x and z are world space coordinates that belong in the Chunk
    void updateChunkVBO(int x, int z);
    // Loops through all Chunks and updates their VBOs
    // Used during game initialization
    void updateAllVBO();

    uint64_t convertToInt(int x, int z) const;
    void splitInt(uint64_t in, int* x, int* z) const;

private:

    OpenGLContext* context; // To pass on to Chunks
    std::map<BlockType, glm::vec4> color_map; // Map of BlockType to a color

    // Maps Chunk Position to the Chunk
    // Chunk Position obtained through getChunkPosition
    std::unordered_map<uint64_t, Chunk*> chunk_map;

    // Adds a square to the VBOs
    void addSquare(glm::vec3 *pos, glm::vec4 *normal, glm::vec4 *color,
                   glm::vec4 *squareStart,
                   std::vector<glm::vec4>* everything,
                   std::vector<GLuint>* indices);

    int getChunkLocalPosition1D(int x) const;


    // Converts global x, z to which Chunk those coordinates are in
    glm::ivec2 getChunkPosition(int x, int z) const;
    // Converts global x, y, z coordinates to a local position within Chunk
    // Does not find which Chunk this position belongs to
    glm::ivec3 getChunkLocalPosition(int x, int y, int z) const;

    // For procedural terrain
    float rand(const glm::vec2 n) const;    // pseudorandom number generator for FBM noise
                                            // returns a pseudorandom value between -1 and 1
    float interpNoise2D(const float x,
                        const float z) const;  // 2D noise interpolation function for smooth FBM noise
    float fbm(const float x,
              const float z,
              const float persistance,
              const int octaves) const;     // returns a pseudorandom number between 0 and 1 for FBM noise
    int mapToHeight(const float val) const; // maps [0, 1] -> [128, 255]
};
