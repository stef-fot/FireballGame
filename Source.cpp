//********************************
//Αυτό το αρχείο θα το χρησιμοποιήσετε
// για να υλοποιήσετε την άσκηση 1C της OpenGL
//
//ΑΜ:4723   Όνομα:ELEFTHERIOS-MARIOS MANIKAS
//ΑΜ:4829   Όνομα:STEFANOS FOTOPOULOS

//*********************************
#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION

// Include standard headers
#include <stdio.h>
#include <ctime>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include "stb_image.h"

//Playsound
#include <Windows.h>
#include <MMSystem.h>
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;


glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}

float Random(float* x_axis, float* y_axis, float* z_axis) {
	srand(time(0));
	*x_axis = 0 + (rand() % 100);
	*y_axis = 0 + (rand() % 100);
	*z_axis = 20;

	cout << *x_axis << "\n";
	cout << *y_axis << "\n";
	return 0;
}
//*******************************************************************************
// Η παρακάτω συνάρτηση είναι από http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
// H συνάρτηση loadOBJ φορτώνει ένα αντικείμενο από το obj αρχείο του και φορτώνει και normals kai uv συντεταγμένες
// Την χρησιμοποιείτε όπως το παράδειγμα που έχω στην main
// Very, VERY simple OBJ loader.
// 

bool loadOBJ(
	const char* path,
	std::vector<glm::vec3>& out_vertices,
	std::vector<glm::vec2>& out_uvs,
	std::vector<glm::vec3>& out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE* file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}
	fclose(file);
	return true;
}



//************************************
// Η LoadShaders είναι black box για σας
//************************************
GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}


//************************
// Βοηθητικές συναρτήσεις


float zoom = 45.0;//ftiaxame variable float zoom gia na kratisoume to arxiko radians tou ProjectionMatrix kai na to peirazoume analogws


///****************************************************************
//  Εδω θα υλοποιήσετε την συνάρτηση της κάμερας
//****************************************************************


