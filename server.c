// 引入标准输入输出头文件，用于 printf 等输出函数
#include <stdio.h>

// 引入标准库头文件，用于通用程序控制
#include <stdlib.h>

// 引入字符串处理头文件，用于 memset 等函数
#include <string.h>

// 引入 Winsock2 头文件，这是 Windows 下 socket 编程的核心头文件
#include <winsock2.h>

// 引入 Windows socket 扩展头文件，提供 inet_pton 等函数声明
#include <ws2tcpip.h>

// 告诉编译器自动链接 Ws2_32.lib 网络库
#pragma comment(lib, "Ws2_32.lib")

// 定义服务器监听端口，客户端必须连接这个端口
#define SERVER_PORT 8888

// 定义收发缓冲区大小
#define BUFFER_SIZE 1024

// main 函数是整个程序的入口
int main(void)
{
    // WSADATA 结构体用于保存 Winsock 初始化后的相关信息
    WSADATA wsaData;

    // listenSock 是监听套接字，专门负责等待客户端连接
    SOCKET listenSock = INVALID_SOCKET;

    // clientSock 是通信套接字，专门负责和某一个客户端收发数据
    SOCKET clientSock = INVALID_SOCKET;

    // serverAddr 用来保存服务器自己的 IP 地址和端口信息
    struct sockaddr_in serverAddr;

    // clientAddr 用来保存客户端的地址信息
    struct sockaddr_in clientAddr;

    // clientAddrLen 用来保存客户端地址结构体的长度，accept 需要它
    int clientAddrLen = sizeof(clientAddr);

    // buffer 是收发数据时使用的字符数组缓冲区
    char buffer[BUFFER_SIZE];

    // recvLen 用于保存 recv 函数实际接收到的字节数
    int recvLen;

    // 调用 WSAStartup 初始化 Winsock 2.2 运行环境
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        // 如果初始化失败，打印错误提示
        printf("WSAStartup failed.\n");

        // 返回 1 表示程序异常结束
        return 1;
    }

    // 创建一个 IPv4、TCP 类型的监听套接字
    listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // 判断套接字是否创建失败
    if (listenSock == INVALID_SOCKET)
    {
        // 打印 socket 创建失败的错误码
        printf("socket failed. error = %d\n", WSAGetLastError());

        // 清理 Winsock 资源
        WSACleanup();

        // 返回 1 表示程序异常结束
        return 1;
    }

    // 将服务器地址结构体清零，防止未初始化的数据造成影响
    memset(&serverAddr, 0, sizeof(serverAddr));

    // 设置地址族为 IPv4
    serverAddr.sin_family = AF_INET;

    // 设置监听端口，并将主机字节序端口转换为网络字节序
    serverAddr.sin_port = htons(SERVER_PORT);

    // 设置监听 IP 为本机所有网卡地址
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 将监听套接字绑定到指定的本地地址和端口
    if (bind(listenSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        // 打印 bind 失败的错误码
        printf("bind failed. error = %d\n", WSAGetLastError());

        // 关闭监听套接字
        closesocket(listenSock);

        // 清理 Winsock 资源
        WSACleanup();

        // 返回 1 表示程序异常结束
        return 1;
    }

    // 将监听套接字设置为监听状态，准备接收客户端连接
    if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR)
    {
        // 打印 listen 失败的错误码
        printf("listen failed. error = %d\n", WSAGetLastError());

        // 关闭监听套接字
        closesocket(listenSock);

        // 清理 Winsock 资源
        WSACleanup();

        // 返回 1 表示程序异常结束
        return 1;
    }

    // 打印提示，说明服务器已经启动并开始监听
    printf("Server started. Listening on port %d...\n", SERVER_PORT);

    // 调用 accept 阻塞等待客户端连接
    clientSock = accept(listenSock, (struct sockaddr *)&clientAddr, &clientAddrLen);

    // 判断客户端连接是否接收失败
    if (clientSock == INVALID_SOCKET)
    {
        // 打印 accept 失败的错误码
        printf("accept failed. error = %d\n", WSAGetLastError());

        // 关闭监听套接字
        closesocket(listenSock);

        // 清理 Winsock 资源
        WSACleanup();

        // 返回 1 表示程序异常结束
        return 1;
    }

    // 打印提示，说明已有客户端连接成功
    printf("Client connected.\n");

    // 进入循环，持续处理来自客户端的数据
    while (1)
    {
        // 每次接收前先清空缓冲区，避免旧数据残留影响输出
        memset(buffer, 0, sizeof(buffer));

        // 从客户端接收数据，最多读取 BUFFER_SIZE - 1 个字节
        recvLen = recv(clientSock, buffer, sizeof(buffer) - 1, 0);

        // 如果 recv 返回值大于 0，表示成功收到数据
        if (recvLen > 0)
        {
            // 在接收到的数据末尾补上字符串结束符，方便按字符串打印
            buffer[recvLen] = '\0';

            // 打印客户端发送来的内容
            printf("Client says: %s", buffer);

            // 将收到的内容原样发回给客户端，形成最经典的回显服务器
            if (send(clientSock, buffer, recvLen, 0) == SOCKET_ERROR)
            {
                // 如果发送失败，打印错误码
                printf("send failed. error = %d\n", WSAGetLastError());

                // 跳出循环，结束通信
                break;
            }
        }
        // 如果 recv 返回 0，表示客户端正常关闭了连接
        else if (recvLen == 0)
        {
            // 打印提示，说明客户端正常断开
            printf("Client disconnected gracefully.\n");

            // 跳出循环
            break;
        }
        // 如果 recv 返回负值，表示接收过程中发生错误
        else
        {
            // 打印 recv 失败的错误码
            printf("recv failed. error = %d\n", WSAGetLastError());

            // 跳出循环
            break;
        }
    }

    // 关闭和客户端通信的套接字
    closesocket(clientSock);

    // 关闭监听套接字
    closesocket(listenSock);

    // 释放 Winsock 资源
    WSACleanup();

    // 返回 0 表示程序正常结束
    return 0;
}
