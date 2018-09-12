#include <scene/terrain.h>

Chunk::Chunk(OpenGLContext* context)

    : Drawable(context),isCreated(false), hasData(false)



{
    // Array in initialization list
    // does not work on Sagar's laptop (Macs suck!)

   for(int i=0;i<65536;i++)
    {
       block_array[i] = EMPTY;   }

}

BlockType Chunk::getBlockType(int x, int y, int z) const {
    return block_array[x + 16 * y + 4096 * z];
}

BlockType& Chunk::getBlockType(int x, int y, int z) {
    return block_array[x + 16 * y + 4096 * z];
}

// Sends the VBO data to the GPU
// Only runs if Chunk has data
void Chunk::createVBO()
{
    if (!hasData || everything.size() == 0 || indices.size() == 0) {
        return;
    }

    count = indices.size();

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    generateEve();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufEve);
    context->glBufferData(GL_ARRAY_BUFFER, everything.size() * sizeof(glm::vec4), everything.data(), GL_STATIC_DRAW);

    isCreated = true;
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

Terrain::Terrain(OpenGLContext* in_context,Camera* camera,QMutex* mutexref) :
    context(in_context),mp_camera(camera),mutex(mutexref),
    dimensions(64, 256, 64),
    chunk_dimensions(16, 256, 16),
    chunk_map(std::unordered_map<uint64_t, Chunk*>()),
    color_map(std::unordered_map<char, glm::vec4>()),
    normal_vec_map(std::unordered_map<char, glm::vec4>()),
    block_uv_map(std::unordered_map<char, std::vector<glm::vec4>>()),


    offset(glm::vec3(0.5, 0.5, 0.5)),
    chunks2Add(std::vector<chunkMapData>())
{
    // Map BlockType to colors
    // Not currently used because we have textures
    color_map[GRASS] = glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f;
    color_map[DIRT] = glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f;
    color_map[STONE] = glm::vec4(0.5, 0.5, 0.5, 1);
    color_map[LAVA] = glm::vec4(207.f, 16.f, 32.f, 128.f) / 255.f;


    // Map a direction to its normal vector
    // Used when drawing faces
    // Letters correspond to movement controls
    // ex: 'W' corresponds to forward z direction
    normal_vec_map['D'] = glm::vec4(1, 0, 0, 0);
    normal_vec_map['A'] = glm::vec4(-1, 0, 0, 0);
    normal_vec_map['W'] = glm::vec4(0, 0, 1, 0);
    normal_vec_map['S'] = glm::vec4(0, 0, -1, 0);
    normal_vec_map['E'] = glm::vec4(0, 1, 0, 0);
    normal_vec_map['Q'] = glm::vec4(0, -1, 0, 0);

    // Map a direction to a point
    // Used when drawing faces. This point is what
    // we rotate to procedurally create a square
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
    // If Block has multiple textures,
    // add direction character and store that.
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
    uv.push_back(glm::vec4(14.f/16.f, 2.f/16.f, 8.f, 1));
    uv.push_back(glm::vec4(13.f/16.f, 2.f/16.f, 8.f, 1));
    uv.push_back(glm::vec4(13.f/16.f, 1.f/16.f, 8.f, 1));
    uv.push_back(glm::vec4(14.f/16.f, 1.f/16.f, 8.f, 1));
    block_uv_map[LAVA] = uv;

    uv.clear();
    uv.push_back(glm::vec4(14.f/16.f, 4.f/16.f, 100.f, 1));
    uv.push_back(glm::vec4(13.f/16.f, 4.f/16.f, 100.f, 1));
    uv.push_back(glm::vec4(13.f/16.f, 3.f/16.f, 100.f, 1));
    uv.push_back(glm::vec4(14.f/16.f, 3.f/16.f, 100.f, 1));
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
    uv.push_back(glm::vec4(2.f/16.f, 12.f/16.f, 64.f, 0));
    uv.push_back(glm::vec4(2.f/16.f, 11.f/16.f, 64.f, 0));
    uv.push_back(glm::vec4(3.f/16.f, 11.f/16.f, 64.f, 0));
    uv.push_back(glm::vec4(3.f/16.f, 12.f/16.f, 64.f, 0));
    block_uv_map[SNOW] = uv;

    uv.clear();
    uv.push_back(glm::vec4(2.f/16.f, 15.f/16.f, 10.f, 0));
    uv.push_back(glm::vec4(2.f/16.f, 14.f/16.f, 10.f, 0));
    uv.push_back(glm::vec4(3.f/16.f, 14.f/16.f, 10.f, 0));
    uv.push_back(glm::vec4(3.f/16.f, 15.f/16.f, 10.f, 0));
    block_uv_map[SAND] = uv;

    uv.clear();
    uv.push_back(glm::vec4(5.f/16.f, 13.f/16.f, 4.f, 0));
    uv.push_back(glm::vec4(5.f/16.f, 12.f/16.f, 4.f, 0));
    uv.push_back(glm::vec4(6.f/16.f, 12.f/16.f, 4.f, 0));
    uv.push_back(glm::vec4(6.f/16.f, 13.f/16.f, 4.f, 0));
    block_uv_map[LEAF] = uv;

    uv.clear();
    uv.push_back(glm::vec4(0.f, 14.f/16.f, 12.f, 0));
    uv.push_back(glm::vec4(0.f, 13.f/16.f, 12.f, 0));
    uv.push_back(glm::vec4(1.f/16.f, 13.f/16.f, 12.f, 0));
    uv.push_back(glm::vec4(1.f/16.f, 14.f/16.f, 12.f, 0));
    block_uv_map[GOLDORE] = uv;

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
}

