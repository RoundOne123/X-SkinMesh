#include "../xShader.h"
#include "../model_obj.h"
#include "../stb_image.h"
#include "../camera.h"
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using namespace std;
using namespace xObjLoader;

// 设置
static unsigned int SCR_WIDTH = 800;
static unsigned int SCR_HEIGHT = 600;

// 函数声明
static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
// 监听光标位置变化
static void mouse_callback(GLFWwindow *window, double xpos, double ypos);
// 监听鼠标滚轮变化
static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
static void processInput(GLFWwindow *window);


// 声明变量
static glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
static float deltaTime = 0.0f;
static float lastFrameTime = 0.0f;
static float lastX = 400, lastY = 300; // 初始值为屏幕中心
static bool firstMouse = true;         // 处理 刚进入窗口时的闪动

// 摄像机
static Camera camera(cameraPos);

int LoadModelByCostomLoaderMain()
{
	// 实例化GLFW窗口
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// 创建一个窗口
	GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Load A Model", NULL, NULL);
	if (window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// 调用任何OpenGL函数之前初始化GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize GLAD" << endl;
		return -1;
	}

	// 视口 （注意和窗口的区别）
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	// 监听窗口变化
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	// 监听光标位置变化
	glfwSetCursorPosCallback(window, mouse_callback);
	// 监听鼠标滚轮变化
	glfwSetScrollCallback(window, scroll_callback);

	glEnable(GL_DEPTH_TEST);
	// 隐藏并捕捉光标
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	// 处理着色器
	xShader ourShader("C:/Users/admin/Desktop/opengl-skinmeshrender/VS-Studio-OpenGL/VS-Studio-OpenGL/1-LoadModel/shader.vs", "C:/Users/admin/Desktop/opengl-skinmeshrender/VS-Studio-OpenGL/VS-Studio-OpenGL/1-LoadModel/shader.fs");

	// 加载模型 + 材质贴图
	M_Model model("C:/Users/admin/Desktop/opengl-skinmeshrender/VS-Studio-OpenGL/VS-Studio-OpenGL/1-LoadModel/model/游戏人物_02.obj");

	// 渲染循环
	while (!glfwWindowShouldClose(window))
	{
		// 输入
		processInput(window);

		// 渲染指令
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 计算相机位移
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrameTime;
		lastFrameTime = currentFrame;

		// 准备变换矩阵
		glm::mat4 modelMat = glm::mat4(1.0f);
		modelMat = glm::translate(modelMat, glm::vec3(0.0f, -1.5f, 0.0f));
		modelMat = glm::scale(modelMat, glm::vec3(0.03f));

		glm::mat4 view;
		view = camera.GetViewMatrix();

		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);


		// 激活着色器程序
		ourShader.use();
		ourShader.setMat4("model", modelMat);
		ourShader.setMat4("view", view);
		ourShader.setMat4("projection", projection);
		
		// 绘制模型
		model.Draw(ourShader);

		// 交换颜色缓冲
		glfwSwapBuffers(window);
		// 检查触发事件
		glfwPollEvents();
	}

	glDeleteProgram(ourShader.ID);

	// 释放所有GLFW资源
	glfwTerminate();

	return 0;
}





// 输入
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
		// 讲道理 这里应该使用 glm::cross(cameraUp, -cameraFront) 求 右轴
		// 这里 直接这样结果也是对的但是 不好理解
		//cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		//cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraFront, cameraUp));
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
}

// 窗口大小调整时的回调
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// 光标移动时的回调
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // 注意这里是相反的，因为y坐标是从底部往顶部依次增大的  不能理解？？？
	lastX = xpos;
	lastY = ypos;
	camera.ProcessMouseMovement(xoffset, yoffset);
}

// 监听鼠标滚轮变化
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}