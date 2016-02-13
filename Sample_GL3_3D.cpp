#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdlib>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
int Player_X=0, Player_Y=0;
GLfloat Player_Z=0.0;

int flag[11][11]={{0 }};
struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;


/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/
float cube_rotation =0;
float player_rotation = 0;
float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
float cube_rot_dir = 1;
float player_rot_dir =1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
bool cube_rot_status = true;
bool player_rot_status = true;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.
            	 GLfloat cameraSpeed = 0.5f;
	    if (action == GLFW_RELEASE) {
        switch (key) { 
            case GLFW_KEY_RIGHT:
            	if((Player_X + 1) < 10){
            		Player_X +=1;
            		if(flag[Player_X][Player_Y]==1){
            			Player_X=0;
            			Player_Y=0;
            		}
            		else if(Player_X==9 && Player_Y==9){
            			cout << "Found U -_-" <<endl;
            		}
            	}
            	break;
			case GLFW_KEY_LEFT:
            	if(Player_X > 0 ){
            		Player_X -= 1;
            		if(flag[Player_X][Player_Y]==1){
            		Player_X=0;
            			Player_Y=0;
            		}
            		else if(Player_X==9 && Player_Y==9){
            			cout << "Found U -_-" <<endl;
            		}
            		}
            	break;           
            case GLFW_KEY_UP:
            	if((Player_Y + 1) < 10){
            		Player_Y += 1;
            		if(flag[Player_X][Player_Y]==1){
            			Player_X=0;
            			Player_Y=0;
            		}
            		else if(Player_X==9 && Player_Y==9){
            			cout << "Found U -_-" <<endl;
            		}
            	}
            	break;
            case GLFW_KEY_DOWN:
            	if((Player_Y ) > 0){
            		Player_Y -= 1;
            		if(flag[Player_X][Player_Y]==1){
            			Player_X=0;
            			Player_Y=0;
            		}
            		else if(Player_X==9 && Player_Y==9){
            			cout << "Found U -_-" <<endl;
            		}
            	}
            	break;
    		case GLFW_KEY_W:
        		cameraPos += cameraSpeed * cameraFront;
    		case GLFW_KEY_S:
        		cameraPos -= cameraSpeed * cameraFront;
    		case GLFW_KEY_A:
        		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    		case GLFW_KEY_D:
        		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                triangle_rot_dir *= -1;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-8.0f, 8.0f, -8.0f, 8.0f, -20.0f, 20.0f);
}

VAO *triangle, *rectangle, *cube[10][10], *player;

// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

//GLfloat cube[4][4]; 