// Given world coordinates, return BlockType
// Returns empty if the block does not exist
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if (y < 0 || y > 255) {
        return NOTHING;
    }

    auto index = chunk_map.find(convertToInt(getChunkPosition1D(x), getChunkPosition1D(z)));
    if (index != chunk_map.end()) {
        Chunk* ch = index->second;
        return ch->getBlockType(getChunkLocalPosition1D(x), y, getChunkLocalPosition1D(z));
    }
    return NOTHING;
}

// Given world coordinates, sets the block at that position
// If the Chunk at that position does not exist,
// create it
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

        ch = createScene(getChunkPosition1D(x), getChunkPosition1D(z));
        chunk_map[chunk_pos] = ch;
    } else {
        ch = index->second;
    }
    ch->getBlockType(getChunkLocalPosition1D(x), y, getChunkLocalPosition1D(z)) = t;
    ch->hasData = false; // Need to update VBO
}

// Same as setBlockAt
// Originally this updated the VBO data
// But that was moved to drawScene
void Terrain::addBlockAt(int x, int y, int z, BlockType t) {
    setBlockAt(x, y, z, t);
    //updateChunkVBO(getChunkPosition1D(x), getChunkPosition1D(z));

    // Update neighboring Chunks
    // They may need to add/delete a wall
//    updateChunkVBO(getChunkPosition1D(x + 1), getChunkPosition1D(z));
//    updateChunkVBO(getChunkPosition1D(x - 1), getChunkPosition1D(z));
//    updateChunkVBO(getChunkPosition1D(x), getChunkPosition1D(z + 1));
//    updateChunkVBO(getChunkPosition1D(x), getChunkPosition1D(z - 1));
}

// Given Chunk position in Chunk coordinates,
// return the Chunk
Chunk* Terrain::getChunk(int x, int z) const {

    auto index = chunk_map.find(convertToInt(x, z));
    if (index != chunk_map.end()) {
        return index->second;
    }
    return nullptr;
}

// Converts world coordinate to Chunk coordinate
int Terrain::getChunkPosition1D(int x) const {
    int temp = x / 16;
    if (x < 0) {
        if (x % 16 != 0) {
            temp = temp - 1;
        }
    }
    return temp;
}

// Converts world coordinate to local Chunk coordinate
// (The local position of the block within the Chunk)
int Terrain::getChunkLocalPosition1D(int x) const {
    int temp = x % 16;
    if (x < 0) {
        if (temp != 0) {
            temp = temp + 16;
        }
    }
    return temp;
}

// Given world coordinates, return Chunk coordinates
// Not used. Instead, we call getChunkPosition1D twice.
// I hope that's faster...
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

