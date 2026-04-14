// 引入标准输入输出头文件，用于 printf、fgets 等函数
#include <stdio.h>

// 引入标准库头文件，用于程序通用控制
#include <stdlib.h>

// 引入字符串处理头文件，用于 memset、strlen、strncmp 等函数
#include <string.h>

// 引入 Winsock2 头文件，这是 Windows 下 socket 编程的核心头文件
#include <winsock2.h>

// 引入 Windows socket 扩展头文件，提供 inet_pton 等函数声明
#include <ws2tcpip.h>

// 告诉编译器自动链接 Ws2_32.lib 网络库
#pragma comment(lib, "Ws2_32.lib")

// 定义服务器的 IP 地址，这里使用本机回环地址
#define SERVER_IP "127.0.0.1"

// 定义服务器端口，必须和服务器程序中的端口保持一致
#define SERVER_PORT 8888

// 定义收发缓冲区大小
#define BUFFER_SIZE 1024

// main 函数是客户端程序入口
int main(void)
{
    // WSADATA 结构体用于保存 Winsock 初始化后的相关信息
    WSADATA wsaData;

    // clientSock 是客户端套接字，用于连接服务器并进行收发
    SOCKET clientSock = INVALID_SOCKET;

    // serverAddr 用于保存目标服务器的 IP 和端口信息
    struct sockaddr_in serverAddr;

    // buffer 用于保存用户输入和服务器返回的数据
    char buffer[BUFFER_SIZE];

    // sendLen 用于保存即将发送的数据长度
    int sendLen;

    // recvLen 用于保存实际接收到的数据长度
    int recvLen;

    // 初始化 Winsock 2.2 运行环境
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        // 如果初始化失败，打印错误提示
        printf("WSAStartup failed.\n");

        // 返回 1 表示程序异常结束
        return 1;
    }

    // 创建一个 IPv4、TCP 类型的客户端套接字
    clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // 判断套接字是否创建失败
    if (clientSock == INVALID_SOCKET)
    {
        // 打印 socket 创建失败的错误码
        printf("socket failed. error = %d\n", WSAGetLastError());

        // 清理 Winsock 资源
        WSACleanup();

        // 返回 1 表示程序异常结束
        return 1;
    }

    // 将服务器地址结构体清零，避免出现未初始化数据
    memset(&serverAddr, 0, sizeof(serverAddr));

    // 设置地址族为 IPv4
    serverAddr.sin_family = AF_INET;

    // 设置服务器端口，并将其转换成网络字节序
    serverAddr.sin_port = htons(SERVER_PORT);

    // 将字符串形式的 IP 地址转换成二进制网络地址
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) != 1)
    {
        // 如果 IP 转换失败，打印提示
        printf("inet_pton failed.\n");

        // 关闭客户端套接字
        closesocket(clientSock);

        // 清理 Winsock 资源
        WSACleanup();

        // 返回 1 表示程序异常结束
        return 1;
    }

    // 主动连接服务器
    if (connect(clientSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        // 如果连接失败，打印错误码
        printf("connect failed. error = %d\n", WSAGetLastError());

        // 关闭客户端套接字
        closesocket(clientSock);

        // 清理 Winsock 资源
        WSACleanup();

        // 返回 1 表示程序异常结束
        return 1;
    }

    // 打印提示，说明客户端已经连接到服务器
    printf("Connected to server %s:%d\n", SERVER_IP, SERVER_PORT);

    // 打印使用说明，提示用户输入消息
    printf("Type message and press Enter.\n");

    // 打印退出说明，提示用户输入 quit 结束程序
    printf("Input \"quit\" to exit.\n");

    // 进入循环，不断读取用户输入并发送到服务器
    while (1)
    {
        // 打印输入提示符
        printf("You: ");

        // 每次读取前先清空缓冲区
        memset(buffer, 0, sizeof(buffer));

        // 从标准输入读取一行文本到缓冲区中
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
            // 如果读取失败或输入结束，则退出循环
            break;
        }

        // 计算当前输入字符串的长度
        sendLen = (int)strlen(buffer);

        // 如果长度为 0，说明没有有效输入，直接进入下一轮
        if (sendLen == 0)
        {
            // 继续下一轮循环
            continue;
        }

        // 如果用户输入 quit，则结束客户端程序
        if (strncmp(buffer, "quit", 4) == 0)
        {
            // 打印退出提示
            printf("Client will exit.\n");

            // 跳出循环
            break;
        }

        // 将用户输入发送给服务器
        if (send(clientSock, buffer, sendLen, 0) == SOCKET_ERROR)
        {
            // 如果发送失败，打印错误码
            printf("send failed. error = %d\n", WSAGetLastError());

            // 跳出循环
            break;
        }

        // 接收服务器发回的数据
        recvLen = recv(clientSock, buffer, sizeof(buffer) - 1, 0);

        // 如果成功收到服务器数据
        if (recvLen > 0)
        {
            // 在数据末尾补上字符串结束符，方便打印
            buffer[recvLen] = '\0';

            // 打印服务器返回的内容
            printf("Server: %s", buffer);
        }
        // 如果 recv 返回 0，表示服务器已经关闭连接
        else if (recvLen == 0)
        {
            // 打印提示
            printf("Server closed the connection.\n");

            // 跳出循环
            break;
        }
        // 如果 recv 返回负值，表示接收时发生错误
        else
        {
            // 打印错误码
            printf("recv failed. error = %d\n", WSAGetLastError());

            // 跳出循环
            break;
        }
    }

    // 关闭客户端套接字
    closesocket(clientSock);

    // 清理 Winsock 资源
    WSACleanup();

    // 返回 0 表示程序正常结束
    return 0;
}
