#include <scene/terrain.h>

#include <scene/cube.h>

Terrain::Terrain(OpenGLContext* in_context) : dimensions(64, 256, 64),
    chunk_map(std::unordered_map<glm::ivec2, Chunk*, KeyFuncs, KeyFuncs>()),
    color_map(std::map<BlockType, glm::vec4>()), context(in_context)
{
    color_map[GRASS] = glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f;
    color_map[DIRT] = glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f;
    color_map[STONE] = glm::vec4(0.5, 0.5, 0.5, 1);
    color_map[LAVA] = glm::vec4(207.f, 16.f, 32.f, 255.f) / 255.f;
}

Terrain::~Terrain() {
    for ( auto it = chunk_map.begin(); it != chunk_map.end(); ++it ) {
        delete it->second;
    }
}

BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    // TODO: Make this work with your new block storage!
    if (y < 0 || y > 255) {
        return EMPTY;
    }

    glm::ivec2 chunk_pos = getChunkPosition(x, z);
    auto index = chunk_map.find(chunk_pos);
    if (index != chunk_map.end()) {
        Chunk* ch = index->second;
        glm::ivec3 local_pos = getChunkLocalPosition(x, y, z);
        return ch->getBlockType(local_pos[0], local_pos[1], local_pos[2]);
    }
    return EMPTY;
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    // TODO: Make this work with your new block storage!
    if (y < 0 || y > 255) {
        return;
    }

    glm::ivec2 chunk_pos = getChunkPosition(x, z);
    Chunk* ch;
    auto index = chunk_map.find(chunk_pos);
    if (index == chunk_map.end()) {
        ch = new Chunk(context);
        chunk_map[chunk_pos] = ch;
    } else {
        ch = chunk_map[chunk_pos];
    }
    glm::ivec3 local_pos = getChunkLocalPosition(x, y, z);
    ch->getBlockType(local_pos[0], local_pos[1], local_pos[2]) = t;
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
                        //m_blocks[x][y][z] = STONE;
                        setBlockAt(x, y, z, STONE);
                    }
                    else
                    {
                        //m_blocks[x][y][z] = DIRT;
                        setBlockAt(x, y, z, DIRT);
                    }
                }
                else
                {
                    //m_blocks[x][y][z] = EMPTY;
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
//        m_blocks[x][129][0] = GRASS;
//        m_blocks[x][130][0] = GRASS;
//        m_blocks[x][129][63] = GRASS;
//        m_blocks[0][130][x] = GRASS;

        setBlockAt(x, 129, 0, GRASS);
        setBlockAt(x, 130, 0, GRASS);
        setBlockAt(x, 129, 63, GRASS);
        setBlockAt(0, 130, x, GRASS);
    }
    for(int y = 129; y < 140; ++y)
    {
        //m_blocks[32][y][32] = GRASS;
        setBlockAt(32, y, 32, GRASS);
    }
}

void Terrain::addBlockAt(int x, int y, int z, BlockType t) {
    setBlockAt(x, y, z, t);
    updateChunkVBO(x, z);
}

void Terrain::destroyBlockAt(int x, int y, int z) {
    setBlockAt(x, y, z, EMPTY);
    updateChunkVBO(x, z);
}

