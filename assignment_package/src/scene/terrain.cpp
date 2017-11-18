#include <scene/terrain.h>

Chunk::Chunk(OpenGLContext* context)
    : Drawable(context), block_array({EMPTY})
{
}

BlockType Chunk::getBlockType(int x, int y, int z) const {
    return block_array[x + 16 * y + 4096 * z];
}

BlockType& Chunk::getBlockType(int x, int y, int z) {
    return block_array[x + 16 * y + 4096 * z];
}

void Chunk::createVBO(std::vector<glm::vec4> &everything,
                      std::vector<GLuint> &indices)
{
    count = indices.size();

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    generateEve();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufEve);
    context->glBufferData(GL_ARRAY_BUFFER, everything.size() * sizeof(glm::vec4), everything.data(), GL_STATIC_DRAW);
}

void Chunk::create() {
    // no-op
}

GLenum Chunk::drawMode()
{
    return GL_TRIANGLES;
}

// CHUNK END
// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// TERRAIN START

Terrain::Terrain(OpenGLContext* in_context) : dimensions(64, 256, 64),
    chunk_map(std::unordered_map<uint64_t, Chunk*>()),
    color_map(std::map<BlockType, glm::vec4>()), chunk_dimensions(16, 256, 16),
    context(in_context),
    x_normal(glm::vec4(1, 0, 0, 0)),
    x_normal_neg(-x_normal),
    y_normal(glm::vec4(0, 1, 0, 0)),
    y_normal_neg(-y_normal),
    z_normal(glm::vec4(0, 0, 1, 0)),
    z_normal_neg(-z_normal),

    start_x(glm::vec4(0.5, 0.5, 0.5, 1)),
    start__x(glm::vec4(-0.5, 0.5, 0.5, 1)),
    start_y(glm::vec4(0.5, 0.5, 0.5, 1)),
    start__y(glm::vec4(0.5, -0.5, 0.5, 1)),
    start_z(glm::vec4(0.5, 0.5, 0.5, 1)),
    start__z(glm::vec4(0.5, 0.5, -0.5, 1))
{
    color_map[GRASS] = glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f;
    color_map[DIRT] = glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f;
    color_map[STONE] = glm::vec4(0.5, 0.5, 0.5, 1);
    color_map[LAVA] = glm::vec4(207.f, 16.f, 32.f, 255.f) / 255.f;
}

Terrain::~Terrain() {
    for ( auto it = chunk_map.begin(); it != chunk_map.end(); ++it ) {
        it->second->destroy();
        delete it->second;
    }
}

BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if (y < 0 || y > 255) {
        return EMPTY;
    }

    auto index = chunk_map.find(convertToInt(getChunkPosition1D(x), getChunkPosition1D(z)));
    if (index != chunk_map.end()) {
        Chunk* ch = index->second;
        return ch->getBlockType(getChunkLocalPosition1D(x), y, getChunkLocalPosition1D(z));
    }
    return EMPTY;
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if (y < 0 || y > 255) {
        return;
    }

    uint64_t chunk_pos = convertToInt(getChunkPosition1D(x), getChunkPosition1D(z));
    Chunk* ch;
    auto index = chunk_map.find(chunk_pos);
    if (index == chunk_map.end()) {
        ch = new Chunk(context);
        chunk_map[chunk_pos] = ch;
    } else {
        ch = chunk_map[chunk_pos];
    }
    ch->getBlockType(getChunkLocalPosition1D(x), y, getChunkLocalPosition1D(z)) = t;
}

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
                        setBlockAt(x, y, z, STONE);
                    }
                    else
                    {
                        setBlockAt(x, y, z, DIRT);
                    }
                }
                else
                {
                    setBlockAt(x, y, z, EMPTY);
                }
            }

            for (int y = 0; y < 127; y++) {
                setBlockAt(x, y, z, EMPTY);
            }
        }
    }
    // Add "walls" for collision testing
    for(int x = 0; x < 64; ++x)
    {
        setBlockAt(x, 129, 0, GRASS);
        setBlockAt(x, 130, 0, GRASS);
        setBlockAt(x, 129, 63, GRASS);
        setBlockAt(0, 130, x, GRASS);
    }
    for(int y = 129; y < 140; ++y)
    {
        setBlockAt(32, y, 32, GRASS);
    }
}

void Terrain::addBlockAt(int x, int y, int z, BlockType t) {
    setBlockAt(x, y, z, t);
    updateChunkVBO(getChunkPosition1D(x), getChunkPosition1D(z));

    // Update neighboring Chunks
    // They may need to add/delete a wall
    updateChunkVBO(getChunkPosition1D(x + 1), getChunkPosition1D(z));
    updateChunkVBO(getChunkPosition1D(x - 1), getChunkPosition1D(z));
    updateChunkVBO(getChunkPosition1D(x), getChunkPosition1D(z + 1));
    updateChunkVBO(getChunkPosition1D(x), getChunkPosition1D(z - 1));
}

