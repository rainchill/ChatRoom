#include<winsock2.h>//winsock2的头文件
#include<iostream>
#include <Shlwapi.h>
#include <direct.h>
#include <string>
#include <vector>
using  namespace std;

#pragma comment(lib,"shlwapi.lib")	

//勿忘，链接dll的lib
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


struct pdu//帧结构
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

	//加载winsock2的环境
	WSADATA  wd;
	if (WSAStartup(MAKEWORD(2, 2), &wd) != 0)
	{
		cout << "WSAStartup  error：" << GetLastError() << endl;
		return 0;
	}

	//1.创建流式套接字
	SOCKET  s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
	{
		cout << "socket  error：" << GetLastError() << endl;
		return 0;
	}

	//2.链接服务器
	sockaddr_in   addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8000);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int len = sizeof(sockaddr_in);
	if (connect(s, (SOCKADDR*)&addr, len) == SOCKET_ERROR)
	{
		cout << "connect  error：" << GetLastError() << endl;
		return 0;
	}



	//3接收服务端的消息
	char buf[100] = { 0 };
	recv(s, buf, 100, 0);
	cout << buf << endl;

	char* p = NULL;
	p = strtok(buf, " ");
	p = strtok(NULL, " ");
	int id = atoi(p);

	//3随时给服务端发消息

	// 用户名 密码 验证
	char username[1024], passwd[1024];
	cout << "用户名：";
	cin.getline(username, 1024);
	cout << "密码：";
	cin.getline(passwd, 1024);
	memset(&sendbuf.username, 0, sizeof(sendbuf.username));
	memset(&sendbuf.password, 0, sizeof(sendbuf.password));
	//char pw[1024] = { 0 };
	//strcat(pw, "\""); strcat(pw, passwd); strcat(pw, "\"");
	strcat(sendbuf.username, username);
	strcat(sendbuf.password, passwd);
	sendbuf.type = login;
	send(s, (char*)&sendbuf, sizeof(sendbuf), 0);
	Sleep(1000);
	recvbuf.pwIsTrue = false;
	recv(s, (char*)&recvbuf, sizeof(recvbuf), 0);
	if (recvbuf.pwIsTrue == false) {
		cout << "密码错误" << endl;
		return 0;
	}
	else {
		cout << "\033[32m[Client] \033[0m登录成功!" << endl;
	}



	// 用户名 密码 验证

	int  ret = 1;

	// 接收消息的线程
	HANDLE hThread = CreateThread(NULL, 0, ThreadFunRecv, (LPVOID)s, 0, NULL);
	// 接收消息的线程
	
	//主线程用来发消息
	do
	{
		char buf[100] = { 0 };
		cout << id << "号请输入内容:";
		cin.getline(buf, 100);
		p = strtok(buf, " ");
		if (p) {
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
				sendbuf.toid = atoi(p);
				sendbuf.fromeid = id;
				p = strtok(NULL, "");
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
					cout << "open file" << p << "failed." << endl;
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
					cout << "\033[32m[Client] \033[0m" << "正在向客户端" << sendbuf.toid << "发送文件" << endl;
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


	//4.关闭监听套接字
	closesocket(s);

	//清理winsock2的环境
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
				cout << recvbuf.data << endl;
			}
			break;
			case tell:
			{
				cout << "\n来自" << recvbuf.fromeid << "的消息：" << recvbuf.data << endl;

			}
			break;
			case tellall:
			{
				cout << "\n来自客户" << recvbuf.fromeid << "的广播消息：" << recvbuf.data << endl;

			}
			break;
			case sls:
			{
				cout << "\n服务器文件：" << endl << recvbuf.data << endl;
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
					cout << "\033[32m[Client] \033[0m" << "正在下载服务端的文件" << "\033[42m" << recvbuf.file_name << "\033[0m" << endl;
				}
				else if (recvbuf.downloading == true && recvbuf.downloaded == false) {
					fwrite(recvbuf.data, 1, sizeof(recvbuf.data), fp);
				}
				else if (recvbuf.downloading == false && recvbuf.downloaded == true) {
					if (fp != NULL)
						fclose(fp); fp = NULL;
					cout << "\033[32m[Client] \033[0m" << "下载完成" << endl;
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
					cout << "\033[32m[Client] \033[0m客户端" << recvbuf.fromeid << "正在发来文件" << "\033[42m" << recvbuf.file_name << "\033[0m" << endl;
				}
				else if (recvbuf.relaying == true && recvbuf.relayed == false) {
					fwrite(recvbuf.data, 1, sizeof(recvbuf.data), fp);
				}
				else if (recvbuf.relaying == false && recvbuf.relayed == true) {
					if (fp != NULL)
						fclose(fp); fp = NULL;
					cout << "\033[32m[Client] \033[0m客户端" << recvbuf.fromeid << "发送成功" << endl;
				}
			}

		}
	} while (ret != SOCKET_ERROR && ret != 0);

	return 0;
}



vector<string> findFile(string dir)
{
	vector<string> files;

	WIN32_FIND_DATAA stFD;							//存放文件信息的结构体
	HANDLE h;
	string temp;

	temp = dir + "\\*";
	h = FindFirstFileA(temp.c_str(), &stFD);			//构建目录句柄

	while (FindNextFileA(h, &stFD))						//提取目录句柄对应目录所包含的文件
	{
		temp = dir + "\\" + stFD.cFileName;

		if (temp == dir + "\\..")										//上一级路径
		{
			continue;
		}
		else if (PathIsDirectoryA(temp.c_str()))		//包含子目录
		{
			vector<string> f2;
			f2 = findFile(temp);												//递归调用
			files.insert(files.end(), f2.begin(), f2.end());
		}
		else
		{
			files.push_back(stFD.cFileName);
			//cout << stFD.cFileName << endl;				//打印文件名
		}
	}

	return files;
}