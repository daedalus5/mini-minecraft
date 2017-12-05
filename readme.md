# Mini Minecraft
## Milestone 2
### Team: Merge Conflict

##### Sagar- Sandbox Mode, Lava and Water Collisions, Multithreading

Sandbox mode was straightforward given what we implemented in Milestone 1. I disabled collisions with lava and water blocks, and slowed the player's velocity down when moving through Lava or Water. I also modified the vertical deceleration when the player is above a water or lava block to create a sinking effect. 

    To create the overlay, I modified the existing lambert shader to mix in a higher contribution of red or blue depending on whether the player was submerged in water or lava. I also modified the sky color seperately in these states by creating a new vec4 variable called skyColor and passing it to glClearColor when the player is submerged.

To improve the efficiency of rendering and terrain generation, I have a thread (apart from the main thread)which invokes the terrain height field generation, and the adding of new chunks in a new class which subclasses from QRunnable. I start this thread in the InitializeGL function. 



##### Connie Chang - Textures and Animation
I created a Texture from the provided image, which the lambert shader samples from. I also added some Blinn-Phong specular light to the shader. For animation, I shifted the UVs based on time to give the illusion of movement. To send all this data to the shader, I passed all of this in a vec4. The first two indices represent the UV. The third is the cosine exponent for Blinn-Phong. The fourth is a flag for animation. Finally, I enabled alpha blending for transparency.

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


