#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include <iostream>
#include <vector>

#include <thread>
#include "Database.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 1234
#define BUFFER_SIZE 512

Database g_database;

struct OVERLAPPED_EX {
    OVERLAPPED overlapped;
    WSABUF wsaBuf;
    char buffer[BUFFER_SIZE];
    SOCKET socket;
};



void WorkerThread(HANDLE hIOCP);

int main() {
    if (!g_database.Connect("127.0.0.1", "root", "1234", "mysql", 3306)) {
        std::cerr << "DB 연결 실패. 서버 종료.\n";
        return 1;
    }

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(listenSocket, SOMAXCONN);

    HANDLE hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    CreateIoCompletionPort((HANDLE)listenSocket, hIOCP, (ULONG_PTR)listenSocket, 0);


    // 워커 스레드 실행
    for (int i = 0; i < 4; ++i) {
        std::thread(WorkerThread, hIOCP).detach();
    }

    while (true) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        std::cout << "클라이언트 접속됨\n";

        CreateIoCompletionPort((HANDLE)clientSocket, hIOCP, (ULONG_PTR)clientSocket, 0);

        auto* overlappedEx = new OVERLAPPED_EX();
        memset(overlappedEx, 0, sizeof(OVERLAPPED_EX));
        overlappedEx->socket = clientSocket;
        overlappedEx->wsaBuf.buf = overlappedEx->buffer;
        overlappedEx->wsaBuf.len = BUFFER_SIZE;

        DWORD recvBytes = 0, flags = 0;
        WSARecv(clientSocket, &overlappedEx->wsaBuf, 1, &recvBytes, &flags, &overlappedEx->overlapped, NULL);
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}

void WorkerThread(HANDLE hIOCP) {
    while (true) {
        DWORD bytesTransferred;
        ULONG_PTR completionKey;
        LPOVERLAPPED lpOverlapped;

        BOOL result = GetQueuedCompletionStatus(hIOCP, &bytesTransferred, &completionKey, &lpOverlapped, INFINITE);

        if (!result || bytesTransferred == 0) {
            std::cout << "클라이언트 연결 끊김\n";
            continue;
        }

        auto* overlappedEx = reinterpret_cast<OVERLAPPED_EX*>(lpOverlapped);
        std::cout << "받은 메시지: " << overlappedEx->buffer << std::endl;

        // 다시 수신 대기
        DWORD flags = 0;
        ZeroMemory(overlappedEx->buffer, BUFFER_SIZE);
        WSARecv(overlappedEx->socket, &overlappedEx->wsaBuf, 1, NULL, &flags, &overlappedEx->overlapped, NULL);
    }
}