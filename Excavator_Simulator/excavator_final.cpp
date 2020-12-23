// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
using namespace std;

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow *window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/texture.hpp>

glm::mat4 View;
glm::mat4 ProjectionMatrix;
glm::mat4 ModelMatrix;

glm::mat4 lastTPView; // 1��Ī ���� ��ȯ�� ��ü ������ ī�޶� ��ǥ ����

// Initial horizontal angle : toward -Z
float horizontalAngle = 0.0f;
// Initial vertical angle : none
float verticalAngle = 0.0f;

float speed = 4.0f; // 3 units / second
float mouseSpeed = 2.0f;

bool firstPress = true;
double xpos_prev = 0.0;
double ypos_prev = 0.0;
double xpos = 0.0;
double ypos = 0.0;

float gOrientation = 0.00f; // ���ı� ���� �� �ڵ�� ��Ŷ ȸ����
double lastTime = 0.0; // ���� �ֱ� �ð�
double lastSpaceTime = 0.0; // ���� �ֱٿ� �����̽��� ���� �ð�
int isFP = 0; // ���� ������ 1��Ī �������� ����
double lastDiggingTime = 0.0; // ������ ���ı� ���� ���� �ð�
int isDigging = 0; // ���� ���ı� ���� ���� ����
double currentTime; // ���� �ð�
float deltaTime; // �ð� ���� ����
double lastFrameTime; // �ֱ� ������ �ð�
int flag; // ���ı� ���� �� ȸ�� ������ �����ϴ� flag

glm::vec3 moving(0.0f, 0.0f, 0.0f); // ������ ������ ��ǥ