void createPlayer(GLfloat centerX, GLfloat centerY, GLfloat centerZ, GLfloat length, GLfloat width, GLfloat height){
	GLfloat halfLen, halfWid, halfHei;
	halfLen = 0.5*length;
	halfWid = 0.5*width;
	halfHei = height;

	GLfloat vertex_value[] = {
		//front face
		centerX - halfLen, centerY + halfWid, centerZ + halfHei,
		centerX + halfLen, centerY + halfWid, centerZ + halfHei,
		centerX + halfLen, centerY - halfWid, centerZ + halfHei,

		centerX + halfLen, centerY - halfWid, centerZ + halfHei,
		centerX - halfLen, centerY - halfWid, centerZ + halfHei,
		centerX - halfLen, centerY + halfWid, centerZ + halfHei,
	
	//back face
		centerX - halfLen, centerY + halfWid, centerZ - halfHei,
		centerX + halfLen, centerY + halfWid, centerZ - halfHei,
		centerX + halfLen, centerY - halfWid, centerZ - halfHei,

		centerX + halfLen, centerY - halfWid, centerZ - halfHei,
		centerX - halfLen, centerY - halfWid, centerZ - halfHei,
		centerX - halfLen, centerY + halfWid, centerZ - halfHei,

	//top face
		centerX - halfLen, centerY + halfWid, centerZ + halfHei,
		centerX - halfLen, centerY + halfWid, centerZ - halfHei,
		centerX + halfLen, centerY + halfWid, centerZ - halfHei,

		centerX + halfLen, centerY + halfWid, centerZ - halfHei,
		centerX + halfLen, centerY + halfWid, centerZ + halfHei,
		centerX - halfLen, centerY + halfWid, centerZ + halfHei,

	//bottom face
		centerX - halfLen, centerY - halfWid, centerZ - halfHei,
		centerX + halfLen, centerY - halfWid, centerZ - halfHei,
		centerX + halfLen, centerY - halfWid, centerZ + halfHei,

		centerX + halfLen, centerY - halfWid, centerZ + halfHei,
		centerX - halfLen, centerY - halfWid, centerZ + halfHei,
		centerX - halfLen, centerY - halfWid, centerZ - halfHei,

	//right face
		centerX + halfLen, centerY + halfWid, centerZ + halfHei,
		centerX + halfLen, centerY + halfWid, centerZ - halfHei,
		centerX + halfLen, centerY - halfWid, centerZ - halfHei,
		
		centerX + halfLen, centerY - halfWid, centerZ - halfHei,
		centerX + halfLen, centerY - halfWid, centerZ + halfHei,
		centerX + halfLen, centerY + halfWid, centerZ + halfHei,

	//left face
		centerX - halfLen, centerY + halfWid, centerZ + halfHei,
		centerX - halfLen, centerY + halfWid, centerZ - halfHei,
		centerX - halfLen, centerY - halfWid, centerZ - halfHei,

		centerX - halfLen, centerY - halfWid, centerZ - halfHei,
		centerX - halfLen, centerY - halfWid, centerZ + halfHei,
		centerX - halfLen, centerY + halfWid, centerZ + halfHei,

	};

	GLfloat vertex_color[] = {
		
		1.0,0.0,0.0,
		1.0,0.0,0.0,
		1.0,0.0,0.0,

		1.0,0.0,0.0,
		1.0,0.0,0.0,
		1.0,0.0,0.0,

		0.0,1.0,0.0,
		0.0,1.0,0.0,
		0.0,1.0,0.0,

		0.0,1.0,0.0,
		0.0,1.0,0.0,
		0.0,1.0,0.0,

		1.0,1.0,1.0,
		1.0,1.0,1.0,
		1.0,1.0,1.0,

		1.0,1.0,1.0,
		1.0,1.0,1.0,
		1.0,1.0,1.0,

		0.0,0.0,1.0,
		0.0,0.0,1.0,
		0.0,0.0,1.0,

		0.0,0.0,1.0,
		0.0,0.0,1.0,
		0.0,0.0,1.0,

		0.0,1.0,1.0,
		0.0,1.0,1.0,
		0.0,1.0,1.0,

		0.0,1.0,1.0,
		0.0,1.0,1.0,
		0.0,1.0,1.0,

		1.0,1.0,0.0,
		1.0,1.0,0.0,
		1.0,1.0,0.0,

		1.0,1.0,0.0,
		1.0,1.0,0.0,
		1.0,1.0,0.0,
	};

	player = create3DObject(GL_TRIANGLES, 36, vertex_value, vertex_color, GL_FILL);
}

