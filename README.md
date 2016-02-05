## Find your path ##

### DEPENDENCIES ###
----------------------------------------------------------------
Linux/Windows/ Mac OSX - Dependencies: (Recommended)
 GLFW
 GLAD
 GLM

Linux - Dependencies: (alternative)
 FreeGLUT
 GLEW
 GLM

----------------------------------------------------------------
### INSTALLATION ###
----------------------------------------------------------------
#### GLFW: ####
 - Install CMake  
 - Obtain & Extract the GLFW source code from  
   https://github.com/glfw/glfw/archive/master.zip  
 - Compile with below commands  
   $ cd glfw-master   
   $ mkdir build  
   $ cd build  
   $ cmake -DBUILD_SHARED_LIBS=ON ..  
   $ make && sudo make install  

#### GLAD: ####
 - Go to http://glad.dav1d.de  
 - Language: C/C++  
   Specification: OpenGL  
   gl: Version 4.5  
   gles1: Version 1.0  
   gles2: Version 3.2  
   Profile: Core  
   Select 'Add All' under extensions and click Generate.  
 - Download the zip file generated.  
 - Copy contents of include/ folder in the downloaded directory  
   to /usr/local/include/  
 - src/glad.c should be always compiled along with your OpenGL   
   code  

#### GLM: ####
 - Download the zip file from   
   ```https://github.com/g-truc/glm/releases/tag/0.9.7.2```.  
 - Unzip it and copy the folder glm/glm/ to /usr/local/include
 
  Ubuntu users can also install these libraries using apt-get.

-----------------------------------------------------------------

## Execution ##

#### To execute the code run the makefile ####  
 $> make  
 $> ./sample3D  

```If the makefile gives errors or doesn't executes the code, execute using the following command.```  
 $> g++ -o sample2D Sample_GL3_3D.cpp glad.c -lglfw3 -lGL -lm -lXrandr -lXi -lX11 -lXxf86vm -lpthread -lGLU -ldl  
 $> ./sample2D  

-----------------------------------------------------------------

## Controls ##

CONTROLS
1.Press UP and DOWN arrow keys to move the player up and down the grid block.  
2.Press RIGHT and LEFT arrow keys to move the player right and left in the grid block.  

-----------------------------------------------------------------

## Game ## 

The game objective is to make the player reach from one end to the other end of the game block avoiding the obstacles in between.  

-----------------------------------------------------------------