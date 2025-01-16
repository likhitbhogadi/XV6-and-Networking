#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#define PORT 8080
#define CHUNK_SIZE 4
#define MAX_CHUNKS 100
#define TIMEOUT 100000  // 100ms timeout

struct packet {
    int seq_num;
    int total_chunks;
    char data[CHUNK_SIZE];
};

int received_chunks = 0;
struct packet pkt;
char **received_data = NULL;

fd_set readfds;
struct timeval tv;

int sending_mode = 0; // if 1 it will send else receive for 1

void send_data(int sockfd, struct sockaddr_in *server_addr, socklen_t addr_len, char *message) {
    int total_chunks = (strlen(message) + CHUNK_SIZE - 1) / CHUNK_SIZE;
    struct packet pkt;
    int *acked_chunks = calloc(total_chunks, sizeof(int));
    fd_set readfds;
    struct timeval tv;

    for (int i = 0; i < total_chunks; ++i) {
        pkt.seq_num = i;
        pkt.total_chunks = total_chunks;
        strncpy(pkt.data, message + i * CHUNK_SIZE, CHUNK_SIZE);

        // Send packet
        // printf("total no of chunks = %d\n", pkt.total_chunks);
        sendto(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)server_addr, addr_len);

        // Set up timeout for ACK
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = TIMEOUT;

        int activity = select(sockfd + 1, &readfds, NULL, NULL, &tv);
        while (activity > 0 && FD_ISSET(sockfd, &readfds)) {
            int ack;
            recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)server_addr, &addr_len);
            if (ack == i) {
                acked_chunks[i] = 1;
                printf("Received ACK for chunk %d\n", ack);
            }
            activity = select(sockfd + 1, &readfds, NULL, NULL, &tv);
        }

        // Retransmit chunks that didn't get ACKed
        for (int j = 0; j < i+1; ++j) {
            if (!acked_chunks[j]) {
                pkt.seq_num = j;
                strncpy(pkt.data, message + j * CHUNK_SIZE, CHUNK_SIZE);
                sendto(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)server_addr, addr_len);
                printf("Retransmitting chunk %d\n", j);
            }
        }
    }

    free(acked_chunks);
}

int receive_data(int sockfd, struct sockaddr_in *client_addr, socklen_t addr_len) {

    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = TIMEOUT;

    int activity = select(sockfd + 1, &readfds, NULL, NULL, &tv);
    if (activity == -1) {
        perror("select error");
        exit(EXIT_FAILURE);
    }

    if (FD_ISSET(sockfd, &readfds)) {
        if (recvfrom(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)client_addr, &addr_len) > 0) {
            if (received_chunks == 0) {
                // Initialize storage for all chunks
                received_data = malloc(pkt.total_chunks * sizeof(char *));
                for (int i = 0; i < pkt.total_chunks; ++i) {
                    received_data[i] = NULL;
                }
            }

            if (received_data[pkt.seq_num] == NULL) {
                received_data[pkt.seq_num] = strdup(pkt.data);
                received_chunks++;
            }

            // Send ACK for the received chunk
            sendto(sockfd, &pkt.seq_num, sizeof(pkt.seq_num), 0, (struct sockaddr *)client_addr, addr_len);
            printf("sent ack for chunk %d\n",pkt.seq_num);

            if (received_chunks == pkt.total_chunks) {
                // printf("total no of chunks = %d\n",pkt.total_chunks);
                printf("All chunks received. \nData:\n");
                for (int i = 0; i < pkt.total_chunks; ++i) {
                    printf("%s", received_data[i]);
                    free(received_data[i]);
                }
                free(received_data);
                printf("\n\n");
                received_chunks=0;
                return 1;
            }
        }
    }
    return 0;
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // printf("Server is waiting for data...\n");

    while(1){
        if(sending_mode){
            printf("\nEnter message to send : ");
            char buffer[CHUNK_SIZE*MAX_CHUNKS];
            fgets(buffer, CHUNK_SIZE*MAX_CHUNKS, stdin);
            buffer[strcspn(buffer, "\n")] = 0;  // Remove newline
            send_data(sockfd, &client_addr, addr_len, buffer);
            sending_mode = 0;
            // sleep(2);  // Sleep for 10 milliseconds (0.1 seconds)
        }
        else{
            printf("\nServer is waiting for data...\n\n");
            while( receive_data(sockfd, &client_addr, addr_len) == 0 ){
                // do ntg;
            }
            sending_mode = 1;
        }
    }

    close(sockfd);
    return 0;
}
