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

glm::mat4 lastFPView;
glm::mat4 lastTPView;

// Initial horizontal angle : toward -Z
float horizontalAngle = 0.0f;
// Initial vertical angle : none
float verticalAngle = 0.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 3.0f;

bool firstPress = true;
double xpos_prev = 0.0;
double ypos_prev = 0.0;
double xpos = 0.0;
double ypos = 0.0;

float gOrientation = 0.00f;
double lastTime = 0.0;
double lastSpaceTime = 0.0;
int isFP = 0;
double lastDiggingTime = 0.0;
int isDigging = 0;

void computeMouseRotates(); // 마우스 입력으로 물체를 회전
void computeKeyboardTranslates(); // 키보드 입력으로 물체를 회전

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
	window = glfwCreateWindow(1024, 768, "Tutorial 04 - Colored Cube", NULL, NULL);
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

	// ===== km =========
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// =============================

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// ==== km? =====
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
	// ===============

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	
	//  ==== 각 도형마다 컬러 다르게 표현하기 위해 사용 =====
	GLuint ColorCheckID = glGetUniformLocation(programID, "colorCheck");

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("cube.obj", vertices, uvs, normals);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	// Projection matrix : 45?Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	// y축 위에서 xz평면을 바라본다.
	View = glm::lookAt(
		glm::vec3(4, 3, -3), // Camera is at (4,3,-3), in World Space
		glm::vec3(0.0f, 0.0f, 0.0f),	// and looks at the origin
		glm::vec3(0.0f, 1.0f, 0.0f)	 // Head is up (set to 0,-1,0 to look upside-down)
	);
	ModelMatrix = glm::mat4(1.0);
	lastTPView = View;
	lastFPView = glm::lookAt(
		glm::vec3(0.8f, 1.7f, -0.5f), 
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 1.0f)
	);

	// For speed computation
	double lastTime = glfwGetTime();
	double lastFrameTime = lastTime;

	//x,z평면에 위치 시킨 후 동작		
	// 몸통
	glm::vec3 gPosition1(0.0f, 0.0f, 0.0f);

	// 핸들
	// 첫번째 큐브의 말단에 존재하도록 위치 조정
	glm::vec3 gPosition2(0.0f, 1.0f, 0.5f);
	glm::vec3 gPosition3(0.0f, 0.7f, 0.8f);
	glm::vec3 gPosition4(0.0f, -0.3f, 1.5f);

	// bucket
	glm::vec3 gPosition5(0.0f, 0.4f, 0.9f);

	// 바퀴1
	glm::vec3 gPosition6(0.6f, -0.8f, 0.0f);

	// 바퀴2
	glm::vec3 gPosition7(-0.6f, -0.8f, 0.0f);

	// 회전 방향을 결정하는 flag
	int flag = 0;
	do
	{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);
		// ===== km
		// Compute the Model matrix from keyboard and mouse input
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			if (firstPress)
			{
				glfwGetCursorPos(window, &xpos_prev, &ypos_prev);
				firstPress = false;
			}

			computeMouseRotates();
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
			firstPress = true;

		computeKeyboardTranslates();
		// ====================

		double currentTime = glfwGetTime();
		float deltaTime = (float)(currentTime - lastFrameTime);
		lastFrameTime = currentTime;

		// 엔터를 누르면 땅파기 동작 실행(전체 시점일때만)
		if (isDigging) {
			//경계값을 넘어가면 flag동작
			if (gOrientation > 0.5f)
				flag = 1;
			else if (gOrientation < -0.5f)
				flag = 0;

			// 중심에 가까울수록 속도가 증가 (진자 운동과 가깝도록 함)
			float closeToCenter = -(abs(gOrientation) - 1.3f);

			//1.2f를 넘어가면 +, -1.2f보다 작아지면 -
			if (flag == 1)
				gOrientation -= 3.14159f / 2.0f * deltaTime * closeToCenter;
			else
				gOrientation += 3.14159f / 2.0f * deltaTime * closeToCenter;
		}

		if (currentTime - lastDiggingTime > 1.25f) {
			isDigging = 0;
		}
		
		// ===== model1 - 몸체 =====
		glm::mat4 TranslationMatrix1 = translate(mat4(), gPosition1);
		glm::mat4 RotationMatrix1 = eulerAngleYXZ(0.0f, 0.0f, 0.0f);
		// 기둥형태를 만들기 위해 scale 조작
		glm::mat4 ScalingMatrix1 = scale(mat4(), vec3(0.7f, 0.5f, 1.0f));
		
		glm::mat4 Model1 = glm::mat4(1.0f);

		Model1 = Model1 * RotationMatrix1 * TranslationMatrix1 * ScalingMatrix1;
		// Our ModelViewProjection : multiplication of our 3 matrices
		glm::mat4 MVP1 = Projection * View * Model1; // Remember, matrix multiplication is the other way around
		// 진자 운동 구현 관련 중요한 부분!!!
		
		// ===== model2 - 핸들1 =====
		// 첫번째 큐브의 Translation + 두번째 큐브의 Translation (첫번째 큐브의 Translation에 종속되게 설정)
		glm::mat4 TranslationMatrix2 = translate(mat4(), gPosition2);
		// 첫번째 큐브의 rotation + 두번째 큐브의 rotation (첫번째 큐브의 rotation에 종속되게 설정)
		glm::mat4 RotationMatrix2 = eulerAngleYXZ(0.0f, gOrientation, 0.0f);
		glm::mat4 ScalingMatrix2 = scale(mat4(), vec3(0.1f, 0.8f, 0.1f));

		glm::mat4 Model2 = glm::mat4(1.0f);
		// M = R * T * S
		Model2 = Model2 * RotationMatrix2 * TranslationMatrix2 * ScalingMatrix2;

		glm::mat4 MVP2 = Projection * View * Model2;

		// ===== model3 - 핸들2 ======
		glm::mat4 TranslationMatrix3 = translate(mat4(), gPosition3 + gPosition2);
		glm::mat4 RotationMatrix3 = eulerAngleYXZ(0.0f, gOrientation, 0.0f);
		glm::mat4 ScalingMatrix3 = scale(mat4(), vec3(0.1f, 0.1f, 0.7f));

		glm::mat4 Model3 = glm::mat4(1.0f);
		Model3 = Model3 * RotationMatrix3 * TranslationMatrix3 * ScalingMatrix3;

		glm::mat4 MVP3 = Projection * View * Model3;

		// ====== model4 - 핸들3 ======
		glm::mat4 TranslationMatrix4 = translate(mat4(), gPosition4 + gPosition3);
		glm::mat4 RotationMatrix4 = eulerAngleYXZ(0.0f, -0.3f + gOrientation, 0.0f);
		glm::mat4 ScalingMatrix4 = scale(mat4(), vec3(0.1f, 0.8f, 0.1f));

		glm::mat4 Model4 = glm::mat4(1.0f);
		Model4 = Model4 * RotationMatrix4 * TranslationMatrix4 * ScalingMatrix4;

		glm::mat4 MVP4 = Projection * View * Model4;

		// ====== model5 - bucket =======
		glm::mat4 TranslationMatrix5 = translate(mat4(), gPosition5 + gPosition4);
		glm::mat4 RotationMatrix5 = eulerAngleYXZ(0.0f, gOrientation, 0.0f);
		glm::mat4 ScalingMatrix5 = scale(mat4(), vec3(0.3f, 0.3f, 0.3f));

		glm::mat4 Model5 = glm::mat4(1.0f);
		Model5 = Model5 * RotationMatrix5 * TranslationMatrix5 * ScalingMatrix5;

		glm::mat4 MVP5 = Projection * View * Model5;

		// ====== model6 - 바퀴1 =======
		glm::mat4 TranslationMatrix6 = translate(mat4(), gPosition6 + gPosition1);
		glm::mat4 RotationMatrix6 = eulerAngleYXZ(0.0f, 0.0f, 0.0f);
		glm::mat4 ScalingMatrix6 = scale(mat4(), vec3(0.2f, 0.3f, 1.0f));

		glm::mat4 Model6 = glm::mat4(1.0f);
		Model6 = Model6 * RotationMatrix6 * TranslationMatrix6 * ScalingMatrix6;

		glm::mat4 MVP6 = Projection * View * Model6;

		// ====== model7 - 바퀴2 =======
		glm::mat4 TranslationMatrix7 = translate(mat4(), gPosition7 + gPosition1);
		glm::mat4 RotationMatrix7 = eulerAngleYXZ(0.0f, 0.0f, 0.0f);
		glm::mat4 ScalingMatrix7 = scale(mat4(), vec3(0.2f, 0.3f, 1.0f));

		glm::mat4 Model7 = glm::mat4(1.0f);
		Model7 = Model7 * RotationMatrix7 * TranslationMatrix7 * ScalingMatrix7;

		glm::mat4 MVP7 = Projection * View * Model7;

		// ===== model1 - 몸체 =====
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
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // 12*3 indices starting at 0 -> 12 triangles

		// ===== model2 - 핸들1 =====
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

		// ======= model3 - 핸들2 ====
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
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // 12*3 indices starting at 0 -> 12 triangles
		//=======================

		// ======= model4 - 핸들3 ===
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
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // 12*3 indices starting at 0 -> 12 triangles
		//=======================

		// ======= model5 - bucket ===
		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP5[0][0]);

		// Set Color-related Variable
		glUniform1i(ColorCheckID, 1);

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
		//=======================

		// ======= model6 - 바퀴1 ===
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
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // 12*3 indices starting at 0 -> 12 triangles
		//=======================

		// ======= model7 - 바퀴2 ===
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
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // 12*3 indices starting at 0 -> 12 triangles
		//=======================


		glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
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

	// 1인칭 시점일 때만 카메라 이동 가능
	if (!isFP) {
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
	}

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

	glm::vec3 translateFactor = glm::vec3(0.0f);

	// 3인칭 시점에만 움직인다.
	if(!isFP && -1) {
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
		// Move up
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			translateFactor += up * deltaTime * speed;
		}
		// Move down
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			translateFactor -= up * deltaTime * speed;
		}

		View *= glm::translate(glm::mat4(1.0f), translateFactor);

		if (isFP) {
			lastFPView = View;
		}
		else {
			lastTPView = View;
		}
	}

	if (!isFP) {
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
		// Move up
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			translateFactor += up * deltaTime * speed;
		}
		// Move down
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			translateFactor -= up * deltaTime * speed;
		}


	}

	// 시점 변경
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && currentTime - lastSpaceTime > 0.5) {
		// 3인칭 시점으로 변경
		if (isFP) {
			isFP = 0;
			lastFPView = View;

			View = lastTPView;
			//View = glm::lookAt(
			//	glm::vec3(4, 5, -5), // Camera is at (4,3,-3), in World Space
			//	glm::vec3(2.0f, -1.0f, 1.0f),	// and looks at the origin
			//	glm::vec3(0.0f, 2.0f, 1.0f)	 // Head is up (set to 0,-1,0 to look upside-down)
			//);
		}
		// 1인칭 시점으로 변경
		else if(!isFP && !isDigging){
			isFP = 1;
			lastTPView = View;

			View = lastFPView;
			//View = glm::lookAt(
			//	glm::vec3(0.8f, 1.7f, -0.5f), // Camera is at (4,3,-3), in World Space
			//	glm::vec3(1.0f, 1.0f, 1.0f),	// and looks at the origin
			//	glm::vec3(0.0f, 1.0f, 1.0f)	 // Head is up (set to 0,-1,0 to look upside-down)
			//);

			//View *= glm::translate(glm::mat4(1.0f), translateFactor + right * 0.5f * speed);
		}
		lastSpaceTime = currentTime;
	}

	// 엔터 눌렀을때: 땅파기 시작
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && currentTime - lastDiggingTime > 0.5 && !isDigging && !isFP) {
		isDigging = 1;
		lastDiggingTime = currentTime;
	}

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}