// Changed to use chunk position
Chunk* Terrain::getChunk(int x, int z) const {

    auto index = chunk_map.find(convertToInt(x, z));
    if (index != chunk_map.end()) {
        return index->second;
    }
    return nullptr;
}

int Terrain::getChunkPosition1D(int x) const {
    int temp = x / 16;
    if (x < 0) {
        if (x % 16 != 0) {
            temp = temp - 1;
        }
    }
    return temp;
}

int Terrain::getChunkLocalPosition1D(int x) const {
    int temp = x % 16;
    if (x < 0) {
        if (temp != 0) {
            temp = temp + 16;
        }
    }
    return temp;
}

glm::ivec2 Terrain::getChunkPosition(int x, int z) const {
    glm::ivec2 chunk_pos = glm::ivec2(x / 16, z / 16);
    if (x < 0) {
        if (x % 16 != 0) {
            chunk_pos[0] = chunk_pos[0] - 1;
        }
    }
    if (z < 0) {
        if (z % 16 != 0) {
            chunk_pos[1] = chunk_pos[1] - 1;
        }
    }
    return chunk_pos;
}

glm::ivec3 Terrain::getChunkLocalPosition(int x, int y, int z) const {
    glm::ivec3 local_pos = glm::ivec3(x % 16, y, z % 16); // position within chunk
    if (x < 0) {
        if (local_pos[0] != 0) {
            local_pos[0] = local_pos[0] + 16;
        }
    }
    if (z < 0) {
        if (local_pos[2] != 0) {
            local_pos[2] = local_pos[2] + 16;
        }
    }
    return local_pos;
}

// x and z are the chunk's world position
// maybe take in the chunk's numbers instead? yes
void Terrain::updateChunkVBO(int x, int z) {

    auto index = chunk_map.find(convertToInt(x, z));
    if (index == chunk_map.end()) {
        return;
    }
    Chunk* ch = index->second;

    std::vector<glm::vec4> everything = std::vector<glm::vec4>();
    std::vector<GLuint> indices = std::vector<GLuint>();

    BlockType block;
    glm::vec3 world_pos;
    glm::vec4 col;

    // Chunk neighbors
    Chunk* neighbor_x = getChunk(x + 1, z);
    Chunk* neighbor__x = getChunk(x - 1, z);
    Chunk* neighbor_z = getChunk(x, z + 1);
    Chunk* neighbor__z = getChunk(x, z - 1);

    for (int i = 0; i < chunk_dimensions[0]; i++) {
        for (int j = 0; j < chunk_dimensions[1]; j++) {
            for (int k = 0; k < chunk_dimensions[2]; k++) {

                block = ch->getBlockType(i, j, k);

                if (block != EMPTY) {

                    world_pos = glm::vec3(i, j, k)
                            + glm::vec3(x * chunk_dimensions[0], 0, z * chunk_dimensions[2]);


                    auto index = color_map.find(block);
                    if (index == color_map.end()) {
                        break;
                    } else {
                        col = index->second;
                    }

                    // Check neighboring Chunk in x direction
                    if (i == 0) {
                        if (neighbor__x != nullptr) {
                            if (neighbor__x->getBlockType(chunk_dimensions[0] - 1, j, k) == EMPTY) {
                                addSquare(&world_pos, &x_normal_neg, &col, &start__x, &everything, &indices);
                            }
                        }
                    } else if (ch->getBlockType(i - 1, j, k) == EMPTY) {
                        // Check neighboring x within this chunk
                        addSquare(&world_pos, &x_normal_neg, &col, &start__x, &everything, &indices);
                    }

                    if (i == chunk_dimensions[0] - 1) {
                        if (neighbor_x != nullptr) {
                            if (neighbor_x->getBlockType(0, j, k) == EMPTY) {
                                addSquare(&world_pos, &x_normal, &col, &start_x, &everything, &indices);
                            }
                        }
                    }
                    else if (ch->getBlockType(i + 1, j, k) == EMPTY) {
                        addSquare(&world_pos, &x_normal, &col, &start_x, &everything, &indices);
                    }


                    // Check neighboring y
                    if (j < 255 && ch->getBlockType(i, j + 1, k) == EMPTY) {
                        addSquare(&world_pos, &y_normal, &col, &start_y, &everything, &indices);
                    }
                    if (j > 0 && ch->getBlockType(i, j - 1, k) == EMPTY) {
                        addSquare(&world_pos, &y_normal_neg, &col, &start__y, &everything, &indices);
                    }

                    // Check neighboring Chunk in z direction
                    if (k == 0) {
                        if (neighbor__z != nullptr) {
                            if (neighbor__z->getBlockType(i, j, chunk_dimensions[2] - 1) == EMPTY) {
                                addSquare(&world_pos, &z_normal_neg, &col, &start__z, &everything, &indices);
                            }
                        }
                    } else if (ch->getBlockType(i, j, k - 1) == EMPTY) {
                        addSquare(&world_pos, &z_normal_neg, &col, &start__z, &everything, &indices);
                    }

                    if (k == chunk_dimensions[2] - 1) {
                        if (neighbor_z != nullptr) {
                            if (neighbor_z->getBlockType(i, j, 0) == EMPTY) {
                                addSquare(&world_pos, &z_normal, &col, &start_z, &everything, &indices);
                            }
                        }
                    } else if (ch->getBlockType(i, j, k + 1) == EMPTY) {
                        addSquare(&world_pos, &z_normal, &col, &start_z, &everything, &indices);
                    }

                }

            }
        }
    }
    ch->createVBO(everything, indices);
}