// Given world coordinates, return the local position within Chunk
// Not used. Instead, we call getChunkLocalPosition1D twice.
// I hope that's faster...
glm::ivec3 Terrain::getChunkLocalPosition(int x, int y, int z) const {
    glm::ivec3 local_pos = glm::ivec3(x % 16, y, z % 16);
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

// Creates the Chunk's VBO data
// Parameters are the Chunk position in Chunk coordinates
// (ex: world 16, 16 -> chunk 1, 1)
void Terrain::updateChunkVBO(int x, int z) {

    // If Chunk does not exist in map,
    // Do not do anything
    auto index = chunk_map.find(convertToInt(x, z));
    if (index == chunk_map.end()) {
        return;
    }

    // Clear out the Chunk's current data
    Chunk* ch = index->second;
    ch->hasData = false; // VBO data has been cleared.
    ch->everything.clear();
    ch->indices.clear();

    // Variables that are reused in loop
    BlockType block, neighbor; // Current BlockType and neighbor BlockType
    glm::vec3 world_pos; // World position of current block

    // Chunk neighbors
    // Used for checking neighboring faces
    Chunk* neighbor_x = getChunk(x + 1, z);
    Chunk* neighbor__x = getChunk(x - 1, z);
    Chunk* neighbor_z = getChunk(x, z + 1);
    Chunk* neighbor__z = getChunk(x, z - 1);

    for (int i = 0; i < chunk_dimensions[0]; i++) {
        for (int j = 0; j < chunk_dimensions[1]; j++) {
            for (int k = 0; k < chunk_dimensions[2]; k++) {

                block = ch->getBlockType(i, j, k);

                if (block != EMPTY && block != NOTHING) {

                    world_pos = glm::vec3(i, j, k)
                            + glm::vec3(x * chunk_dimensions[0], 0, z * chunk_dimensions[2]);

                    // Check neighbor in -x direction
                    if (i == 0) {
                        if (neighbor__x != nullptr) {
                            // Check neighboring Chunk in -x direction
                            neighbor = neighbor__x->getBlockType(chunk_dimensions[0] - 1, j, k);
                            if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                                addSquare(&world_pos, &(ch->everything), &(ch->indices), 'A', block);
                            }
                        }
                    } else {
                        // Check neighboring -x within this chunk
                        neighbor = ch->getBlockType(i - 1, j, k);
                        if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                            addSquare(&world_pos, &(ch->everything), &(ch->indices), 'A', block);
                        }
                    }

                    // Check neighbor in +x direction
                    if (i == chunk_dimensions[0] - 1) {
                        // Check neighboring Chunk in +x direction
                        if (neighbor_x != nullptr) {
                            neighbor = neighbor_x->getBlockType(0, j, k);
                            if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                                addSquare(&world_pos, &(ch->everything), &(ch->indices), 'D', block);
                            }
                        }
                    } else {
                        // Check neighboring +x within this chunk
                        neighbor = ch->getBlockType(i + 1, j, k);
                        if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                            addSquare(&world_pos, &(ch->everything), &(ch->indices), 'D', block);
                        }
                    }


                    // Check neighboring +y within this Chunk
                    if (j < 255) {
                        neighbor = ch->getBlockType(i, j + 1, k);
                        if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                            addSquare(&world_pos, &(ch->everything), &(ch->indices), 'E', block);
                        }
                    }
                    // Check neighboring -y within this Chunk
                    if (j > 0) {
                        neighbor = ch->getBlockType(i, j - 1, k);
                        if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                            addSquare(&world_pos, &(ch->everything), &(ch->indices), 'Q', block);
                        }
                    }

                    // Check neighbor in -z direction
                    if (k == 0) {
                        // Check neighboring Chunk in -z direction
                        if (neighbor__z != nullptr) {
                            neighbor = neighbor__z->getBlockType(i, j, chunk_dimensions[2] - 1);
                            if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                                addSquare(&world_pos, &(ch->everything), &(ch->indices), 'S', block);
                            }
                        }
                    } else {
                        // Check neighboring -z within this chunk
                        neighbor = ch->getBlockType(i, j, k - 1);
                        if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                            addSquare(&world_pos, &(ch->everything), &(ch->indices), 'S', block);
                        }
                    }

                    // Check neighbor in +z direction
                    if (k == chunk_dimensions[2] - 1) {
                        // Check neighboring Chunk in +z direction
                        if (neighbor_z != nullptr) {
                            neighbor = neighbor_z->getBlockType(i, j, 0);
                            if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                                addSquare(&world_pos, &(ch->everything), &(ch->indices), 'W', block);
                            }
                        }
                    } else {
                        // Check neighboring +z within this chunk
                        neighbor = ch->getBlockType(i, j, k + 1);
                        if (neighbor == EMPTY || ((neighbor == LAVA || neighbor == WATER) && neighbor != block)) {
                            addSquare(&world_pos, &(ch->everything), &(ch->indices), 'W', block);
                        }
                    }

                }

            }
        }
    }
    ch->hasData = true; // VBO data has been created
    ch->isCreated = false; // Need to pass VBO to GPU
}

// Loops through all existing Chunks and creates VBO data
// Originally for use in initialization, but currently not used
//void Terrain::updateAllVBO() {
//    int x, z;
//    for ( auto it = chunk_map.begin(); it != chunk_map.end(); ++it ) {
//        splitInt(it->first, &x, &z);
//        updateChunkVBO(x, z);
//    }
//}

