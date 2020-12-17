// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <stack>   
#include <sstream>
#include <iostream>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
using namespace std;
// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/texture.hpp>
#include <common/texture.hpp>
const int window_width = 600, window_height = 600;
typedef struct Vertex {
	float Position[4];
	float Color[4];
	float Normal[3];
	void SetPosition(float *coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0;
	}
	void SetColor(float *color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
	}
	void SetNormal(float *coords) {
		Normal[0] = coords[0];
		Normal[1] = coords[1];
		Normal[2] = coords[2];
	}
};

// ATTN: USE POINT STRUCTS FOR EASIER COMPUTATIONS
typedef struct point {
	float x, y, z;
	point(const float x = 0, const float y = 0, const float z = 0) : x(x), y(y), z(z) {};
	point(float* coords) : x(coords[0]), y(coords[1]), z(coords[2]) {};
	point operator -(const point& a)const {
		return point(x - a.x, y - a.y, z - a.z);
	}
	point operator +(const point& a)const {
		return point(x + a.x, y + a.y, z + a.z);
	}
	point operator *(const float& a)const {
		return point(x * a, y * a, z * a);
	}
	point operator /(const float& a)const {
		return point(x / a, y / a, z / a);
	}
	float* toArray() {
		float array[] = { x, y, z, 1.0f };
		return array;
	}
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void loadObject(char*, glm::vec4, Vertex * &, GLushort* &, int);
void createVAOs(Vertex[], GLushort[], int);
void createObjects(void);
void pickObject(void);
void renderScene(void);
void cleanup(void);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);
void createVAOsForTexture(Vertex[], GLushort[], int);
void renderSmoothSurface(void);

// GLOBAL VARIABLES
GLFWwindow* window;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex = -1;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;
GLuint textureProgramID;

const GLuint NumObjects = 10;	// ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
GLuint VertexArrayId[NumObjects] = { 0 };
GLuint VertexBufferId[NumObjects] = { 0 };
GLuint IndexBufferId[NumObjects] = { 0 };
size_t NumVert[NumObjects] = { 0 };

size_t NumIndices[NumObjects] = { 0 };
size_t VertexBufferSize[NumObjects] = { 0 };
size_t IndexBufferSize[NumObjects] = { 0 };

GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint ProjMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorID;
GLuint pickingColorArrayID;
GLuint LightID;

GLuint texID;
GLuint textureID;

Vertex* Verts_Quad;
Vertex Verts_Quad_Array[144];

float pickingColor[144];
int ispress = 0;
float r1, g1, b1;

GLushort* Idcs_Quad;
GLushort Idcs_Quad_Array[576];
GLushort Idcs_Texture_Array[864];

Vertex Verts_Smooth_Array[1156] = { 0.0f };
GLushort Idcs_Smooth_Array[4624];
GLushort Idcs_Smooth_Texture_Array[6936];


std::vector<glm::vec2> uvs;

GLfloat uvs_Array[288];
GLfloat uvs_smooth_Array[2312];

glm::vec4 vecColorQuad = glm::vec4(1.0, 1.0, 1.0, 1.0);

float colorBlue[4] = { 0.0, 1.0, 1.0, 1.0 };
float colorYellow[4] = { 1.0, 1.0, 0.0, 1.0 };
float faceNormal[3] = { 0.0f, 0.0f, 1.0f };

bool displayFace = false;
bool displayTexture = false;
bool displaySmoothSurface = false;
bool displaySmoothTexture = false;
bool startSmile = false;
bool startFrown = false;
bool displaySmile = false;
bool displayFrown = false;

GLint gX = 0.0;
GLint gZ = 0.0;

//initail point of axes
const float PI = 3.1415926;
float startAngle1 = PI / 4;
float startAngle2 = asin(sqrt(3) / 3);

//initial point of lips and eyebrow
float startSmileY1 = 0.0f;
float startSmileY2 = 0.0f;
float startFrownY1 = 0.0f;
float startFrownY2 = 0.0f;
float startFrownY3 = 0.0f;
float startFrownY4 = 0.0f;
float startFrownY5 = 0.0f;
float startFrownY6 = 0.0f;

bool moveCameraLEFT = false;
bool moveCameraRIGHT = false;
bool moveCameraUP = false;
bool moveCameraDOWN = false;
bool resetCamera = false;

std::string instr1;
std::string instr2;
std::string instr3;
std::string instr4;
std::string instr5;
std::string instr6;
std::string instr7;

// animation control
bool animation = false;
GLfloat phi = 0.0;

void loadObject(char* file, glm::vec4 color, Vertex * &out_Vertices, GLushort* &out_Indices,  int ObjectId)
{
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	//std::vector<glm::vec2> uvs;//make uvs a global variable
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(file, vertices, uvs, normals);

	std::vector<GLushort> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	const size_t vertCount = indexed_vertices.size();

	const size_t uvCount = indexed_uvs.size();

	const size_t idxCount = indices.size();

	// populate output arrays
	out_Vertices = new Vertex[vertCount];
	for (int i = 0; i < vertCount; i++) {
		out_Vertices[i].SetPosition(&indexed_vertices[i].x);
		out_Vertices[i].SetNormal(&indexed_normals[i].x);
		out_Vertices[i].SetColor(&color[0]);
	}
	out_Indices = new GLushort[idxCount];
	for (int i = 0; i < idxCount; i++) {
		out_Indices[i] = indices[i];
	}
	// set global variables!!
	NumIndices[ObjectId] = idxCount;
	VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
	IndexBufferSize[ObjectId] = sizeof(GLushort) * idxCount;
}


