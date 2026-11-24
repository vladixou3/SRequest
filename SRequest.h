/*
* SRequest: Single-Header Windows Library
* Copyright(�) Donart Ajeti 2026 
* 
* Version Alpha 1.0.0
*
* Don't forget the -lws2_32 at the compilation!
*
*/

#ifndef SREQUEST_H
#define SREQUEST_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    SOCKET sock;
    char ip[16];
    int port;
    struct sockaddr_in serv;
    bool debug;
    bool socket;
    bool listening;
}Request;

typedef struct {
    long long size;
    long long sent;
}SentData;

bool IniRequest() {
    WSADATA wsa;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (iResult != 0) {
        return false;
    } return true;
}

Request IniServer(int port, bool debug) {
    Request r = {0};

    r.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(r.sock == INVALID_SOCKET)
        return r;

    r.serv.sin_family = AF_INET;
    r.serv.sin_port = htons(port);
    r.serv.sin_addr.s_addr = INADDR_ANY;

    r.port = port;
    r.debug = debug;
    r.socket = true;
    r.listening = false;

    return r;
}

Request IniConnexion(const char* ip, int port, bool debug) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    Request r;

    if (sock == INVALID_SOCKET) {
        r.socket = false;
        return (Request){0};
    }

    struct sockaddr_in serv = {0};

    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv.sin_addr) != 1) {
        r.socket = false;
        closesocket(sock);
        return (Request){0};
    }
    r.sock = sock;
    strcpy_s(r.ip, sizeof(r.ip), ip);
    r.port = port;
    r.serv = serv;
    r.debug = debug;
    r.socket = true;

    return r;
}

bool SConnect(Request* request) {
    int result = connect(
        request->sock,
        (struct sockaddr*)&request->serv,
        sizeof(request->serv)
    );

    if (result == SOCKET_ERROR) {
        if(request->debug) 
            printf("[SRequest] Winsock Error: %d\n", WSAGetLastError());
        return false;
    }

   if(request->debug)
        printf("[SRequest] TCP connection established!\n");
    return true;
}

SentData SendData(const char* data, Request* request) {
    int sent = send(request->sock, data, strlen(data), 0);
    SentData sd;
    if(sent == SOCKET_ERROR) {
        if(request->debug) 
             printf("[SRequest] Winsock Error: %d\n", WSAGetLastError());
        sd.sent = -1;
        sd.size = 0;
    } else {
        sd.size = strlen(data);
        sd.sent = sent;
    }
    return sd;
}

void Close(Request* request) {
    if(request->socket)
        closesocket(request->sock);
    WSACleanup();
}

bool RecData(Request* request, int data_size, char* buffer) {
    if(!request || !request->socket || !buffer || data_size <= 0)
        return false;
    if(!request->listening) {
        if(bind(request->sock,
            (struct sockaddr*)&request->serv,
            sizeof(request->serv)) == SOCKET_ERROR) {
            if(request->debug)
                printf("[SRequest] Bind failed: %d\n", WSAGetLastError());
            return false;
        }
        if(listen(request->sock, SOMAXCONN) == SOCKET_ERROR) {
            if(request->debug)
                printf("[SRequest] Listen failed: %d\n", WSAGetLastError());
            return false;
        }
        request->listening = true;
    }
    SOCKET client = accept(request->sock, NULL, NULL);
    if(client == INVALID_SOCKET) {
        if(request->debug)
            printf("[SRequest] Accept failed: %d\n", WSAGetLastError());
        return false;
    }
    if(request->debug)
        printf("[SRequest] Client connected!\n");
    while(true) {
        int received = recv(client, buffer, data_size, 0);
        if(received == 0)
            break;
        if(received == SOCKET_ERROR) {
            if(request->debug)
                printf("[SRequest] Recv failed: %d\n", WSAGetLastError());
            closesocket(client);
            return false;
        }
        if(request->debug)
            printf("[SRequest] Received: %d bytes\n", received);
        fwrite(buffer, 1, received, stdout);
    }
    if(request->debug)
        printf("[SRequest] Client disconnected.\n");
    closesocket(client);
    return true;
}

#endif