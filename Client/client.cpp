#include<winsock2.h>//winsock2��ͷ�ļ�
#include<iostream>
#include <Shlwapi.h>
#include <direct.h>
#include <string>
#include <conio.h>
#include <vector>
using  namespace std;

#pragma comment(lib,"shlwapi.lib")	

//����������dll��lib
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)

DWORD WINAPI ThreadFunRecv(LPVOID lpThreadParameter);

enum type_set {
	zhan_wei,
	online,
	tell,
	tellall,
	sls,
	uploads,
	download,
	relay,
	login
};


struct pdu//֡�ṹ
{
	type_set type;
	int len;
	int fromeid;
	int toid;
	char file_name[1024];
	bool uploadsing;
	bool uploaded;
	bool downloading;
	bool downloaded;
	bool relaying;
	bool relayed;
	bool pwIsTrue=true;
	char data[1024];
	char username[1024];
	char password[1024];
}sendbuf, recvbuf;

vector<string> findFile(string dir);
FILE* fp = NULL;

int  main()
{

	//����winsock2�Ļ���
	WSADATA  wd;
	if (WSAStartup(MAKEWORD(2, 2), &wd) != 0)
	{
		cout << "WSAStartup  error��" << GetLastError() << endl;
		return 0;
	}

	//������ʽ�׽���
	SOCKET  s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
	{
		cout << "socket  error��" << GetLastError() << endl;
		return 0;
	}

	//���ӷ�����
	sockaddr_in   addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8000);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int len = sizeof(sockaddr_in);
	if (connect(s, (SOCKADDR*)&addr, len) == SOCKET_ERROR)
	{
		cout << "connect  error��" << GetLastError() << endl;
		return 0;
	}



	//���շ���˵���Ϣ
	char buf[100] = { 0 };
	recv(s, buf, 100, 0);
	cout << buf << endl;

	char* p = NULL;
	p = strtok(buf, " ");
	p = strtok(NULL, " ");
	int id = atoi(p);

	//3��ʱ������˷���Ϣ

	// �û��� ���� ��֤
	char username[1024], passwd[1024];
	while (1)
	{
		cout << "�û�����";
		cin.getline(username, 1024);
		cout << "���룺";
		// �����Ե���������
		char ch;
		int index = 0;
		while ((ch = _getch()) != '\r') {
			if (ch != '\b') {
				cout << "*";
				passwd[index++] = ch;
			}
			else {
				if (index > 0) {
					cout << "\b \b";
					index--;
				}
			}
		}
		passwd[index] = '\0';
		cout << endl;

		//cin.getline(passwd, 1024);
		memset(&sendbuf.username, 0, sizeof(sendbuf.username));
		memset(&sendbuf.password, 0, sizeof(sendbuf.password));
		//char pw[1024] = { 0 };
		//strcat(pw, "\""); strcat(pw, passwd); strcat(pw, "\"");
		strcat(sendbuf.username, username);
		strcat(sendbuf.password, passwd);
		sendbuf.type = login;
		sendbuf.pwIsTrue = false;
		send(s, (char*)&sendbuf, sizeof(sendbuf), 0);
		Sleep(1000);
		recvbuf.pwIsTrue = false;
		recv(s, (char*)&recvbuf, sizeof(recvbuf), 0);
		if (recvbuf.pwIsTrue == false) {
			cout << "\033[31m[Client] \033[0m" << "�������" << endl;
			continue;
		}
		else {
			cout << "\033[32m[Client] \033[0m��¼�ɹ�!" << endl;
			break;
		}
	}



	// �û��� ���� ��֤

	int  ret = 1;

	// ������Ϣ���߳�
	HANDLE hThread = CreateThread(NULL, 0, ThreadFunRecv, (LPVOID)s, 0, NULL);
	// ������Ϣ���߳�
	
	//���߳���������Ϣ
	do
	{
		char buf[100] = { 0 };
		cout << id << "������������:";
		cin.getline(buf, 100);
		p = strtok(buf, " ");
		if (p) {
			if (strcmp(p, "/help") == 0) {
				cout << "ʹ��: /[����] [����]\n"											\
					"/online �鿴��ǰ��������\n"											\
					"/tell [id]  [content] ��ָ��id���û���������\n"						\
					"/tall [content] �������û��㲥��Ϣ\n"									\
					"/cls �鿴�û���ǰ����Ŀ¼�������ļ�\n"									\
					"/sls �鿴����˵�ǰ����Ŀ¼�µ������ļ�\n"								\
					"/uploads [file name] ���û�Ŀ¼�µ��ļ��ϴ��������\n"					\
					"/download [file name] ������˵��ļ����ص��ͻ��˵�ǰĿ¼��\n"			\
					"/relay [id] [file name] ����ǰĿ¼��ָ�����ļ����͸�ָ��id���û�\n";
			}
			if (strcmp(p, "/online") == 0) {
				sendbuf.type = online;
				//sendbuf.data[1024] = {0};
				memset(&sendbuf.data, 0, sizeof(sendbuf.data));
				ret = send(s, (char*)&sendbuf, sizeof(sendbuf), 0);
				Sleep(100);
				//ret = recv(s, (char*)&recvbuf, sizeof(recvbuf), 0);
				//cout << recvbuf.data << endl;
			}
			else if (strcmp(p, "/tell") == 0) {
				p = strtok(NULL, " ");
				if (p == NULL) {
					cout << "\033[31m[Client] \033[0m" << "��������ȷ�������ʽ" << endl;
					continue;
				}
				sendbuf.toid = atoi(p);
				if (sendbuf.toid == 0) {
					cout << "\033[31m[Client] \033[0m" << "��������ȷ�������ʽ" << endl;
					continue;
				}
				sendbuf.fromeid = id;
				p = strtok(NULL, "");
				if (p == NULL) {
					cout << "\033[31m[Client] \033[0m" << "��������ȷ�������ʽ" << endl;
					continue;
				}
				memset(&sendbuf.data, 0, sizeof(sendbuf.data));
				strcat(sendbuf.data, p);
				sendbuf.type = tell;
				ret = send(s, (char*)&sendbuf, sizeof(sendbuf), 0);

			}
			else if (strcmp(p, "/tall") == 0) {
				p = strtok(NULL, "");
				memset(&sendbuf.data, 0, sizeof(sendbuf.data));
				strcat(sendbuf.data, p);
				sendbuf.fromeid = id;
				sendbuf.type = tellall;
				ret = send(s, (char*)&sendbuf, sizeof(sendbuf), 0);
			}
			else if (strcmp(p, "/cls") == 0) {
				vector<string> files;
				int c = 1;
				char file_dir[MAX_PATH];
				_getcwd(file_dir, MAX_PATH);
				//dir_files.clear();
				files = findFile(file_dir);
				for (auto fl : files) {
					string strHead = "\033[42m", strEnd = "\033[0m";

					cout << "	" << strHead << fl << strEnd;
					if (c % 4 == 0 && c != 0) {
						cout << endl;
					}
					c++;
				}
				cout << endl;
			}
			else if (strcmp(p, "/sls") == 0) {
				sendbuf.type = sls;
				memset(&sendbuf.data, 0, sizeof(sendbuf.data));
				ret = send(s, (char*)&sendbuf, sizeof(sendbuf), 0);
				Sleep(100);
			}
			else if (strcmp(p, "/uploads") == 0) {
				sendbuf.type = uploads;
				sendbuf.uploadsing = false;
				sendbuf.uploaded = false;
				p = strtok(NULL, " ");
				char srcfile[1024] = { 0 };
				_getcwd(srcfile, 1024);
				strcat(srcfile, "\\"); strcat(srcfile, p);
				fp = fopen(srcfile, "rb");
				if (fp == NULL) {
					cout << "\033[31m[Client] \033[0m" <<"open file" << p << "failed." << endl;
					continue;
				}
				memset(&sendbuf.file_name, 0, sizeof(sendbuf.file_name));
				memset(&sendbuf.data, 0, sizeof(sendbuf.data));
				strcat(sendbuf.file_name, p);
				ret = send(s, (char*)&sendbuf, sizeof(sendbuf), 0);
				int rdbyt = 0;
				while (!feof(fp)) {
					sendbuf.uploadsing = true;
					rdbyt = fread(sendbuf.data, 1, sizeof(sendbuf.data), fp);
					if (rdbyt == 0)
						break;
					ret = send(s, (char*)&sendbuf, sizeof(sendbuf), 0);
				}
				if (fp != NULL)
					fclose(fp);
				fp = NULL;
				sendbuf.uploadsing = false; sendbuf.uploaded = true;
				ret = send(s, (char*)&sendbuf, sizeof(sendbuf), 0);
			}
			else if (strcmp(p, "/download") == 0) {
				sendbuf.type = download;
				p = strtok(NULL, " ");
				memset(&sendbuf.file_name, 0, sizeof(sendbuf.file_name));
				strcat(sendbuf.file_name, p);
				ret = send(s, (char*)&sendbuf, sizeof(sendbuf), 0);
			}
			else if (strcmp(p, "/relay") == 0) {
				p = strtok(NULL, " ");
				sendbuf.toid = atoi(p);
				sendbuf.fromeid = id;
				p = strtok(NULL, "");
				memset(&sendbuf.file_name, 0, sizeof(sendbuf.file_name));
				strcat(sendbuf.file_name, p);
				char file_dir[MAX_PATH];
				_getcwd(file_dir, MAX_PATH);
				if (strlen(sendbuf.file_name) != 0) {
					strcat(file_dir, "\\");   strcat(file_dir, sendbuf.file_name);
					fp = fopen(file_dir, "rb");
					cout << "\033[32m[Client] \033[0m" << "������ͻ���" << sendbuf.toid << "�����ļ�" << endl;
				}
				sendbuf.type = relay;
				sendbuf.relaying = false; sendbuf.relayed = false;
				ret = send(s, (char*)&sendbuf, sizeof(sendbuf), 0);
				int rdbyt = 0;
				while (!feof(fp)) {
					sendbuf.relaying = true;
					rdbyt = fread(sendbuf.data, 1, sizeof(sendbuf.data), fp);
					if (rdbyt == 0)
						break;
					ret = send(s, (char*)&sendbuf, sizeof(sendbuf), 0);
				}
				if (fp != NULL)
					fclose(fp);
				fp = NULL;
				sendbuf.relaying = false; sendbuf.relayed = true;
				ret = send(s, (char*)&sendbuf, sizeof(sendbuf), 0);

			}
		}
		//ret = send(s, buf, 100, 0);
	} while (ret != SOCKET_ERROR && ret != 0);


	//�رռ����׽���
	closesocket(s);

	//����winsock2�Ļ���
	WSACleanup();



	return 0;
}




