#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

namespace xTools
{

	/// 获取当前行关键字
	static string FirstToken(const string &content) {
		// &content 就相当于 给 传入的参数起了个别名，
		// 相当于在本函数中直接操作了 传入的参数

		if (!content.empty())
		{
			// 获取conten中，第一个不与 " \t" 匹配的字符 并返回index
			// 如果没找到就返回string::npos
			size_t token_start = content.find_first_not_of(" \t");
			// 从某个index开始查找第一个  找到返回 index
			size_t token_end = content.find_first_of(" \t", token_start);

			// 找到了第一个 前后 都被 " \t" 隔开的字符串
			if (token_start != string::npos && token_end != string::npos) 
			{
				// 截取指定的字符串
				string str = content.substr(token_start, token_end - token_start);
				return str;
			}
			else if(token_start != string::npos)	// 如果只找到了  不是 " \t" 的头
			{
				// 返回token_start开始的所有content内容
				return content.substr(token_start);
			}
		}
		return "";
	}

	/// 获取当前行关键字后面的内容
	static string TailToken(const string &content)
	{
		// v  -4.1028 57.3829 -2.0208 
		// 截取：  -4.1028 57.3829 -2.0208
		size_t token_start = content.find_first_not_of(" \t");
		size_t space_start = content.find_first_of(" \t", token_start);	// 关键字 后面的 " \t" 的index
		// 这个是找到 去掉关键字之后的 
		size_t tail_start = content.find_first_not_of(" \t", space_start);
		size_t tail_end = content.find_last_not_of(" \t");	// 这里使用 xx_last_xx 导致后面计算要 + 1

		if (tail_start != string::npos && tail_end != string::npos) 
		{
			string str = content.substr(tail_start, tail_end - tail_start + 1);
			return str;
		}
		else if(tail_start != string::npos)
		{
			return content.substr(tail_start);
		}
		return "";
	}

	// 将字符串拆分为给定标记的字符串数组
	static void Split(const string &content, vector<string> &outContent, string token)
	{
		outContent.clear();

		string temp;

		// 遍历index  并从index处 节去 和token长度的字符
		for (int i = 0; i < int(content.size()); i++)
		{
			string test = content.substr(i, token.size());

			if (test == token)	// 找到token
			{
				if (!temp.empty())
				{
					outContent.push_back(temp);	// 将想获得的数据push进容器
					temp.clear();
					i += (int)token.size() - 1;	// index 直接跳过 token的长度
				}
				else
				{
					outContent.push_back("");	// 这个当前字符串并不会被执行
				}
			}
			else if(i + token.size() >= content.size())	// 超出了 content 最大长度
			{
				temp += content.substr(i, token.size()); // 最后会加一些空的字符串？
				outContent.push_back(temp);
				break;
			}
			else
			{
				temp += content[i];	// 累加以获得想要的字符串
			}
		}
	}

	// 获取指定下标下的元素
	template <class T>
	static const T & GetElement(const vector<T> &elements, string &index)
	{
		int idx = stoi(index);
		if (idx < 0)	// 如果为负
			idx = int(elements.size()) + idx;
		else
			idx--;	// 要从0开始所以 --
		return elements[idx];
	}

	// 获取文件长度
	static int GetFileLength(ifstream &file)
	{
		/*
		获取文件初始读取位置（用于后续的恢复）
		->
		将文件读取位置设置到文件的最后
		->
		计算出文件的长度
		->
		将文件读取位置设置到初始位置
		*/

		// file.tellg 和 file.tellp 
		// 它们的目的是将文件读写位置的当前【字节编号】作为一个 long 类型整数返回。
		// tellp 用于返回写入位置，tellg 则用于返回读取位置。
		int oriPos = file.tellg();		// 获取初始读取位置
		// 对输入文件（读取）的定位，参数1偏移量，参数2是基地址
		file.seekg(0, ios::end);		// 设置读取位置到文件最终点
		int length = file.tellg();		// 获取文件长度
		file.seekg(oriPos);				// 还原初始读取位置
		return length;
	}

	// 跳过"\n"前最大100个字符
	static void IgnoreLine(ifstream &mfile, int length)
	{
		// 跳过一定数量的某个字符，
		// 参数1：要跳过的最大字符数，参数2：一个字符，当遇到的时候就会停止
		mfile.ignore(length, '\n');
	}

	// 删除第一个和最后一个字符
	static void RemoveNotes(string &str)
	{
		// erase 删除 从参数1开始的，参数2个字符
		str.erase(0, 1);				// 删除第一个字符
		str.erase(str.size() - 1, 1);	// 删除最后一个字符？
	}

	// 计算四元数的的W分量  w 为负 或者 0
	static void ComputeQuatW(glm::quat &q)
	{
		float t = 1 - q.x * q.x - q.y * q.y - q.z * q.z;
		q.w = t < 0 ? 0 : -sqrtf(t);	// 为什么要取个负号呢？
	}

	// 根据文件路径 和 文件名 生成 最终的路径
	static string GetSameDirFile(string &oriPath, string &fileName)
	{
		string newPath;
		vector<string> pathArr;
		Split(oriPath, pathArr, "/");

		for (int i = 0; i < pathArr.size() - 1; i++)
		{
			newPath += pathArr[i] + "/";
		}

		newPath += fileName;
		return newPath;

	}

}