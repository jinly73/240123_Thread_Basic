#include <iostream>
#include <winsock2.h>
#include <Windows.h>
#include <process.h>
#include <vector>
#include <set>

using namespace std;

#pragma comment(lib, "ws2_32")

std::set<SOCKET> SessionList;

CRITICAL_SECTION SessionCS;

unsigned WINAPI WorkerThread(void* Arg)
{
	SOCKET ClientSocket = *((SOCKET*)Arg);
	while (true)
	{
		char Buffer[1024] = { 0, };
		int RecvLength = recv(ClientSocket, Buffer, sizeof(Buffer), 0);
		if (RecvLength <= 0)
		{
			EnterCriticalSection(&SessionCS);
			SessionList.erase(ClientSocket); // 지우는 것
			LeaveCriticalSection(&SessionCS);
			closesocket(ClientSocket);
			break;
		}
		else
		{
			EnterCriticalSection(&SessionCS);
			for (auto ConenctSoket : SessionList)
			{
				int SendLenght = send(ClientSocket, Buffer, RecvLength, 0);
				cout << ClientSocket << "" << Buffer << endl;
			}
			LeaveCriticalSection(&SessionCS);
		}
	}
	return 0;
}

int main()
{
	InitializeCriticalSection(&SessionCS);

	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ListenSockAddr;
	memset(&ListenSockAddr, 0, sizeof(ListenSockAddr));
	ListenSockAddr.sin_family = AF_INET;
	ListenSockAddr.sin_addr.s_addr = INADDR_ANY;
	ListenSockAddr.sin_port = htons(22223);

	bind(ListenSocket, (SOCKADDR*)&ListenSockAddr, sizeof(ListenSockAddr));

	listen(ListenSocket, 5);

	while (true)
	{
		SOCKADDR_IN ClientSockAddr;
		memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
		int ClientAddrLenth = sizeof(ClientSockAddr);
		SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSockAddr, &ClientAddrLenth);
		//thread 실행, worker thread
		//CreadteThread()
		HANDLE ThreadHandle = (HANDLE)_beginthreadex(0, 0, WorkerThread, (void*) & ClientSocket, 0, 0);
		//SessionList.insert(ClientSocket);
		EnterCriticalSection(&SessionCS);
		SessionList.erase(ClientSocket); // 지우는 것
		LeaveCriticalSection(&SessionCS);
	}

	closesocket(ListenSocket);

	WSACleanup();

	DeleteCriticalSection(&SessionCS);

	return 0;
}