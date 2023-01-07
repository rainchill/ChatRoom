#include <winsock2.h> // winsock2的头文件
#include <iostream>
#include <vector>
#include <queue>
#include <string> 
#include <Shlwapi.h>
#include <direct.h>
#include <conio.h>
#include <json/json.h>
#include <fstream>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib,"shlwapi.lib")
using namespace std;
using namespace Json;

#pragma warning(disable:4996)

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
	bool pwIsTrue;
	char data[1024];
	char username[1024];
	char password[1024];
}sendbuf[1024], recvbuf[1024];

typedef struct __THREAD_DATA
{
	int seq;
	int socket_id;
}THREAD_DATA;

DWORD WINAPI ThreadFun(LPVOID lpThreadParameter);
DWORD WINAPI ThreadFunKeyInput(LPVOID lpThreadParameter);
vector<string> findFile(string dir);

vector<int> client_id_group;
queue<int> client_seq;


int main()
{
	// 初始化
	HANDLE hThread = CreateThread(NULL, 0, ThreadFunKeyInput, NULL, 0, NULL);
	CloseHandle(hThread); // 关闭对线程的引用

	for (int i = 0; i < 1024; i++) {
		client_seq.push(i);
	}

	WSADATA wd;
	if (WSAStartup(MAKEWORD(2, 2), &wd) != 0)
	{
		cout << "WSAStartup Error:" << WSAGetLastError() << endl;
		return 0;
	}

	// 创建流式套接字
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
	{
		cout << "socket error:" << WSAGetLastError() << endl;
		return 0;
	}

	// 绑定端口和ip
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8000);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int len = sizeof(sockaddr_in);
	if (bind(s, (SOCKADDR*)&addr, len) == SOCKET_ERROR)
	{
		cout << "bind Error:" << WSAGetLastError() << endl;
		return 0;
	}

	// 监听
	listen(s, 5);

	// 主线程循环接收客户端的连接
	while (true)
	{
		sockaddr_in addrClient;
		len = sizeof(sockaddr_in);
		// 接受成功返回与client通讯的Socket
		SOCKET c = accept(s, (SOCKADDR*)&addrClient, &len);
		int seq;
		client_id_group.push_back(c);
		if (c != INVALID_SOCKET)
		{
			// 创建线程，并且传入与client通讯的套接字
			THREAD_DATA threadData;
			threadData.seq = client_seq.front();
			client_seq.pop();
			threadData.socket_id = c;
			HANDLE hThread = CreateThread(NULL, 0, ThreadFun, &threadData, 0, NULL);
			CloseHandle(hThread); // 关闭对线程的引用
		}

	}

	// 关闭监听套接字
	closesocket(s);

	// 清理winsock2的环境
	WSACleanup();

	return 0;
}

