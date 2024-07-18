#include <stdio.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define PORT 36
#define IP "192.168.254.17"

int connectSocket(SOCKET s) {

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    fd_set fdW, fdE;
    FD_ZERO(&fdW);
    FD_ZERO(&fdE);
    FD_SET(s, &fdW);
    FD_SET(s, &fdE);
    int max_fd = s + 1;

    switch (select(max_fd, NULL, &fdW, &fdE, &tv)) {
    case SOCKET_ERROR:
        return SOCKET_ERROR;
    case 1:
        if (FD_ISSET(s, &fdW)) {
            return 1;
        }
        if (FD_ISSET(s, &fdE)) {
            return SOCKET_ERROR;
        }
        break;

    default:
        printf("Trying to connect..\n");
        break;
    }
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET client_fd;
    struct sockaddr_in serv_addr;
    char* hello = "Hello from client";
    char buffer[1024] = { 0 };
    int valread;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }


    if ((client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        printf("\n Socket creation error \n");
        return 1;
    }
    // non blocking moda al
    u_long mode = 1; // 1 : non-blocking mode // 0: blocking mode
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);


    if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return 1;
    }


    int a, b;
    char sign;
    char sumBuffer[16];


    while (1) {
        if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                printf("Connection in progress...\n");
                int connectStatus = connectSocket(client_fd);
                switch (connectStatus) {
                case 1:
                    printf("Connected to server\n");
                    break;
                case 0:
                    printf("Connection timed out\n");
                    continue;
                case SOCKET_ERROR:
                    printf("Connection fail\n");
                    closesocket(client_fd);
                    continue;
                }
            }
            else {
                printf("Connection failed: %d\n", WSAGetLastError());
                closesocket(client_fd);
                continue;
            }
        }
        else {
            printf("Connection success\n");
            break;
        }
    }
    if (ioctlsocket(client_fd, FIONBIO, &mode) != 0) {
        printf("Error setting non-blocking mode: %d\n", WSAGetLastError());
        closesocket(client_fd);
        WSACleanup();
        return 1;
    }

    while (1) {
        scanf_s("%d %1c %d", &a, &sign, 1, &b);
        if (sign == '+') {
            int c = a + b;
            sprintf_s(sumBuffer, 16, "%d", c);
        }
        else {
            printf("Invalid sign enter + \n");
            continue;
        }
        int send_result = send(client_fd, sumBuffer, strlen(sumBuffer), 0);
        if (send_result == SOCKET_ERROR) {
            int le = WSAGetLastError();
            if (le == WSAEWOULDBLOCK) {
                Sleep(100);
                continue;
            }
            else {
                printf("Send failed. Error Code: %d\n", WSAGetLastError());
                closesocket(client_fd);
                WSACleanup();
                break;
            }
        }
    }
    closesocket(client_fd);
    WSACleanup();
    return 0;
}