void createCube(GLfloat centerX, GLfloat centerY, GLfloat centerZ, GLfloat edgeLength, int x, int y){
	GLfloat halfLen = edgeLength*0.5;
	GLfloat halfWid = edgeLength*0.5;
	GLfloat halfHei = edgeLength*2;
	GLfloat vertex[] = {
		//front face
		centerX - halfLen, centerY + halfWid, centerZ + halfHei,
		centerX + halfLen, centerY + halfWid, centerZ + halfHei,
		centerX + halfLen, centerY - halfWid, centerZ + halfHei,

		centerX + halfLen, centerY - halfWid, centerZ + halfHei,
		centerX - halfLen, centerY - halfWid, centerZ + halfHei,
		centerX - halfLen, centerY + halfWid, centerZ + halfHei,
	
	//back face
		centerX - halfLen, centerY + halfWid, centerZ - halfHei,
		centerX + halfLen, centerY + halfWid, centerZ - halfHei,
		centerX + halfLen, centerY - halfWid, centerZ - halfHei,

		centerX + halfLen, centerY - halfWid, centerZ - halfHei,
		centerX - halfLen, centerY - halfWid, centerZ - halfHei,
		centerX - halfLen, centerY + halfWid, centerZ - halfHei,

	//top face
		centerX - halfLen, centerY + halfWid, centerZ + halfHei,
		centerX - halfLen, centerY + halfWid, centerZ - halfHei,
		centerX + halfLen, centerY + halfWid, centerZ - halfHei,

		centerX + halfLen, centerY + halfWid, centerZ - halfHei,
		centerX + halfLen, centerY + halfWid, centerZ + halfHei,
		centerX - halfLen, centerY + halfWid, centerZ + halfHei,

	//bottom face
		centerX - halfLen, centerY - halfWid, centerZ - halfHei,
		centerX + halfLen, centerY - halfWid, centerZ - halfHei,
		centerX + halfLen, centerY - halfWid, centerZ + halfHei,

		centerX + halfLen, centerY - halfWid, centerZ + halfHei,
		centerX - halfLen, centerY - halfWid, centerZ + halfHei,
		centerX - halfLen, centerY - halfWid, centerZ - halfHei,

	//right face
		centerX + halfLen, centerY + halfWid, centerZ + halfHei,
		centerX + halfLen, centerY + halfWid, centerZ - halfHei,
		centerX + halfLen, centerY - halfWid, centerZ - halfHei,
		
		centerX + halfLen, centerY - halfWid, centerZ - halfHei,
		centerX + halfLen, centerY - halfWid, centerZ + halfHei,
		centerX + halfLen, centerY + halfWid, centerZ + halfHei,

	//left face
		centerX - halfLen, centerY + halfWid, centerZ + halfHei,
		centerX - halfLen, centerY + halfWid, centerZ - halfHei,
		centerX - halfLen, centerY - halfWid, centerZ - halfHei,

		centerX - halfLen, centerY - halfWid, centerZ - halfHei,
		centerX - halfLen, centerY - halfWid, centerZ + halfHei,
		centerX - halfLen, centerY + halfWid, centerZ + halfHei,

	};
/*
	GLfloat vertex_color[] = {
	//
		0, 0, 0,
		0, 0, 1,   
		0, 1, 1,    
		
		0, 0, 0,   
		0, 0, 1,  
		0, 1, 1,   
        
        1, 0, 0,   
        1, 0, 1,   
        1, 1, 1,     
        
    //
        1, 0, 0,   
        1, 0, 1,   
        1, 1, 1,   
        
        0, 0, 0,   
		0, 0, 1,  
		0, 1, 1,     
        
        0, 0, 0,
		0, 0, 1,   
		0, 1, 1,    
        
     //
        0, 1, 0,   
        0, 1, 1,   
        1, 1, 1,     
        
        0, 1, 0,   
        0, 1, 1,   
        1, 1, 1,   
        
        0, 0, 0,   
        0, 1, 0,   
        1, 1, 0,     
     //
        0, 0, 0,   
        0, 1, 0,   
        1, 1, 0,   
        
        0, 1, 0,   
        0, 1, 1,   
        1, 1, 1,     
        
        0, 1, 0,   
        0, 1, 1,   
        1, 1, 1,
    };
*/
    GLfloat vertex_color[] = {
  //
    	0.0,0.0,0.0,
    	0.0,0.0,1.0,
    	0.0,1.0,1.0,

    	0.0,1.0,1.0,
    	0.0,0.0,1.0,
    	0.0,0.0,0.0,
//
    	1.0,0.0,0.0,
    	1.0,0.0,1.0,
    	1.0,1.0,1.0,

    	1.0,1.0,1.0,
    	1.0,0.0,1.0,
    	1.0,0.0,0.0,
//
    	0.0,1.0,0.0,
    	0.0,1.0,1.0,
    	1.0,1.0,1.0,

    	1.0,1.0,1.0,
    	0.0,1.0,1.0,
    	0.0,1.0,1.0,
//
    	0.0,1.0,0.0,
    	0.0,1.0,1.0,
    	1.0,1.0,1.0,

    	1.0,1.0,1.0,
    	0.0,1.0,1.0,
    	0.0,1.0,1.0,
//
    	0.0,0.0,0.0,
    	0.0,1.0,0.0,
    	0.0,1.0,1.0,

    	0.0,1.0,1.0,
    	0.0,1.0,0.0,
    	0.0,0.0,0.0,
//
    	0.0,0.0,0.0,
    	0.0,0.0,1.0,
    	0.0,1.0,1.0,

    	0.0,1.0,1.0,
    	0.0,0.0,1.0,
    	0.0,0.0,0.0,
    };
    cube[x][y] = create3DObject(GL_TRIANGLES, 36, vertex, vertex_color, GL_FILL);
}


// Creates the rectangle object used in this sample code
void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_LINE);
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;


