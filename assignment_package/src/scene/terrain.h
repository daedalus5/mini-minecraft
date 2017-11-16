#pragma once
#include <QList>
#include <la.h>

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, LAVA
};

class Terrain
{
public:
    Terrain();
    BlockType m_blocks[64][256][64];                       // A 3D list of the blocks in the world.
                                                           // You'll need to replace this with a far more
                                                           // efficient system of storing terrain.
    void CreateTestScene();     // base code default
    void CreateHighland();      // a Scottish "highland"

    glm::ivec3 dimensions;

    BlockType getBlockAt(int x, int y, int z) const;       // Given a world-space coordinate (which may have negative
                                                           // values) return the block stored at that point in space.
    void setBlockAt(int x, int y, int z, BlockType t);     // Given a world-space coordinate (which may have negative
                                                           // values) set the block at that point in space to the
                                                           // given type.
private:
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