// Adds a square face to the VBO data
void Terrain::addSquare(glm::vec3* pos,
                      std::vector<glm::vec4> *everything,
                      std::vector<GLuint> *indices,
                      char direction, BlockType block) {

    // grab size of positions for current index
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

    // Make the face
    for (int k = 0; k < 4; k++) {
        // Rotate by 90 degrees 4 times
        // Rotate about the normal vector stored in normal_vec_map. Depends on direction
        // The point to rotate is from start_draw_map. Depends on direction
        // Push position into positions vector
        everything->push_back(glm::translate(glm::mat4(), *pos + offset)
                            * glm::rotate(glm::mat4(), glm::radians(90.f * k), normal3)
                            * draw_start_map[direction]);

        // Push normal
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

// Takes the Chunk position (in Chunk coordinates. ex: world 16, 16 -> chunk 1, 1)
// Converts the x and z into a single 64 bit integer to use for
// chunk_map's key
uint64_t Terrain::convertToInt(int x, int z)  const {
    uint64_t blah = ((uint64_t) x << 32) | ((uint64_t) z << 32 >> 32);
    return ((uint64_t) x << 32) | ((uint64_t) z << 32 >> 32);
}

// Takes a 64 bit integer and splits it into two 32 bit ints
// Basically, takes a chunk_map key and converts to
// Chunk position in Chunk coordinates
void Terrain::splitInt(uint64_t in, int *x, int *z) const {
    *x = (int) (in>>32);
    *z = (int) in;
}

// Given which Chunk to create
// Make the terrain for that Chunk
Chunk* Terrain::createScene(int chunkX, int chunkZ) {

    // If Chunk already exists, no need to make it again
    auto index = chunk_map.find(convertToInt(chunkX, chunkZ));
    if (index != chunk_map.end()) {
        return nullptr;
    }

    Chunk* chunk = new Chunk(context);

    // The chunk's position in world coordinates
    int chunkWorldX = chunkX * chunk_dimensions[0];
    int chunkWorldZ = chunkZ * chunk_dimensions[2];

    Highland highland = Highland();
    Foothills foothills = Foothills();
    SnowMountains snowyMountains = SnowMountains();

    // river lands
    if (chunkWorldX >= -4 * 16 && chunkWorldX <= 4 * 16){
        highland.drawMe(chunkWorldX, chunkWorldZ, chunk);
    }
    // foothills
    else if (chunkWorldX >= -5 * 16 && chunkWorldX < -4 * 16){
        interpolateTerrain(&foothills, &highland, GRASS, chunkWorldX, chunkWorldZ, chunk);
    }
    else if (chunkWorldX <= 5 * 16 && chunkWorldX > 4 * 16){
        interpolateTerrain(&highland, &foothills, GRASS, chunkWorldX, chunkWorldZ, chunk);
    }
    else if ((chunkWorldX >= -10 * 16 && chunkWorldX < -5 * 16) || (chunkWorldX <= 10 * 16 && chunkWorldX > 5 * 16)){
        foothills.drawMe(chunkWorldX, chunkWorldZ, chunk);
    }
    // snowy mountains
    else if (chunkWorldX >= -11 * 16 && chunkWorldX < -10 * 16){
        interpolateTerrain(&snowyMountains, &foothills, SNOW, chunkWorldX, chunkWorldZ, chunk);
    }
    else if (chunkWorldX <= 11 * 16 && chunkWorldX > 10 * 16){
        interpolateTerrain(&foothills, &snowyMountains, SNOW, chunkWorldX, chunkWorldZ, chunk);
    }
    else{
        snowyMountains.drawMe(chunkWorldX, chunkWorldZ, chunk);
    }

    return chunk;
}

//////////////////////// PROCEDURAL TERRAIN FUNCTIONS ///////////////////////////

void Terrain::createRivers()
{

    srand(time(NULL));
    // pass river desired start position and general heading (x, z)
    RiverDelta riverDelta = RiverDelta(glm::vec2(0,0), glm::vec2(0.0,1.0f), 4.0f);
    RiverDelta* deltaPtr = &riverDelta;
    // set number of turtle steps and pass intial path seed
    deltaPtr->generatePath(MAX_DEPTH_DELTA, "F+-F+-FX");
    // populate LSystems set of operations passed to turtle
    deltaPtr->populateOps();

    // offsets are used to draw rivers with some thickness according to turtle recursion depth
    std::vector<int> offsets = {4, 2, 1, 0};
    traceRiverPath(deltaPtr, offsets);

    River river = River(glm::vec2(0,0), glm::vec2(0.0,-1.0f), 4.0f);
    River* riverPtr = &river;
    riverPtr->generatePath(MAX_DEPTH_RIVER, "F+-F+-FX");
    riverPtr->populateOps();
    offsets.clear();
    offsets = {4, 0, 0, 0};
    traceRiverPath(riverPtr, offsets);
}

void Terrain::traceRiverPath(LSystem* lsys, const std::vector<int>& offsets){
    glm::vec2 start;
    glm::vec2 end;
    int zMin;
    int zMax;
    int xMin;
    int xMax;
    float intersect;

    const int zeroHeight = 135;
    int depth;
    int offset;
    int numShelves = 4;

    // loop over LSystem string path
    for(int i = 0; i < lsys->path.length(); ++i){
        start = lsys->activeTurtle.position;
        // dereference function pointer mapped to path character
        (lsys->*(lsys->charToDrawingOperation[lsys->path[i]]))();
        // if turtle moves forward, draw its path
        if (lsys->path[i] == 'F'){
            // draw linesegment2D along turtle path
            end = lsys->activeTurtle.position;
            LineSegment2D line = LineSegment2D(start, end);
            zMin = start[1] <= end[1] ? start[1] : end[1];
            zMax = end[1] >= start[1] ? end[1] : start[1];

            // for drawing rivers with width
            depth = lsys->activeTurtle.depth;
            if (depth == 1){
                offset = offsets[0];
            }
            if (depth == 2){
                offset = offsets[1];
            }
            if (depth == 3){
                offset = offsets[2];
            }
            if (depth > 3){
                offset = offsets[3];
            }

            for(int k = zMin; k <= zMax; ++k){
                // river by rasterization
                bool check = line.intersectAt(k, &intersect);
                if (check){ // river is travelling N/S
                    int l = floor(intersect);
                    for(int p = -offset; p <= offset; p++){
                        setBlockAt(l + p, zeroHeight, k, WATER);
                        setBlockAt(l + p, zeroHeight + 1, k, WATER);
                        setBlockAt(l + p, zeroHeight + 2, k, WATER);
                        setBlockAt(l + p, zeroHeight - 1, k, SAND);
                        for(int q = zeroHeight + 3; q < 256; ++q){
                            setBlockAt(l + p, q, k, EMPTY);
                        }
                    }
                    // riverbanks
                    for(int p = 0; p < 3; ++p){
                        for(int q = zeroHeight + 1; q + p < zeroHeight + 3; ++q){
                            setBlockAt(l - offset - 1 - 2*p, q + p, k, WATER);
                            setBlockAt(l - offset - 1 - 2*p - 1, q + p, k, WATER);
                            setBlockAt(l + offset + 1 + 2*p, q + p, k, WATER);
                            setBlockAt(l + offset + 1 + 2*p + 1, q + p, k, WATER);
                        }
                        for(int q = zeroHeight + 3; q < 256; ++q){
                            setBlockAt(l - offset - 1 - 2*p, q, k, EMPTY);
                            setBlockAt(l - offset - 1 - 2*p - 1, q, k, EMPTY);
                            setBlockAt(l + offset + 1 + 2*p, q, k, EMPTY);
                            setBlockAt(l + offset + 1 + 2*p + 1, q, k, EMPTY);
                        }
                    }
                    for(int p = 3; p < numShelves; ++p){
                        for(int q = zeroHeight; q + p < 256; ++q){
                            setBlockAt(l - offset - 1 - 2*p, q + p, k, EMPTY);
                            setBlockAt(l - offset - 1 - 2*p - 1, q + p, k, EMPTY);
                            setBlockAt(l + offset + 1 + 2*p, q + p, k, EMPTY);
                            setBlockAt(l + offset + 1 + 2*p + 1, q + p, k, EMPTY);
                        }
                    }
                }
                else{   // river is travelling E/W
                    xMin = start[0] <= end[0] ? start[0] : end[0];
                    xMax = end[0] >= start[0] ? end[0] : start[0];
                    for(int j = xMin; j <= xMax; ++j){
                        for(int p = -offset; p <= offset; ++p){
                            setBlockAt(j, zeroHeight, zMin + p, WATER);
                            setBlockAt(j, zeroHeight + 1, zMin + p, WATER);
                            setBlockAt(j, zeroHeight + 2, zMin + p, WATER);
                            setBlockAt(j, zeroHeight - 1, zMin + p, SAND);
                            for(int q = zeroHeight + 3; q < 256; ++q){
                                setBlockAt(j, q, zMin + p, EMPTY);
                            }
                        }
                        // riverbanks
                        for(int p = 0; p < 3; ++p){
                            for(int q = zeroHeight + 1; q + p < zeroHeight + 3; ++q){
                                setBlockAt(j, q + p, zMin - offset - 1 - 2*p, WATER);
                                setBlockAt(j, q + p, zMin - offset - 1 - 2*p - 1, WATER);
                                setBlockAt(j, q + p, zMin + offset + 1 + 2*p, WATER);
                                setBlockAt(j, q + p, zMin + offset + 1 + 2*p + 1, WATER);
                            }
                            for(int q = zeroHeight + 3; q < 256; ++q){
                                setBlockAt(j, q, zMin - offset - 1 - 2*p, EMPTY);
                                setBlockAt(j, q, zMin - offset - 1 - 2*p - 1, EMPTY);
                                setBlockAt(j, q, zMin + offset + 1 + 2*p, EMPTY);
                                setBlockAt(j, q, zMin + offset + 1 + 2*p + 1, EMPTY);
                            }
                        }
                        for(int p = 3; p < numShelves; ++p){
                            for(int q = zeroHeight + 1; q + p < 256; ++q){
                                setBlockAt(j, q + p, zMin - offset - 1 - 2*p, EMPTY);
                                setBlockAt(j, q + p, zMin - offset - 1 - 2*p - 1, EMPTY);
                                setBlockAt(j, q + p, zMin + offset + 1 + 2*p, EMPTY);
                                setBlockAt(j, q + p, zMin + offset + 1 + 2*p + 1, EMPTY);
                            }
                        }
                    }
                }
            }
        }
    }
}

// Loops through Chunks surrounding the camera
// Creates Chunk data if it does not have data
// Invoked by thread
void Terrain::drawScene()
{

    int chunkX = getChunkPosition1D(mp_camera->eye[0]);
    int chunkZ = getChunkPosition1D(mp_camera->eye[2]);

    // List of Chunks that need VBO updated
    // Save this until end, after all new Chunks have been made
    std::set<uint64_t> chunks2Update = std::set<uint64_t>();

    int num = 10;
    int x, z;

    Chunk* ch;
    for (int i = -num; i < num; i++) {
        for (int k = -num; k < num; k++) {
            x = chunkX + i;
            z = chunkZ + k;
            ch = getChunk(x, z);
            if (ch != nullptr) {

                if(!ch->hasData)
                {
                    chunks2Update.insert(convertToInt(x, z));
                    // Update neighboring Chunks
                    chunks2Update.insert(convertToInt(x + 1, z));
                    chunks2Update.insert(convertToInt(x - 1, z));
                    chunks2Update.insert(convertToInt(x, z + 1));
                    chunks2Update.insert(convertToInt(x, z - 1));
                }
            } else {

                //mutex->lock();
                ch = createScene(x, z);
                //mutex->unlock();
                if (ch != nullptr) {

                    // Add new Chunk to list, so it can be
                    // added to map by PlayState
                    chunkMapData data;
                    data.ch = ch;
                    data.key = convertToInt(x, z);
                    chunks2Add.push_back(data);

                    //chunk_map[convertToInt(x, z)] = ch;
                    chunks2Update.insert(convertToInt(x, z));

                    // Update neighboring Chunks
                    chunks2Update.insert(convertToInt(x + 1, z));
                    chunks2Update.insert(convertToInt(x - 1, z));
                    chunks2Update.insert(convertToInt(x, z + 1));
                    chunks2Update.insert(convertToInt(x, z - 1));

                }

            }
        }
    }

    int tempX, tempZ;
    for (uint64_t i : chunks2Update) {
        splitInt(i, &tempX, &tempZ);
        updateChunkVBO(tempX, tempZ);
    }

}

void Terrain::createForest(){
    Forest forest = Forest();
    int size = 16;
    int plotWidth_2 = 12;
    float prob;
    for(int i = -plotWidth_2 * size; i < plotWidth_2 * size; ++i){
        for(int j = -plotWidth_2 * size; j < plotWidth_2 * size; ++j){
            if (i*i + j*j < plotWidth_2 * size * plotWidth_2 * size){
                prob = forest.worley(i, j);
                if (prob > 0.6){
                    drawTree(glm::ivec2(i, j));
                }
            }
        }
    }
}

void Terrain::drawTree(const glm::ivec2 pos){
    setBlockAt(pos[0], 255, pos[1], EMPTY); // temporary fix

    int h = getHeightAt(pos);
    BlockType baseBlock = getBlockAt(pos[0], h, pos[1]);
    if (baseBlock != GRASS){
        return;
    }

    Tree tree = Tree(glm::vec2(0.5f, 0.5f), 1.0f);
    Tree *tr = &tree;

    tr->generatePath(TREE_DEPTH, "FX");
    tr->populateOps();

    // draw the tree trunk
    int n = 5 + (int)(4.0f * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)));

    for(int i = 1; i <= n; ++i){
        setBlockAt(pos[0], h + i, pos[1], WOOD);
    }

    glm::vec2 turtlePos;
    int yPos;

    // loop over LSystem string path
    for(int i = 0; i < tr->path.length(); ++i){
        // dereference function pointer mapped to path character
        (tr->*(tr->charToDrawingOperation[tr->path[i]]))();
        // if turtle moves forward, draw a leaf block
        if (tr->path[i] == 'F'){
            turtlePos = tr->activeTurtle.position;
            yPos = floor(turtlePos[1]);
            glm::vec2 leafDir = glm::vec2(turtlePos[0], 0.0f);
            float angle = 0.0f;
            for(int j = 0; j < 16; ++j){
                angle = 11.25f;
                leafDir = glm::vec2(leafDir[0] * cosf(angle) - leafDir[1] * sinf(angle),
                                    leafDir[0] * sinf(angle) + leafDir[1] * cosf(angle));
                BlockType t = getBlockAt(leafDir[0] + pos[0], h + n + yPos, leafDir[1] + pos[1]);
                if (t == EMPTY){
                    setBlockAt(leafDir[0] + pos[0], h + n + yPos, leafDir[1] + pos[1], LEAF);
                }
            }
        }
    }
}