Chunk* Terrain::getChunk(int x, int z) const {
    glm::ivec2 chunk_pos = getChunkPosition(x, z);
    auto index = chunk_map.find(chunk_pos);
    if (index != chunk_map.end()) {
        return index->second;
    }
    return nullptr;
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
void Terrain::updateChunkVBO(int x, int z) {
    glm::ivec2 chunk_pos = getChunkPosition(x, z);
    auto index = chunk_map.find(chunk_pos);
    if (index == chunk_map.end()) {
        return;
    }

    Chunk* ch = index->second;

    std::vector<glm::vec4> positions = std::vector<glm::vec4>();
    std::vector<glm::vec4> normals = std::vector<glm::vec4>();
    std::vector<glm::vec4> colors = std::vector<glm::vec4>();
    std::vector<GLuint> indices = std::vector<GLuint>();

    glm::vec4 x_normal = glm::vec4(1, 0, 0, 0);
    glm::vec4 y_normal = glm::vec4(0, 1, 0, 0);
    glm::vec4 z_normal = glm::vec4(0, 0, 1, 0);

    for (int i = 0; i < ch->chunk_dimensions[0]; i++) {
        for (int j = 0; j < ch->chunk_dimensions[1]; j++) {
            for (int k = 0; k < ch->chunk_dimensions[2]; k++) {

                BlockType block = ch->getBlockType(i, j, k);

                if (block != EMPTY) {

                    glm::vec4 world_pos = glm::vec4(i, j, k, 1)
                            + glm::vec4(chunk_pos[0] * ch->chunk_dimensions[0], 0, chunk_pos[1] * ch->chunk_dimensions[2], 0);

                    glm::vec4 col = color_map[block];

                    // Check neighboring x
                    if (getBlockAt(world_pos[0] + 1, world_pos[1], world_pos[2]) == EMPTY) {
                        addSquare(world_pos, x_normal, col, glm::vec4(0.5, 0.5, 0.5, 1), positions, normals, colors, indices);
                    }
                    if (getBlockAt(world_pos[0] - 1, world_pos[1], world_pos[2]) == EMPTY) {
                        addSquare(world_pos, -x_normal, col, glm::vec4(-0.5, 0.5, 0.5, 1), positions, normals, colors, indices);
                    }

                    // Check neighboring y
                    if (getBlockAt(world_pos[0], world_pos[1] + 1, world_pos[2]) == EMPTY) {
                        addSquare(world_pos, y_normal, col, glm::vec4(0.5, 0.5, 0.5, 1), positions, normals, colors, indices);
                    }
                    if (getBlockAt(world_pos[0], world_pos[1] - 1, world_pos[2]) == EMPTY) {
                        addSquare(world_pos, -y_normal, col, glm::vec4(0.5, -0.5, 0.5, 1), positions, normals, colors, indices);
                    }

                    // Check neighboring z
                    if (getBlockAt(world_pos[0], world_pos[1], world_pos[2] + 1) == EMPTY) {
                        addSquare(world_pos, z_normal, col, glm::vec4(0.5, 0.5, 0.5, 1), positions, normals, colors, indices);
                    }
                    if (getBlockAt(world_pos[0], world_pos[1], world_pos[2] - 1) == EMPTY) {
                        addSquare(world_pos, -z_normal, col, glm::vec4(0.5, 0.5, -0.5, 1), positions, normals, colors, indices);
                    }
                }

            }
        }
    }
    ch->createVBO(positions, normals, colors, indices);
}

void Terrain::updateAllVBO() {
    for ( auto it = chunk_map.begin(); it != chunk_map.end(); ++it ) {
        updateChunkVBO(16 * it->first[0], 16 * it->first[1]);
    }
}

void Terrain::addSquare(glm::vec4 pos, glm::vec4 normal, glm::vec4 color,
                      glm::vec4 squareStart,
                      std::vector<glm::vec4> &positions,
                      std::vector<glm::vec4> &normals,
                      std::vector<glm::vec4> &colors,
                      std::vector<GLuint> &indices) {

    // grab size of positions
    int index = positions.size();

    for (int k = 0; k < 4; k++) {
        // Rotate by 90 degrees 4 times
        // Push them into positions vector
        // Push normals
        // Push colors
        positions.push_back(glm::translate(glm::mat4(), glm::vec3(pos) + glm::vec3(0.5, 0.5, 0.5))
                            * glm::rotate(glm::mat4(), glm::radians(90.f * k), glm::vec3(normal))
                            * squareStart);
        normals.push_back(normal);
        colors.push_back(color);
    }
    // Push indices
    indices.push_back(index);
    indices.push_back(index + 1);
    indices.push_back(index + 2);
    indices.push_back(index);
    indices.push_back(index + 2);
    indices.push_back(index + 3);

}

// TERRAIN END
//*********************************************************************************************
// CHUNK START

Chunk::Chunk(OpenGLContext* context)
    : Drawable(context), chunk_dimensions(16, 256, 16), block_array({EMPTY})
{
}

BlockType Chunk::getBlockType(int x, int y, int z) const {
    return block_array[x + 16 * y + 16 * 256 * z];
}

BlockType& Chunk::getBlockType(int x, int y, int z) {
    return block_array[x + 16 * y + 16 * 256 * z];
}

void Chunk::createVBO(std::vector<glm::vec4> &positions,
                      std::vector<glm::vec4> &normals,
                      std::vector<glm::vec4> &colors,
                      std::vector<GLuint> &indices)
{
    count = indices.size();

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // SPH_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generatePos();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    context->glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec4), positions.data(), GL_STATIC_DRAW);

    generateNor();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    context->glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec4), normals.data(), GL_STATIC_DRAW);

    generateCol();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    context->glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_STATIC_DRAW);
}

void Chunk::create() {
    // no-op
}

GLenum Chunk::drawMode()
{
    return GL_TRIANGLES;
}

void Terrain::CreateHighland(){
    // 0 -> 127 is STONE
    for(int x = 0; x < 64; ++x){
        for(int z = 0; z < 64; ++z){
            for(int y = 0; y < 128; ++y){
                setBlockAt(x, y, z, STONE);
            }
        }
    }
    // 128 -> height - 1 is DIRT, height is GRASS
    for(int x = 0; x < 64; ++x){
        for(int z = 0; z < 64; ++z){
            float persistance = 0.4f;
            int octaves = 4;
            float greyscale = fbm(x + 0.5f, z + 0.5f, persistance, octaves);
            int height = mapToHeight(greyscale);
            for(int y = 128; y < height; ++y){
                setBlockAt(x, y, z, DIRT);
            }
            setBlockAt(x, height, z, GRASS);
        }
    }
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
