# Mini Minecraft
## Milestone 1
### Team: Merge Conflict

##### Sagar- Camera Controls, Player Physics, Collisions.

For collision detection, I tried to predict the next position of each vertex of the bounding box of the player, and then preemptively stop the motion of the player if a potential collision was detected. The main challenges I faced with this assignment were the implementation of the collision detection and keeping track of the flow of information across the files.

##### Connie Chang - Chunking, Efficient VBOs/Rendering

Chunking is pretty straightforward. It is an array of 16 x 256 x 16 BlockTypes. I used an unordered map to keep track of Chunks in the Terrain class. The key was a uint64_t where the first 32 bits are the x position and last 32 bits are the z position. The Terrain class creates the data for VBO generation, which is passed to corresponding Chunk. That way, I can check neighboring Chunks for EMPTY blocks. The VBOs are interleaved with position, normal, and color all together, so I had to modify Drawable.cpp and ShaderProgram.cpp. To draw the Chunks, I looped through 10 Chunks in each direction (+x, -x, +z, -z) in MyGL::GLDrawScene and drew them. If the Chunk did not exist in that position, I would create a new one. Creating 20 new Chunks at once is a little laggy, taking about 1 - 2 seconds.

To make VBO data generation even more efficient, I stored variables that are used over and over again (such as the 6 possible normals for a grid-aligned square) as const member variables in Terrain. I also made a color map to map from BlockType to a glm::vec4 color. To add even more efficiency, I modified Zach's Terrain::CreateScene method. Originally, we had him use Terrain::setBlockAt by passing in the world coordinates. However, all the coordinates he passed in were part of the same Chunk. So having setBlockAt convert from world coordinates to Chunk was a lot of extra overhead to call on every Block and receive the same result on all of them. Therefore, I changed the method to directly use the Chunk's getBlockAt.

##### Zach Corse - Procedural Terrain, Block Creation and Destruction, Crosshairs

Procedural Terrain: This relies on the basic FBM method we learned in class. It samples the given noise function at a finer scale, however, to give the terrain smoother height transitions between neighboring blocks. Additionally, a dampening parameter is introduced to control the general height difference between peaks and valleys in the terrain such that one can choose whether one wants the terrain to consist of hills or mountains.

Block Destruction: A ray is cast down the player's look vector. The Kay & Kayjia ray-box intersection algorithm is used to determine whether the player's look ray intersects with one or more of the cubes surrounding the player. We currently test against 3 x 4 x 3 - 1 = 35 cubes (we test two cubes beneath the player's eye because the player is two cubes tall). This test produces the shortest ray intersection distance, and uses that value to destroy the cube associated with the intersection point at that distance along the look ray.

Block Creation: A ray is cast down the player's look vector. It looks for a box intersection two look vectors away. If there is a box intersection, it detects the closest face of the box. If that face is adjacent to an EMPTY cube, a new box is constructed in that cube of space.

Crosshairs: Added crosshairs to the center of the screen so the player knows where he/she is looking/aiming.
