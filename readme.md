# Mini Minecraft
## Milestone 1
### Team: Merge Conflict

##### Sagar- Camera Controls, Player Physics, Collisions.

For collision detection, I tried to predict the next position of each vertex of the bounding box of the player, and then preemptively stop the motion of the player if a potential collision was detected. The main challenges I faced with this assignment were the implementation of the collision detection and keeping track of the flow of information across the files.

##### Connie Chang - Chunking, Efficient VBOs/Rendering

Chunking is pretty straightforward. It is an array of 16 x 256 x 16 BlockTypes. I used an unordered map to keep track of Chunks in the Terrain class. The key was a uint64_t where the first 32 bits are the x position and last 32 bits are the z position. The Terrain class creates the data for VBO generation, which is passed to corresponding Chunk. That way, I can check neighboring Chunks for EMPTY blocks. The VBOs are interleaved with position, normal, and color all together, so I had to modify Drawable.cpp and ShaderProgram.cpp. To draw the Chunks, I looped through 10 Chunks in each direction (+x, -x, +z, -z) in MyGL::GLDrawScene and drew them. If the Chunk did not exist in that position, I would create a new one. Creating 20 new Chunks at once is a little laggy, taking about 1 - 2 seconds.

To make VBO data generation even more efficient, I stored variables that are used over and over again (such as the 6 possible normals for a grid-aligned square) as const member variables in Terrain. I also made a color map to map from BlockType to a glm::vec4 color. To add even more efficiency, I modified Zach's Terrain::CreateScene method. Originally, we had him use Terrain::setBlockAt by passing in the world coordinates. However, all the coordinates he passed in were part of the same Chunk. So having setBlockAt convert from world coordinates to Chunk was a lot of extra overhead to call on every Block and receive the same result on all of them. Therefore, I changed the method to directly use the Chunk's getBlockAt. 