void Terrain::excavateCave(){
    setBlockAt(74, 255, 0, EMPTY); // temporary fix

    int numSteps = 300;
    int excavateRadius = 4;
    int h = getHeightAt(glm::ivec2(74, 0));
    Cave cave = Cave(glm::ivec3(74, h, 74));

    float r1;

    int cavernSize = 32;

    for(int n = 0; n < numSteps; ++n){
        for(int i = -excavateRadius; i <= excavateRadius; ++i){
            for(int j = -excavateRadius; j <= excavateRadius; ++j){
                for(int k = -excavateRadius; k <= excavateRadius; ++k){
                    r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                    // excavate sphere
                    if (i*i + j*j + k*k < excavateRadius * excavateRadius){
                        setBlockAt(cave.pos[0] + i, cave.pos[1] + j, cave.pos[2] + k, EMPTY);
                    }
                    // randomly place resources
                    else{
                        if (cave.pos[1] + j < 128 &&
                            getBlockAt(cave.pos[0] + i, cave.pos[1] + j, cave.pos[2] + k) != EMPTY &&
                            r1 < 0.01f){
                            setBlockAt(cave.pos[0] + i, cave.pos[1] + j, cave.pos[2] + k, GOLDORE);
                        }
                    }
                }
            }
        }
        // move turtle
        cave.step();
    }
    // excavate cavern
    for(int i = -cavernSize; i <= cavernSize; ++i){
        for(int k = -cavernSize; k <= cavernSize; ++k){
            int q = glm::floor(cavernSize * cave.fbm(cave.pos[0] + i + 0.5f, cave.pos[2] + k + 0.5f, 0.25, 6));
            r1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            if (r1 < 0.95){
                for(int j = -q; j <= q; ++j){
                    if (i*i + j*j + k*k < cavernSize * cavernSize){
                        setBlockAt(cave.pos[0] + i, cave.pos[1] + j, cave.pos[2] + k, EMPTY);
                    }
                }
            }
        }
    }
    // flood with lava
    for(int i = -cavernSize; i <= cavernSize; ++i){
        for(int k = -cavernSize; k <= cavernSize; ++k){
            for(int j = -cavernSize; j < -cavernSize + (cavernSize / 2); ++j){
                if (getBlockAt(cave.pos[0] + i, cave.pos[1] + j, cave.pos[2] + k) == EMPTY){
                    setBlockAt(cave.pos[0] + i, cave.pos[1] + j, cave.pos[2] + k, LAVA);
                }
            }
        }
    }
    // middle dais
    int daisRadius = 5;
    for(int i = -daisRadius; i <= daisRadius; ++i){
        for(int k = -daisRadius; k <= daisRadius; ++k){
            for(int j = -cavernSize; j < 1; ++j){
                if (i*i + k*k < daisRadius * daisRadius){
                    setBlockAt(cave.pos[0] + i, cave.pos[1] + j, cave.pos[2] + k, STONE);
                }
            }
        }
    }
}

