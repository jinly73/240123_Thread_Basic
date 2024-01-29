#define FD_SETSIZE  100
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <iostream>
#include <Winsock2.h>
#include <Windows.h>

using namespace std;

#pragma comment (lib, "ws2_32")

int main()
{

	WSAData wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);

	SOCKET ListenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN ListenSocketAddr;
	memset(&ListenSocketAddr, 0, sizeof(&ListenSocketAddr));
	ListenSocketAddr.sin_family = AF_INET;
	ListenSocketAddr.sin_addr.s_addr = INADDR_ANY; // inet_addr("127.0.0.1");
	ListenSocketAddr.sin_port = htons(40001);

	bind(ListenSocket, (SOCKADDR*)&ListenSocketAddr, sizeof(ListenSocketAddr));

	listen(ListenSocket, 0);

	fd_set ReadSocketList;
	fd_set CopySocketList;
	FD_ZERO(&ReadSocketList);
	FD_ZERO(&CopySocketList);

	FD_SET(ListenSocket, &ReadSocketList);

	struct timeval TimeOut;

	TimeOut.tv_sec = 0;

	while (true)
	{
		CopySocketList = ReadSocketList;

		int ChangeSocketCount = select(0, &CopySocketList, nullptr, nullptr, &TimeOut);
		if (ChangeSocketCount == 0)
		{
			// seccese
			continue;
		}
		else
		{
			for (int i = 0; i < (int)ReadSocketList.fd_count; i++)
			{
				if (FD_ISSET(ReadSocketList.fd_array[i], &CopySocketList)) // 나 리스트에 있어? 라고 OS에게 물어보는 것
				{
					if (ReadSocketList.fd_array[i] == ListenSocket)
					{
						// accept
						SOCKADDR_IN ClientSockAddr;
						memset(&ClientSockAddr, 0, sizeof(ClientSockAddr));
						int ClientAddrLenth = sizeof(ClientSockAddr);
						SOCKET ClientSocket = accept(ListenSocket, (SOCKADDR*)&ClientSockAddr, &ClientAddrLenth); // 클라이언트
						FD_SET(ClientSocket, &ReadSocketList);
						cout << "connect client : " << inet_ntoa(ClientSockAddr.sin_addr) << endl;
					}
					else
					{
						//recv
						char Buffer[1024] = { 0, };
						int RecvLength = recv(ReadSocketList.fd_array[i], Buffer, sizeof(Buffer), 0);
						if (RecvLength <= 0)
						{
							closesocket(ReadSocketList.fd_array[i]);
							FD_CLR(ReadSocketList.fd_array[i], &ReadSocketList);
						}
						else
						{
							int SendLenght = send(ReadSocketList.fd_array[i], Buffer, RecvLength, 0);
						}
					}
				}
			}
		}

	}
	


	closesocket(ListenSocket);


	WSACleanup();

	return 0;
}