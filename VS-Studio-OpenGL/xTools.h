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

	/// ��ȡ��ǰ�йؼ���
	static string FirstToken(const string &content) {
		// &content ���൱�� �� ����Ĳ������˸�������
		// �൱���ڱ�������ֱ�Ӳ����� ����Ĳ���

		if (!content.empty())
		{
			// ��ȡconten�У���һ������ " \t" ƥ����ַ� ������index
			// ���û�ҵ��ͷ���string::npos
			size_t token_start = content.find_first_not_of(" \t");
			// ��ĳ��index��ʼ���ҵ�һ��  �ҵ����� index
			size_t token_end = content.find_first_of(" \t", token_start);

			// �ҵ��˵�һ�� ǰ�� ���� " \t" �������ַ���
			if (token_start != string::npos && token_end != string::npos) 
			{
				// ��ȡָ�����ַ���
				string str = content.substr(token_start, token_end - token_start);
				return str;
			}
			else if(token_start != string::npos)	// ���ֻ�ҵ���  ���� " \t" ��ͷ
			{
				// ����token_start��ʼ������content����
				return content.substr(token_start);
			}
		}
		return "";
	}

	/// ��ȡ��ǰ�йؼ��ֺ��������
	static string TailToken(const string &content)
	{
		// v  -4.1028 57.3829 -2.0208 
		// ��ȡ��  -4.1028 57.3829 -2.0208
		size_t token_start = content.find_first_not_of(" \t");
		size_t space_start = content.find_first_of(" \t", token_start);	// �ؼ��� ����� " \t" ��index
		// ������ҵ� ȥ���ؼ���֮��� 
		size_t tail_start = content.find_first_not_of(" \t", space_start);
		size_t tail_end = content.find_last_not_of(" \t");	// ����ʹ�� xx_last_xx ���º������Ҫ + 1

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

	// ���ַ������Ϊ������ǵ��ַ�������
	static void Split(const string &content, vector<string> &outContent, string token)
	{
		outContent.clear();

		string temp;

		// ����index  ����index�� ��ȥ ��token���ȵ��ַ�
		for (int i = 0; i < int(content.size()); i++)
		{
			string test = content.substr(i, token.size());

			if (test == token)	// �ҵ�token
			{
				if (!temp.empty())
				{
					outContent.push_back(temp);	// �����õ�����push������
					temp.clear();
					i += (int)token.size() - 1;	// index ֱ������ token�ĳ���
				}
				else
				{
					outContent.push_back("");	// �����ǰ�ַ��������ᱻִ��
				}
			}
			else if(i + token.size() >= content.size())	// ������ content ��󳤶�
			{
				temp += content.substr(i, token.size()); // �����һЩ�յ��ַ�����
				outContent.push_back(temp);
				break;
			}
			else
			{
				temp += content[i];	// �ۼ��Ի����Ҫ���ַ���
			}
		}
	}

	// ��ȡָ���±��µ�Ԫ��
	template <class T>
	static const T & GetElement(const vector<T> &elements, string &index)
	{
		int idx = stoi(index);
		if (idx < 0)	// ���Ϊ��
			idx = int(elements.size()) + idx;
		else
			idx--;	// Ҫ��0��ʼ���� --
		return elements[idx];
	}

	// ��ȡ�ļ�����
	static int GetFileLength(ifstream &file)
	{
		/*
		��ȡ�ļ���ʼ��ȡλ�ã����ں����Ļָ���
		->
		���ļ���ȡλ�����õ��ļ������
		->
		������ļ��ĳ���
		->
		���ļ���ȡλ�����õ���ʼλ��
		*/

		// file.tellg �� file.tellp 
		// ���ǵ�Ŀ���ǽ��ļ���дλ�õĵ�ǰ���ֽڱ�š���Ϊһ�� long �����������ء�
		// tellp ���ڷ���д��λ�ã�tellg �����ڷ��ض�ȡλ�á�
		int oriPos = file.tellg();		// ��ȡ��ʼ��ȡλ��
		// �������ļ�����ȡ���Ķ�λ������1ƫ����������2�ǻ���ַ
		file.seekg(0, ios::end);		// ���ö�ȡλ�õ��ļ����յ�
		int length = file.tellg();		// ��ȡ�ļ�����
		file.seekg(oriPos);				// ��ԭ��ʼ��ȡλ��
		return length;
	}

	// ����"\n"ǰ���100���ַ�
	static void IgnoreLine(ifstream &mfile, int length)
	{
		// ����һ��������ĳ���ַ���
		// ����1��Ҫ����������ַ���������2��һ���ַ�����������ʱ��ͻ�ֹͣ
		mfile.ignore(length, '\n');
	}

	// ɾ����һ�������һ���ַ�
	static void RemoveNotes(string &str)
	{
		// erase ɾ�� �Ӳ���1��ʼ�ģ�����2���ַ�
		str.erase(0, 1);				// ɾ����һ���ַ�
		str.erase(str.size() - 1, 1);	// ɾ�����һ���ַ���
	}

	// ������Ԫ���ĵ�W����  w Ϊ�� ���� 0
	static void ComputeQuatW(glm::quat &q)
	{
		float t = 1 - q.x * q.x - q.y * q.y - q.z * q.z;
		q.w = t < 0 ? 0 : -sqrtf(t);	// ΪʲôҪȡ�������أ�
	}

	// �����ļ�·�� �� �ļ��� ���� ���յ�·��
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