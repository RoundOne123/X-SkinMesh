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

	// ��дģʽ���ļ�
	ofstream outfile;
	outfile.open("afile.dat");

	cout << "Write to the file" << endl;
	cout << "Enter your name: ";
	// ��ȡ���У�����ǰ����Ƕ��Ŀո񣬲�����洢���ַ���������
	// �ڶ�ȡָ����Ŀ���ַ����������з�ʱֹͣ��ȡ
	cin.getline(data, 100);

	// ���ļ�д���û����������
	outfile << data << endl;

	cout << "Enter your age: ";
	cin >> data;
	// ɾ�������������ݣ��Ի������е�ɾ�����ݿ��ƵĽϾ�ȷ
	// ���Ե�֮ǰ��������µĶ����ַ�
	cin.ignore(2);	// �������������з�

	// �ٴ����ļ���д���û����������
	outfile << data << endl;

	// �رմ򿪵��ļ�
	outfile.close();


	//******************************************************


	// �Զ�ģʽ���ļ�
	ifstream infile;
	infile.open("afile.dat");

	cout << "Reading from the file" << endl;
	infile >> data;

	// ����Ļ��д������
	cout << data << endl;
	
	// �ٴδ��ļ���ȡ���ݣ�����ʾ��
	infile >> data;
	cout << data << endl;

	// �رմ򿪵��ļ�
	infile.close();


	system("pause");

	return 0;
}