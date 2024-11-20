#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>

// Link with ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024

void parseConfig(const std::string& filename, std::string& ip, int& port) {
    std::ifstream configFile(filename);
    if (!configFile) {
        std::cerr << "Failed to open configuration file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            if (key == "ip") {
                ip = value;
            }
            else if (key == "port") {
                port = std::stoi(value);
            }
        }
    }

    if (ip.empty() || port == 0) {
        std::cerr << "Invalid configuration file format." << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    std::string configFile = "config.txt"; // Default configuration file
    if (argc > 1) {
        configFile = argv[1]; // Use the provided configuration file
    }

    /*char sztest[100] = "test string";
    std::cout << sztest << std::endl;*/

    // Parse configuration
    std::string ipAddress;
    int port = 0;
    parseConfig(configFile, ipAddress, port);

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return EXIT_FAILURE;
    }

    // Create a UDP socket
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Bind the socket to the specified IP and port
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ipAddress.c_str(), &serverAddr.sin_addr);
    serverAddr.sin_port = htons(port);

    if (bind(udpSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(udpSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    std::cout << "UDP server is listening on " << ipAddress << ":" << port << std::endl;

    // Listen for incoming messages
    char buffer[BUFFER_SIZE];
    
    sockaddr_in clientAddr{};
    int clientAddrLen = sizeof(clientAddr);

    while (true) {
        int bytesReceived = recvfrom(udpSocket, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrLen);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recvfrom failed with error: " << WSAGetLastError() << std::endl;
            break;
        }

        buffer[bytesReceived] = '\0'; // Null-terminate the received message
        char clientIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);

        std::cout << "Received message from " << clientIp << ":" << ntohs(clientAddr.sin_port) << " -> " << buffer << std::endl;

        
        
    }

    // Clean up
    closesocket(udpSocket);
    WSACleanup();
    return 0;
}