void createObjects(void)
{
	//-- COORDINATE AXES --//
	Vertex CoordVerts[] =
	{
		{ { 0.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 6.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 6.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 6.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
	};

	VertexBufferSize[0] = sizeof(CoordVerts);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
	createVAOs(CoordVerts, NULL, 0);
	
	//-- GRID --//
	// ATTN: create your grid vertices here!
	Vertex CoordGridVerts[52];
	int i = 0;
	for (float x = -6; x <= 6; x++)
	{
		CoordGridVerts[i] = { { x, 0.0, -6.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
		i++;
		CoordGridVerts[i] = { { x, 0.0, 6.0, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
		i++;
	}
	for (float z = -6; z <= 6; z++)
	{
		CoordGridVerts[i] = { { -6.0, 0.0, z, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
		i++;
		CoordGridVerts[i] = { { 6.0, 0.0, z, 1.0 }, { 1.0, 1.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } };
		i++;
	}
	VertexBufferSize[1] = sizeof(CoordGridVerts);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
	createVAOs(CoordGridVerts, NULL, 1);
	
	//create uv coordinates array of face obj
	for (int i = 0; i < 12; i++)
	{
		for (int j = 0; j < 12; j++)
		{
			uvs_Array[2 * (12 * i + j)] = 0.28 + i * (0.73 - 0.28) / 11;//x coordinate in uv
			uvs_Array[2 * (12 * i + j) + 1] = 0.3 + j * (0.89 - 0.3) / 11;//y coordinate in uv
		}
	}
	//-- .OBJs --//
	texID =  loadBMP_custom("model/face.bmp");
	// ATTN: load your models here
	loadObject("model/facemesh.obj", vecColorQuad, Verts_Quad, Idcs_Quad, 2);
	for (int i = 0; i < 144; i++)
	{	
		float arr[3] = { *(Verts_Quad[i].Position),*(Verts_Quad[i].Position + 1),*(Verts_Quad[i].Position + 2) };
		Verts_Quad_Array[i].SetPosition(arr);
		Verts_Quad_Array[i].SetColor(colorBlue);
	}

	for (int i = 0; i < 144; i++) {
		
		if ((i + 1) % 12 != 0 && i < 132 && i != 143) {
			Idcs_Quad_Array[4 * i] = i;
			Idcs_Quad_Array[4 * i + 1] = i + 1;
		}
		else {
			Idcs_Quad_Array[4 * i] = i;
			Idcs_Quad_Array[4 * i + 1] = i;
		}
		if (i < 132) {
			Idcs_Quad_Array[4 * i + 2] = i;
			Idcs_Quad_Array[4 * i + 3] = i + 12;
		}
		else if (i != 143) {
			Idcs_Quad_Array[4 * i + 2] = i;
			Idcs_Quad_Array[4 * i + 3] = i + 1;
		}
		//notice the order
		if((i+1)%12!=0 && i<132)
		{
			Idcs_Texture_Array[6 * i] = i + 1;
			Idcs_Texture_Array[6 * i + 1] = i;
			Idcs_Texture_Array[6 * i + 2] = i + 13;
			Idcs_Texture_Array[6 * i + 3] = i + 13;
			Idcs_Texture_Array[6 * i + 4] = i;
			Idcs_Texture_Array[6 * i + 5] = i + 12;
		}

	}

	VertexBufferSize[2] = sizeof(Verts_Quad_Array);
	IndexBufferSize[2] = sizeof(Idcs_Quad_Array);
	createVAOs(Verts_Quad_Array, Idcs_Quad_Array, 2);

	VertexBufferSize[3] = sizeof(Verts_Quad_Array);
	IndexBufferSize[3] = sizeof(Idcs_Texture_Array);
	createVAOsForTexture(Verts_Quad_Array, Idcs_Texture_Array, 3);
}

void pickObject(void)
{
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 ModelMatrix;
	glUseProgram(pickingProgramID);
	{
		ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform1fv(pickingColorArrayID, NumVert[2], pickingColor);	// here we pass in the picking marker array

		// ATTN: DRAW YOUR PICKING SCENE HERE. REMEMBER TO SEND IN A DIFFERENT PICKING COLOR FOR EACH OBJECT BEFOREHAND
		// Draw the points
		glEnable(GL_PROGRAM_POINT_SIZE);
		glBindVertexArray(VertexArrayId[2]);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts_Quad_Array), Verts_Quad_Array);	// update buffer data
		glDrawElements(GL_POINTS, NumVert[2], GL_UNSIGNED_SHORT, (void*)0);

		glBindVertexArray(0);
	}
	glUseProgram(0);
	// Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow ! 
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel, 
	// because the framebuffer is on the GPU.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	// Convert the color back to an integer ID
	gPickedIndex = int(data[0]);

	if (gPickedIndex == 255) { // Full white, must be the background !
		gMessage = "background";
	}
	else {
		std::ostringstream oss;
		oss << "point " << gPickedIndex;
		gMessage = oss.str();
	}
	// Convert the color back to an integer ID
	if (gPickedIndex != 255) { // Full white, must be the background !

		if (ispress == 0 && gPickedIndex < 144)
		{
			if (gPickedIndex == 50 || gPickedIndex == 62 || gPickedIndex == 74 || gPickedIndex == 86) {
				displaySmile = true;
			}
			if (gPickedIndex == 31 || gPickedIndex == 43 || gPickedIndex == 55 || gPickedIndex == 91 || gPickedIndex == 103 || gPickedIndex == 115) {
				displayFrown = true;
			}

			r1 = Verts_Quad_Array[gPickedIndex].Color[0];
			g1 = Verts_Quad_Array[gPickedIndex].Color[1];
			b1 = Verts_Quad_Array[gPickedIndex].Color[2];
			ispress = 1;
			Verts_Quad_Array[gPickedIndex].SetColor(colorYellow);
		}
	}
	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
}

void renderScene(void)
{
	//ATTN: DRAW YOUR SCENE HERE. MODIFY/ADAPT WHERE NECESSARY!
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (moveCameraLEFT == true) {
		startAngle1 -= 0.001f;
	}
	if (moveCameraRIGHT == true) {
		startAngle1 += 0.001f;
	}
	if (moveCameraUP == true) {
		startAngle2 += 0.001f;
	}
	if (moveCameraDOWN == true) {
		startAngle2 -= 0.001f;
	}
	if (moveCameraLEFT || moveCameraRIGHT || moveCameraUP || moveCameraDOWN || resetCamera)
	{
		float cameraRadius = 10 * sqrt(3);
		float cameraXcoord = cameraRadius * cos(startAngle2) * sin(startAngle1);
		float cameraYcoord = cameraRadius * sin(startAngle2);
		float cameraZcoord = cameraRadius * cos(startAngle2) * cos(startAngle1);
		gViewMatrix = glm::lookAt(glm::vec3(cameraXcoord, cameraYcoord, cameraZcoord),	// eye
			glm::vec3(0.0, 5.0, 0.0),	// center
			glm::vec3(0.0, 1.0, 0.0));	// up
	}
	glUseProgram(programID);
	{
		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glm::mat4x4 ModelMatrix = glm::mat4(1.0);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

		glBindVertexArray(VertexArrayId[0]);	// draw CoordAxes
		glDrawArrays(GL_LINES, 0, 6);

		glBindVertexArray(VertexArrayId[1]);	// draw grid
		glDrawArrays(GL_LINES, 0, 52);
		glEnable(GL_PROGRAM_POINT_SIZE);

		if (displayFace == true) {          //draw quad mesh(face)
			glBindVertexArray(VertexArrayId[2]);

			//draw control points of quad mesh
			glDrawElements(GL_POINTS, NumVert[2], GL_UNSIGNED_SHORT, (void*)0);
			//glDrawArrays(GL_POINTS, 0, 144); //draw face
			//draw lines between points
			glDrawElements(GL_LINES, 576, GL_UNSIGNED_SHORT, (GLvoid*)0);
		}

		if (displaySmoothSurface == true) {
			glBindVertexArray(VertexArrayId[4]);

			glDrawArrays(GL_POINTS, 0, 1156); //draw smooth face
			glDrawElements(GL_LINES, 4624, GL_UNSIGNED_SHORT, (GLvoid*)0);
		}
		//animate smile:
		if (startSmile == false)
		{
			startSmileY1 = Verts_Quad_Array[51].Position[1];
			startSmileY2 = Verts_Quad_Array[99].Position[1];
			startSmile = true;
		}
		if (displaySmile == true && startSmile == true) {
			if (Verts_Quad_Array[51].Position[1] - startSmileY1 <= 0.27)
			{
				Verts_Quad_Array[51].Position[1] = Verts_Quad_Array[51].Position[1] + 0.02;
				Verts_Quad_Array[99].Position[1] = Verts_Quad_Array[99].Position[1] + 0.02;
			}
		}
		//animate frown:
		if (startFrown == false)
		{
			startFrownY1 = Verts_Quad_Array[55].Position[1];
			startFrownY2 = Verts_Quad_Array[43].Position[1];
			startFrownY3 = Verts_Quad_Array[91].Position[1];
			startFrownY4 = Verts_Quad_Array[103].Position[1];
			startFrownY5 = Verts_Quad_Array[31].Position[1];
			startFrownY6 = Verts_Quad_Array[115].Position[1];

			startFrown = true;
		}
		if (displayFrown == true && startFrown == true) {
			if (Verts_Quad_Array[55].Position[1] - startFrownY1 <= 0.25)
			{
				Verts_Quad_Array[55].Position[1] = Verts_Quad_Array[55].Position[1] + 0.002;
				Verts_Quad_Array[91].Position[1] = Verts_Quad_Array[91].Position[1] + 0.002;
			}
			if (startFrownY2 - Verts_Quad_Array[43].Position[1] <= 0.15) 
			{
				Verts_Quad_Array[43].Position[1] = Verts_Quad_Array[43].Position[1] - 0.002;
				Verts_Quad_Array[103].Position[1] = Verts_Quad_Array[103].Position[1] - 0.002;
				Verts_Quad_Array[31].Position[1] = Verts_Quad_Array[31].Position[1] - 0.002;
				Verts_Quad_Array[115].Position[1] = Verts_Quad_Array[115].Position[1] - 0.002;
			}
		}
		if (resetCamera == true) {
			Verts_Quad_Array[51].Position[1] = startSmileY1;
			Verts_Quad_Array[99].Position[1] = startSmileY2;
			Verts_Quad_Array[55].Position[1] = startFrownY1;
			Verts_Quad_Array[43].Position[1] = startFrownY2;
			Verts_Quad_Array[91].Position[1] = startFrownY3;
			Verts_Quad_Array[103].Position[1] = startFrownY4;
			Verts_Quad_Array[31].Position[1] = startFrownY5;
			Verts_Quad_Array[115].Position[1] = startFrownY6;

			if (displaySmoothSurface == true)
			{
				renderSmoothSurface();
			}
		}

		//change the coordinates of quad mesh
		VertexBufferSize[2] = sizeof(Verts_Quad_Array);
		IndexBufferSize[2] = sizeof(Idcs_Quad_Array);
		createVAOs(Verts_Quad_Array, Idcs_Quad_Array, 2);

		VertexBufferSize[3] = sizeof(Verts_Quad_Array);
		IndexBufferSize[3] = sizeof(Idcs_Texture_Array);
		createVAOsForTexture(Verts_Quad_Array, Idcs_Texture_Array, 3);

		glBindVertexArray(0);

	}
	if ((displayFace == true && displayTexture == true) || (displaySmoothSurface == true && displaySmoothTexture == true)) {
		glUseProgram(textureProgramID);
		{
			glm::vec3 lightPos = glm::vec3(4, 4, 4);
			glm::mat4x4 ModelMatrix = glm::mat4(1.0);
			glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
			glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texID);
			glUniform1i(textureID, 0);
			if (displayFace == true && displayTexture == true)
			{
				glBindVertexArray(VertexArrayId[3]);
				glDrawElements(GL_TRIANGLES, 864, GL_UNSIGNED_SHORT, (void*)0);
			}

			if (displaySmoothSurface == true && displaySmoothTexture == true) {
				glBindVertexArray(VertexArrayId[5]);
				glDrawElements(GL_TRIANGLES, 6936, GL_UNSIGNED_SHORT, (void*)0);
			}
			glBindVertexArray(0);
		}
	}
	glUseProgram(0);
	// Draw GUI
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}



void renderSmoothSurface(void)
{
	int j = 0;
	for (int i = 0; i < 144; i++) {
		point* s00, * s01, * s02, * s10, * s11, * s12, * s20, * s21, * s22;
		if (i < 12) {
			s00 = new point(Verts_Quad_Array[i].Position[0] - 1, Verts_Quad_Array[i].Position[1] - 1, Verts_Quad_Array[i].Position[2]);
			s01 = new point(Verts_Quad_Array[i].Position[0] - 1, Verts_Quad_Array[i].Position[1], Verts_Quad_Array[i].Position[2]);
			s02 = new point(Verts_Quad_Array[i].Position[0] - 1, Verts_Quad_Array[i].Position[1] + 1, Verts_Quad_Array[i].Position[2]);
			if (i == 0) {
				s10 = new point(Verts_Quad_Array[i].Position[0], Verts_Quad_Array[i].Position[1] - 1, Verts_Quad_Array[i].Position[2]);
			}
			else {
				s10 = new point(Verts_Quad_Array[i - 1].Position[0], Verts_Quad_Array[i - 1].Position[1], Verts_Quad_Array[i - 1].Position[2]);
			}
			s11 = new point(Verts_Quad_Array[i].Position[0], Verts_Quad_Array[i].Position[1], Verts_Quad_Array[i].Position[2]);
			if (i == 11) {
				s12 = new point(Verts_Quad_Array[i].Position[0], Verts_Quad_Array[i].Position[1] + 1, Verts_Quad_Array[i].Position[2]);
			}
			else {
				s12 = new point(Verts_Quad_Array[i + 1].Position[0], Verts_Quad_Array[i + 1].Position[1], Verts_Quad_Array[i + 1].Position[2]);
			}
			if (i == 0) {
				s20 = new point(Verts_Quad_Array[i].Position[0] + 1, Verts_Quad_Array[i].Position[1] - 1, Verts_Quad_Array[i].Position[2]);
			}
			else {
				s20 = new point(Verts_Quad_Array[i + 11].Position[0], Verts_Quad_Array[i + 11].Position[1], Verts_Quad_Array[i + 11].Position[2]);
			}
			s21 = new point(Verts_Quad_Array[i + 12].Position[0], Verts_Quad_Array[i + 12].Position[1], Verts_Quad_Array[i + 12].Position[2]);
			if (i == 11) {
				s22 = new point(Verts_Quad_Array[i].Position[0] + 1, Verts_Quad_Array[i].Position[1] + 1, Verts_Quad_Array[i].Position[2]);
			}
			else {
				s22 = new point(Verts_Quad_Array[i + 13].Position[0], Verts_Quad_Array[i + 13].Position[1], Verts_Quad_Array[i + 13].Position[2]);
			}

		}

		else if ((i + 1) % 12 == 0 && i > 12 && i < 132) {
			s00 = new point(Verts_Quad_Array[i - 13].Position[0], Verts_Quad_Array[i - 13].Position[1], Verts_Quad_Array[i - 13].Position[2]);
			s01 = new point(Verts_Quad_Array[i - 12].Position[0], Verts_Quad_Array[i - 12].Position[1], Verts_Quad_Array[i - 12].Position[2]);
			s02 = new point(Verts_Quad_Array[i].Position[0] - 1, Verts_Quad_Array[i].Position[1] + 1, Verts_Quad_Array[i].Position[2]);
			s10 = new point(Verts_Quad_Array[i - 1].Position[0], Verts_Quad_Array[i - 1].Position[1], Verts_Quad_Array[i - 1].Position[2]);
			s11 = new point(Verts_Quad_Array[i].Position[0], Verts_Quad_Array[i].Position[1], Verts_Quad_Array[i].Position[2]);
			s12 = new point(Verts_Quad_Array[i].Position[0], Verts_Quad_Array[i].Position[1] + 1, Verts_Quad_Array[i].Position[2]);
			s20 = new point(Verts_Quad_Array[i + 11].Position[0], Verts_Quad_Array[i + 11].Position[1], Verts_Quad_Array[i + 11].Position[2]);
			s21 = new point(Verts_Quad_Array[i + 12].Position[0], Verts_Quad_Array[i + 12].Position[1], Verts_Quad_Array[i + 12].Position[2]);
			s22 = new point(Verts_Quad_Array[i].Position[0] + 1, Verts_Quad_Array[i].Position[1] + 1, Verts_Quad_Array[i].Position[2]);
		}
		else if (i > 132 && ((i + 1) % 12 != 0 || i == 143)) {
			s00 = new point(Verts_Quad_Array[i - 13].Position[0], Verts_Quad_Array[i - 13].Position[1], Verts_Quad_Array[i - 13].Position[2]);
			s01 = new point(Verts_Quad_Array[i - 12].Position[0], Verts_Quad_Array[i - 12].Position[1], Verts_Quad_Array[i - 12].Position[2]);
			if (i == 143) {
				s02 = new point(Verts_Quad_Array[i].Position[0] - 1, Verts_Quad_Array[i].Position[1] + 1, Verts_Quad_Array[i].Position[2]);
			}
			else {
				s02 = new point(Verts_Quad_Array[i - 11].Position[0], Verts_Quad_Array[i - 11].Position[1], Verts_Quad_Array[i - 11].Position[2]);
			}
			s10 = new point(Verts_Quad_Array[i - 1].Position[0], Verts_Quad_Array[i - 1].Position[1], Verts_Quad_Array[i - 1].Position[2]);
			s11 = new point(Verts_Quad_Array[i].Position[0], Verts_Quad_Array[i].Position[1], Verts_Quad_Array[i].Position[2]);
			if (i == 143) {
				s12 = new point(Verts_Quad_Array[i].Position[0], Verts_Quad_Array[i].Position[1] + 1, Verts_Quad_Array[i].Position[2]);
			}
			else {
				s12 = new point(Verts_Quad_Array[i + 1].Position[0], Verts_Quad_Array[i + 1].Position[1], Verts_Quad_Array[i + 1].Position[2]);
			}
			s20 = new point(Verts_Quad_Array[i].Position[0] + 1, Verts_Quad_Array[i].Position[1] - 1, Verts_Quad_Array[i].Position[2]);
			s21 = new point(Verts_Quad_Array[i].Position[0] + 1, Verts_Quad_Array[i].Position[1], Verts_Quad_Array[i].Position[2]);
			s22 = new point(Verts_Quad_Array[i].Position[0] + 1, Verts_Quad_Array[i].Position[1] + 1, Verts_Quad_Array[i].Position[2]);
		}

		else if (i % 12 == 0 && i >= 12 && i <= 132) {
			s00 = new point(Verts_Quad_Array[i].Position[0] - 1, Verts_Quad_Array[i].Position[1] - 1, Verts_Quad_Array[i].Position[2]);
			s01 = new point(Verts_Quad_Array[i - 12].Position[0], Verts_Quad_Array[i - 12].Position[1], Verts_Quad_Array[i - 12].Position[2]);
			s02 = new point(Verts_Quad_Array[i - 11].Position[0], Verts_Quad_Array[i - 11].Position[1], Verts_Quad_Array[i - 11].Position[2]);
			s10 = new point(Verts_Quad_Array[i].Position[0], Verts_Quad_Array[i].Position[1] - 1, Verts_Quad_Array[i].Position[2]);
			s11 = new point(Verts_Quad_Array[i].Position[0], Verts_Quad_Array[i].Position[1], Verts_Quad_Array[i].Position[2]);
			s12 = new point(Verts_Quad_Array[i + 1].Position[0], Verts_Quad_Array[i + 1].Position[1], Verts_Quad_Array[i + 1].Position[2]);
			s20 = new point(Verts_Quad_Array[i].Position[0] + 1, Verts_Quad_Array[i].Position[1] - 1, Verts_Quad_Array[i].Position[2]);
	
			if (i == 132) {
				s21 = new point(Verts_Quad_Array[i].Position[0] + 1, Verts_Quad_Array[i].Position[1], Verts_Quad_Array[i].Position[2]);
				
			}
			else {
				s21 = new point(Verts_Quad_Array[i + 12].Position[0], Verts_Quad_Array[i + 12].Position[1], Verts_Quad_Array[i + 12].Position[2]);
			}
			if (i == 132) {
				s22 = new point(Verts_Quad_Array[i].Position[0] + 1, Verts_Quad_Array[i].Position[1] + 1, Verts_Quad_Array[i].Position[2]);
			}
			else {
				s22 = new point(Verts_Quad_Array[i + 13].Position[0], Verts_Quad_Array[i + 13].Position[1], Verts_Quad_Array[i + 13].Position[2]);
			}
		}

		else {
			s00 = new point(Verts_Quad_Array[i - 13].Position[0], Verts_Quad_Array[i - 13].Position[1], Verts_Quad_Array[i - 13].Position[2]);
			s01 = new point(Verts_Quad_Array[i - 12].Position[0], Verts_Quad_Array[i - 12].Position[1], Verts_Quad_Array[i - 12].Position[2]);
			s02 = new point(Verts_Quad_Array[i - 11].Position[0], Verts_Quad_Array[i - 11].Position[1], Verts_Quad_Array[i - 11].Position[2]);
			s10 = new point(Verts_Quad_Array[i - 1].Position[0], Verts_Quad_Array[i - 1].Position[1], Verts_Quad_Array[i - 1].Position[2]);
			s11 = new point(Verts_Quad_Array[i].Position[0], Verts_Quad_Array[i].Position[1], Verts_Quad_Array[i].Position[2]);
			s12 = new point(Verts_Quad_Array[i + 1].Position[0], Verts_Quad_Array[i + 1].Position[1], Verts_Quad_Array[i + 1].Position[2]);
			s20 = new point(Verts_Quad_Array[i + 11].Position[0], Verts_Quad_Array[i + 11].Position[1], Verts_Quad_Array[i + 11].Position[2]);
			s21 = new point(Verts_Quad_Array[i + 12].Position[0], Verts_Quad_Array[i + 12].Position[1], Verts_Quad_Array[i + 12].Position[2]);
			s22 = new point(Verts_Quad_Array[i + 13].Position[0], Verts_Quad_Array[i + 13].Position[1], Verts_Quad_Array[i + 13].Position[2]);
		}
		point c00 = (*s11 * (float)(16.0f / 36.0f)) + ((*s21 + *s12 + *s01 + *s10) * (float)(4.0f / 36.0f)) + ((*s22 + *s02 + *s00 + *s20) * (float)(1.0f / 36.0f));

		point c01 = (*s11 * (float)(8.0f / 18.0f)) + ((*s01 + *s21) * (float)(2.0f / 18.0f)) + (*s12 * (float)(4.0f / 18.0f)) + ((*s22 + *s02) * (float)(1.0f / 18.0f));

		point c02 = (*s12 * (float)(8.0f / 18.0f)) + ((*s02 + *s22) * (float)(2.0f / 18.0f)) + (*s11 * (float)(4.0f / 18.0f)) + ((*s21 + *s01) * (float)(1.0f / 18.0f));

		point c10 = (*s11 * (float)(8.0f / 18.0f)) + ((*s10 + *s12) * (float)(2.0f / 18.0f)) + (*s21 * (float)(4.0f / 18.0f)) + ((*s22 + *s20) * (float)(1.0f / 18.0f));

		point c11 = (*s11 * (float)(4.0f / 9.0f)) + ((*s21 + *s12) * (float)(2.0f / 9.0f)) + ((*s22) * (float)(1.0f / 9.0f));

		point c12 = (*s12 * (float)(4.0f / 9.0f)) + ((*s11 + *s22) * (float)(2.0f / 9.0f)) + (*s21 * (float)(1.0f / 9.0f));

		point c20 = (*s21 * (float)(8.0f / 18.0f)) + ((*s20 + *s22) * (float)(2.0f / 18.0f)) + (*s11 * (float)(4.0f / 18.0f) + (*s12 + *s10) * (float)(1.0f / 18.0f));

		point c21 = (*s21 * (float)(4.0f / 9.0f)) + ((*s11 + *s22) * (float)(2.0f / 9.0f)) + (*s12 * (float)(1.0f / 9.0f));

		point c22 = (*s22 * (float)(4.0f / 9.0f)) + ((*s11 + *s22) * (float)(2.0f / 9.0f)) + (*s11 * (float)(1.0f / 9.0f));

		Verts_Smooth_Array[3 * j].Position[0] = c00.x;
		Verts_Smooth_Array[3 * j].Position[1] = c00.y;
		Verts_Smooth_Array[3 * j].Position[2] = c00.z - 0.3f;
		Verts_Smooth_Array[3 * j].Position[3] = 1.0f;
		Verts_Smooth_Array[3 * j].SetColor(colorBlue);
		Verts_Smooth_Array[3 * j].SetNormal(faceNormal);

		if ((i + 1) % 12 != 0) {
			Verts_Smooth_Array[3 * j + 1].Position[0] = c01.x;
			Verts_Smooth_Array[3 * j + 1].Position[1] = c01.y;
			Verts_Smooth_Array[3 * j + 1].Position[2] = c01.z - 0.3f;
			Verts_Smooth_Array[3 * j + 1].Position[3] = 1.0f;
			Verts_Smooth_Array[3 * j + 1].SetColor(colorBlue);
			Verts_Smooth_Array[3 * j + 1].SetNormal(faceNormal);

			Verts_Smooth_Array[3 * j + 2].Position[0] = c02.x;
			Verts_Smooth_Array[3 * j + 2].Position[1] = c02.y;
			Verts_Smooth_Array[3 * j + 2].Position[2] = c02.z - 0.3f;
			Verts_Smooth_Array[3 * j + 2].Position[3] = 1.0f;
			Verts_Smooth_Array[3 * j + 2].SetColor(colorBlue);
			Verts_Smooth_Array[3 * j + 2].SetNormal(faceNormal);
		}

		if (i < 132) {
			Verts_Smooth_Array[3 * j + 34].Position[0] = c10.x;
			Verts_Smooth_Array[3 * j + 34].Position[1] = c10.y;
			Verts_Smooth_Array[3 * j + 34].Position[2] = c10.z - 0.3f;
			Verts_Smooth_Array[3 * j + 34].Position[3] = 1.0f;
			Verts_Smooth_Array[3 * j + 34].SetColor(colorBlue);
			Verts_Smooth_Array[3 * j + 34].SetNormal(faceNormal);

			if ((i + 1) % 12 != 0) {
				Verts_Smooth_Array[3 * j + 35].Position[0] = c11.x;
				Verts_Smooth_Array[3 * j + 35].Position[1] = c11.y;
				Verts_Smooth_Array[3 * j + 35].Position[2] = c11.z - 0.3f;
				Verts_Smooth_Array[3 * j + 35].Position[3] = 1.0f;
				Verts_Smooth_Array[3 * j + 35].SetColor(colorBlue);
				Verts_Smooth_Array[3 * j + 35].SetNormal(faceNormal);

				Verts_Smooth_Array[3 * j + 36].Position[0] = c12.x;
				Verts_Smooth_Array[3 * j + 36].Position[1] = c12.y;
				Verts_Smooth_Array[3 * j + 36].Position[2] = c12.z - 0.3f;
				Verts_Smooth_Array[3 * j + 36].Position[3] = 1.0f;
				Verts_Smooth_Array[3 * j + 36].SetColor(colorBlue);
				Verts_Smooth_Array[3 * j + 36].SetNormal(faceNormal);
			}

			Verts_Smooth_Array[3 * j + 68].Position[0] = c20.x;
			Verts_Smooth_Array[3 * j + 68].Position[1] = c20.y;
			Verts_Smooth_Array[3 * j + 68].Position[2] = c20.z - 0.3f;
			Verts_Smooth_Array[3 * j + 68].Position[3] = 1.0f;
			Verts_Smooth_Array[3 * j + 68].SetColor(colorBlue);
			Verts_Smooth_Array[3 * j + 68].SetNormal(faceNormal);

			if ((i + 1) % 12 != 0) {
				Verts_Smooth_Array[3 * j + 69].Position[0] = c21.x;
				Verts_Smooth_Array[3 * j + 69].Position[1] = c21.y;
				Verts_Smooth_Array[3 * j + 69].Position[2] = c21.z - 0.3f;
				Verts_Smooth_Array[3 * j + 69].Position[3] = 1.0f;
				Verts_Smooth_Array[3 * j + 69].SetColor(colorBlue);
				Verts_Smooth_Array[3 * j + 69].SetNormal(faceNormal);

				Verts_Smooth_Array[3 * j + 70].Position[0] = c22.x;
				Verts_Smooth_Array[3 * j + 70].Position[1] = c22.y;
				Verts_Smooth_Array[3 * j + 70].Position[2] = c22.z - 0.3f;
				Verts_Smooth_Array[3 * j + 70].Position[3] = 1.0f;
				Verts_Smooth_Array[3 * j + 70].SetColor(colorBlue);
				Verts_Smooth_Array[3 * j + 70].SetNormal(faceNormal);
			}
		}

		if (i != 0 && (i + 1) % 12 == 0) {
			j = j + 23;
		}
		else {
			j++;
		}
	}

	for (int i = 0; i < 1156; i++) {
		
		if ((i + 1) % 34 != 0 && i < 1122 && i != 1155) {
			Idcs_Smooth_Array[4 * i] = i;
			Idcs_Smooth_Array[4 * i + 1] = i + 1;
		}
		else {
			Idcs_Smooth_Array[4 * i] = i;
			Idcs_Smooth_Array[4 * i + 1] = i;
		}
		if (i < 1122) {
			Idcs_Smooth_Array[4 * i + 2] = i;
			Idcs_Smooth_Array[4 * i + 3] = i + 34;
		}
		else if (i != 1155) {
			Idcs_Smooth_Array[4 * i + 2] = i;
			Idcs_Smooth_Array[4 * i + 3] = i + 1;
		}

		if (i == 0 || (i + 1) % 34 != 0 && i < 1122) {
			Idcs_Smooth_Texture_Array[6 * i] = i + 1;
			Idcs_Smooth_Texture_Array[6 * i + 1] = i;
			Idcs_Smooth_Texture_Array[6 * i + 2] = i + 35;
			Idcs_Smooth_Texture_Array[6 * i + 3] = i + 35;
			Idcs_Smooth_Texture_Array[6 * i + 4] = i;
			Idcs_Smooth_Texture_Array[6 * i + 5] = i + 34;
		}

		for (int i = 0; i < 34; i++)
		{
			for (int j = 0; j < 34; j++)
			{
				uvs_smooth_Array[2 * (34 * i + j)] = 0.28 + i * (0.73 - 0.28) / 33;//x coordinate in uv
				uvs_smooth_Array[2 * (34 * i + j) + 1] = 0.3 + j * (0.89 - 0.3) / 33;//y coordinate in uv
			}
		}
	}

	VertexBufferSize[4] = sizeof(Verts_Smooth_Array);
	IndexBufferSize[4] = sizeof(Idcs_Smooth_Array);
	createVAOs(Verts_Smooth_Array, Idcs_Smooth_Array, 4);

	VertexBufferSize[5] = sizeof(Verts_Smooth_Array);
	IndexBufferSize[5] = sizeof(Idcs_Smooth_Texture_Array);
	createVAOsForTexture(Verts_Smooth_Array, Idcs_Smooth_Texture_Array, 5);
}

int initWindow(void)
{
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "Wei,Yixin(5114-6181)", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width, window_height);
	TwBar * GUI = TwNewBar("Picking"); 
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	TwBar* InforUI = TwNewBar("Instru");
	TwDefine(" Instru position='20 400' ");
	TwDefine(" Instru size='200 150' ");
	TwSetParam(InforUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");

	instr1 = "Press R";
	TwAddVarRW(InforUI, "Reset:", TW_TYPE_STDSTRING, &instr1, NULL);
	instr2 = "Press F";
	TwAddVarRW(InforUI, "Face Mesh:", TW_TYPE_STDSTRING, &instr2, NULL);
	instr3 = "Press T";
	TwAddVarRW(InforUI, "Face Texture:", TW_TYPE_STDSTRING, &instr3, NULL);
	instr4 = "Press B";
	TwAddVarRW(InforUI, "Smooth Face:", TW_TYPE_STDSTRING, &instr4, NULL);
	instr5 = "Press S";
	TwAddVarRW(InforUI, "Smooth Texture:", TW_TYPE_STDSTRING, &instr5, NULL);
	instr6 = "Pick lip";
	TwAddVarRW(InforUI, "Smile:", TW_TYPE_STDSTRING, &instr6, NULL);
	instr7 = "Eyebrow";
	TwAddVarRW(InforUI, "Frown:", TW_TYPE_STDSTRING, &instr7, NULL);
	// Set up inputs
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);
	
	return 0;
}

void initOpenGL(void)
{
	for (int i = 0; i < 144; i++)
	{
		pickingColor[i] = i / 255.0f;
	}

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Projection matrix : 45?Field of View, 600:600 ratio, display range : 0.1 unit <-> 100 units
	gProjectionMatrix = glm::perspective(45.0f, 1.0f / 1.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	gViewMatrix = glm::lookAt(glm::vec3(10.0, 10.0, 10.0f),	// eye
		glm::vec3(0.0, 5.0, 0.0),	// center
		glm::vec3(0.0, 1.0, 0.0));	// up

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("p3_StandardShading.vertexshader", "p3_StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("p3_Picking.vertexshader", "p3_Picking.fragmentshader");

	textureProgramID = LoadShaders("p3_TransformVertexShader.vertexshader", "p3_TextureFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ProjMatrixID = glGetUniformLocation(programID, "P");
	
	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	//pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	pickingColorArrayID = glGetUniformLocation(pickingProgramID, "PickingColorArray");
	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	
	textureID = glGetUniformLocation(textureProgramID, "myTextureSampler");

	createObjects();
}

void createVAOs(Vertex Vertices[], unsigned short Indices[], int ObjectId) {

	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);	//
	glBindVertexArray(VertexArrayId[ObjectId]);		//

	// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}
	NumVert[ObjectId] = IndexBufferSize[ObjectId] / (sizeof GLubyte);

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset); 
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal

	// Disable our Vertex Buffer Object 
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
			);
	}
}

void createVAOsForTexture(Vertex Vertices[], unsigned short Indices[], int ObjectId) {
	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);	//
	glBindVertexArray(VertexArrayId[ObjectId]);		//

													// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);

	glEnableVertexAttribArray(0);	// position
									//glEnableVertexAttribArray(2);	// normal
	
	if (ObjectId == 3) {
		GLuint uvbuffer;
		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);

		glBufferData(GL_ARRAY_BUFFER, sizeof(uvs_Array), uvs_Array, GL_STATIC_DRAW);
		//glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(1);	// color
	}

	if (ObjectId == 5)
	{
		GLuint uvbuffer;
		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);

		glBufferData(GL_ARRAY_BUFFER, sizeof(uvs_smooth_Array), uvs_smooth_Array, GL_STATIC_DRAW);
		//glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		glEnableVertexAttribArray(1);	// color
	}
	// Disable our Vertex Buffer Object
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
		);
	}
}

