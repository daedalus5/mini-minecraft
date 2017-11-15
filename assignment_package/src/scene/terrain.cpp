#include <scene/terrain.h>

#include <scene/cube.h>

/////
// constructors
/////

Terrain::Terrain() : dimensions(64, 256, 64)
{}

/////
// public functions
/////

BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    // TODO: Make this work with your new block storage!
    return m_blocks[x][y][z];
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    // TODO: Make this work with your new block storage!
    m_blocks[x][y][z] = t;
}

/////
// terrain generation functions
/////
void Terrain::CreateTestScene()
{
    // Create the basic terrain floor
    for(int x = 0; x < 64; ++x)
    {
        for(int z = 0; z < 64; ++z)
        {
            for(int y = 127; y < 256; ++y)
            {
                if(y <= 128)
                {
                    if((x + z) % 2 == 0)
                    {
                        m_blocks[x][y][z] = STONE;
                    }
                    else
                    {
                        m_blocks[x][y][z] = DIRT;
                    }
                }
                else
                {
                    m_blocks[x][y][z] = EMPTY;
                }
            }
        }
    }
    // Add "walls" for collision testing
    for(int x = 0; x < 64; ++x)
    {
        m_blocks[x][129][0] = GRASS;
        m_blocks[x][130][0] = GRASS;
        m_blocks[x][129][63] = GRASS;
        m_blocks[0][130][x] = GRASS;
    }
    for(int y = 129; y < 140; ++y)
    {
        m_blocks[32][y][32] = GRASS;
    }
}

void Terrain::CreateHighland(){
//    for(int x = 0; x < 64; ++x){
//        for(int z = 0; z < 64; ++z){
//            for(int y = 0; y < 127; ++y){
//                m_blocks[x][y][z] = STONE;
//            }
//        }
//    }
    for(int x = 0; x < 64; ++x){
        for(int z = 0; z < 64; ++z){
            float persistance = 0.4f;
            int octaves = 4;
            float greyscale = fbm(x + 0.5f, z + 0.5f, persistance, octaves);
            int height = mapToHeight(greyscale);
//            for(int y = 128; y < height; ++y){
//                m_blocks[x][y][z] = DIRT;
//            }
            m_blocks[x][height][z] = GRASS;
        }
    }
}

/////
// private functions
/////

float Terrain::rand(const glm::vec2 n) const{
    return (glm::fract(sin(glm::dot(n, glm::vec2(12.9898, 4.1414))) * 43758.5453));
}

float Terrain::interpNoise2D(const float x, const float z) const{
    float intX = glm::floor(x);
    float fractX = glm::fract(x);
    float intZ = glm::floor(z);
    float fractZ = glm::fract(z);

    float v1 = rand(glm::vec2(intX, intZ));
    float v2 = rand(glm::vec2(intX + 1, intZ));
    float v3 = rand(glm::vec2(intX, intZ + 1));
    float v4 = rand(glm::vec2(intX + 1, intZ + 1));

    // interpolate for smooth transitions
    float i1 = glm::mix(v1, v2, fractX);
    float i2 = glm::mix(v3, v4, fractX);
    return glm::mix(i1, i2, fractZ);
}

float Terrain::fbm(const float x, const float z, const float persistance, const int octaves) const{
    float total = 0.0f;
    float c = 20.0f;

    for(int i = 0; i < octaves; i++){
        float freq = pow(2.0f, i);
        float amp = pow(persistance, i);

        total += amp * interpNoise2D(x * freq / c, z * freq / c);
    }
    float a = 1 - persistance;  // normalization

    return a * (1.0f + total) / 2.0f;  // normalized, pseudorandom number between 0 and 1

}

int Terrain::mapToHeight(const float val) const{
    float dampen = 0.2;
    return 128 + glm::floor(val * 128 * dampen);
}
