/*
	COMP 371 - ASSIGNMENT 2
	TRI-LUONG STEVEN DIEN
	27415281
*/

// GLEW
#include "glew.h"

// GLFW
#include "glfw3.h"

// GLM Mathematics
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "gtc/constants.hpp"

// Various headers
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cctype>
#include <cmath>

using namespace std;

#define M_PI        3.14159265358979323846264338327950288   /* pi */
#define DEG_TO_RAD	M_PI/180.0f

// Function prototypes
void window_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void moveCamera();
bool initialize();
bool cleanUp();
GLuint loadShaders(std::string vertex_shader_path, std::string fragment_shader_path);
void draw(GLuint vao, GLuint vbo, vector<GLfloat> vector);
GLfloat computePoints(GLfloat u, GLfloat p1, GLfloat p2, GLfloat t1, GLfloat t2);
void subdivide(GLfloat u0, GLfloat u1, GLfloat maxLineLength, int index, GLfloat tangentMagnitude_1, GLfloat tangentMagnitude_2);

// GLFWwindow pointer
GLFWwindow* window = 0x00;

// Window dimensions
GLfloat width = 800.0, height = 800.0;

// Program ID
GLuint shaderProgram = 0;

// View, Model and Projection matrices ID
GLuint view_matrix_id = 0;
GLuint model_matrix_id = 0;
GLuint proj_matrix_id = 0;

// Transformations matrices
glm::mat4 proj_matrix;
glm::mat4 view_matrix;
glm::mat4 model_matrix;

// Vertex Buffer Object, Vertex Array Object and Element Buffer Object
GLuint VBO[4], VAO[4];

// Model
int renderingMode = 0; // 0 = Points | 1 = Line Strip

// Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
bool keys[1024];
bool buttons[1024];
bool cameraMoveAllowed = false;

// Mouse positions
GLfloat xPos;
GLfloat yPos;

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

// Vectors for the control points, tangent points, spline points and triangles points
vector<GLfloat> controlPoints;
vector<GLfloat> tangentPoints;
vector<GLfloat> splinePoints;
vector<GLfloat> trianglePoints;

// User input
bool correctInput = false;
int numControlPoints;
int numTangentPoints;
int counter1 = 0;
int counter2 = 0;
bool doneWithCP = false;
bool drawSpline = false;
bool readyToDrawSpline = false;

// Triangle
GLfloat triangle[] = {
	-0.05f, -0.05f, 0.0f, // Left  
	0.05f, -0.05f, 0.0f, // Right 
	0.0f,	0.010f, 0.0f  // Top   
};
double time = 0.0;
double animationSpeed = 10.0;
int indexTriangle = 0;
GLfloat angle;

// Variable to be used to increase the size of the points
GLfloat point_size = 2.5f;

// Is called whenever the window got resized via GLFW
void window_size_callback(GLFWwindow* window, int newWidth, int newHeight)
{
	// Keep the same aspect ratio when the user tries to resize the window
	if (newHeight >= newWidth)
	{
		height = newHeight;
		width = newHeight;
	}
	else
	{
		height = newWidth;
		width = newWidth;
	}

	// Force the window to always resize with the smallest value between width and height for the dimension
	glViewport(0, 0, width, height);
	glfwSetWindowSize(window, width, height);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
		renderingMode = 0; // The spline will be drawn with points
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
		renderingMode = 1; // The spline will be drawn with line strips
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS && drawSpline == false && readyToDrawSpline)
	{
		for (int i = 0, j = 0, k = 0; k < controlPoints.size() / 3 - 1; i += 6, j += 3, k++)
		{
			// Calculate the magnitude for the first tangent vector
			GLfloat xSquared_1 = (tangentPoints[i + 3] - controlPoints[j]) * (tangentPoints[i + 3] - controlPoints[j]);
			GLfloat ySquared_1 = (tangentPoints[i + 4] - controlPoints[j + 1]) * (tangentPoints[i + 4] - controlPoints[j + 1]);
			GLfloat tangentMagnitude_1 = sqrt(xSquared_1 + ySquared_1);

			// Calculate the magnitude for the second tangent vector
			GLfloat xSquared_2 = (tangentPoints[i + 9] - controlPoints[j + 3]) * (tangentPoints[i + 9] - controlPoints[j + 3]);
			GLfloat ySquared_2 = (tangentPoints[i + 10] - controlPoints[j + 4]) * (tangentPoints[i + 10] - controlPoints[j + 4]);
			GLfloat tangentMagnitude_2 = sqrt(xSquared_2 + ySquared_2);

			// Use the recursive subdivide method to get the spline points for different "u"
			subdivide(0.0f, 1.0f, 0.025f, j, tangentMagnitude_1, tangentMagnitude_2);
		}

		// Allow the user to press the "Enter" key in order to draw the spline with the triangle
		drawSpline = true;
	}

	// Reset the application, put everything to their initial value
	if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
	{
		// Reset to their default value
		cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
		cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		numControlPoints = 0;
		numTangentPoints = 0;
		angle = 0.0;
		counter1 = 0;
		counter2 = 0;
		time = 0.0;
		drawSpline = false;
		doneWithCP = false;
		cameraMoveAllowed = false;
		correctInput = false;
		readyToDrawSpline = false;

		// Clear all the vectors
		controlPoints.clear();
		tangentPoints.clear();
		splinePoints.clear();

		// Call the cleanUp function and reInitialize the application
		cleanUp();
		initialize();
	}

	if (key >= 0 && key < 1024)
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;

}

