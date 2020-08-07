#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <fstream>

#include "xTools.h"
#include "1-LoadModel/LoadModeByCustomLoader.h"
#include "2-SkinMeshCPU/LoadMD5Model.h"
#include "2-SkinMeshCPU/LoadMD5Model_GPU.h"


using namespace xTools;
using namespace std;

int Test();

int main()
{
	
	//return LoadModelByCostomLoaderMain();

	//return LoadMD5ModelMain();

	return LoadMD5ModelGPUMain();

	//return Test();
}







// *********************************************************************
int Test()
{

	char data[100];

	// 以写模式打开文件
	ofstream outfile;
	outfile.open("afile.dat");

	cout << "Write to the file" << endl;
	cout << "Enter your name: ";
	// 读取整行，包括前导和嵌入的空格，并将其存储在字符串对象中
	// 在读取指定数目的字符或遇到换行符时停止读取
	cin.getline(data, 100);

	// 向文件写入用户输入的数据
	outfile << data << endl;

	cout << "Enter your age: ";
	cin >> data;
	// 删除缓冲区中数据，对缓冲区中的删除数据控制的较精确
	// 忽略掉之前读语句留下的多余字符
	cin.ignore(2);	// 这里就是清除换行符

	// 再次向文件中写入用户输入的数据
	outfile << data << endl;

	// 关闭打开的文件
	outfile.close();


	//******************************************************


	// 以读模式打开文件
	ifstream infile;
	infile.open("afile.dat");

	cout << "Reading from the file" << endl;
	infile >> data;

	// 在屏幕上写入数据
	cout << data << endl;
	
	// 再次从文件读取数据，并显示它
	infile >> data;
	cout << data << endl;

	// 关闭打开的文件
	infile.close();


	system("pause");

	return 0;
}