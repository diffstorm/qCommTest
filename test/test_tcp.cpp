#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

// Constants
const std::string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 6666;
const int SEND_RECEIVE_TIMEOUT_MS = 500; // 500 ms timeout for sending/receiving
const int TEST_INDEX_MAX = 400;
const int FAIL_TRY_MAX = 100;

// The initial data sequence to start communication
const std::vector<unsigned char> START_DATA = {0x00, 0x00, 0x01, 0x00, 0xd2, 0x02, 0xef, 0x8d};

#ifdef _WIN32
void init_winsock() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        throw std::runtime_error("WSAStartup failed: " + std::to_string(iResult));
    }
}

void cleanup_winsock() {
    WSACleanup();
}
#else
void init_winsock() {}
void cleanup_winsock() {}
#endif

int connect_to_server(const std::string& ip, int port, int timeout_ms) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
        perror("setsockopt SO_RCVTIMEO");
        close(sock);
        return -1;
    }
    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
        perror("setsockopt SO_SNDTIMEO");
        close(sock);
        return -1;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &(server.sin_addr));

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        // perror("connect"); // connect will often fail with EINPROGRESS if non-blocking, but we are blocking
        close(sock);
        return -1;
    }

    std::cout << "Connected to server at " << ip << ":" << port << std::endl;
    return sock;
}

std::vector<unsigned char> recv_all(int sock, int length) {
    std::vector<unsigned char> data(length);
    int total_received = 0;
    while (total_received < length) {
        int received = recv(sock, reinterpret_cast<char*>(data.data()) + total_received, length - total_received, 0);
        if (received <= 0) {
            throw std::runtime_error("Socket connection broken or timeout");
        }
        total_received += received;
    }
    return data;
}

std::vector<unsigned char> send_receive(int sock, const std::vector<unsigned char>& send_data, int expected_recv_length) {
    if (send_data.empty()) {
        std::cout << "No data to send" << std::endl;
        return {};
    }

    try {
        if (send(sock, reinterpret_cast<const char*>(send_data.data()), send_data.size(), 0) < 0) {
            perror("send");
            return {};
        }
        std::cout << "Sent " << send_data.size() << " bytes" << std::endl;

        std::vector<unsigned char> received_data = recv_all(sock, expected_recv_length);
        std::cout << "Received " << received_data.size() << " bytes" << std::endl;

        return received_data;
    } catch (const std::runtime_error& e) {
        std::cerr << "Communication error: " << e.what() << std::endl;
        return {};
    }
}

int main() {
    init_winsock();

    int test_index = 1;
    int fail_try = FAIL_TRY_MAX;
    std::vector<unsigned char> response;

    int sock = -1;
    while (fail_try > 0) {
        sock = connect_to_server(SERVER_IP, SERVER_PORT, SEND_RECEIVE_TIMEOUT_MS);
        if (sock == -1) {
            fail_try--;
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            continue;
        }

        int expected_recv_length = 7 + test_index;
        response = send_receive(sock, START_DATA, expected_recv_length);
        if (!response.empty() && response.size() == expected_recv_length) {
            std::cout << "Test started successfully" << std::endl;
            test_index++;
            break;
        } else {
            fail_try--;
            close(sock);
            sock = -1;
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    }

    if (fail_try == 0) {
        std::cout << "Failed to start communication test" << std::endl;
        cleanup_winsock();
        return 1;
    }

    long long total_time_us = 0;
    while (test_index <= TEST_INDEX_MAX && fail_try > 0) {
        int expected_rx_length = 7 + test_index;
        std::vector<unsigned char> send_buffer = response; // Use previous response as next send data

        auto start_time = std::chrono::high_resolution_clock::now();
        response = send_receive(sock, send_buffer, expected_rx_length);
        auto end_time = std::chrono::high_resolution_clock::now();
        long long elapsed_time_us = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

        if (!response.empty() && response.size() == expected_rx_length) {
            std::cout << "Success " << test_index << ", Time: " << elapsed_time_us << " us" << std::endl;
            total_time_us += elapsed_time_us;
            test_index++;
        } else {
            fail_try--;
            std::cout << "Fail " << fail_try << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    if (test_index > TEST_INDEX_MAX && fail_try > 0) {
        std::cout << "Test passed" << std::endl;
    } else {
        std::cout << "Test failed" << std::endl;
    }

    if (sock != -1) {
        close(sock);
    }
    cleanup_winsock();

    return 0;
}
