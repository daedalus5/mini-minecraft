# Mini Minecraft
## Milestone 3
### Team: Merge Conflict

##### Sagar- Sandbox Mode, Lava and Water Collisions, Multithreading

Sandbox mode was straightforward given what we implemented in Milestone 1. I disabled collisions with lava and water blocks, and slowed the player's velocity down when moving through Lava or Water. I also modified the vertical deceleration when the player is above a water or lava block to create a sinking effect. 

To create the overlay, I modified the existing lambert shader to mix in a higher contribution of red or blue depending on whether the player was submerged in water or lava. I also modified the sky color seperately in these states by creating a new vec4 variable called skyColor and passing it to glClearColor when the player is submerged.

To improve the efficiency of rendering and terrain generation, I have a thread (apart from the main thread)which invokes the terrain height field generation, and the adding of new chunks in a new class which subclasses from QRunnable. I start this thread in the InitializeGL function. 

#Sounds

To improve the aesthetic of the game, I used the QMediaPlayer class to play themes for different areas and states of the game. Currently, there is a theme for the title screen, a theme for when the player is under water, in a cavern, and when the player is traversing the world. These themes were composed on Garageband and imported into the resources folder of the project.

#PlayerDeath

The player has health which deprecates when the player comes in contact with LAVA. If the player stays in contact with a LAVA block for a long period of time, he dies and the game enters sandbox mode.

# Procedural Sky - 

I tried implementing the Procedural Sky based on Adam's demo in class. This crashes and so I did not merge it with the main milestone code. As of now, it pretty much follows the demo code almost exactly as i'm trying to get the base implementation working', though I have tried to comment as scrupulously as I can. 
The code for this is on my branch which I will include in my commit link textfile.





##### Connie Chang - Shadow Map, Title Screen, Distance Fog
I implemented distance fog in milestone 2. The weight of the fog depends on the distance of the object, so the weight is larger as the object gets farther. I shifted the function out by about 100, so there would be a radius around the player without no fog at all.

I also made the title screen. It's just a texture on a quad overlay. I did a lot of refactoring for this, creating GameState classes. I moved all of the game logic to PlayState. The title screen is drawn in MenuState. MyGL has a pointer to GameState, which can be switched between PlayState and MenuState, and calls its update and paint functions.

Last, I implemented shadow mapping. I have a light camera with a orthographic projection matrix that is above the player. I render a first pass using the light camera, writing to a depth texture. On the second pass, I render the final scene, and use the depth texture to determine if there is a shadow. If the current fragment is farther away than the depth in the depth texture, then a shadow is drawn. I had to add a little bias to get rid of shadow acne, but now there is Peter Panning. The depth texture resolution is also not too high, so the shadows come out jagged and blocky.


##### Zach Corse - MS3: L-System Trees, Worley Noise Forests, 3D FBM Random Walk Caves & Cavern, Biomes, and Biome Interpolation
Previous milestones: L-System Rivers, 2D FBM Terrain Generation, Block Creation and Destruction, Crosshairs

--- L-System Trees --- 

The branches (leaf blocks) of each tree were individually shaped using a standard L-System grammar "X -> [-FX]+FX". Branching probability was not incorporated, but each branch was given a random orientation at each branching event. This gives each tree a unique appearance while maintaining the fullness of its appearance. The leaves/branches were perched atop a tree trunk given a random height between 5 and 9 blocks high.

Each tree begins with a 2D L-System. The blocks this L-System intersects in the world are drawn as leaf blocks. The 2D L-System is then rotated by 180 degrees to sweep out the full three-dimensional tree.

--- Worley Noise Forests ---

Trees were distributed within a forest using Worley noise. A cell grid was layered atop the terrain, with a single cell occupying the width and length of a terrain chunk. The function was written such that trees closest to the random point distributed within a cell had a higher probability of being drawn. This left realistic-looking gaps, or small clearings, in the forest, as though a space had opened in the canopy because a couple of trees fell.

--- 3D FBM Random Walk Caves & Cavern ---

I wrote a 3D FBM function to generate a cave "worm" that eats into the terrain in the (generally speaking) downward direction. The worm moves in one block intervals. It uses the 3D FBM function to generate a random angle between 0 and 360 degrees at each step. These angles are mapped to block steps to be carried out when the worm steps next. There are 8 blocks in the xz plane the worm can step to next, so the range [0, 360] is subdivided evenly between these options. I found that this map gives reasonable stretches of linear movement while still having a sufficient amount of path deviation to make the player's descent interesting. The same idea is applied to movement in the y-direction, however, more bias is given to movement in the downward direction, because we want our caves to tunnel in that general direction while still allowing for some upward deviations.