int Terrain::getHeightAt(const glm::ivec2 pos) const{
    int x = pos[0];
    int z = pos[1];
    BlockType t = getBlockAt(x, 255, z);
    int yMin = 255;
    for(int i = 255; t == EMPTY; i--){
        t = getBlockAt(x, i, z);
        yMin = i;
    }
    return yMin;
}

void Terrain::interpolateTerrain(TerrainType* tA, TerrainType* tB, BlockType b,  // interpolates between two terrain types in the x-direction
                                 const int chunkWorldX, const int chunkWorldZ, Chunk *const chunk)
{
    // 0 -> 127 is BEDROCK
    for(int x = 0; x < 16; ++x){
        for(int z = 0; z < 16; ++z){
            for(int y = 0; y < 128; ++y){
                chunk->getBlockType(x, y, z) = BEDROCK;
            }
        }
    }
    // 128 -> height - 1 is DIRT, height is BlockType b
    float greyscaleA;
    float greyscaleB;
    int heightA;    // height terrain A would draw with
    int heightB;    // height terrain B would draw with
    int height;     // height to draw with
    for(int x = 0; x < 16; ++x){
        for(int z = 0; z < 16; ++z){
            greyscaleA = tA->fbm(x + chunkWorldX + 0.5f, z + chunkWorldZ + 0.5f, tA->getPersistance(), tA->getOctaves());
            heightA = tA->mapToHeight(greyscaleA);
            greyscaleB = tB->fbm(x + chunkWorldX + 0.5f, z + chunkWorldZ + 0.5f, tB->getPersistance(), tB->getOctaves());
            heightB = tB->mapToHeight(greyscaleB);
            height = glm::mix(heightA, heightB, (float)(x) / 16.f);
            for(int y = 128; y < height; ++y){
                chunk->getBlockType(x, y, z) = DIRT;
            }
            chunk->getBlockType(x, height, z) = b;
        }
    }
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

float TerrainType::rand3D(const glm::vec3 n) const{
    // return pseudorandom number between -1 and 1
    return rand(glm::vec2(n[0], n[1]));
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

float TerrainType::interpNoise3D(const float x, const float y, const float z) const{
    float intX = glm::floor(x);
    float fractX = glm::fract(x);
    float intY = glm::floor(y);
    float fractY = glm::fract(y);
    float intZ = glm::floor(z);
    float fractZ = glm::fract(z);

    float v1 = rand3D(glm::vec3(intX, intY, intZ));
    float v2 = rand3D(glm::vec3(intX + 1, intY, intZ));

    float v3 = rand3D(glm::vec3(intX, intY + 1, intZ));
    float v4 = rand3D(glm::vec3(intX + 1, intY + 1, intZ));

    float v5 = rand3D(glm::vec3(intX, intY, intZ + 1));
    float v6 = rand3D(glm::vec3(intX + 1, intY, intZ + 1));

    float v7 = rand3D(glm::vec3(intX, intY + 1, intZ + 1));
    float v8 = rand3D(glm::vec3(intX + 1, intY + 1, intZ + 1));

    // interpolate for smooth transitions
    float i1 = glm::mix(v1, v2, fractX);
    float i2 = glm::mix(v3, v4, fractX);

    float i3 = glm::mix(v5, v6, fractX);
    float i4 = glm::mix(v7, v8, fractX);

    float ii1 = glm::mix(i1, i3, fractZ);
    float ii2 = glm::mix(i2, i4, fractZ);

    return glm::mix(ii1, ii2, fractY);
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

float TerrainType::fbm3D(const float x, const float y, const float z, const float persistance, const int octaves) const{
    float total = 0.0f;

    for(int i = 0; i < octaves; i++){
        float freq = pow(2.0f, i);
        float amp = pow(persistance, i);

        total += amp * interpNoise3D(x * freq * 16 / resolution, y * freq * 64 / resolution, z * freq * 32 / resolution);
    }

    return (1.0f + total) / 2.0f;  // pseudorandom number between 0 and 1

}

// returns random vec2
glm::vec2 TerrainType::random2(const glm::vec2 p) const{
    float first = glm::fract(sinf(15.3 * glm::dot(p, glm::vec2(62.1, 311.7))) * 4375.545243);

    float second = glm::fract(sinf(1234.9 * glm::dot(p, glm::vec2(269.5, 183.3))) * 4375.545243);
    return glm::vec2(first, second);
}

float TerrainType::distance(const glm::vec2 p1, const glm::vec2 p2) const{
    glm::vec2 l = p1 - p2;
    return glm::sqrt(glm::dot(l, l));
}

float TerrainType::worley(const int x, const int z) const{
    glm::vec2 cell_ID = glm::vec2(x / 16, z / 16);
    glm::vec2 cell_fract = glm::vec2((float)(x % 16) / 16.f, (float)(z % 16) / 16.f);
    glm::vec2 cell_uv = cell_ID + cell_fract;

    float minDistance = 64.f;
    glm::vec2 currCell_ID;
    glm::vec2 randPoint;

    for(int j = -1; j <= 1; j++){
        for(int i = -1; i <= 1; i++){
            currCell_ID = glm::vec2(cell_ID[0] + i, cell_ID[1] + j);
            randPoint = glm::vec2(currCell_ID + rand(currCell_ID));
            float dist = distance(randPoint, cell_uv);
            if (dist < minDistance){
                minDistance = dist;
            }
        }
    }

    //float cellProb = fbm(cell_ID[0], cell_ID[1], persistance, octaves);
    if(1.0 - minDistance > 1.0){
        return 1.0;
    }
    return 1.0 - minDistance;
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

void Highland::drawMe(const int chunkWorldX, const int chunkWorldZ, Chunk *const chunk) const{

    // 0 -> 127 is BEDROCK
    for(int x = 0; x < 16; ++x){
        for(int z = 0; z < 16; ++z){
            for(int y = 0; y < 128; ++y){
                chunk->getBlockType(x, y, z) = BEDROCK;
            }
        }
    }
    // 128 -> height - 1 is DIRT, height is GRASS
    float greyscale;
    int height;
    for(int x = 0; x < 16; ++x){
        for(int z = 0; z < 16; ++z){
            greyscale = fbm(x + chunkWorldX + 0.5f, z + chunkWorldZ + 0.5f, persistance, octaves);
            height = mapToHeight(greyscale);
            for(int y = 128; y < height; ++y){
                chunk->getBlockType(x, y, z) = DIRT;
            }
            chunk->getBlockType(x, height, z) = GRASS;
        }
    }
}

Foothills::Foothills() :
    TerrainType(4, 0.4f, 25.0f, 0.4f)
{}

Foothills::~Foothills(){}

void Foothills::drawMe(const int chunkWorldX, const int chunkWorldZ, Chunk *const chunk) const{

    // 0 -> 127 is BEDROCK
    for(int x = 0; x < 16; ++x){
        for(int z = 0; z < 16; ++z){
            for(int y = 0; y < 128; ++y){
                chunk->getBlockType(x, y, z) = BEDROCK;
            }
        }
    }
    // 128 -> height - 1 is DIRT, height is GRASS or SNOW
    float greyscale;
    int height;
    for(int x = 0; x < 16; ++x){
        for(int z = 0; z < 16; ++z){
            greyscale = fbm(x + chunkWorldX + 0.5f, z + chunkWorldZ + 0.5f, persistance, octaves);
            height = mapToHeight(greyscale);
            for(int y = 128; y < height; ++y){
                chunk->getBlockType(x, y, z) = DIRT;
            }
            if (height < 154){
                chunk->getBlockType(x, height, z) = SNOW;
            }
            else{
                chunk->getBlockType(x, height, z) = GRASS;
            }
        }
    }
}

SnowMountains::SnowMountains() :
    TerrainType(4, 0.4f, 20.0f, 0.65f)
{}

SnowMountains::~SnowMountains(){}

void SnowMountains::drawMe(const int chunkWorldX, const int chunkWorldZ, Chunk *const chunk) const{

    // 0 -> 127 is BEDROCK
    for(int x = 0; x < 16; ++x){
        for(int z = 0; z < 16; ++z){
            for(int y = 0; y < 128; ++y){
                chunk->getBlockType(x, y, z) = BEDROCK;
            }
        }
    }
    // 128 -> height - 1 is DIRT, height is STONE or SNOW
    float greyscale;
    int height;
    for(int x = 0; x < 16; ++x){
        for(int z = 0; z < 16; ++z){
            greyscale = fbm(x + chunkWorldX + 0.5f, z + chunkWorldZ + 0.5f, persistance, octaves);
            height = mapToHeight(greyscale);
            for(int y = 128; y < height; ++y){
                chunk->getBlockType(x, y, z) = STONE;
            }
            if (height < 168){
                chunk->getBlockType(x, height, z) = STONE;
            }
            else{
                chunk->getBlockType(x, height, z) = SNOW;
            }
        }
    }
}

Cave::Cave(glm::ivec3 pos) :
    TerrainType(4, 0.5f, 0.1f, 1.0f), pos(pos)
{}

Cave::~Cave(){}

float Cave::mapToAngle(const float num) const{
    return num * 360.0f;
}

glm::ivec2 Cave::mapToXZOffset(const float angle) const{
    if (angle <= 22.5){
        return glm::ivec2(1, 0);
    }
    else if (angle <= 67.5){
        return glm::ivec2(-1, 0);
    }
    else if (angle <= 112.5){
        return glm::ivec2(1, -1);
    }
    else if (angle <= 157.5){
        return glm::ivec2(-1, 1);
    }
    else if (angle <= 202.5){
        return glm::ivec2(1, 1);
    }
    else if (angle <= 247.5){
        return glm::ivec2(-1, -1);
    }
    else if (angle <= 292.5){
        return glm::ivec2(0, 1);
    }
    else if (angle <= 337.5){
        return glm::ivec2(0, -1);
    }
    else{
        return glm::ivec2(1, 0);
    }
}

int Cave::mapToYOffset(const float angle) const{
    if (angle <= 50.0){
        return 0;
    }
    else if (angle <= 145.0){
        return 1;
    }
    else if (angle <= 195.0){
        return 0;
    }
    else{
        return -1;
    }
}

void Cave::step(){
    float theta = fbm3D(pos[0] + 0.5, pos[1] + 0.5, pos[2] + 0.5, persistance, octaves);
    theta = mapToAngle(theta);
    glm::ivec2 xzOffset = mapToXZOffset(theta);
    float phi = static_cast <float> (::rand()) / static_cast <float> (RAND_MAX);
    phi = mapToAngle(phi);
    int yOffset = mapToYOffset(phi);

    pos = glm::ivec3(pos[0] + xzOffset[0], pos[1] + yOffset, pos[2] + xzOffset[1]);
}

Forest::Forest() :
    TerrainType(4, 0.4f, 20.0f, 0.2f)
{}

Forest::~Forest(){}
















