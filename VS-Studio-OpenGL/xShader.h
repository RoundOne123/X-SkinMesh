#ifndef X_SHADER_H_
#define X_SHADER_H_

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

class xShader
{
public:
	unsigned int ID;
	
	// 从文件读取并构建着色器
	xShader(const char *vertexPath, const char *fragmentPath)
	{
		//std::cout << "vertexPath : " << vertexPath << std::endl;
		//std::cout << "fragmentPath : " << fragmentPath << std::endl;

		// 1、从文件路径中获取顶点/片段着色器
		string vertexCode;
		string fragmentCode;
		ifstream vShaderFile;
		ifstream fShaderFile;

		// 保证ifstream对象可以抛出异常
		vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
		fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);

		try
		{
			// 打开文件
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			stringstream vShaderStream, fShaderStream;
			// 读取文件的缓冲内容到数据流中
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// 关闭文件处理器
			vShaderFile.close();
			fShaderFile.close();
			// 转换数据流到string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}

		const char *vShaderCode = vertexCode.c_str();
		const char *fShaderCode = fragmentCode.c_str();

		// 2、编译着色器
		unsigned int vertex, fragment;
		int success;
		char infoLog[512];

		// 顶点着色器
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);

		// 打印编译错误 如果有的话
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				<< infoLog << std::endl;
		}

		// 片段着色器
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);

		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				<< infoLog << std::endl;
		}

		// 着色器程序
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);

		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(ID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
				<< infoLog << std::endl;
		}

		// 删除着色器
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	void use()
	{
		glUseProgram(ID);
	}


	// *****************【设置shader中uniform的工具方法】*************
	
	void setBool(const string &name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}

	void setInt(const string &name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}

	void setFloat(const string &name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}

	void setMat4(const string &name, const glm::mat4 &mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	void setMat4List(const string &name, unsigned int size, const glm::mat4 &mat) const
	{

		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), size, GL_FALSE, &mat[0][0]);
	}

	void setVec3(const string &name, const glm::vec3 &value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}

	void setVec3(const string &name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}

};

#endif // !X_SHADER_H_
