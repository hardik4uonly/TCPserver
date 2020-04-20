#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#define SAPERATOR ","

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include<vector>
#include<string>
#include<sstream>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
//#include <bits/stdc++.h> 

using namespace std;

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "9999"
string processApi(vector<string>);
void tokenize(string, vector<string>*);
string JSONresponse(string data, bool error);

int __cdecl main(void)
{
    string  api = "{\"text\":\"here is text\",\"data\":\"this is data\"}";

    cout << api << endl;
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    std::cout << "server started" << endl;

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    else {
        printf("conncted");
    }
    vector<string> Api_parts;
    // No longer need server socket
    closesocket(ListenSocket);
    string sendbuf;
    do {
        memset(recvbuf, 0, DEFAULT_BUFLEN);
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);
            cout << recvbuf << endl;

            tokenize(recvbuf, &Api_parts);
            sendbuf = processApi(Api_parts);

            // Echo the buffer back to the sender
            iSendResult = send(ClientSocket, sendbuf.c_str(), sendbuf.size(), 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}

void tokenize(string api, vector<string>* saparated_api) {
    stringstream ss(api);
    string intermediate;
    vector<string>& ptr = *saparated_api;

    // Tokenizing w.r.t. space ' ' 
    while (getline(ss, intermediate, ','))
    {
        (*saparated_api).push_back(intermediate);
    }
    for (int i = 0;i < saparated_api->size();i++)
    {
        cout << ptr[i] << endl;
    }
}

string processApi(vector<string> api)
{
    for (int i = 0;i < api.size();i++)
    {
        cout << api[i] << endl;
    }
    if(api[0] == "1")
        return JSONresponse(api[0] + SAPERATOR + api[1] + SAPERATOR + "1",false);
    else
        return JSONresponse("400", false);
}

string JSONresponse(string data, bool error)
{
    if (error) {
        return ("{\"error\":\"" + data + "\"}");
    }
    else {
        return ("{\"data\":\"" + data + "\"}");
    }
}