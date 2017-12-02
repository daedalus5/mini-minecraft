#include <scene/terrain.h>

Chunk::Chunk(OpenGLContext* context)
    : Drawable(context), block_array({EMPTY})
{
    // Array in initialization list
    // does not work on Sagar's laptop

//    for(int i=0;i<65536;i++)
//    {
//        block_array[i] = EMPTY;
//    }

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

Terrain::Terrain(OpenGLContext* in_context) :
    terrainType(nullptr),
    dimensions(64, 256, 64),
    chunk_dimensions(16, 256, 16),
    chunk_map(std::unordered_map<uint64_t, Chunk*>()),
    color_map(std::unordered_map<char, glm::vec4>()),
    normal_vec_map(std::unordered_map<char, glm::vec4>()),
    block_uv_map(std::unordered_map<char, std::vector<glm::vec4>>()),
    context(in_context),

    offset(glm::vec3(0.5, 0.5, 0.5))
{
    color_map[GRASS] = glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f;
    color_map[DIRT] = glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f;
    color_map[STONE] = glm::vec4(0.5, 0.5, 0.5, 1);
    color_map[LAVA] = glm::vec4(207.f, 16.f, 32.f, 128.f) / 255.f;


    // Letters correspond to movement controls
    // ex: 'W' corresponds to forward z direction
    normal_vec_map['D'] = glm::vec4(1, 0, 0, 0);
    normal_vec_map['A'] = glm::vec4(-1, 0, 0, 0);
    normal_vec_map['W'] = glm::vec4(0, 0, 1, 0);
    normal_vec_map['S'] = glm::vec4(0, 0, -1, 0);
    normal_vec_map['E'] = glm::vec4(0, 1, 0, 0);
    normal_vec_map['Q'] = glm::vec4(0, -1, 0, 0);

    draw_start_map['D'] = glm::vec4(0.5, 0.5, 0.5, 1);
    draw_start_map['A'] = glm::vec4(-0.5, 0.5, -0.5, 1);
    draw_start_map['W'] = glm::vec4(-0.5, 0.5, 0.5, 1);
    draw_start_map['S'] = glm::vec4(0.5, 0.5, -0.5, 1);
    draw_start_map['E'] = glm::vec4(0.5, 0.5, 0.5, 1);
    draw_start_map['Q'] = glm::vec4(0.5, -0.5, 0.5, 1);

    // Create UV values for each type of block
    // Stored in a vec4
    // index 0, 1 are the UVs
    // index 2 is the specular cosine power
    // index 3 is flag for whether this block is animated
    std::vector<glm::vec4> uv = std::vector<glm::vec4>();

    uv.clear();
    uv.push_back(glm::vec4(2.f/16.f, 16.f/16.f, 1.f, 0));
    uv.push_back(glm::vec4(2.f/16.f, 15.f/16.f, 1.f, 0));
    uv.push_back(glm::vec4(3.f/16.f, 15.f/16.f, 1.f, 0));
    uv.push_back(glm::vec4(3.f/16.f, 16.f/16.f, 1.f, 0));
    block_uv_map[DIRT] = uv;

    uv.clear();
    uv.push_back(glm::vec4(1.f/16.f, 16.f/16.f, 2.f, 0));
    uv.push_back(glm::vec4(1.f/16.f, 15.f/16.f, 2.f, 0));
    uv.push_back(glm::vec4(2.f/16.f, 15.f/16.f, 2.f, 0));
    uv.push_back(glm::vec4(2.f/16.f, 16.f/16.f, 2.f, 0));
    block_uv_map[STONE] = uv;

    uv.clear();
    uv.push_back(glm::vec4(13.f/16.f, 2.f/16.f, 8.f, 1));
    uv.push_back(glm::vec4(13.f/16.f, 1.f/16.f, 8.f, 1));
    uv.push_back(glm::vec4(14.f/16.f, 1.f/16.f, 8.f, 1));
    uv.push_back(glm::vec4(14.f/16.f, 2.f/16.f, 8.f, 1));
    block_uv_map[LAVA] = uv;

    uv.clear();
    uv.push_back(glm::vec4(13.f/16.f, 4.f/16.f, 100.f, 1));
    uv.push_back(glm::vec4(13.f/16.f, 3.f/16.f, 100.f, 1));
    uv.push_back(glm::vec4(14.f/16.f, 3.f/16.f, 100.f, 1));
    uv.push_back(glm::vec4(14.f/16.f, 4.f/16.f, 100.f, 1));
    block_uv_map[WATER] = uv;

    uv.clear();
    uv.push_back(glm::vec4(8.f/16.f, 14.f/16.f, 3.f, 0));
    uv.push_back(glm::vec4(8.f/16.f, 13.f/16.f, 3.f, 0));
    uv.push_back(glm::vec4(9.f/16.f, 13.f/16.f, 3.f, 0));
    uv.push_back(glm::vec4(9.f/16.f, 14.f/16.f, 3.f, 0));
    block_uv_map[GRASS + 'E'] = uv;

    uv.clear();
    block_uv_map[GRASS + 'Q'] = block_uv_map[DIRT];

    uv.clear();
    uv.push_back(glm::vec4(3.f/16.f, 16.f/16.f, 1.f, 0));
    uv.push_back(glm::vec4(3.f/16.f, 15.f/16.f, 1.f, 0));
    uv.push_back(glm::vec4(4.f/16.f, 15.f/16.f, 1.f, 0));
    uv.push_back(glm::vec4(4.f/16.f, 16.f/16.f, 1.f, 0));
    block_uv_map[GRASS + 'W'] = uv;

    block_uv_map[GRASS + 'A'] = block_uv_map[GRASS + 'W'];
    block_uv_map[GRASS + 'S'] = block_uv_map[GRASS + 'W'];
    block_uv_map[GRASS + 'D'] = block_uv_map[GRASS + 'W'];

    uv.clear();
    uv.push_back(glm::vec4(1.f/16.f, 15.f/16.f, 2.f, 0));
    uv.push_back(glm::vec4(1.f/16.f, 14.f/16.f, 2.f, 0));
    uv.push_back(glm::vec4(2.f/16.f, 14.f/16.f, 2.f, 0));
    uv.push_back(glm::vec4(2.f/16.f, 15.f/16.f, 2.f, 0));
    block_uv_map[BEDROCK] = uv;

    uv.clear();
    uv.push_back(glm::vec4(3.f/16.f, 12.f/16.f, 64.f, 0));
    uv.push_back(glm::vec4(3.f/16.f, 11.f/16.f, 64.f, 0));
    uv.push_back(glm::vec4(4.f/16.f, 11.f/16.f, 64.f, 0));
    uv.push_back(glm::vec4(4.f/16.f, 12.f/16.f, 64.f, 0));
    block_uv_map[ICE] = uv;

    uv.clear();
    uv.push_back(glm::vec4(5.f/16.f, 13.f/16.f, 4.f, 0));
    uv.push_back(glm::vec4(5.f/16.f, 12.f/16.f, 4.f, 0));
    uv.push_back(glm::vec4(6.f/16.f, 12.f/16.f, 4.f, 0));
    uv.push_back(glm::vec4(6.f/16.f, 13.f/16.f, 4.f, 0));
    block_uv_map[LEAF] = uv;

    uv.clear();
    uv.push_back(glm::vec4(5.f/16.f, 15.f/16.f, 2.f, 0));
    uv.push_back(glm::vec4(5.f/16.f, 14.f/16.f, 2.f, 0));
    uv.push_back(glm::vec4(6.f/16.f, 14.f/16.f, 2.f, 0));
    uv.push_back(glm::vec4(6.f/16.f, 15.f/16.f, 2.f, 0));
    block_uv_map[WOOD + 'E'] = uv;

    uv.clear();
    block_uv_map[WOOD + 'Q'] = block_uv_map[WOOD + 'E'];

    uv.clear();
    uv.push_back(glm::vec4(4.f/16.f, 15.f/16.f, 2.f, 0));
    uv.push_back(glm::vec4(4.f/16.f, 14.f/16.f, 2.f, 0));
    uv.push_back(glm::vec4(5.f/16.f, 14.f/16.f, 2.f, 0));
    uv.push_back(glm::vec4(5.f/16.f, 15.f/16.f, 2.f, 0));
    block_uv_map[WOOD + 'W'] = uv;

    block_uv_map[WOOD + 'A'] = block_uv_map[WOOD + 'W'];
    block_uv_map[WOOD + 'S'] = block_uv_map[WOOD + 'W'];
    block_uv_map[WOOD + 'D'] = block_uv_map[WOOD + 'W'];
}

Terrain::~Terrain() {
    for ( auto it = chunk_map.begin(); it != chunk_map.end(); ++it ) {
        it->second->destroy();
        delete it->second;
    }
    delete terrainType;
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
        // If Chunk does not exist, create the terrain for this chunk
        // Then at end, we will set the specific block type for one position
        // We're doing it this way because this method is mostly called
        // by river generation, which might need to create new chunks
        // but only sets the water blocks

        //ch = new Chunk(context);
        chunk_map[chunk_pos] = createScene(getChunkPosition1D(x), getChunkPosition1D(z));
    } else {
        ch = chunk_map[chunk_pos];
    }
    ch->getBlockType(getChunkLocalPosition1D(x), y, getChunkLocalPosition1D(z)) = t;
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

    BlockType block, neighbor;
    glm::vec3 world_pos;

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

                    // Check neighboring Chunk in x direction
                    if (i == 0) {
                        if (neighbor__x != nullptr) {
                            neighbor = neighbor__x->getBlockType(chunk_dimensions[0] - 1, j, k);
                            if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                                addSquare(&world_pos, &everything, &indices, 'A', block);
                            }
                        }
                    } else {
                        neighbor = ch->getBlockType(i - 1, j, k);
                        if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                            // Check neighboring x within this chunk
                            addSquare(&world_pos, &everything, &indices, 'A', block);
                        }
                    }

                    if (i == chunk_dimensions[0] - 1) {
                        if (neighbor_x != nullptr) {
                            neighbor = neighbor_x->getBlockType(0, j, k);
                            if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                                addSquare(&world_pos, &everything, &indices, 'D', block);
                            }
                        }
                    }
                    else {
                        neighbor = ch->getBlockType(i + 1, j, k);
                        if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                            addSquare(&world_pos, &everything, &indices, 'D', block);
                        }
                    }


                    // Check neighboring y
                    if (j < 255) {
                        neighbor = ch->getBlockType(i, j + 1, k);
                        if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                            addSquare(&world_pos, &everything, &indices, 'E', block);
                        }
                    }
                    if (j > 0) {
                        neighbor = ch->getBlockType(i, j - 1, k);
                        if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                            addSquare(&world_pos, &everything, &indices, 'Q', block);
                        }
                    }

                    // Check neighboring Chunk in z direction
                    if (k == 0) {
                        if (neighbor__z != nullptr) {
                            neighbor = neighbor__z->getBlockType(i, j, chunk_dimensions[2] - 1);
                            if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                                addSquare(&world_pos, &everything, &indices, 'S', block);
                            }
                        }
                    } else {
                        neighbor = ch->getBlockType(i, j, k - 1);
                        if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                            addSquare(&world_pos, &everything, &indices, 'S', block);
                        }
                    }

                    if (k == chunk_dimensions[2] - 1) {
                        if (neighbor_z != nullptr) {
                            neighbor = neighbor_z->getBlockType(i, j, 0);
                            if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                                addSquare(&world_pos, &everything, &indices, 'W', block);
                            }
                        }
                    } else {
                        neighbor = ch->getBlockType(i, j, k + 1);
                        if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                            addSquare(&world_pos, &everything, &indices, 'W', block);
                        }
                    }

                }

            }
        }
    }
    ch->createVBO(everything, indices);
}