void cleanup(void)
{
	// Cleanup VBO and shader
	for (int i = 0; i < NumObjects; i++) {
		glDeleteBuffers(1, &VertexBufferId[i]);
		glDeleteBuffers(1, &IndexBufferId[i]);
		glDeleteVertexArrays(1, &VertexArrayId[i]);
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);
	glDeleteProgram(textureProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// ATTN: MODIFY AS APPROPRIATE
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_R:
			resetCamera = true;
			startAngle1 = PI / 4;
			startAngle2 = asin(sqrt(3) / 3);
			displaySmile = false;
			displayFrown = false;

			break;
		case GLFW_KEY_F:
			if (displayFace == false)
			{
				displayFace = true;
				displaySmoothSurface = false;
			}
			else
				displayFace = false;
			break;
		case GLFW_KEY_T:
			if(displayFace == true && displayTexture == false)
				displayTexture = true;
			else
				displayTexture = false;
			break;
		case GLFW_KEY_B:
			if (displaySmoothSurface == false)
			{
				displaySmoothSurface = true;
				displayFace = false;
				displaySmoothTexture = false;
				displayTexture = false;
				renderSmoothSurface();
			}
			else
				displaySmoothSurface = false;
			break;
		case GLFW_KEY_S:
			if (displaySmoothSurface == true && displaySmoothTexture == false)
				displaySmoothTexture = true;
			else
				displaySmoothTexture = false;
			break;
		case GLFW_KEY_SPACE:
			break;
		case GLFW_KEY_LEFT:
			moveCameraLEFT = true;
			break;
		case GLFW_KEY_RIGHT:
			moveCameraRIGHT = true;
			break;
		case GLFW_KEY_UP:
			moveCameraUP = true;
			break;
		case GLFW_KEY_DOWN:
			moveCameraDOWN = true;
			break;
		default:
			break;
		}
	}
	else if (action == GLFW_RELEASE) {
		switch (key)
		{
		case GLFW_KEY_R:
			resetCamera = false;
			break;
			 
		case GLFW_KEY_LEFT:
			moveCameraLEFT = false;
			break;
		case GLFW_KEY_RIGHT:
			moveCameraRIGHT = false;
			break;
		case GLFW_KEY_UP:
			moveCameraUP = false;
			break;
		case GLFW_KEY_DOWN:
			moveCameraDOWN = false;
			break;
		default:
			break;
		}
	}
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickObject();
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		if (ispress == 1 && gPickedIndex < 144)
		{
			Verts_Quad_Array[gPickedIndex].Color[0] = r1;
			Verts_Quad_Array[gPickedIndex].Color[1] = g1;
			Verts_Quad_Array[gPickedIndex].Color[2] = b1;
			ispress = 0;
		}
	}
}

int main(void)
{
	// initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;

	// initialize OpenGL pipeline
	initOpenGL();

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		//// Measure speed
		//double currentTime = glfwGetTime();
		//nbFrames++;
		//if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
		//	// printf and reset
		//	printf("%f ms/frame\n", 1000.0 / double(nbFrames));
		//	nbFrames = 0;
		//	lastTime += 1.0;
		//}
		
		if (animation){
			phi += 0.01;
			if (phi > 360)
				phi -= 360;
		}

		// DRAWING POINTS
		renderScene();


	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}