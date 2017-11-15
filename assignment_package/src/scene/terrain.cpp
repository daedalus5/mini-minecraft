#include <scene/terrain.h>

#include <scene/cube.h>

Terrain::Terrain() : dimensions(64, 256, 64)
{}

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

Chunk::Chunk(OpenGLContext* context)
    : Drawable(context), dimensions(16, 256, 16) {
    size = dimensions[0] * dimensions[1] * dimensions[2];
}

BlockType Chunk::getBlockType(int x, int y, int z) const {
    glm::ivec3 temp = getPosition(x + 16 * y + 16 * 256 * z);
    return block_array[x + 16 * y + 16 * 256 * z];
}

BlockType& Chunk::getBlockType(int x, int y, int z) {
    glm::ivec3 temp = getPosition(x + 16 * y + 16 * 256 * z);
    return block_array[x + 16 * y + 16 * 256 * z];
}

void Chunk::create() {
    std::vector<glm::vec4> positions = std::vector<glm::vec4>();
    std::vector<glm::vec4> normals = std::vector<glm::vec4>();
    std::vector<glm::vec4> colors = std::vector<glm::vec4>();
    std::vector<GLuint> indices = std::vector<GLuint>();

    glm::vec4 grass_color = glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f;
    glm::vec4 dirt_color = glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f;
    glm::vec4 stone_color = glm::vec4(0.5, 0.5, 0.5, 1);

    glm::vec4 x_normal = glm::vec4(1, 0, 0, 0);
    glm::vec4 y_normal = glm::vec4(0, 1, 0, 0);
    glm::vec4 z_normal = glm::vec4(0, 0, 1, 0);

    for (int i = 0; i < size; i++) {
        BlockType block = block_array[i];
        if (block != EMPTY) {
            glm::vec4 pos = glm::vec4(getPosition(i), 1);

            glm::vec4 col = glm::vec4(0, 0, 0, 1);
            switch (block_array[i]) {
                case DIRT:
                    col = dirt_color;
                    break;
                case GRASS:
                    col = grass_color;
                    break;
                case STONE:
                    col = stone_color;
                    break;
            }


            // Check neighboring x by +/- 1
            if (pos[0] == dimensions[0] - 1 || getBlockType(pos[0] + 1, pos[1], pos[2]) == EMPTY) {
                addSquare(pos, x_normal, col, glm::vec4(0.5, 0.5, 0.5, 1), positions, normals, colors, indices);
            }
            if (pos[0] == 0 || getBlockType(pos[0] - 1, pos[1], pos[2]) == EMPTY) {
                addSquare(pos, -x_normal, col, glm::vec4(-0.5, 0.5, 0.5, 1), positions, normals, colors, indices);
            }
            // Check neighboring y by +/- 16
            if (pos[1] == dimensions[1] - 1 || getBlockType(pos[0], pos[1] + 1, pos[2]) == EMPTY) {
                addSquare(pos, y_normal, col, glm::vec4(0.5, 0.5, 0.5, 1), positions, normals, colors, indices);
            }
            if (pos[1] == 0 || getBlockType(pos[0], pos[1] - 1, pos[2]) == EMPTY) {
                addSquare(pos, -y_normal, col, glm::vec4(0.5, -0.5, 0.5, 1), positions, normals, colors, indices);
            }
            // Check neighboring z by +/- 16 * 256
            if (pos[2] == dimensions[2] - 1 || getBlockType(pos[0], pos[1], pos[2] + 1) == EMPTY) {
                addSquare(pos, z_normal, col, glm::vec4(0.5, 0.5, 0.5, 1), positions, normals, colors, indices);
            }
            if (pos[2] == 0 || getBlockType(pos[0], pos[1], pos[2] - 1) == EMPTY) {
                addSquare(pos, -z_normal, col, glm::vec4(0.5, 0.5, -0.5, 1), positions, normals, colors, indices);
            }


        }
    }

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

glm::ivec3 Chunk::getPosition(int i) const {
    glm::ivec3 result;
    result[2] = i / (dimensions[0] * dimensions[1]);
    i = i - (result[2] * dimensions[0] * dimensions[1]);
    result[1] = i / dimensions[0];
    result[0] = i % dimensions[0];
    return result;
}

void Chunk::addSquare(glm::vec4 pos, glm::vec4 normal, glm::vec4 color,
                      glm::vec4 squareStart,
                      std::vector<glm::vec4> &positions,
                      std::vector<glm::vec4> &normals,
                      std::vector<glm::vec4> &colors,
                      std::vector<GLuint> &indices) {

    // grab size of positions
    int index = positions.size();
    // Rotate by 90 degrees 4 times
    // Push them into positions vector
    // Push normals
    // Push colors
    for (int k = 0; k < 4; k++) {
        positions.push_back(glm::translate(glm::mat4(), glm::vec3(pos))
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

GLenum Chunk::drawMode()
{
    return GL_TRIANGLES;
}
