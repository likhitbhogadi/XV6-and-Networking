// xoxo_client.c
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
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    struct pollfd fds[2];

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Define server address
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, ip, &address.sin_addr)<=0){
        perror("Invalid address/ Address not supported");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if(connect(sockfd, (struct sockaddr*)&address, sizeof(address)) < 0){
        perror("Connection Failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

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
            int n = recv(sockfd, buffer, BUFFER_SIZE-1, 0);
            if(n <= 0){
                printf("Disconnected from server.\n");
                break;
            }
            printf("%s", buffer);

            // If server is prompting for a move, allow user input
            if(strstr(buffer, "your turn") != NULL){
                // Wait for user input
                memset(buffer, 0, BUFFER_SIZE);
                if(fgets(buffer, BUFFER_SIZE-1, stdin) != NULL){
                    send(sockfd, buffer, strlen(buffer), 0);
                }
            }

            // If server is asking to play again, handle it
            if(strstr(buffer, "Do you want to play again?") != NULL){
                printf("Enter yes or no: ");
                memset(buffer, 0, BUFFER_SIZE);
                if(fgets(buffer, BUFFER_SIZE-1, stdin) != NULL){
                    // Remove newline character
                    buffer[strcspn(buffer, "\n")] = 0;
                    send(sockfd, buffer, strlen(buffer), 0);
                }
            }
        }

        // Check if there's user input (not strictly necessary since we handle input after prompts)
        /*
        if(fds[0].revents & POLLIN){
            memset(buffer, 0, BUFFER_SIZE);
            if(fgets(buffer, BUFFER_SIZE-1, stdin) != NULL){
                send(sockfd, buffer, strlen(buffer), 0);
            }
        }
        */
    }

    // Close socket
    close(sockfd);
    return 0;
}
