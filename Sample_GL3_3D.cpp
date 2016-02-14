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

int jump_flag=0, flag_up=1;
float xDragStart;
int Player_X=0, Player_Y=0;
GLfloat Player_Z=0.0;
int Life_Remaining=4;
float width, height;
int flag[11][11]={{0 }};
int flag_pits[10][10]={{0 }};
int flag_obstacles[10][10]={{0 }};
float zoom=1;
float angle=0;
float deltaAngle = 0;
int isDragging=0; 
float pyramid_Z=5.1;
int flag_pyramid=1;
int pyramid_down=1;


float cdx=0, cdy=0, cdz=0;
float ldx=0, ldy=0, ldz=0;
float udx=0, udy=0, udz=0;
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
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) 
{

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
    for (int i=0; i<numVertices; i++) 
    {
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
float player_rotation = 0;
float rectangle_rot_dir = 1;
float cube_rot_dir = 1;
float player_rot_dir =1;
bool player_rot_status = true;
bool pyramid_rot_status = true;
float pyramid_rot_dir =1;
float cube_rotation=0;
float triangle_rot_dir =1;
int View_cam=0;
int back_cam=0;
int front_cam=0;
float posX, posY, posZ, UpX, UpY, UpZ, PointX, PointY, PointZ;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */

void playerCam()
{
	if(View_cam==1)
	{
		posX= -5.4+Player_X*1.22;
		posY=-5.4+Player_Y*1.22;
		posZ=1.5;
		PointX=-5.4+Player_X*1.22+1.0;
		PointY=-5.4+Player_Y*1.22+1.0;
		PointZ=0.0;
		UpX=0.0;
		UpY=1.0;
		UpZ=0.0;
	}
}

void playerBackView()
{
		if(back_cam==1){
			cout << Player_X  << endl;
			cout << Player_Y << endl;
			cout << Player_Z << endl;
			posX=-5.4+Player_X*1.22+1.0;
			posY= -5.4+Player_Y*1.22+0.5;
			posZ=0.5;
			PointX=-5.4+Player_X*1.22 + 1.0;
		    PointY=-5.4+Player_Y*1.22 + 1.0;
		    PointZ=Player_Z;
		    UpX=0.0;
		    UpY=2.0;
		    UpZ=0.0;
		}
}

void playerFrontView(){
	if(front_cam==1){
			posX=-5.4+Player_X*1.22+6.1;
			posY= -5.4+Player_Y*1.22+2.1;
			posZ=0.7;
			PointX=-5.4+Player_X*1.22;
		    PointY=-5.4+Player_Y*1.22;
		    PointZ=Player_Z;
		    UpX=0.0;
		    UpY=2.0;
		    UpZ=0.0;
	}
}

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     	GLfloat cameraSpeed = 0.5f;
	    if (action == GLFW_RELEASE) 
	    {
        	switch (key) 
        	{ 
            	case GLFW_KEY_RIGHT:
            	if(((Player_X + 1) < 10) && (flag_obstacles[Player_X+1][Player_Y]!=1) && (flag[Player_X+1][Player_Y]!=1)){
            		Player_X +=1;
            		if(flag_pits[Player_X][Player_Y]==1){
            			Player_X=0;
            			Player_Y=0;
            			Life_Remaining--;
            			cout << "Life Remaining:" ;
            			cout << Life_Remaining << endl;
            			if(Life_Remaining==0){
            				cout << "Game Over" << endl;
            				quit(window);
            			}
            		}
            		else if(Player_X==9 && Player_Y==9){
            			cout << "Found U -_-" <<endl;
            		}
            	}
            	if(Player_X==9 && Player_Y==9){
            		cout << "CONGRATULATIONS" << endl;
            		cout << "You Won!" << endl;
            		quit(window);
            	}
            	if(View_cam==1){
            		playerCam();
            	}
            	else if(back_cam==1){
            		playerBackView();
            	}
            	else if(front_cam==1){
            		playerFrontView();
            	}
            	break;
			case GLFW_KEY_LEFT:
            	if((Player_X > 0 )  && (flag_obstacles[Player_X-1][Player_Y]!=1) && (flag[Player_X-1][Player_Y]!=1)){
            		Player_X -= 1;
            		if((flag_pits[Player_X][Player_Y]==1)){
            			Player_X=0;
            			Player_Y=0;
            			Life_Remaining--;
            			cout << "Life Remaining:" ;
            			cout << Life_Remaining << endl;
            			if(Life_Remaining==0){
            				cout << "Game Over" << endl;
            				quit(window);
            			}
            		}
            		else if(Player_X==9 && Player_Y==9){
            			cout << "Found U -_-" <<endl;
            		}
            	}
            	if(Player_X==9 && Player_Y==9){
            		cout << "CONGRATULATIONS" << endl;
            		cout << "You Won!" << endl;
            		quit(window);
            	}
            	if(View_cam==1){
            		playerCam();
            		}
            		else if(back_cam==1){
            		playerBackView();
            	}
            	else if(front_cam==1){
            		playerFrontView();
            	}
            	break;           
            case GLFW_KEY_UP:
            	if(((Player_Y + 1) < 10) && (flag_obstacles[Player_X][Player_Y+1]!=1) && (flag[Player_X][Player_Y+1]!=1)){
            		Player_Y += 1;
            		if((flag_pits[Player_X][Player_Y]==1)){
            			Player_X=0;
            			Player_Y=0;
            			Life_Remaining--;
            			cout << "Life Remaining:" ;
            			cout << Life_Remaining << endl;
            			if(Life_Remaining==0){
            				cout << "Game Over" << endl;
            				quit(window);
            			}
            		}
            		else if(Player_X==9 && Player_Y==9){
            			cout << "Found U -_-" <<endl;
            		}
            	}
            	if(Player_X==9 && Player_Y==9){
            		cout << "CONGRATULATIONS" << endl;
            		cout << "You Won!" << endl;
            		quit(window);
            	}
            	if(View_cam==1){
            		playerCam();
            	}
            	else if(back_cam==1){
            		playerBackView();
            	}
            	else if(front_cam==1){
            		playerFrontView();
            	}
            	break;
            case GLFW_KEY_DOWN:
            	if(((Player_Y ) > 0)  && (flag_obstacles[Player_X][Player_Y-1]!=1) && (flag[Player_X][Player_Y-1]!=1))
            	{
            		Player_Y -= 1;
            		if((flag_pits[Player_X][Player_Y]==1)){
            			Player_X=0;
            			Player_Y=0;
            			Life_Remaining--;
            			cout << "Life Remaining:" ;
            			cout << Life_Remaining << endl;
            			if(Life_Remaining==0){
            				cout << "Game Over" << endl;
            				quit(window);
            			}
            		}
            		else if(Player_X==9 && Player_Y==9){
            			cout << "Found U -_-" <<endl;
            		}
            	}
            	if(Player_X==9 && Player_Y==9){
            		cout << "CONGRATULATIONS" << endl;
            		cout << "You Won!" << endl;
            		quit(window);
            	}
            	 if(View_cam==1){
            	 	playerCam();
            	 }
            	 else if(back_cam==1){
            		playerBackView();
            	}
            	else if(front_cam==1){
            		playerFrontView();
            	}
            	break;
            case GLFW_KEY_SPACE:
            	jump_flag=1;
            	// cout << jump_flag << endl;
            	// cout << flag_up << endl;
            	if(View_cam==1){
            		playerCam();
            	}
            	else if(back_cam==1){
            		playerBackView();
            	}
            	else if(front_cam==1){
            		playerFrontView();
            	}
            	break;
            case GLFW_KEY_T:
            	posX=0.0;
            	posY=0.0;
            	posZ=6.0;
            	PointX=0;
            	PointY=0;
            	PointZ=0;
            	UpX=0;
            	UpY=1.0;
            	UpZ=0;
            	View_cam=0;
            	front_cam=0;
            	back_cam=0;
            	break;
            case GLFW_KEY_N:
            	posX=1.0;
				posY=-1.9;
				posZ=5.0;
				PointX=0.0;
				PointY=0.0;
				PointZ=0.0;
				UpX=0.0;
				UpY=1.0;
				UpZ=0.0;
				View_cam=0;
				front_cam=0;
				back_cam=0;
            	break;
            case GLFW_KEY_P:
            	back_cam=0;
            	front_cam=0;
            	View_cam=1;
            	playerCam();
            	break;
            case GLFW_KEY_B:
            	View_cam=0;
            	front_cam=0;
            	back_cam=1;
            	playerBackView();
            	break;
            case GLFW_KEY_F:
            	View_cam=0;
            	front_cam=1;
            	back_cam=0;
            	playerFrontView();
            	break;
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
		    case 'z':
    		if(zoom < 1.7)
            	zoom+=0.009;
                	    Matrices.projection = glm::ortho(-zoom*8.0f, zoom*8.0f, -zoom*8.0f, zoom*8.0f, -20.0f, 20.0f);
         	   break;
    		case 'x':
    			if(zoom > 0.01)
            	zoom-=0.009;
                Matrices.projection = glm::ortho(-zoom*8.0f, zoom*8.0f, -zoom*8.0f, zoom*8.0f, -20.0f, 20.0f);
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
            if (action == GLFW_PRESS){
                isDragging=1;
                xDragStart=posX;
                //mouseMotion(posX, posY);
            }
            else{
            	angle+=deltaAngle;
            	isDragging=0;
            }
            break;
        default:
            break;
    }
}

void mouseMotion(GLFWwindow* window, double posX, double posY){
	if(isDragging){
		deltaAngle=(posX - xDragStart-1)*0.003;
		ldx = -sin(angle+deltaAngle);
		ldz = cos(angle + deltaAngle);
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
    Matrices.projection = glm::ortho(-zoom*8.0f, zoom*8.0f, -zoom*8.0f, zoom*8.0f, -20.0f, 20.0f);
}

VAO *triangle, *rectangle, *cube[10][10], *player, *obstacles[10][10] , *pyramid;

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

void createPlayer(GLfloat centerX, GLfloat centerY, GLfloat centerZ, GLfloat length, GLfloat width, GLfloat height)
{
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

void createPyramid(float l,float h)
{
  GLfloat pyramid_vertex_buffer_data[]={
    l/2,l/2,0,
    -l/2,l/2,0,
    l/2,-l/2,0,
    
    l/2,-l/2,0,
    -l/2,l/2,0,
    l/2,l/2,0,

    0,0,-h,
    l/2,l/2,0,
    -l/2,l/2,0,
    
    0,0,-h,
    l/2,l/2,0,
    l/2,-l/2,0,

    0,0,-h,
    -l/2,-l/2,0,
    l/2,-l/2,0,

    0,0,-h,
    -l/2,l/2,0,
    -l/2,-l/2,0,    
  };

  GLfloat pyramid_color_buffer_data[]={

  0.0,1.0,0.0,
  0.0,1.0,0.0,
  0.0,1.0,0.0,
  
  0.0,1.0,0.0,
  0.0,1.0,0.0,
  0.0,1.0,0.0,

  1.0,0.0,1.0,
  1.0,0.0,1.0,
  1.0,0.0,1.0,

  0.0,0.3,1.0,
  0.0,0.3,1.0,
  0.0,0.3,1.0,

  0.0,0.0,0.9,
  0.0,0.0,0.9,
  0.0,0.0,0.9,

  1.0,0.7,0.9,
  1.0,0.7,0.9,
  1.0,0.7,0.9,
  };

  pyramid = create3DObject(GL_TRIANGLES, 18, pyramid_vertex_buffer_data, pyramid_color_buffer_data, GL_FILL);
}


// creating the obstacles
void createObstacles(GLfloat centerX, GLfloat centerY, GLfloat centerZ, GLfloat edgeLength, int x, int y){
	GLfloat halfLen = edgeLength*0.6;
	GLfloat halfWid = edgeLength*0.6;
	GLfloat halfHei = edgeLength*1.0;
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

    GLfloat vertex_color[] = {
  //
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

    	0.0,0.0,0.0,
    	0.0,0.0,0.0,
    	0.0,0.0,0.0,

    	0.0,0.0,0.0,
    	0.0,0.0,0.0,
    	0.0,0.0,0.0,

    	0.0,0.0,0.0,
    	0.0,0.0,0.0,
    	0.0,0.0,0.0,

    	0.0,0.0,0.0,
    	0.0,0.0,0.0,
    	0.0,0.0,0.0,

    	0.0,0.0,0.0,
    	0.0,0.0,0.0,
    	0.0,0.0,0.0,

    	0.0,0.0,0.0,
    	0.0,0.0,0.0,
    	0.0,0.0,0.0,

    	0.0,0.0,0.0,
    	0.0,0.0,0.0,
    	0.0,0.0,0.0,

    	0.0,0.0,0.0,
    	0.0,0.0,0.0,
    	0.0,0.0,0.0

    };
    
    obstacles[x][y] = create3DObject(GL_TRIANGLES, 36, vertex, vertex_color, GL_FILL);

}

// creating random cubes to move and all.
void random_cubes(){

	int count_flag=17,i,j;
 	for(i=0;i<10;i++){
 		for(j=0;j<10;j++){
 			flag[i][j]=0;
 		}
 	}


 	while(count_flag>0){
  		int X= rand()%10;
  		int Y= rand()%10;
 		if(flag_obstacles[X][Y]!=1 && flag[X][Y]!=1 && !(X==0 && Y==0) && !(X==9 && Y==9) && (flag_pits[X][Y]!=1) && !(X==Player_X && Y==Player_Y)){
 			flag[X][Y]=1;
 			count_flag--;
 		}
 		else{
 			continue;
 		}
 	}
}

void random_pits()
{
	int count_pits=7,i,j;
	for(i=0;i<10;i++)
	{
		for(j=0;j<10;j++)
		{
			flag_pits[i][j]=0;
		}
	}

	while(count_pits>0){
		int X=rand()%10;
		int Y=rand()%10;
		if(!(X==0 && Y==0) && !(X==9 && Y==9) && (flag_pits[X][Y]!=1) && (flag[X][Y]!=1)){
			flag_pits[X][Y]=1;
			count_pits--;
		}
		else{
			continue;
		}
	}
}


float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
float obstacles_rotation = 0;
float pyramid_rotation =0;



/* Render the scene with openGL */
/* Edit this function according to your assignment */
float y=0;
float grid_flag=1;


void draw ()
{
	int i,j;
  	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram (programID);
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	
	glm::vec3 cameraPos   = glm::vec3(posX + cdx, posY + cdy, posZ + cdz);
	glm::vec3 cameraFront = glm::vec3(PointX+ldx, PointY + ldy, PointZ + ldz);
	glm::vec3 cameraUp    = glm::vec3(UpX + udx, UpY+ udy, UpZ + udz);

	Matrices.view = glm::lookAt(cameraPos, cameraFront, cameraUp); // Fixed camera for 2D (ortho) in XY plane
	
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 VP = Matrices.projection * Matrices.view;
	glm::mat4 MVP;	// MVP = Projection * View * Mode

  	for(i=0;i<10;i++)
  	{
  		for(j=0;j<10;j++)
  		{

			Matrices.model = glm::mat4(1.0f);
		    glm::mat4 translateCube = glm::translate (glm::vec3(0, 0, flag[i][j]*y));        // glTranslatef
			glm::mat4 rotateCube = glm::rotate((float)(cube_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
			Matrices.model *= (translateCube * rotateCube);
			MVP = VP * Matrices.model;
			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	  		if(flag_pits[i][j]!=1)
	  		{
	  				draw3DObject(cube[i][j]);
  			}
  	    }
    }

	if(grid_flag==1)
	{
	  	y+=0.008;
	}
	else if(grid_flag==0)
	{
	  	y-=0.008;
	}

	if(y>=2.0)
	{
		grid_flag=0;
	}
	else if(y<0)
	{
	  	grid_flag=1;
	  	random_cubes();
	  	y=0;
	}

	for(i=0;i<10;i++){
		for(j=0;j<10;j++){
			if(flag_obstacles[i][j]==1){
				Matrices.model = glm::mat4(1.0f);

				glm::mat4 translateObstacles = glm::translate (glm::vec3(0.0,0.0, 0.0));        // glTranslatef
				glm::mat4 rotateObstacles = glm::rotate((float)(obstacles_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= (translateObstacles * rotateObstacles);
				MVP = VP * Matrices.model;
				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
				draw3DObject(obstacles[i][j]);
			}
		}
	}
	

	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translatePlayer = glm::translate (glm::vec3(-5.4+Player_X*1.22, -5.4+Player_Y*1.22, Player_Z));        // glTranslatef
	glm::mat4 rotatePlayer = glm::rotate((float)(player_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translatePlayer * rotatePlayer);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(player);
    

	if(jump_flag==1)
	{
		if(Player_Z <= 4.0 && flag_up==1)
		{
			Player_Z+=0.06;
		}
		else if(Player_Z>4.0)
		{
			flag_up=0;
		}
		if(Player_Z >=0.0 && flag_up==0)
		{
			Player_Z-=0.06;
		}
		else if(Player_Z < 0.0 && flag_up==0){
			jump_flag=0;
			flag_up=1;
		}
	}


Matrices.model = glm::mat4(1.0f);

	glm::mat4 translatePyramid = glm::translate (glm::vec3(5.55, 5.55, pyramid_Z));        // glTranslatef
	glm::mat4 rotatePyramid = glm::rotate((float)(pyramid_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translatePyramid * rotatePyramid);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(pyramid);

	if(flag_pyramid==1){
		if(pyramid_down==1 && pyramid_Z>=2.5){
			pyramid_Z-=0.04;
		}
		else if(pyramid_Z<2.5){
			pyramid_down=0;
		}
		if(pyramid_down==0 && pyramid_Z<=5.3){
			pyramid_Z+=0.04;
		}
		else if(pyramid_Z>5.3){
			pyramid_down=1;
		}
	}





	float increments = 1;
	player_rotation = player_rotation + increments*player_rot_dir*player_rot_status;
	//pyramid_rotation=pyramid_rotation+3.0*pyramid_rot_dir*pyramid_rot_status;



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
    glfwSetCursorPosCallback(window, mouseMotion);

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */


void initGL (GLFWwindow* window, int width, int height)
{
    GLfloat x=-5.4f, y=-5.4f, z=0.0f;
	int i,j;
	random_pits();
  	
  	for(i=0;i<10;i++)
	{
		for(j=0;j<10;j++)
		{
			createCube(x+1.22*i, y+j*1.22, z, 1.2f, i, j);
		}
	}
	
	int count_obstacles=7;
	while(count_obstacles>0)
	{
		int X=rand()%10;
		int Y=rand()%10;
		if(flag_obstacles[X][Y]!=1 && !(X==0 && Y==0) && !(X==9 && Y==9))
		{
			createObstacles(-5.4+1.22*X, -5.4+1.22*Y, 3.0, 1.0, X, Y);
			flag_obstacles[X][Y]=1;
			count_obstacles--;
		}
		else
		{
			continue;
		}
	}
	createPlayer(0, 0, 4.62, 0.8, 0.8, 1.0);
	random_cubes();
	createPyramid(1.0, 0.8);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.0f, 0.5f, 1.0f, 0.0f); // R, G, B, A
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
	width = 1920;
	height = 1920;
	GLFWwindow* window = initGLFW(width, height);
	initGL (window, width, height);
	posX=1.0;
	posY=-1.9;
	posZ=5.0;
	PointX=0.0;
	PointY=0.0;
	PointZ=0.0;
	UpX=0.0;
	UpY=1.0;
	UpZ=0.0;

    double last_update_time = glfwGetTime(), current_time;
	
	while (!glfwWindowShouldClose(window)) 
    {	
    	draw();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