DWORD WINAPI ThreadFunRecv(LPVOID lpThreadParameter)
{
	int  ret = 0;
	SOCKET s = (SOCKET)lpThreadParameter;
	do 
	{
		ret = recv(s, (char*)&recvbuf, sizeof(recvbuf), 0);
		switch (recvbuf.type) {
			case online:
			{
				//ret = recv(s, (char*)&recvbuf, sizeof(recvbuf), 0);
				cout << "\033[32m[Client] \033[0m" << recvbuf.data << endl;
			}
			break;
			case tell:
			{
				cout << "\033[32m[Client] \033[0m" <<"\n����" << recvbuf.fromeid << "����Ϣ��" << recvbuf.data << endl;

			}
			break;
			case tellall:
			{
				cout << "\033[32m[Client] \033[0m" <<"\n���Կͻ�" << recvbuf.fromeid << "�Ĺ㲥��Ϣ��" << recvbuf.data << endl;

			}
			break;
			case sls:
			{
				cout << "\033[32m[Client] \033[0m" << "\n�������ļ���" << endl << recvbuf.data << endl;
			}
			break;
			case download:
			{
				if (recvbuf.downloading == false && recvbuf.downloaded == false) {
					char file_dir[MAX_PATH];
					_getcwd(file_dir, MAX_PATH);
					if (strlen(recvbuf.file_name) != 0) {
						strcat(file_dir, "\\");   strcat(file_dir, recvbuf.file_name);
						fp = fopen(file_dir, "wb");
					}
					cout << "\033[32m[Client] \033[0m" << "�������ط���˵��ļ�" << "\033[42m" << recvbuf.file_name << "\033[0m" << endl;
				}
				else if (recvbuf.downloading == true && recvbuf.downloaded == false) {
					fwrite(recvbuf.data, 1, sizeof(recvbuf.data), fp);
				}
				else if (recvbuf.downloading == false && recvbuf.downloaded == true) {
					if (fp != NULL)
						fclose(fp); fp = NULL;
					cout << "\033[32m[Client] \033[0m" << "�������" << endl;
				}
			}
			break;
			case relay:
			{
				if (recvbuf.relaying == false && recvbuf.relayed == false) {
					char file_dir[MAX_PATH];
					_getcwd(file_dir, MAX_PATH);
					if (strlen(recvbuf.file_name) != 0) {
						strcat(file_dir, "\\");   strcat(file_dir, recvbuf.file_name);
						fp = fopen(file_dir, "wb");
					}
					cout << "\033[32m[Client] \033[0m�ͻ���" << recvbuf.fromeid << "���ڷ����ļ�" << "\033[42m" << recvbuf.file_name << "\033[0m" << endl;
				}
				else if (recvbuf.relaying == true && recvbuf.relayed == false) {
					fwrite(recvbuf.data, 1, sizeof(recvbuf.data), fp);
				}
				else if (recvbuf.relaying == false && recvbuf.relayed == true) {
					if (fp != NULL)
						fclose(fp); fp = NULL;
					cout << "\033[32m[Client] \033[0m�ͻ���" << recvbuf.fromeid << "���ͳɹ�" << endl;
				}
			}

		}
	} while (ret != SOCKET_ERROR && ret != 0);

	return 0;
}



vector<string> findFile(string dir)
{
	vector<string> files;

	WIN32_FIND_DATAA stFD;							//����ļ���Ϣ�Ľṹ��
	HANDLE h;
	string temp;

	temp = dir + "\\*";
	h = FindFirstFileA(temp.c_str(), &stFD);			//����Ŀ¼���

	while (FindNextFileA(h, &stFD))						//��ȡĿ¼�����ӦĿ¼���������ļ�
	{
		temp = dir + "\\" + stFD.cFileName;

		if (temp == dir + "\\..")										//��һ��·��
		{
			continue;
		}
		else if (PathIsDirectoryA(temp.c_str()))		//������Ŀ¼
		{
			vector<string> f2;
			f2 = findFile(temp);												//�ݹ����
			files.insert(files.end(), f2.begin(), f2.end());
		}
		else
		{
			files.push_back(stFD.cFileName);
			//cout << stFD.cFileName << endl;				//��ӡ�ļ���
		}
	}

	return files;
}