DWORD WINAPI ThreadFun(LPVOID lpThreadParameter)
{
	
	THREAD_DATA* pThreadData = (THREAD_DATA*)lpThreadParameter;
	SOCKET c = (SOCKET)pThreadData->socket_id;
	int seq = pThreadData->seq;
	cout << "\033[32m[log] \033[0m欢迎" << c << "进入聊天室！" << endl;

	// 发送数据
	char buf[100] = { 0 };
	sprintf(buf, "欢迎 %d 进入聊天室！", c);
	send(c, buf, 100, 0);

	// 循环接收客户端数据
	int ret = 0;
	FILE* fp = NULL;
	do
	{
		//char buf2[100] = { 0 };
		memset(&recvbuf[seq].data, 0, sizeof(recvbuf[seq].data));
		ret = recv(c, (char*)&recvbuf[seq], sizeof(recvbuf[seq]), 0);
		switch (recvbuf[seq].type) {
			case online:
			{
				memset(sendbuf[seq].data, 0, sizeof(sendbuf[seq].data));
				sendbuf[seq].type = online;
				strcat(sendbuf[seq].data, "当前在线用户：");
				for (auto id : client_id_group) {
					char temp[10];
					strcat(sendbuf[seq].data, "-用户");
					strcat(sendbuf[seq].data, _itoa(id, temp, 10));
				}
				cout << "\033[32m[log] \033[0m" << sendbuf[seq].data << endl;
				send(c, (char*)&sendbuf[seq], sizeof(sendbuf[seq]), 0);
			}
			break;
			case tell:
			{
				//cout << "tell" << recvbuf[seq].data << endl;
				send(recvbuf[seq].toid, (char *) & recvbuf[seq], sizeof(recvbuf[seq]), 0);
				cout << "\033[32m[log] \033[0m客户端" << recvbuf[seq].fromeid << "向客户端" << recvbuf[seq].toid << "发送消息" << endl;

			}
			break;
			case tellall:
			{
				for (auto id : client_id_group) {
					if (id != recvbuf[seq].fromeid)
						send(id, (char*)&recvbuf[seq], sizeof(recvbuf[seq]), 0);
				}
				cout << "\033[32m[log] \033[0m客户端" << recvbuf[seq].fromeid << "发送广播消息" << endl;
			}
			break;
			case sls:
			{
				vector<string> files;
				char file_dir[MAX_PATH];
				_getcwd(file_dir, MAX_PATH);
				files = findFile(file_dir);
				memset(sendbuf[seq].data, 0, sizeof(sendbuf[seq].data));
				sendbuf[seq].type = sls;
				int count = 1;
				for (auto fl : files) {
					strcat(sendbuf[seq].data, "    \033[46m");
					strcat(sendbuf[seq].data, fl.c_str());
					strcat(sendbuf[seq].data, "\033[0m");
					if (count % 4 == 0) {
						strcat(sendbuf[seq].data, "\n");
					}
					count++;
				}
				cout << "\033[32m[log] \033[0m发送给客户端" << c << "服务器当前目录下所有文件" << endl;
				send(c, (char*)&sendbuf[seq], sizeof(sendbuf[seq]), 0);
			}
			break;
			case uploads:
			{
				if (recvbuf[seq].uploadsing == false && recvbuf[seq].uploaded == false) {
					char file_dir[MAX_PATH];
					_getcwd(file_dir, MAX_PATH);
					if (strlen(recvbuf[seq].file_name) != 0) {
						strcat(file_dir, "\\");   strcat(file_dir, recvbuf[seq].file_name);
						fp = fopen(file_dir, "wb");
					}
					cout << "\033[32m[log] \033[0m客户端" << c << "正在向服务端发送文件" << "\033[46m" << recvbuf[seq].file_name << "\033[0m" << endl;
				} 
				else if (recvbuf[seq].uploadsing == true && recvbuf[seq].uploaded == false) {
					fwrite(recvbuf[seq].data, 1, sizeof(recvbuf[seq].data), fp);
				}
				else if (recvbuf[seq].uploadsing == false && recvbuf[seq].uploaded == true) {
					if (fp != NULL)
						fclose(fp); fp = NULL;
					cout << "\033[32m[log] \033[0m客户端" << c << "发送给服务端的文件" << "\033[46m" << recvbuf[seq].file_name  << "\033[0m" << "已接收" <<  endl;
				}
			}
			break;
			case download:
			{
				char file_dir[MAX_PATH];
				_getcwd(file_dir, MAX_PATH);
				if (strlen(recvbuf[seq].file_name) != 0) {
					strcat(file_dir, "\\");   strcat(file_dir, recvbuf[seq].file_name);
					fp = fopen(file_dir, "rb");
				}
				if (fp == NULL) {
					cout << "\033[31m[log] \033[0m open file" << recvbuf[seq].file_name << "failed." << endl;
				}
				memset(&sendbuf[seq].file_name, 0, sizeof(sendbuf[seq].file_name));
				memset(&sendbuf[seq].data, 0, sizeof(sendbuf[seq].data));
				strcat(sendbuf[seq].file_name, recvbuf[seq].file_name);
				sendbuf[seq].downloading = false; sendbuf[seq].downloaded = false;
				sendbuf[seq].type = download;
				ret = send(c, (char*)&sendbuf[seq], sizeof(sendbuf[seq]), 0);
				int rdbyt = 0;
				while (!feof(fp)) {
					sendbuf[seq].downloading = true;
					rdbyt = fread(sendbuf[seq].data, 1, sizeof(sendbuf[seq].data), fp);
					if (rdbyt == 0)
						break;
					ret = send(c, (char*)&sendbuf[seq], sizeof(sendbuf[seq]), 0);
				}
				if (fp != NULL)
					fclose(fp);
				fp = NULL;
				sendbuf[seq].downloading = false; sendbuf[seq].downloaded = true;
				ret = send(c, (char*)&sendbuf[seq], sizeof(sendbuf[seq]), 0);
			}
			break;
			case relay:
			{
				send(recvbuf[seq].toid, (char*)&recvbuf[seq], sizeof(recvbuf[seq]), 0);
			}
			break;
			case login:
			{
				ifstream ifs("users.json");
				Value root;
				Reader r;
				r.parse(ifs, root);
				if (strcmp(root[recvbuf[seq].username].asString().c_str(), "") == 0) {
					sendbuf[seq].pwIsTrue = false;
					send(c, (char*)&sendbuf[seq], sizeof(sendbuf[seq]), 0);
					continue;
				}
				if (strcmp(root[recvbuf[seq].username].asString().c_str(), recvbuf[seq].password) == 0)
					sendbuf[seq].pwIsTrue = true;
				else
					sendbuf[seq].pwIsTrue = false;
				send(c, (char*)&sendbuf[seq], sizeof(sendbuf[seq]), 0);
			}
		}

		//cout << c << " 说：" << buf2 << endl;

	} while (ret != SOCKET_ERROR && ret != 0);


	vector<int>::iterator iter;
	iter = find(client_id_group.begin(), client_id_group.end(), c);
	client_id_group.erase(iter);

	client_seq.push(seq);
	cout << "\033[32m[log] \033[0m" << c << "离开了聊天室！\n";


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


DWORD WINAPI ThreadFunKeyInput(LPVOID lpThreadParameter)
{
	int ch;
	char* p = NULL;
	while (1) {
		if (_kbhit()) {//如果有按键按下，则_kbhit()函数返回真

			char buf[100] = { 0 };
			cin.getline(buf, 100);
			p = strtok(buf, " ");
			if (p) {
				if (strcmp(p, "/online") == 0) {
					char sbuf[1024] = { 0 };
					strcat(sbuf, "当前在线用户：");
					for (auto id : client_id_group) {
						char temp[10];
						strcat(sbuf, "-用户");
						strcat(sbuf, _itoa(id, temp, 10));
					}
					cout << "\033[32m[log] \033[0m" << sbuf << endl;
				}
			}
		}
	}
}