void computeMouseRotates(); // ���콺 �Է����� ��ü�� ȸ��
void computeKeyboardTranslates(); // Ű���� �Է����� ��ü�� ȸ��
void digging();  // ���ı� ���� ����

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
	window = glfwCreateWindow(1024, 768, "Computer Graphics - Excavator Simulator", NULL, NULL);
	if (window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	//glfwSetCursorPos(window, 1024 / 2, 768 / 2);


	// sky blue background
	glClearColor(0.0f, 0.8f, 1.0f, 1.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE); // �̰� ���ϸ� �� �ؽ��İ� �����Ÿ�

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	
	// �� �������� �÷� �ٸ��� ǥ���ϱ� ���� ���
	GLuint ColorCheckID = glGetUniformLocation(programID, "colorCheck");

	// ���� �ؽ���
	GLuint Texture = loadBMP_custom("ground.bmp");
	GLuint TextureCheckID = glGetUniformLocation(programID, "TextureCheck");

	// Read cube.obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("cube.obj", vertices, uvs, normals);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	// Projection matrix : 45?Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

	// Camera matrix
	// y�� ������ xz����� �ٶ󺻴�.
	View = glm::lookAt(
		glm::vec3(7, 6, -5), // Camera is at (11,10,-7), in World Space
		glm::vec3(0.0f, 0.0f, 0.0f),	// and looks at the origin
		glm::vec3(0.0f, 1.0f, 0.0f)	 // Head is up
	);
	ModelMatrix = glm::mat4(1.0);

	// For speed computation
	lastTime = glfwGetTime();
	lastFrameTime = lastTime;

	//x,z��鿡 ��ġ ��Ų �� ����		
	// ����
	glm::vec3 gPosition1(0.0f, 0.0f, 0.0f); //�Ϻ�
	glm::vec3 gPosition12(0.3f, 0.8f, 0.5f); //������
	glm::vec3 gPosition13(0.3f, 0.8f, 0.5f); //�� â��
	glm::vec3 gPosition14(0.3f, 0.8f, 0.5f); //�յ� â��

	// �ڵ�
	glm::vec3 gPosition2(-0.35f, 1.0f, 0.5f); //�ڵ�1
	glm::vec3 gPosition3(0.0f, 0.3f, 1.5f); //�ڵ�2
	glm::vec3 gPosition4(-0.35f, 0.2f, 1.15f); //�ڵ�3

	// bucket
	glm::vec3 gPosition5(0.0f, -0.26f, 1.645f);

	// ����1
	glm::vec3 gPosition6(0.6f, -0.8f, 0.0f);
	// ����2
	glm::vec3 gPosition7(-0.6f, -0.8f, 0.0f);

	// �ٴ�
	glm::vec3 gPosition8(0.0f, -1.1f, 0.0f);

	flag = 0; // �ڵ��� ȸ�� ������ �����ϴ� flag

	do
	{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the Model matrix from keyboard and mouse input
		// ��ü ���������� ���콺�� ���� �̵� ����
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !isFP)
		{
			if (firstPress)
			{
				glfwGetCursorPos(window, &xpos_prev, &ypos_prev);
				firstPress = false;
			}
			computeMouseRotates();
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && !isFP)
			firstPress = true;

		computeKeyboardTranslates();

		digging(); // ����Ű �Է� ���ο� ���� ���ı� ���� ����

		currentTime = glfwGetTime();
		deltaTime = (float)(currentTime - lastFrameTime);
		lastFrameTime = currentTime;
	
		// ===== model1 - ��ü =====
		glm::mat4 TranslationMatrix1 = translate(mat4(), gPosition1);
		glm::mat4 RotationMatrix1 = eulerAngleYXZ(0.0f, 0.0f, 0.0f);
		glm::mat4 ScalingMatrix1 = scale(mat4(), vec3(0.7f, 0.5f, 1.0f));
		
		glm::mat4 Model1 = glm::mat4(1.0f);

		Model1 = Model1 * RotationMatrix1 * TranslationMatrix1 * ScalingMatrix1;
		// Our ModelViewProjection : multiplication of our 3 matrices
		glm::mat4 MVP1 = Projection * View * Model1; // Remember, matrix multiplication is the other way around
		
		// ===== model12 - ��ü2(������) =====
		// ù��° ��ü ť���� Translation�� ���ӵǰ� ����
		glm::mat4 TranslationMatrix12 = translate(mat4(), gPosition12 + gPosition1);
		glm::mat4 RotationMatrix12 = eulerAngleYXZ(0.0f, 0.0f, 0.0f);
		glm::mat4 ScalingMatrix12 = scale(mat4(), vec3(0.41f, 0.7f, 0.51f));//39 100 49

		glm::mat4 Model12 = glm::mat4(1.0f);

		Model12 = Model12 * RotationMatrix12 * TranslationMatrix12 * ScalingMatrix12;
		// Our ModelViewProjection : multiplication of our 3 matrices
		glm::mat4 MVP12 = Projection * View * Model12; 

		// ===== model13 - ��ü3(�� â��) =====
		glm::mat4 TranslationMatrix13 = translate(mat4(), gPosition13 + gPosition1);
		glm::mat4 RotationMatrix13 = eulerAngleYXZ(0.0f, 0.0f, 0.0f);
		glm::mat4 ScalingMatrix13 = scale(mat4(), vec3(0.415f, 0.6f, 0.4f));

		glm::mat4 Model13 = glm::mat4(1.0f);

		Model13 = Model13 * RotationMatrix13 * TranslationMatrix13 * ScalingMatrix13;
		// Our ModelViewProjection : multiplication of our 3 matrices
		glm::mat4 MVP13 = Projection * View * Model13;

		// ===== model14 - ��ü4(�յ� â��) =====
		glm::mat4 TranslationMatrix14 = translate(mat4(), gPosition14 + gPosition1);
		glm::mat4 RotationMatrix14 = eulerAngleYXZ(0.0f, 0.0f, 0.0f);
		glm::mat4 ScalingMatrix14 = scale(mat4(), vec3(0.3f, 0.6f, 0.515f));

		glm::mat4 Model14 = glm::mat4(1.0f);

		Model14 = Model14 * RotationMatrix14 * TranslationMatrix14 * ScalingMatrix14;
		// Our ModelViewProjection : multiplication of our 3 matrices
		glm::mat4 MVP14 = Projection * View * Model14;

		// ===== model2 - �ڵ�1 =====
		glm::mat4 TranslationMatrix2 = translate(mat4(), gPosition2 + gPosition1);
		// ��ü ť���� rotation�� ���ӵǰ� ����
		glm::mat4 RotationMatrix2 =  eulerAngleYXZ(0.0f, 0.5f + gOrientation, 0.0f);
		glm::mat4 ScalingMatrix2 = scale(mat4(), vec3(0.1f, 0.9f, 0.2f));

		glm::mat4 Model2 = glm::mat4(1.0f);
		// M = R * T * S
		Model2 = Model2 * RotationMatrix2 * TranslationMatrix2 * ScalingMatrix2;

		glm::mat4 MVP2 = Projection * View * Model2;

		// ===== model3 - �ڵ�2 ======
		glm::mat4 TranslationMatrix3 = translate(mat4(), gPosition3 + gPosition2);
		glm::mat4 RotationMatrix3 = eulerAngleYXZ(0.0f, gOrientation, 0.0f);
		glm::mat4 ScalingMatrix3 = scale(mat4(), vec3(0.1f, 0.2f, 0.85f));

		glm::mat4 Model3 = glm::mat4(1.0f);
		Model3 = Model3 * RotationMatrix3 * TranslationMatrix3 * ScalingMatrix3;

		glm::mat4 MVP3 = Projection * View * Model3;

		// ====== model4 - �ڵ�3 ======
		glm::mat4 TranslationMatrix4 = translate(mat4(), gPosition4 + gPosition3);
		glm::mat4 RotationMatrix4 = eulerAngleYXZ(0.0f, gOrientation, 0.0f);
		glm::mat4 ScalingMatrix4 = scale(mat4(), vec3(0.1f, 0.9f, 0.2f));

		glm::mat4 Model4 = glm::mat4(1.0f);
		Model4 = Model4 * RotationMatrix4 * TranslationMatrix4 * ScalingMatrix4;

		glm::mat4 MVP4 = Projection * View * Model4;

		// ====== model5 - bucket =======
		glm::mat4 TranslationMatrix5 = translate(mat4(), gPosition5 + gPosition4);
		glm::mat4 RotationMatrix5 = eulerAngleYXZ(0.0f, gOrientation, 0.0f);
		glm::mat4 ScalingMatrix5 = scale(mat4(), vec3(0.4f, 0.35f, 0.35f));

		glm::mat4 Model5 = glm::mat4(1.0f);
		Model5 = Model5 * RotationMatrix5 * TranslationMatrix5 * ScalingMatrix5;

		glm::mat4 MVP5 = Projection * View * Model5;

		// ====== model6 - ����1 =======
		glm::mat4 TranslationMatrix6 = translate(mat4(), gPosition6 + gPosition1);
		glm::mat4 RotationMatrix6 = eulerAngleYXZ(0.0f, 0.0f, 0.0f);
		glm::mat4 ScalingMatrix6 = scale(mat4(), vec3(0.2f, 0.3f, 1.0f));

		glm::mat4 Model6 = glm::mat4(1.0f);
		Model6 = Model6 * RotationMatrix6 * TranslationMatrix6 * ScalingMatrix6;

		glm::mat4 MVP6 = Projection * View * Model6;

		// ====== model7 - ����2 =======
		glm::mat4 TranslationMatrix7 = translate(mat4(), gPosition7 + gPosition1);
		glm::mat4 RotationMatrix7 = eulerAngleYXZ(0.0f, 0.0f, 0.0f);
		glm::mat4 ScalingMatrix7 = scale(mat4(), vec3(0.2f, 0.3f, 1.0f));

		glm::mat4 Model7 = glm::mat4(1.0f);
		Model7 = Model7 * RotationMatrix7 * TranslationMatrix7 * ScalingMatrix7;

		glm::mat4 MVP7 = Projection * View * Model7;

		// ===== model8 - �ٴ� =====
		glm::mat4 TranslationMatrix8 = translate(mat4(), gPosition8 - moving);
		glm::mat4 RotationMatrix8 = eulerAngleYXZ(0.0f, 0.0f, 0.0f);
		glm::mat4 ScalingMatrix8 = scale(mat4(), vec3(30.0f, 0.0f, 30.0f));

		glm::mat4 Model8 = glm::mat4(1.0f);

		Model8 = Model8 * RotationMatrix8 * TranslationMatrix8 * ScalingMatrix8;
		// Our ModelViewProjection : multiplication of our 3 matrices
		glm::mat4 MVP8 = Projection * View * Model8; // Remember, matrix multiplication is the other way around


		// ===== model1 - ��ü =====
		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP1[0][0]);

		// Set Color-related Variable
		glUniform1i(ColorCheckID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,		  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void *)0 // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		// ===== model12 - ��ü2(������) =====
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP12[0][0]);

		// Set Color-related Variable
		glUniform1i(ColorCheckID, 5);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,		  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void*)0 // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()); 

		// ===== model13 - ��ü3(�� â��) =====
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP13[0][0]);

		// Set Color-related Variable
		glUniform1i(ColorCheckID, 3);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,		  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void*)0 // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		// ===== model14 - ��ü4(�յ� â��) =====
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP14[0][0]);

		// Set Color-related Variable
		glUniform1i(ColorCheckID, 3);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,		  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void*)0 // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // 12*3 indices starting at 0 -> 12 triangles

		// ===== model2 - �ڵ�1 =====
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);

		// Set Color-related Variable
		glUniform1i(ColorCheckID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,		  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void *)0 // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // 12*3 indices starting at 0 -> 12 triangles

		// ======= model3 - �ڵ�2 ====
		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP3[0][0]);

		// Set Color-related Variable
		glUniform1i(ColorCheckID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,		  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void*)0 // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		// ======= model4 - �ڵ�3 ===
		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP4[0][0]);

		// Set Color-related Variable
		glUniform1i(ColorCheckID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,		  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void*)0 // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		// ======= model5 - bucket ===
		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP5[0][0]);

		// Set Color-related Variable
		glUniform1i(ColorCheckID, 5);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,		  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void*)0 // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()); 

		// ======= model6 - ����1 ===
		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP6[0][0]);

		// Set Color-related Variable
		glUniform1i(ColorCheckID, 5);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,		  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void*)0 // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		// ======= model7 - ����2 ===
		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP7[0][0]);

		// Set Color-related Variable
		glUniform1i(ColorCheckID, 5);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,		  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void*)0 // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		// ===== model8 - �ٴ� =====
		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP8[0][0]);

		// �ؽ���
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(ColorCheckID, 11);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,		  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,		  // size
			GL_FLOAT, // type
			GL_FALSE, // normalized?
			0,		  // stride
			(void*)0 // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

