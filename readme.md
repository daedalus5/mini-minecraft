# Mini Minecraft
## Milestone 3
### Team: Merge Conflict

##### Sagar- Sandbox Mode, Lava and Water Collisions, Multithreading

Sandbox mode was straightforward given what we implemented in Milestone 1. I disabled collisions with lava and water blocks, and slowed the player's velocity down when moving through Lava or Water. I also modified the vertical deceleration when the player is above a water or lava block to create a sinking effect. 

    To create the overlay, I modified the existing lambert shader to mix in a higher contribution of red or blue depending on whether the player was submerged in water or lava. I also modified the sky color seperately in these states by creating a new vec4 variable called skyColor and passing it to glClearColor when the player is submerged.

To improve the efficiency of rendering and terrain generation, I have a thread (apart from the main thread)which invokes the terrain height field generation, and the adding of new chunks in a new class which subclasses from QRunnable. I start this thread in the InitializeGL function. 



##### Connie Chang - Shadow Map, Title Screen, Distance Fog
I implemented distance fog in milestone 2. The weight of the fog depends on the distance of the object, so the weight is larger as the object gets farther. I shifted the function out by about 100, so there would be a radius around the player without no fog at all.

I also made the title screen. It's just a texture on a quad overlay. I did a lot of refactoring for this, creating GameState classes. I moved all of the game logic to PlayState. The title screen is drawn in MenuState. MyGL has a pointer to GameState, which can be switched between PlayState and MenuState, and calls its update and paint functions.

Last, I implemented shadow mapping. I have a light camera with a orthographic projection matrix that is above the player. I render a first pass using the light camera, writing to a depth texture. On the second pass, I render the final scene, and use the depth texture to determine if there is a shadow. If the current fragment is farther away than the depth in the depth texture, then a shadow is drawn. I had to add a little bias to get rid of shadow acne, but now there is Peter Panning. The depth texture resolution is also not too high, so the shadows come out jagged and blocky.


##### Zach Corse - L-System Rivers

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


