Assignment 2 Notes:
--------------------------------------------------------------------------------------------------------------------
Because of technical issues, the library GEDUtils for vs2015 is used for the assignment 2.
It works with vs2017 without a problem, just be sure to select cancel, 
when vs2017 asks you if you want to update the library, in case you select OK, just re-clone the repository.

Also make sure to change the start-up project to TerrainGenerator, and enter the command line arguments.
The Server was down at 11.05.2018-01.00, that is why the last push was at 10.20.
--------------------------------------------------------------------------------------------------------------------

Assignment 3 Notes:
--------------------------------------------------------------------------------------------------------------------
Created png files are also inside the project.Without changing the parameters, the files are only overwritten.
Debug and Release folders are ignored by the version control system.Build the project to see the results.
There is a color difference when looking through TerranViewer between color.png and color.tiff (Tiff seems lighter)

Also note that there are two blend weight calculation functions are implemented. One is the basic approach
from the slides and is used, other one is self created (and commented out, works actually 
nicer than the simple approach, in our opinion).

TextureBlending task suggests that the working directory of TerrainGenerator should be changed from $(ProjectDir) to
$(TargetDir), however this causes a conflict with the makefile in the ResourceGenerator, since ResourceGenerator only
works from $(ProjectDir) (Tried to change, just like how it is done in the TerrainGenerator, but no effect). That is
why both projects has $(ProjectDir) as their working directory.

The vs2015 template is used again, make sure to hit cancel when asked for upgrade.
--------------------------------------------------------------------------------------------------------------------

Assignment 4 Notes:
--------------------------------------------------------------------------------------------------------------------
The path directories in the game.cfg changed from "path" to "resources\path", so it works out of the box.
diffuseTexture is safe released just like the assignment wanted, but is not actually created (in the assigment before),
can be commented out.
If your terrain viewer with (tiff files, no dds) does not match with your terrain, it is because of the height scaling
in the game.cfg file, change it to 400.0, and you will see.
The last issue is the the points that get no light is completely dark(in terrain viewer with tiff files it is 
more like shadowed), but I asked this to another tutor, and she said that it was normal since we use a fixed shader
function and will be implemented in later assignments (just like the diffuseTexture). So, the calculation in the code
is solid right. Looks are deceptive :D
debug/release in 32 bit compiles/works, vs2015 template again.

***Assignment 4 Questions***
--------------------------------------------------------------------------------------------------------------------
1) If we want to use the primitive Draw() method, then we will only use the vertex buffer to store vertices. This causes the fact that we have to
to store some vertices multiple times in the vertex buffer which are used by the same triangles. With this way, the vertex buffer loading process would be
complicated, meaning for each index we have in the index buffer, we put the information(position, normal and texture coordinates) of that vertex in the vertex buffer, 
without changing the order of the indices in a triangle (see the initData in Terrain.cpp). If we draw the indices by an index buffer, the vertex indices are 
stored in the index buffer which point to the vertices from the vertex buffer so that the renderer has no performance issues if it wants to access to a shared vertex.
Also if we use DrawIndexed(), we prevent the redundant repeating data in the vertex buffer, which saves memory. A random example to illustrate the difference:

Given a vertex and index buffer for method DrawIndexed():
IndexBuffer = {0, 1, 4, 4, 1, 5, 1, 2, 5, 5, 2, 6};
VertexBuffer = {Vertex_0, Vertex_1, Vertex_2, Vertex_3, Vertex_4, Vertex_5, Vertex_6};

To render with the method Draw() in the same way as DrawIndexed() we would directly put the information in IndexBuffer to VertexBuffer, so:
VertexBuffer = {Vertex_0, Vertex_1, Vertex_4, Vertex_4, Vertex_1, Vertex_5, Vertex_1, Vertex_2, Vertex_5, Vertex_5, Vertex_2, Vertex_6};
Don't forget you had to store ten floating points for each vertex in vertex buffer (this is why you get redundant data and memory loss). 