/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
	int i,j;
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  // glm::vec3 target (0, 0, 0);
  // // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  // glm::vec3 up (0, 1, 0);


  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
	 glm::vec3 cameraPos   = glm::vec3(1,-1.9,5.0);
	glm::vec3 cameraFront = glm::vec3(0,0,0);
	glm::vec3 cameraUp    = glm::vec3(0,1.2,0);


 //  Matrices.view = glm::lookAt(glm::vec3(1,-1.9,5.0), glm::vec3(0,0,0), glm::vec3(0,1.2,0)); // Fixed camera for 2D (ortho) in XY plane
	Matrices.view = glm::lookAt(cameraPos, cameraFront, cameraUp); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  /*
  Matrices.model = glm::mat4(1.0f);


  glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(triangle);

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle);
*/
//for cube 
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateCube = glm::translate (glm::vec3(0, 0, 0));        // glTranslatef
  glm::mat4 rotateCube = glm::rotate((float)(cube_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateCube * rotateCube);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  


  for(i=0;i<10;i++){
  	for(j=0;j<10;j++){
  //		if(!((i==2 && j==8) || (i==6 && j==2) || (i==9 && j==1) || (i==5 && j==3) || (i==3 && j==4) || (i==5 && j==7) || (i==0 && j==3) || (i==3 && j==1) || (i==6 && j==9) || (i==1 && j== 6)|| (i==8 && j==7) || (i==6 && j==5) || (i==7 && j==3))){
	  		if(flag[i][j]!=1){
	  			draw3DObject(cube[i][j]);
	  		}
	  
  		}
  }

  // int count = rand()%3 +2;
  // for(i=0;i<count;i++){
  // 	int x = rand()%10;
  // 	int y = rand()%10;
  // 	flag[x][y]=1;

  // }
  
  if(flag[Player_X][Player_Y]==1){
	  	while(Player_Z>-2.0){
		  	Matrices.model = glm::mat4(1.0f);

		  glm::mat4 translatePlayer = glm::translate (glm::vec3(-5.4+Player_X*1.22, -5.4+Player_Y*1.22, Player_Z));        // glTranslatef
		  glm::mat4 rotatePlayer = glm::rotate((float)(player_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		  Matrices.model *= (translatePlayer * rotatePlayer);
		  MVP = VP * Matrices.model;
		  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(player);
		  // Increment angles
		  float increments = 1;
		 	player_rotation = player_rotation + increments*player_rot_dir*player_rot_status;
			Player_Z-=0.3;
			}
		}
	else{
		Matrices.model = glm::mat4(1.0f);

		  glm::mat4 translatePlayer = glm::translate (glm::vec3(-5.4+Player_X*1.22, -5.4+Player_Y*1.22, 0.0));        // glTranslatef
		  glm::mat4 rotatePlayer = glm::rotate((float)(player_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
		  Matrices.model *= (translatePlayer * rotatePlayer);
		  MVP = VP * Matrices.model;
		  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(player);
		  // Increment angles
		  float increments = 1;
		 	player_rotation = player_rotation + increments*player_rot_dir*player_rot_status;

	  // draw3DObject draws the VAO given to it using current MVP matrix
	}


  //camera_rotation_angle++; // Simulating camera rotation
  //triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  //rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
  
//cube rotation.
  //cube_rotation = cube_rotation + increments*cube_rot_dir*cube_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */





GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Find my Way Out", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 0.2 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();
	GLfloat x=-5.4f, y=-5.4f, z=0.0f;
	// Player_X=x;
	// Player_Y=y;
	int i,j;
	int count_flag=17;
 	while(count_flag>0){
  		int X= rand()%10;
  		int Y= rand()%10;
 		if(flag[X][Y]!=1 && (X!=0 && Y!=0) && (X!=9 && Y!=9)){
 			flag[X][Y]=1;
 			count_flag--;
 		}
 		else{
 			continue;
 		}
 	}
  // draw3DObject draws the VAO given to it using current MVP matrix
 	
	for(i=0;i<10;i++){
		for(j=0;j<10;j++){
			createCube(x+1.22*i, y+j*1.22, z, 1.2f, i, j);
		}
	}

	createPlayer(0, 0, 4.62, 0.8, 0.8, 2.0);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.0f, 0.0f, 0.0f, 0.0f); // R, G, B, A
	glClearDepth (5.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1920;
	int height = 1920;

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