void Terrain::updateAllVBO() {
    for ( auto it = chunk_map.begin(); it != chunk_map.end(); ++it ) {
        //updateChunkVBO(16 * it->first[0], 16 * it->first[1]);
    }
}

void Terrain::addSquare(glm::vec3* pos, const glm::vec4* normal, glm::vec4* color,
                      const glm::vec4* squareStart,
                      std::vector<glm::vec4> *everything,
                      std::vector<GLuint> *indices) {

    // grab size of positions
    int index = everything->size() / 3;
    glm::vec3 offset = glm::vec3(0.5, 0.5, 0.5);
    glm::vec3 normal3 = glm::vec3(*normal);

    for (int k = 0; k < 4; k++) {
        // Rotate by 90 degrees 4 times
        // Push them into positions vector
        // Push normals
        // Push colors
        everything->push_back(glm::translate(glm::mat4(), *pos + offset)
                            * glm::rotate(glm::mat4(), glm::radians(90.f * k), normal3)
                            * *squareStart);

        everything->push_back(*normal);
        everything->push_back(*color);
    }
    // Push indices
    indices->push_back(index);
    indices->push_back(index + 1);
    indices->push_back(index + 2);
    indices->push_back(index);
    indices->push_back(index + 2);
    indices->push_back(index + 3);

}

uint64_t Terrain::convertToInt(int x, int z)  const {
    return ((uint64_t) x << 32) | ((uint64_t) z << 32 >> 32);
}

void Terrain::splitInt(uint64_t in, int *x, int *z) const {
    *x = (int) (in>>32);
    *z = (int) in;
}

// Given which Chunk to create
// Make the highland terrain for that Chunk
Chunk* Terrain::CreateHighland(int chunkX, int chunkZ) {
    //glm::ivec2 chunk_pos = getChunkPosition(worldX, worldZ);
    //uint64_t chunk_pos = convertToInt(worldX, worldZ);
    //Chunk* chunk = getChunk(chunkX, chunkZ);

    // If Chunk already exists, no need to make it again
    auto index = chunk_map.find(convertToInt(chunkX, chunkZ));
    if (index != chunk_map.end()) {
        return nullptr;
    }
    Chunk* chunk = new Chunk(context);
    chunk_map[convertToInt(chunkX, chunkZ)] = chunk;

    // The chunk's position in world coordinates
    int chunkWorldX = chunkX * chunk_dimensions[0];
    int chunkWorldZ = chunkZ * chunk_dimensions[2];

    // 0 -> 127 is STONE
    for(int x = 0; x < chunk_dimensions[0]; ++x){
        for(int z = 0; z < chunk_dimensions[2]; ++z){
            for(int y = 0; y < 128; ++y){
                //setBlockAt(x, y, z, STONE);
                chunk->getBlockType(x, y, z) = STONE;
            }
        }
    }
    // 128 -> height - 1 is DIRT, height is GRASS
    float persistance = 0.4f;
    int octaves = 4;
    float greyscale;
    int height;
    for(int x = 0; x < chunk_dimensions[0]; ++x){
        for(int z = 0; z < chunk_dimensions[2]; ++z){
            //greyscale = fbm(x + 0.5f, z + 0.5f, persistance, octaves);
            greyscale = fbm(x + chunkWorldX + 0.5f, z + chunkWorldZ + 0.5f, persistance, octaves);
            height = mapToHeight(greyscale);
            for(int y = 128; y < height; ++y){
                //setBlockAt(x, y, z, DIRT);
                chunk->getBlockType(x, y, z) = DIRT;
            }
            //setBlockAt(x, height, z, GRASS);
            chunk->getBlockType(x, height, z) = GRASS;
        }
    }
    return chunk;
}

float Terrain::rand(const glm::vec2 n) const{
    // return pseudorandom number between -1 and 1
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
    // dampen noise amplitude for flatter terrain
    float dampen = 0.2;
    // begin noisey terrain at middle map height
    return 128 + glm::floor(val * 128 * dampen);
}
