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
    EMPTY, GRASS, DIRT, STONE
};

// Copied from
// https://stackoverflow.com/questions/9047612/glmivec2-as-key-in-unordered-map
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

    glm::ivec3 dimensions;

    void create() override;
    GLenum drawMode() override;

    void createVBO(std::vector<glm::vec4> &positions,
                      std::vector<glm::vec4> &normals,
                      std::vector<glm::vec4> &colors,
                      std::vector<GLuint> &indices);

private:
    int size;
    BlockType block_array[65536]; // 16 x 256 x 16 (x by y by z)
    glm::ivec3 getPosition(int i) const;
//    void addSquare(glm::vec4 pos, glm::vec4 normal, glm::vec4 color,
//                   glm::vec4 squareStart,
//                   std::vector<glm::vec4>& positions,
//                   std::vector<glm::vec4>& normals,
//                   std::vector<glm::vec4>& colors,
//                   std::vector<GLuint>& indices);
};

class Terrain
{
public:
    Terrain(OpenGLContext* context);
    BlockType m_blocks[64][256][64];                    // A 3D list of the blocks in the world.
                                                           // You'll need to replace this with a far more
                                                           // efficient system of storing terrain.
    std::unordered_map<glm::ivec2, Chunk*, KeyFuncs, KeyFuncs> chunk_map;

    void CreateTestScene();

    glm::ivec3 dimensions;

    BlockType getBlockAt(int x, int y, int z) const;   // Given a world-space coordinate (which may have negative
                                                           // values) return the block stored at that point in space.
    void setBlockAt(int x, int y, int z, BlockType t); // Given a world-space coordinate (which may have negative
                                                           // values) set the block at that point in space to the
                                                           // given type.

    void addBlockAt(int x, int y, int z, BlockType t); // does almost same as setBlockAt, but also updates VBO
    void destroyBlockAt(int x, int y, int z);
    void updateChunkVBO(int x, int z);
    void updateAllVBO();
    Chunk* getChunk(int x, int z);

private:

    OpenGLContext* context; // To pass on to Chunks

    std::map<BlockType, glm::vec4> color_map;
    void addSquare(glm::vec4 pos, glm::vec4 normal, glm::vec4 color,
                   glm::vec4 squareStart,
                   std::vector<glm::vec4>& positions,
                   std::vector<glm::vec4>& normals,
                   std::vector<glm::vec4>& colors,
                   std::vector<GLuint>& indices);


    glm::ivec2 getChunkPosition(int x, int z) const;
    glm::ivec3 getChunkLocalPosition(int x, int y, int z) const;
    //Need to convert xyz coordinates to chunk coordinates
    //And update a block in there?


    //Store a collection of chunks.
    //Store in a map. Key is world space position, just x and z coordinates. Value is chunk
    //std::unordered_map
};