The cave is excavated by hollowing out a sphere from the earth centered at the worm's position. The worm is directed to begin at the terrain surface, so the player can "discover" this cave while traversing the terrain. Additionally, gold ore blocks are randomly distributed along the walls of the cave. Also, I used Connie's distance fog when the player moves underground. The fog instead interpolates using black, and appears much closer to the player than the world fog to give the impression the player is descending into a dark cave carrying only a lantern.

The cavern is excavated using 2D FBM noise. By choosing the right noise function sampling interval, the cavern can be excavated identically upwards and downwards in such a way that it looks as though stalactites are hanging from the ceiling and stalagmites are growing from the ground. Some of these meet to form "columns", a typical feature found in caves. The bottom of the cavern is flooded with lava, and a circular stone dais is placed in the center of the cavern as a point of interest for the player. To visit this dais, the player must risk the pit of lava.

--- Biomes ---

There are two major biomes in the world we've created - riverlands, and snowy mountains. The riverland chunks have small grassy hillocks, rivers, and forests. The snowy mountains have tall, sharp, snowy peaks and stone basins in lower-lying areas.

--- Biome Interpolation ---

There is a third, intermediary biome in this scene, that is meant to act as a transition biome between the two identifiable ones noted above. We call this biome the "foothills." It is meant to provide a visual transition between riverlands and snowy mountins. It resembles the riverlands but has sharper, taller hills, and in basins formed between multiple hills, it has snow blocks. These are meant to mimic the fact that basins receive less sunlight than hilltops, so snow will generally persist here the longest.

Additionally, there is a function that can interpolate between two chunks of different biomes. It interpolates the height map of the two chunks over the span of a single chunk, and is passed the BlockType that should be placed at the top of this interpolated height.

Using an intermediatry biome and biome interpolation, we were able to craft the scene in our commit. It is a forested river valley situated between two mountain ranges. At the bottom is the river, which originates as a "linear" river to the south and ends in a delta river to the north. Moving either east or west, perpendicular to the river's direction of travel, the player will move up a hill interpolating between riverlands and foothills, cross the foothills (passing over smaller patches of forest and snowy basins), move up another hill interpolating between foothills and snowy mountains, and then be in the cold, forbidding snowy mountains themselves.

--- Crosshairs ---

Added crosshairs to the center of the screen so the player knows where he/she is looking/aiming.

Previous Milestones:

--- L-System Rivers ---

I worked with two L-System grammars to generate two different river systems. The first is what one would think of as a "normal" river, which includes the possibility of small creek offshoots. The second is a river delta, which includes more frequent branching and varying branch thicknesses according to turtle recursion depth.

The "normal" river grammar is:
X -> [+FX]+--+FY
Y -> [-FY]+--+FX
the difference in X and Y rules allows creeks to offshoot to the left and right of the main river. +--+FX is meant to direct the primary river branch forward.

The river delta grammar is:
X->[-FX]+FX

Both river types include a probability of branch generation. If a branch is to be drawn, it uses the L-System grammar described above. Otherwise, it uses "X -> +--+FX". They're both drawn by the "turtle" method we discussed in class. (-) turns the turtle left, and (+) turns the turtle right. Both incorporate some randomness. Hence, I decided to use +--+, which is more random than +-, when I wanted the river to travel (generally speaking) in the forward direction.

The turtle step size is set to 4 blocks. I found that this step size allowed the river to "meander" naturally. In order to maintain a natural amount of branching, this meant that the probability of branch generation had to be kept relatively small (as there were many potential branching events).

The rivers have banks as well. These are carved away from the surrouding landscape in L shapes (1 up, 2 to the side). The number of these "L-shelves" can be specified, but 5 seems to work well. The cubes beneath the river are set to sand.

***Lastly, and importantly I've connected the two rivers, so that it appears that the primary river meanders for awhile then terminates in a river delta. What looks like a single river is in fact two rivers. One is initialized at (0,0) and is directed North, and the other is intialized at (0,0) and is directed South.

--- 2D FBM Terrain Generation ---

Each terrain height field is generated using 2D FBM noise. I had to adjust the random noise function sampling interval to get reasonable-looking terrain. There is also a dampening parameter, which is applied to smooth the terrain even further, and controls hill peak to trough displacement.

--- Block Creation and Destruction ---

Block Destruction: A ray is cast down the player's look vector. The Kay & Kayjia ray-box intersection algorithm is used to determine whether the player's look ray intersects with one or more of the cubes surrounding the player. We currently test against 3 x 4 x 3 - 1 = 35 cubes (we test two cubes beneath the player's eye because the player is two cubes tall). This test produces the shortest ray intersection distance, and uses that value to destroy the cube associated with the intersection point at that distance along the look ray.

Block Creation: A ray is cast down the player's look vector. It looks for a box intersection two look vectors away. If there is a box intersection, it detects the closest face of the box. If that face is adjacent to an EMPTY cube, a new box is constructed in that cube of space.