void camera_function()
{
	// STRAFE STON AXONA X  UP
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		ViewMatrix = glm::rotate(ViewMatrix, radians(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	}
	//STRAFE STON AXONA X Down 
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		ViewMatrix = glm::rotate(ViewMatrix, radians(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f));

	}
	//STRAFE STON AXONA D  Dexiostrofa
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		ViewMatrix = glm::rotate(ViewMatrix, radians(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	}
	//STRAFE STON AXONA A Aristerostrofa
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		ViewMatrix = glm::rotate(ViewMatrix, radians(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	}
	// ZOOM IN
	if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
		zoom = zoom - 0.7;
		if (zoom)
		{
			ProjectionMatrix = glm::perspective(glm::radians(zoom), float(1000 / 1000), 0.1f, 10000.0f);
		}
	}
	//ZOOM OUT 
	if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
	{
		zoom = zoom + 0.7;
		if (zoom)
		{
			ProjectionMatrix = glm::perspective(glm::radians(zoom), float(1000 / 1000), 0.1f, 10000.0f);
		}
	}
}


int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1000, 1000, u8"Εργασία 1Γ Καταστροφή", NULL, NULL);


	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Black background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders

	GLuint programID = LoadShaders("ProjCVertexShader.vertexshader", "ProjCFragmentShader.fragmentshader");

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Load the texture
	//load grid
	int width, height, nrChannels;
	unsigned char* data = stbi_load("ground2.jpg", &width, &height, &nrChannels, 0);

	if (data)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;
	bool res = loadOBJ("grid.obj", vertices, uvs, normals);

	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);


	//************************************************
	// **Προσθέστε κώδικα για την κάμερα
	// **Δείγμα κλήσης των συναρτήσεων για δημιουργία του MVP - είναι τυχαίες οι ρυθμίσεις και δεν ανταποκρίνονται στην άσκησή σας

	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), float(1000 / 1000), 0.1f, 10000.0f);
	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(-20.0f, -20.0f, 40.0f),//position
		glm::vec3(50.0f, 50.0f, 0.0f),// target
		glm::vec3(0.0f, 0.0f, 1.0f)//anion dianysma
	);
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around
	ViewMatrix = View;//thetw to ViewMatrix = View gia na mporw na xrhsimopoihsw thn getViewMatrix() sthn camera_function()
	ProjectionMatrix = Projection;//thetw to ProjectionMatrix = Projection gia na mporw na xrhsimopoihsw thn getProjectionMatrix() sthn camera_function()
	//syntetagmenes koryfwn prismatos

	//*************************************************
	//load sfairas
	int widthball, heightball, nrChannelsball;
	unsigned char* data1 = stbi_load("fire.jpg", &widthball, &heightball, &nrChannelsball, 0);

	if (data1)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	GLuint textureIDball;
	glGenTextures(1, &textureIDball);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureIDball);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widthball, heightball, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureIDball = glGetUniformLocation(programID, "myTextureSampler");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Read our .obj file
	std::vector<glm::vec3> verticesball;
	std::vector<glm::vec3> normalsball;
	std::vector<glm::vec2> uvsball;
	bool resball = loadOBJ("ball.obj", verticesball, uvsball, normalsball);

	GLuint vertexbufferball;
	glGenBuffers(1, &vertexbufferball);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferball);
	glBufferData(GL_ARRAY_BUFFER, verticesball.size() * sizeof(glm::vec3), &verticesball[0], GL_STATIC_DRAW);

	GLuint uvbufferball;
	glGenBuffers(1, &uvbufferball);
	glBindBuffer(GL_ARRAY_BUFFER, uvbufferball);
	glBufferData(GL_ARRAY_BUFFER, uvsball.size() * sizeof(glm::vec2), &uvsball[0], GL_STATIC_DRAW);

	//*************************************************
	//load kratira
	int widthkrat, heightkrat, nrChannelskrat;
	unsigned char* datakrat = stbi_load("crater1.jpg", &widthkrat, &heightkrat, &nrChannelskrat, 0);

	if (datakrat)
	{

	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	GLuint textureIDkrat;
	glGenTextures(1, &textureIDkrat);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureIDkrat);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widthkrat, heightkrat, 0, GL_RGB, GL_UNSIGNED_BYTE, datakrat);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureIDkrat = glGetUniformLocation(programID, "myTextureSampler");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Read our .obj file
	std::vector<glm::vec3> verticeskrat;
	std::vector<glm::vec3> normalskrat;
	std::vector<glm::vec2> uvskrat;
	bool reskrat = loadOBJ("gridkratira.obj", verticeskrat, uvskrat, normalskrat);


	GLuint vertexbufferkrat;
	glGenBuffers(1, &vertexbufferkrat);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferkrat);
	glBufferData(GL_ARRAY_BUFFER, verticeskrat.size() * sizeof(glm::vec3), &verticeskrat[0], GL_STATIC_DRAW);

	GLuint uvbufferkrat;
	glGenBuffers(1, &uvbufferkrat);
	glBindBuffer(GL_ARRAY_BUFFER, uvbufferkrat);
	glBufferData(GL_ARRAY_BUFFER, uvskrat.size() * sizeof(glm::vec2), &uvskrat[0], GL_STATIC_DRAW);


	int flag = 0;
	float x_axis = 0;
	float y_axis = 0;
	float z_axis = 20;


	struct Coordinates
	{
		int x_axis;
		int y_axis;
	};
	struct Coordinates save_crater[400];
	int thesi_pinaka = 0;
	int counter_of_craters = 0;

	do {
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		//*************************************************
		// Να προστεθεί κώδικας για τον υπολογισμό του νέο MVP
		camera_function();//Kalw thn camera_function() h opoia allazei to ViewMatrix  & to projectionMatrix 
		View = getViewMatrix();//eisagw sthn view to allagmeno ViewMatrix an patithike kapoio apo ta apodekta plhktra
		Projection = getProjectionMatrix(); //eisagw sthn Projection to allagmeno ProjectionMatrix an patithike kapoio apo ta apodekta plhktra


		MVP = Projection * View * Model;// ypologizw to kainourgio mvp symfwna me ta kainourgia dedomena
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);



		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && flag == 0) {

			flag = 1;
			Random(&x_axis, &y_axis, &z_axis);
		}
		if (flag == 1) {
			glm::vec3 RandomVector = glm::vec3(x_axis, y_axis, z_axis);
			glm::mat4 Modelball = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
			glm::mat4 TranslationMatrix = glm::translate(glm::mat4(1.0f), RandomVector);
			Modelball = TranslationMatrix * Modelball;



			glm::mat4 MVPball = Projection * View * Modelball;// ypologizw to kainourgio mvp symfwna me ta kainourgia dedomena
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPball[0][0]);
			// Load it into a VBO

			// Bind our texture in Texture Unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureIDball);
			// Set our "myTextureSampler" sampler to use Texture Unit 0
			glUniform1i(TextureIDball, 0);

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbufferball);
			glVertexAttribPointer(
				0,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbufferball);
			glVertexAttribPointer(
				1,                                // attribute
				2,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);


			if (z_axis > 0.0) {
				z_axis -= 0.2;
				glm::mat4 TranslationMatrix1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.2f));
				Modelball = TranslationMatrix1 * Modelball;
				MVPball = Projection * View * Modelball;// ypologizw to kainourgio mvp symfwna me ta kainourgia dedomena
				if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
					z_axis = z_axis - 0.5;
					glm::mat4 TranslationMatrix1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.5f));
					Modelball = TranslationMatrix1 * Modelball;
					MVPball = Projection * View * Modelball;
				}
				if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
					z_axis = z_axis + 0.1;
					glm::mat4 TranslationMatrix1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.1f));
					Modelball = TranslationMatrix1 * Modelball;
					MVPball = Projection * View * Modelball;
				}
				//glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPball[0][0]);
				glDrawArrays(GL_TRIANGLES, 0, verticesball.size());
				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);

				if (z_axis <= 0.0) {
					PlaySound(TEXT("thunder2.wav"), NULL, SND_ASYNC | SND_FILENAME);
					save_crater[thesi_pinaka].x_axis = x_axis;
					save_crater[thesi_pinaka].y_axis = y_axis;
					counter_of_craters += 1;
					thesi_pinaka += 1;
					flag = 0;

				}
			}
		}

		for (int i = 0; i < counter_of_craters; i++) {
			glm::vec3 Vectorkratira = glm::vec3(save_crater[i].x_axis, save_crater[i].y_axis, 1);
			glm::mat4 TranslationMatrixkratira = glm::translate(glm::mat4(1.0f), Vectorkratira);
			glm::mat4 Modelkratira = glm::mat4(1.0f);
			Modelkratira = TranslationMatrixkratira * Modelkratira;
			glm::mat4 MVPkratira = Projection * View * Modelkratira;// ypologizw to kainourgio mvp symfwna me ta kainourgia dedomena
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPkratira[0][0]);

			//bind kratira
			// Bind our texture in Texture Unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureIDkrat);
			// Set our "myTextureSampler" sampler to use Texture Unit 0
			glUniform1i(TextureIDkrat, 0);

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vertexbufferkrat);
			glVertexAttribPointer(
				0,                  // attribute
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			// 2nd attribute buffer : UVs
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, uvbufferkrat);
			glVertexAttribPointer(
				1,                                // attribute
				2,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			glDrawArrays(GL_TRIANGLES, 0, verticeskrat.size());




		}


		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);


		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();


	}
	// Check if the SPACE key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &vertexbufferball);
	glDeleteBuffers(1, &uvbufferball);
	glDeleteBuffers(1, &vertexbufferkrat);
	glDeleteBuffers(1, &uvbufferkrat);
	glDeleteProgram(programID);
	glDeleteTextures(1, &textureID);
	glDeleteTextures(1, &textureIDball);
	glDeleteTextures(1, &textureIDkrat);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;

}