2) A = (3, 0, 6); B = (0, 2, 0); C = (6, 0, 0); c_a = (1, 0, 0); c_b = (0, 1, 1); c_c = (1, 0, 1)
   Find the color values c_p of point P = (2, 1, 2):

   With barycentic coordinates we get the equations:
   c_p = alpha1 * c_a + alpha2 * c_b + alpha3 * c_c and alpha1 + alpha2 + alpha3 = 1; 
   all alphas lie in [0,1] according to the inside triangle criteria;
   Also to calculate alphas we use:
   alpha1 = Area(PBC) / Area(ABC)
   alpha2 = Area(PCA) / Area(ABC)
   alpha3 = Area(PAB) / Area(ABC)

   To calculate the area of the triangle XYZ, we use the formula given in the assignment:
   Area(XYZ) = (1 / 2) * ||Vector(XY) x Vector(XZ)||
   With that we get:
   Area(ABC) = 3 * 41^(1/2), Area(PBC) = 41^(1/2), Area(PCA) = (3/2) * 41^(1/2), Area(PAB) = (1/2) * 41^(1/2)
   From that we have:
   alpha1 = 1/3, alpha2 = 1/2, alpha3 = 1/6
   and with the equation at the start we finally have:
   c_p = (1/2  1/2  2/3)

--------------------------------------------------------------------------------------------------------------------

Assignment 5 Notes:
--------------------------------------------------------------------------------------------------------------------
The vs2015 template is used, changed the lib dependencies from 2015 to 2017 for release win32, just like the way
suggested in the assignment 4 grading, release/debug win32 should compile and work fine.

***Assignment 5 Questions***
--------------------------------------------------------------------------------------------------------------------
Look to assignment5questions.pdf for the solutions (its path is the same as this ReadMe.txt)

the cosinus formula in the second question (cos(alpha) = n . l) assumes that the vectors 
n and l have unit length (length of 1).

--------------------------------------------------------------------------------------------------------------------

Assignment 6 Notes:
--------------------------------------------------------------------------------------------------------------------
The vs2015 template is used, release/debug win32 should compile and work fine.
At the Adjust Camera Task, we did not set Eye.z = 0 (instead we did Eye.z = 0.00000001f) because when it gets
assigned to zero, nothing can be displayed because the camera and the scene stays at the exact same point

***Assignment 6 Questions***
--------------------------------------------------------------------------------------------------------------------
Look to assignment6questions.pdf for the solutions (its path is the same as this ReadMe.txt)

--------------------------------------------------------------------------------------------------------------------

Assignment 7 Notes:
--------------------------------------------------------------------------------------------------------------------
The vs2015 template is used, release/debug win32 should compile and work fine.
set vAt.x = 100.0f so that all ground objects can be seen in one screen.
6 Ground Objects (3 Meshes), which is tower, radar, barracks are added.
One of the radars is on top of a barracks. This was done on purpose, I don't know, I just liked the way it looks :)

***Assignment 7 Questions***
--------------------------------------------------------------------------------------------------------------------
Look to assignment7questions.pdf for the solutions (its path is the same as this ReadMe.txt)

--------------------------------------------------------------------------------------------------------------------

Assignment 8 Notes:
--------------------------------------------------------------------------------------------------------------------
The vs2015 template is used, release/debug win32 should compile and work fine.
All enemy objects are facing its velocity direction (for some meshes, it is rather not clear which part is the head,
and which part is the body, so we took a decision and choose the best one in our opinion). Enemy is represented in
the Enemy class.

--------------------------------------------------------------------------------------------------------------------

Assignment 9 Notes:
--------------------------------------------------------------------------------------------------------------------
The vs2015 template is used, release/debug win32 should compile and work fine.
3 sprites are rendered, all with different values, one is plasma other two are gatling, should all be seen at the
camera start point.

***Assignment 9 Questions***
--------------------------------------------------------------------------------------------------------------------
Look to assignment9questions.pdf for the solutions (its path is the same as this ReadMe.txt)

--------------------------------------------------------------------------------------------------------------------

Assignment 10 Notes:
--------------------------------------------------------------------------------------------------------------------
The vs2015 template is used, release/debug win32 should compile and work fine.
TO-DO: Change config parameters of the EnemyType and Gun to make the game feel "right"

Key-Bindings:
'Z': Fire gatling cannon
'X': Fire plasma cannon
'C': Lock/Unlock camera movement

--------------------------------------------------------------------------------------------------------------------

Assignment 11 Notes:
--------------------------------------------------------------------------------------------------------------------
The vs2015 template is used, release/debug win32 should compile and work fine.
Key bindings are the same as the assignment 10.

All assignments done, thank you for controlling our solutions and your efforts!
I wish all of the luck in the exams of people, who see this message.

___________.__             ___________ _______  ________   
\__    ___/|  |__   ____   \_   _____/ \      \ \______ \  
  |    |   |  |  \_/ __ \   |    __)_  /   |   \ |    |  \ 
  |    |   |   Y  \  ___/   |        \/    |    \|    `   \
  |____|   |___|  /\___  > /_______  /\____|__  /_______  /
                \/     \/          \/         \/        \/ 