// Is called whenever a mouse button is pressed/released via GLFW
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button >= 0 && button < 1024)
	{
		if (action == GLFW_PRESS)
			buttons[button] = true;
		else if (action == GLFW_RELEASE)
		{
			buttons[button] = false;

			float xNormalized, yNormalized;

			// Normalize the x and y screen coordinates
			xNormalized = xPos / height * 2 - 1;
			yNormalized = -1 * (yPos / height * 2 - 1);

			// Push the normalize x y z into the control points vector
			if (doneWithCP == false && counter1 < numControlPoints)
			{
				controlPoints.push_back(xNormalized);
				controlPoints.push_back(yNormalized);
				controlPoints.push_back(0.0f);
				counter1++;

				if (counter1 == numControlPoints)
				{
					doneWithCP = true;
					counter1 = 0;
				}
			}
			else
				// Push the normalized x y z into tangent point vector (including their respective control points)
				if (counter1 < numTangentPoints)
				{
					tangentPoints.push_back(controlPoints[counter2]);
					tangentPoints.push_back(controlPoints[counter2 + 1]);
					tangentPoints.push_back(controlPoints[counter2 + 2]);
					tangentPoints.push_back(xNormalized);
					tangentPoints.push_back(yNormalized);
					tangentPoints.push_back(0.0f);

					counter1++;
					counter2 += 3;

					if (counter1 == numTangentPoints)
					{
						cameraMoveAllowed = true;
						readyToDrawSpline = true;
					}

				}
		}
	}
}

// Is called whenever the mouse is moved via GLFW
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	// Get the position of the cursor when the user clicked (mouse_button_callback)
	xPos = xpos;
	yPos = ypos;
}

// Rotate the model around
void moveCamera()
{
	GLfloat cameraSpeed = 1.5f * deltaTime;

	if (keys[GLFW_KEY_UP]) // Move the camera up
	{
		cameraPos += glm::vec3(0.0f, cameraSpeed, 0.0f);
		view_matrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	}
	if (keys[GLFW_KEY_DOWN]) // Move the camera down
	{
		cameraPos -= glm::vec3(0.0f, cameraSpeed, 0.0f);
		view_matrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	}
	if (keys[GLFW_KEY_LEFT]) // Move the camera left
	{
		cameraPos -= glm::vec3(cameraSpeed, 0.0f, 0.0f);
		view_matrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	}
	if (keys[GLFW_KEY_RIGHT]) // Move the camera right
	{
		cameraPos += glm::vec3(cameraSpeed, 0.0f, 0.0f);
		view_matrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	}
}

// Initialize the application
bool initialize() {
	// Initialize GL context and O/S window using the GLFW helper library
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

	// Create a window of size 800x800 and with title "Lecture 2: First Triangle"
	window = glfwCreateWindow(width, height, "COMP371: Assignment 2", NULL, NULL);
	if (!window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);

	// At initialization, set the mouse cursor at the center of the window
	glfwSetCursorPos(window, width / 2, height / 2);

	// At initialization, set the position of the window
	glfwSetWindowPos(window, 3 * width / 4, height / 5);

	// Set the required callback functions
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	// Initialize GLEW extension handler
	glewExperimental = GL_TRUE;	// Needed to get the latest version of OpenGL
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, width, height);

	// Get the current OpenGL version
	const GLubyte* renderer = glGetString(GL_RENDERER); // Get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // Version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	// Enable the depth test i.e. draw a pixel if it's closer to the viewer
	glEnable(GL_DEPTH_TEST); // Enable depth-testing
	glDepthFunc(GL_LESS);	// The type of testing i.e. a smaller value as "closer"

	// Load the shaders
	shaderProgram = loadShaders("COMP371_HW2.vs", "COMP371_HW2.fs");

	// Ask the user to enter the number of control points, error message if the user enter less than 2 control points
	while (!correctInput)
	{
		cout << "\nPlease enter the number of control points to draw: ";
		cin >> numControlPoints;
		numTangentPoints = numControlPoints;

		if (numControlPoints < 2)
			cout << "\nSorry, you need to have at least 2 control points" << endl;
		else
			correctInput = true;
	}

	cout << "\nClick anywhere on the next window to place the " << numControlPoints << " control points\nthen the " << numTangentPoints << " tangent points." << endl;
	cout << "\nPress the key \"Enter\" to draw the Hermite spline.\n" << endl;

	for (int i = 0; i < 9; i++)
		trianglePoints.push_back(triangle[i]);

	// Set up vertex data (and buffer(s)) and attribute pointers
	glGenVertexArrays(4, VAO);
	glGenBuffers(4, VBO);

	return true;
}

