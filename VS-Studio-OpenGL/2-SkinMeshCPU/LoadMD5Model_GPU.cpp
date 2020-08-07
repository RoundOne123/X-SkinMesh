#include "../xShader.h"
#include "../model_md5mesh_gpu.h"
#include "../stb_image.h"
#include "../camera.h"
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using namespace std;
using namespace xMD5Loader;

// ����
static unsigned int SCR_WIDTH = 800;
static unsigned int SCR_HEIGHT = 600;

// ��������
static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
// �������λ�ñ仯
static void mouse_callback(GLFWwindow *window, double xpos, double ypos);
// ���������ֱ仯
static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
static void processInput(GLFWwindow *window);

// ��������
static glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
static float deltaTime = 0.0f;
static float lastFrameTime = 0.0f;
static float lastX = 400, lastY = 300; // ��ʼֵΪ��Ļ����
static bool firstMouse = true;         // ���� �ս��봰��ʱ������

// �����
static Camera camera(cameraPos);

int LoadMD5ModelGPUMain()
{
	// ��ʼ�� +���ص������ ***********************************************************************

		// ʵ����GLFW����
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// ����һ������
	GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Load MD5 Model - GPU", NULL, NULL);
	if (window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// �����κ�OpenGL����֮ǰ��ʼ��GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
		return -1;
	}

	// �ӿ� ��ע��ʹ��ڵ�����
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	// �������ڱ仯
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	// �������λ�ñ仯
	glfwSetCursorPosCallback(window, mouse_callback);
	// ���������ֱ仯
	glfwSetScrollCallback(window, scroll_callback);


	// ������� ******************************************************************************************

	glEnable(GL_DEPTH_TEST);
	// ���ز���׽���
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



	// ��ɫ����ģ�͡���ͼ *********************************************************************************

	xShader ourShader("C:/Users/admin/Desktop/opengl-skinmeshrender/VS-Studio-OpenGL/VS-Studio-OpenGL/2-SkinMeshCPU/shader_gpu.vs", "C:/Users/admin/Desktop/opengl-skinmeshrender/VS-Studio-OpenGL/VS-Studio-OpenGL/2-SkinMeshCPU/shader_gpu.fs");

	MD5_Model_GPU ourModel;
	string objPath = "C:/Users/admin/Desktop/opengl-skinmeshrender/VS-Studio-OpenGL/VS-Studio-OpenGL/2-SkinMeshCPU/model/Bob/boblampclean.md5mesh";
	ourModel.LoadModel(objPath);
	// ����
	string animPath = "C:/Users/admin/Desktop/opengl-skinmeshrender/VS-Studio-OpenGL/VS-Studio-OpenGL/2-SkinMeshCPU/model/Bob/boblampclean.md5anim";
	ourModel.animation.LoadAnimation(animPath);


	// ��Ⱦѭ��
	while (!glfwWindowShouldClose(window))
	{
		// ����
		processInput(window);

		// ��Ⱦָ��
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// �������λ��
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrameTime;
		lastFrameTime = currentFrame;

		// ׼���任����
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, -3.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
		model = glm::scale(model, glm::vec3(0.05f));

		glm::mat4 view;
		view = camera.GetViewMatrix();

		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		// ������ɫ������
		ourShader.use();
		ourShader.setMat4("model", model);
		ourShader.setMat4("view", view);
		ourShader.setMat4("projection", projection);

		// ���㶥��
		ourModel.Update(deltaTime);
		ourModel.Render(ourShader);

		// ������ɫ����
		glfwSwapBuffers(window);
		// ��鴥���¼�
		glfwPollEvents();
	}

	glDeleteProgram(ourShader.ID);

	// �ͷ�����GLFW��Դ
	glfwTerminate();

	return 0;
}




// ����
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = 2.5f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		//cameraPos += cameraSpeed * cameraFront;
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		//cameraPos -= cameraSpeed * cameraFront;
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		// ������ ����Ӧ��ʹ�� glm::cross(cameraUp, -cameraFront) �� ����
		// ���� ֱ���������Ҳ�ǶԵĵ��� �������
		//cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		//cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
}

// ���ڴ�С����ʱ�Ļص�
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// ����ƶ�ʱ�Ļص�
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // ע���������෴�ģ���Ϊy�����Ǵӵײ����������������  ������⣿����
	lastX = xpos;
	lastY = ypos;
	camera.ProcessMouseMovement(xoffset, yoffset);
}

// ���������ֱ仯
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}