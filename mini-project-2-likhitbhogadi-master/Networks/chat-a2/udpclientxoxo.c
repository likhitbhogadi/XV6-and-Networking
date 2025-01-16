// xoxo_client_udp.c
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define PORT 9999
#define BUFFER_SIZE 512
#define ip "192.168.73.165"

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    struct pollfd fds[2];
    socklen_t addrlen = sizeof(server_addr);

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0){
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    // Convert IPv4 address from text to binary form
    if(inet_pton(AF_INET, ip, &server_addr.sin_addr)<=0){
        perror("Invalid address/ Address not supported");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Send JOIN message to server
    snprintf(buffer, BUFFER_SIZE, "JOIN\n");
    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, addrlen);
    printf("Sent JOIN request to server.\n");

    // Set up poll
    fds[0].fd = 0; // stdin
    fds[0].events = POLLIN;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN;

    // Main loop
    while(1){
        int ret = poll(fds, 2, -1);
        if(ret < 0){
            perror("poll");
            break;
        }

        // Check if there's data from server
        if(fds[1].revents & POLLIN){
            memset(buffer, 0, BUFFER_SIZE);
            struct sockaddr_in recv_addr;
            socklen_t recv_addrlen = sizeof(recv_addr);
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE-1, 0, (struct sockaddr*)&recv_addr, &recv_addrlen);
            if(n < 0){
                perror("recvfrom");
                break;
            }
            buffer[n] = '\0';
            printf("%s", buffer);

            // Handle specific prompts
            if(strstr(buffer, "it's your turn") != NULL){
                // Wait for user input to send move
                printf("Enter your move (row and column, e.g., 1 1): ");
                memset(buffer, 0, BUFFER_SIZE);
                if(fgets(buffer, BUFFER_SIZE-1, stdin) != NULL){
                    // Remove newline character
                    buffer[strcspn(buffer, "\n")] = 0;
                    // Send move to server
                    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, addrlen);
                }
            }

            if(strstr(buffer, "Do you want to play again?") != NULL){
                // Prompt user to enter yes or no
                printf("Enter yes or no: ");
                memset(buffer, 0, BUFFER_SIZE);
                if(fgets(buffer, BUFFER_SIZE-1, stdin) != NULL){
                    // Remove newline character
                    buffer[strcspn(buffer, "\n")] = 0;
                    // Send response to server
                    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, addrlen);
                }
            }

            // // Optionally handle other messages like "WELCOME", "Game over", etc.
            // if(strstr(buffer, "Game over") != NULL || strstr(buffer, "Disconnecting") != NULL){
            //     printf("Game ended by server.\n");
            //     break;
            // }
        }

        // Check if there's user input (Not strictly necessary as input is handled after prompts)
        /*
        if(fds[0].revents & POLLIN){
            memset(buffer, 0, BUFFER_SIZE);
            if(fgets(buffer, BUFFER_SIZE-1, stdin) != NULL){
                // Send input to server
                sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&server_addr, addrlen);
            }
        }
        */
    }

    // Close socket
    close(sockfd);
    return 0;
}