//void Terrain::updateAllVBO() {
//    for ( auto it = chunk_map.begin(); it != chunk_map.end(); ++it ) {
//        //updateChunkVBO(16 * it->first[0], 16 * it->first[1]);
//    }
//}

void Terrain::addSquare(glm::vec3* pos,
                      std::vector<glm::vec4> *everything,
                      std::vector<GLuint> *indices,
                      char direction, BlockType block) {

    // grab size of positions
    int index = everything->size() / 3;
    glm::vec4 normal = normal_vec_map[direction];
    glm::vec3 normal3 = glm::vec3(normal);

    // Get list of UVs
    // If the BlockType is not stored in the map,
    // add the direction to it.
    // Used for GRASS and WOOD which have different UVs
    // depending on the direction of face
    std::vector<glm::vec4> uv_list;
    auto mapped_uv = block_uv_map.find(block);
    if (mapped_uv == block_uv_map.end()) {
        uv_list = block_uv_map[block + direction];
    } else {
        uv_list = mapped_uv->second;
    }

    for (int k = 0; k < 4; k++) {
        // Rotate by 90 degrees 4 times
        // Push them into positions vector
        everything->push_back(glm::translate(glm::mat4(), *pos + offset)
                            * glm::rotate(glm::mat4(), glm::radians(90.f * k), normal3)
                            * draw_start_map[direction]);

        // Push normals
        everything->push_back(normal);
        // Push UV
        everything->push_back(uv_list[k]);
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
Chunk* Terrain::createScene(int chunkX, int chunkZ) {
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
    float persistance = terrainType->getPersistance();
    int octaves = terrainType->getOctaves();
    float greyscale;
    int height;
    for(int x = 0; x < chunk_dimensions[0]; ++x){
        for(int z = 0; z < chunk_dimensions[2]; ++z){
            //greyscale = fbm(x + 0.5f, z + 0.5f, persistance, octaves);
            greyscale = terrainType->fbm(x + chunkWorldX + 0.5f, z + chunkWorldZ + 0.5f, persistance, octaves);
            height = terrainType->mapToHeight(greyscale);
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

void Terrain::setTerrainType(TerrainType *t){
    delete terrainType;
    this->terrainType = t;
}

// TERRAIN END
// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// TERRAINTYPE START

TerrainType::TerrainType(int octaves, float persistance, float resolution, float dampen) :
    octaves(octaves), persistance(persistance), resolution(resolution), dampen(dampen)
{}

TerrainType::~TerrainType(){}

float TerrainType::rand(const glm::vec2 n) const{
    // return pseudorandom number between -1 and 1
    return (glm::fract(sin(glm::dot(n, glm::vec2(12.9898, 4.1414))) * 43758.5453));
}

float TerrainType::interpNoise2D(const float x, const float z) const{
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

float TerrainType::fbm(const float x, const float z, const float persistance, const int octaves) const{
    float total = 0.0f;

    for(int i = 0; i < octaves; i++){
        float freq = pow(2.0f, i);
        float amp = pow(persistance, i);

        total += amp * interpNoise2D(x * freq / resolution, z * freq / resolution);
    }
    float a = 1 - persistance;  // normalization

    return a * (1.0f + total) / 2.0f;  // normalized, pseudorandom number between 0 and 1

}

int TerrainType::mapToHeight(const float val) const{
    // dampen noise amplitude for flatter terrain
    // begin noisey terrain at middle map height
    return 128 + glm::floor(val * 128 * dampen);
}

int TerrainType::getOctaves() const{
    return octaves;
}

float TerrainType::getPersistance() const{
    return persistance;
}

float TerrainType::getResolution() const{
    return resolution;
}

float TerrainType::getDampen() const{
    return dampen;
}

// TERRAINTYPE END
// **********************************************************************************************
// **********************************************************************************************
// **********************************************************************************************
// TERRAINTYPE SUBCLASSES START

Highland::Highland() :
    TerrainType(4, 0.4f, 20.0f, 0.2f)
{}

Highland::~Highland(){}

Foothills::Foothills() :
    TerrainType(4, 0.4f, 25.0f, 0.5f)
{}

Foothills::~Foothills(){}
