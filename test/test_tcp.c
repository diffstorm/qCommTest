#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6666
#define SEND_RECEIVE_TIMEOUT_S 0
#define SEND_RECEIVE_TIMEOUT_US 500000
#define TEST_INDEX_MAX 400
#define FAIL_TRY_MAX 100

const unsigned char START_DATA[] = {0x00, 0x00, 0x01, 0x00, 0xd2, 0x02, 0xef, 0x8d};

int connect_to_server(const char *ip, int port, struct timeval timeout) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        close(sock);
        return -1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    printf("Connected to server at %s:%d\n", ip, port);
    return sock;
}

int recv_all(int sock, unsigned char *buffer, int length) {
    int total_received = 0;
    while (total_received < length) {
        int received = recv(sock, buffer + total_received, length - total_received, 0);
        if (received <= 0) {
            return -1;
        }
        total_received += received;
    }
    return total_received;
}

int send_receive(int sock, const unsigned char *send_data, int send_len, unsigned char *recv_buffer, int recv_len) {
    if (send(sock, send_data, send_len, 0) < 0) {
        perror("send");
        return -1;
    }
    printf("Sent %d bytes\n", send_len);

    int received = recv_all(sock, recv_buffer, recv_len);
    if (received > 0) {
        printf("Received %d bytes\n", received);
    }
    return received;
}

int main() {
    int test_index = 1;
    int fail_try = FAIL_TRY_MAX;
    unsigned char *response = NULL;

    struct timeval timeout;
    timeout.tv_sec = SEND_RECEIVE_TIMEOUT_S;
    timeout.tv_usec = SEND_RECEIVE_TIMEOUT_US;

    int sock = -1;
    while (fail_try > 0) {
        sock = connect_to_server(SERVER_IP, SERVER_PORT, timeout);
        if (sock == -1) {
            fail_try--;
            usleep(2000);
            continue;
        }

        int expected_recv_length = 7 + test_index;
        response = (unsigned char *)malloc(expected_recv_length);
        if (send_receive(sock, START_DATA, sizeof(START_DATA), response, expected_recv_length) == expected_recv_length) {
            printf("Test started successfully\n");
            test_index++;
            break;
        } else {
            fail_try--;
            close(sock);
            free(response);
            response = NULL;
            usleep(2000);
        }
    }

    if (fail_try == 0) {
        printf("Failed to start communication test\n");
        return 1;
    }

    long long total_time = 0;
    while (test_index <= TEST_INDEX_MAX && fail_try > 0) {
        int expected_rx_length = 7 + test_index;
        unsigned char *send_buffer = response;
        response = (unsigned char *)malloc(expected_rx_length);

        struct timespec start_time, end_time;
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        int received = send_receive(sock, send_buffer, 7 + test_index - 1, response, expected_rx_length);
        clock_gettime(CLOCK_MONOTONIC, &end_time);

        long long elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000000LL + (end_time.tv_nsec - start_time.tv_nsec) / 1000LL;

        free(send_buffer);

        if (received == expected_rx_length) {
            printf("Success %d, Time: %lld us\n", test_index, elapsed_time);
            total_time += elapsed_time;
            test_index++;
        } else {
            fail_try--;
            printf("Fail %d\n", fail_try);
        }
        usleep(2000);
    }

    if (test_index > TEST_INDEX_MAX && fail_try > 0) {
        printf("Test passed\n");
    } else {
        printf("Test failed\n");
    }

    if (response) {
        free(response);
    }
    close(sock);

    return 0;
}