// Clean the vertex array and buffers then shutdown the window
bool cleanUp() {
	glDisableVertexAttribArray(0);
	//Properly de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(4, VAO);
	glDeleteBuffers(4, VBO);

	// Close GL context and any other GLFW resources
	glfwTerminate();

	return true;
}

// Load the shaders (Compile and Link)
GLuint loadShaders(std::string vertex_shader_path, std::string fragment_shader_path) {
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_shader_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_shader_path.c_str());
		getchar();
		exit(-1);
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_shader_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("\nCompiling shader : %s\n", vertex_shader_path.c_str());
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_shader_path.c_str());
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);

	glBindAttribLocation(ProgramID, 0, "in_Position");

	//appearing in the vertex shader.
	glBindAttribLocation(ProgramID, 1, "in_Color");

	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	//The three variables below hold the id of each of the variables in the shader
	//If you read the vertex shader file you'll see that the same variable names are used.
	view_matrix_id = glGetUniformLocation(ProgramID, "view_matrix");
	model_matrix_id = glGetUniformLocation(ProgramID, "model_matrix");
	proj_matrix_id = glGetUniformLocation(ProgramID, "proj_matrix");

	return ProgramID;
}

// Draw the control points, tangent vectors, spline points or the triangle
void draw(GLuint vao, GLuint vbo, vector<GLfloat> vector)
{
	//Pass the values of the three matrices to the shaders
	glUniformMatrix4fv(proj_matrix_id, 1, GL_FALSE, glm::value_ptr(proj_matrix));
	glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, glm::value_ptr(view_matrix));
	glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, glm::value_ptr(model_matrix));

	// Bind Vertex Array Object
	glBindVertexArray(vao);

	if (vector.size() > 0)
	{
		// Copy our vertices array in a buffer for OpenGL to use
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vector.size() * sizeof(GLfloat), &vector[0], GL_STATIC_DRAW);
		// Then set our vertex attributes pointers.
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	}

	if (vector == controlPoints) // If the given vector is the control points vector, draw them with points
	{
		GLint vertexColorLocation = glGetUniformLocation(shaderProgram, "color");
		glUniform3f(vertexColorLocation, 1.0f, 0.0f, 0.0f);
		glDrawArrays(GL_POINTS, 0, vector.size() / 3);
	}


	if (vector == tangentPoints) // If the given vector is the tangent points vector, draw them with lines
	{
		GLint vertexColorLocation = glGetUniformLocation(shaderProgram, "color");
		glUniform3f(vertexColorLocation, 0.0f, 1.0f, 0.0f);
		glDrawArrays(GL_LINES, 0, vector.size() / 3);
	}

	if (vector == splinePoints)  // If the given vector is the tangent points vector, draw them initialy with points but if the user press "L", draw then with line strips
	{
		GLint vertexColorLocation = glGetUniformLocation(shaderProgram, "color");
		glUniform3f(vertexColorLocation, 1.0f, 0.0f, 0.0f);

		switch (renderingMode)
		{
		case 0:
			glDrawArrays(GL_POINTS, 0, vector.size() / 3);
			break;
		case 1:
			glDrawArrays(GL_LINE_STRIP, 0, vector.size() / 3);
			break;
		}
	}

	if (vector == trianglePoints) // If the given vector is the triangle points vector, translate and rotate the triangle accordingly to the spline path
	{
		GLint vertexColorLocation = glGetUniformLocation(shaderProgram, "color");
		glUniform3f(vertexColorLocation, 0.0f, 0.0f, 1.0f);

		glm::mat4 triangle;

		// Get the angle between two spline points
		if (indexTriangle + 3 != splinePoints.size())
			angle = glm::atan((splinePoints[indexTriangle + 4] - splinePoints[indexTriangle + 1]) / (splinePoints[indexTriangle + 3] - splinePoints[indexTriangle]));

		// Translate the triangle to a spline point
		triangle = glm::translate(triangle, glm::vec3(splinePoints[indexTriangle], splinePoints[indexTriangle + 1], splinePoints[indexTriangle + 2]));

		// Rotate the triangle accordingly
		if (indexTriangle + 3 != splinePoints.size() && splinePoints[indexTriangle + 3] > splinePoints[indexTriangle])
			triangle = glm::rotate(triangle, angle + 80, glm::vec3(0.0f, 0.0f, 1.0f));
		else
			triangle = glm::rotate(triangle, angle - 80, glm::vec3(0.0f, 0.0f, 1.0f));

		glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, glm::value_ptr(triangle));
		glDrawArrays(GL_TRIANGLES, 0, vector.size() / 3);

		time += glfwGetTime() / glfwGetTime();

		// Move to the next pair of spline points
		if (time >= animationSpeed / numControlPoints) // Slow down the drawing of the triangle so it doesn't move too fast
		{
			if (indexTriangle + 3 == splinePoints.size())
				indexTriangle = 0;
			else
				indexTriangle += 3;

			time = 0.0; // Reset the time counter
		}
	}

	// Unbind Vertex Array Object
	glBindVertexArray(0);
}