void computeMouseRotates() {

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position	
	glfwGetCursorPos(window, &xpos, &ypos);

	// Compute new orientation
	if (xpos < xpos_prev)
		horizontalAngle = -deltaTime * mouseSpeed;
	else if (xpos > xpos_prev)
		horizontalAngle = deltaTime * mouseSpeed;
	else
		horizontalAngle = 0.0;

	if (ypos < ypos_prev)
		verticalAngle = -deltaTime * mouseSpeed;
	else if (ypos > ypos_prev)
		verticalAngle = deltaTime * mouseSpeed;
	else
		verticalAngle = 0.0;

	View *= glm::eulerAngleYXZ(horizontalAngle, verticalAngle, 0.0f);

	xpos_prev = xpos;
	ypos_prev = ypos;

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}

void computeKeyboardTranslates()
{
	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Direction : Spherical coordinates to Cartesian coordinates conversion	
	glm::vec3 right = glm::vec3(-1.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 rotation = glm::vec3(3.14159f / 2.0f, 0.0f, 0.0f);

	glm::vec3 translateFactor = glm::vec3(0.0f);

	// Move forward
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		translateFactor += forward * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		translateFactor -= forward * deltaTime * speed;
	}
	// Move right
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		translateFactor += right * deltaTime * speed;
	}
	// Move left
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		translateFactor -= right * deltaTime * speed;
	}

	moving += translateFactor;

	// ��ü ���������� View ����
	if (!isFP) {
		View *= glm::translate(glm::mat4(1.0f), translateFactor);
	}

	// ���� ����(�����̽��� ������ ��)
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && currentTime - lastSpaceTime > 0.5f) {
		// 3��Ī �������� ����
		if (isFP) {
			isFP = 0; // 1��Ī ���� ���� flag ����
			View = lastTPView; // �ֱ� ��ü���� ī�޶� ��ǥ�� ����
		}
		// 1��Ī �������� ����. ���ı� ���� ���� �߿��� ���� ���� �Ұ�
		else if (!isFP && !isDigging) {
			lastTPView = View; // �ֱ� ��ü���� ī�޶� ��ǥ ������Ʈ

			isFP = 1;
			View = glm::lookAt(
				glm::vec3(0.8f, 1.7f, -0.5f), 
				glm::vec3(1.0f, 1.0f, 1.0f),
				glm::vec3(0.0f, 1.0f, 1.0f)	 
			);
			View *= glm::translate(glm::mat4(1.0f), glm::vec3(0.7f, 0.2f, -1.5f));
			View *= glm::eulerAngleYXZ(0.0f, 0.0f, 0.08f);

		}
		lastSpaceTime = currentTime; // �ֱٿ� �����̽��� ���� �ð� ������Ʈ
	}

	// ���� ��������: ���ı� ����
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !isDigging) {
		isDigging = 1;
		lastDiggingTime = currentTime;
	}
	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}

// ���ı� ���� ���� �Լ�
void digging() {
	// isDigging ���� 1�̸� ���ı� ���� ����
	if (isDigging) {
		//��谪�� �Ѿ�� flag����
		if (gOrientation > 0.5f)
			flag = 1;
		else if (gOrientation < -0.5f)
			flag = 0;

		// �߽ɿ� �������� �ӵ��� ���� (���� ��� �������� ��)
		float closeToCenter = -(abs(gOrientation) - 1.3f);

		// ��谪�� �Ѿ�� +, ��谪���� �۾����� -
		if (flag == 1)
			gOrientation -= 3.14159f / 2.0f * deltaTime * closeToCenter;
		else
			gOrientation += 3.14159f / 2.0f * deltaTime * closeToCenter;
	}

	// ���� �ð��� ������ ���ı� ������ ��ħ
	if (currentTime - lastDiggingTime > 1.25f) {
		gOrientation = 0.00f;
		isDigging = 0;
	}
}