// Compute the spline point
GLfloat computePoints(GLfloat u, GLfloat p1, GLfloat p2, GLfloat t1, GLfloat t2)
{
	GLfloat pointCoordinate;
	GLfloat h1 = (2 * u*u*u) - (3 * u*u) + 1;
	GLfloat h2 = (-2 * u*u*u) + (3 * u*u);
	GLfloat h3 = (u*u*u) - (2 * u*u) + u;
	GLfloat h4 = (u*u*u) - (u*u);
	pointCoordinate = (h1*p1) + (h2*p2) + (h3*t1) + (h4*t2);
	return pointCoordinate;
}

// Recursive subdivide method
void subdivide(GLfloat u0, GLfloat u1, GLfloat maxLineLength, int index, GLfloat tangentMagnitude_1, GLfloat tangentMagnitude_2)
{
	GLfloat uMid = (u0 + u1) / 2;

	// Pythagorean theorem to get the magnitude of the line between two points
	GLfloat x0 = computePoints(u0, controlPoints[index], controlPoints[index + 3], tangentMagnitude_1, tangentMagnitude_2);
	GLfloat x1 = computePoints(u1, controlPoints[index], controlPoints[index + 3], tangentMagnitude_1, tangentMagnitude_2);
	GLfloat x_Squared = (x1 - x0) * (x1 - x0);
	GLfloat y0 = computePoints(u0, controlPoints[index + 1], controlPoints[index + 4], tangentMagnitude_1, tangentMagnitude_2);
	GLfloat y1 = computePoints(u1, controlPoints[index + 1], controlPoints[index + 4], tangentMagnitude_1, tangentMagnitude_2);
	GLfloat y_Squared = (y1 - y0) * (y1 - y0);

	GLfloat lineLength = sqrt(x_Squared + y_Squared);

	if (lineLength > maxLineLength)
	{
		subdivide(u0, uMid, maxLineLength, index, tangentMagnitude_1, tangentMagnitude_2);
		subdivide(uMid, u1, maxLineLength, index, tangentMagnitude_1, tangentMagnitude_2);
	}
	else
	{
		// Put the correct x y z with uMid as the "u"
		splinePoints.push_back(computePoints(uMid, controlPoints[index], controlPoints[index + 3], tangentMagnitude_1, tangentMagnitude_2));
		splinePoints.push_back(computePoints(uMid, controlPoints[index + 1], controlPoints[index + 4], tangentMagnitude_1, tangentMagnitude_2));
		splinePoints.push_back(computePoints(uMid, controlPoints[index + 2], controlPoints[index + 5], tangentMagnitude_1, tangentMagnitude_2));
	}
}

int main() {

	initialize();

	// Game Loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Wipe the drawing surface clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.1f, 0.2f, 0.2f, 1.0f);
		glPointSize(point_size);

		// Camera/View transformation
		view_matrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		// Projection
		proj_matrix = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

		// Activate shader program
		glUseProgram(shaderProgram);

		// Draw the points and the tangent vectors
		draw(VAO[0], VBO[0], controlPoints);
		draw(VAO[1], VBO[1], tangentPoints);

		// Draw the spline and the triangles
		if (drawSpline)
		{
			draw(VAO[2], VBO[2], splinePoints);
			draw(VAO[3], VBO[3], trianglePoints);
		}

		// Update other events like input handling
		glfwPollEvents();
		if (cameraMoveAllowed)
			moveCamera();

		// put the stuff we've been drawing onto the display
		glfwSwapBuffers(window);
	}

	cleanUp();
	return 0;
}