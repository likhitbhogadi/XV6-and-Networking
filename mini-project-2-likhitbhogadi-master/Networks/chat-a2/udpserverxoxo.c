// xoxo_server_udp.c
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define PORT 9999
#define MAX_CLIENTS 2
#define BUFFER_SIZE 512

// Function to initialize the board
void init_board(char board[3][3]) {
    memset(board, ' ', sizeof(char) * 9);
}

// Function to print the board as a string
void board_to_string(char board[3][3], char *buffer) {
    sprintf(buffer,
            " %c | %c | %c \n---+---+---\n %c | %c | %c \n---+---+---\n %c | %c | %c \n",
            board[0][0], board[0][1], board[0][2],
            board[1][0], board[1][1], board[1][2],
            board[2][0], board[2][1], board[2][2]);
}

// Function to check for a win
int check_win(char board[3][3], char symbol) {
    // Check rows, columns, and diagonals
    for(int i=0;i<3;i++) {
        if(board[i][0]==symbol && board[i][1]==symbol && board[i][2]==symbol)
            return 1;
        if(board[0][i]==symbol && board[1][i]==symbol && board[2][i]==symbol)
            return 1;
    }
    if(board[0][0]==symbol && board[1][1]==symbol && board[2][2]==symbol)
        return 1;
    if(board[0][2]==symbol && board[1][1]==symbol && board[2][0]==symbol)
        return 1;
    return 0;
}

// Function to check for a draw
int check_draw(char board[3][3]) {
    for(int i=0;i<3;i++) {
        for(int j=0;j<3;j++) {
            if(board[i][j] == ' ')
                return 0;
        }
    }
    return 1;
}

// Function to compare two sockaddr_in structures
int compare_addr(struct sockaddr_in a, struct sockaddr_in b){
    return (a.sin_family == b.sin_family) &&
           (a.sin_port == b.sin_port) &&
           (a.sin_addr.s_addr == b.sin_addr.s_addr);
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen = sizeof(client_addr);
    char board[3][3];
    int current_player = 0; // 0 or 1
    char symbols[2] = {'X', 'O'};
    char buffer[BUFFER_SIZE];
    int play_again[2] = {0, 0};
    struct sockaddr_in clients[MAX_CLIENTS];
    int client_count = 0;

    // Initialize board
    init_board(board);

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket to PORT
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP Server listening on port %d\n", PORT);
    printf("Waiting for %d players to join...\n", MAX_CLIENTS);

    // Register clients
    while(client_count < MAX_CLIENTS){
        memset(buffer, 0, BUFFER_SIZE);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE-1, 0, (struct sockaddr*)&client_addr, &addrlen);
        if(n < 0){
            perror("recvfrom");
            continue;
        }

        buffer[n] = '\0';

        // Expecting "JOIN" message
        if(strncmp(buffer, "JOIN", 4) == 0){
            // Check if client is already registered
            int already_registered = 0;
            for(int i=0; i < client_count; i++){
                if(compare_addr(client_addr, clients[i])){
                    already_registered = 1;
                    break;
                }
            }
            if(!already_registered && client_count < MAX_CLIENTS){
                clients[client_count] = client_addr;
                // Assign symbol
                snprintf(buffer, BUFFER_SIZE, "WELCOME Player %d! You are '%c'\n", client_count+1, symbols[client_count]);
                sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&client_addr, addrlen);
                printf("Player %d joined from %s:%d\n", client_count+1,
                       inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                client_count++;
            }
            // If already registered, ignore or resend welcome
            else if(already_registered){
                snprintf(buffer, BUFFER_SIZE, "WELCOME Player %d! You are '%c'\n", 
                         client_count <= MAX_CLIENTS ? client_count : MAX_CLIENTS, 
                         symbols[client_count <= MAX_CLIENTS ? client_count : MAX_CLIENTS -1]);
                sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&client_addr, addrlen);
            }
            // If maximum clients reached
            else{
                snprintf(buffer, BUFFER_SIZE, "Server full. Try again later.\n");
                sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&client_addr, addrlen);
            }
        }
    }

    // Notify both clients that the game is starting
    snprintf(buffer, BUFFER_SIZE, "Both players connected. Game starts!\n");
    for(int i=0; i < MAX_CLIENTS; i++) {
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clients[i], addrlen);
    }

    // Main game loop
    while(1){
        // Send the current board to both clients
        char board_str[BUFFER_SIZE];
        board_to_string(board, board_str);
        for(int i=0; i < MAX_CLIENTS; i++) {
            sendto(sockfd, board_str, strlen(board_str), 0, (struct sockaddr*)&clients[i], addrlen);
        }

        // Prompt the current player to make a move
        snprintf(buffer, BUFFER_SIZE, "Player %d ('%c'), it's your turn. Enter row and column (e.g., 1 1):\n", 
                 current_player+1, symbols[current_player]);
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clients[current_player], addrlen);

        // Receive move from current player
        memset(buffer, 0, BUFFER_SIZE);
        struct sockaddr_in move_addr;
        socklen_t move_addrlen = sizeof(move_addr);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE-1, 0, (struct sockaddr*)&move_addr, &move_addrlen);
        if(n < 0){
            perror("recvfrom");
            continue;
        }

        buffer[n] = '\0';

        // Verify that the move is from the current player
        if(!compare_addr(move_addr, clients[current_player])){
            // Optionally, inform the sender that it's not their turn
            snprintf(buffer, BUFFER_SIZE, "It's not your turn.\n");
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&move_addr, move_addrlen);
            continue;
        }

        // Parse move
        int row, col;
        if(sscanf(buffer, "%d %d", &row, &col) != 2){
            snprintf(buffer, BUFFER_SIZE, "Invalid input. Please enter row and column numbers.\n");
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clients[current_player], addrlen);
            continue;
        }

        // Validate move
        if(row < 1 || row >3 || col <1 || col >3){
            snprintf(buffer, BUFFER_SIZE, "Invalid position. Rows and columns are 1-3.\n");
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clients[current_player], addrlen);
            continue;
        }

        row--; col--; // Convert to 0-indexed

        if(board[row][col] != ' '){
            snprintf(buffer, BUFFER_SIZE, "Cell already occupied. Choose another.\n");
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clients[current_player], addrlen);
            continue;
        }

        // Make the move
        board[row][col] = symbols[current_player];

        // Check for win
        if(check_win(board, symbols[current_player])){
            board_to_string(board, board_str);
            for(int i=0; i < MAX_CLIENTS; i++) {
                sendto(sockfd, board_str, strlen(board_str), 0, (struct sockaddr*)&clients[i], addrlen);
            }
            snprintf(buffer, BUFFER_SIZE, "Player %d Wins!\n", current_player+1);
            for(int i=0; i < MAX_CLIENTS; i++) {
                sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clients[i], addrlen);
            }

            // Ask to play again
            for(int i=0; i < MAX_CLIENTS; i++) {
                snprintf(buffer, BUFFER_SIZE, "Do you want to play again? (yes/no):\n");
                sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clients[i], addrlen);
            }

            // Receive responses
            for(int i=0; i < MAX_CLIENTS; i++) {
                memset(buffer, 0, BUFFER_SIZE);
                struct sockaddr_in response_addr;
                socklen_t response_addrlen = sizeof(response_addr);
                n = recvfrom(sockfd, buffer, BUFFER_SIZE-1, 0, (struct sockaddr*)&response_addr, &response_addrlen);
                if(n < 0){
                    perror("recvfrom");
                    play_again[i] = 0;
                    continue;
                }
                buffer[n] = '\0';

                // Verify response is from the correct client
                if(!compare_addr(response_addr, clients[i])){
                    // Optionally, handle unexpected responses
                    play_again[i] = 0;
                    continue;
                }

                if(strncmp(buffer, "no", 2) == 0){
                    play_again[i] = 0;
                }
                else{
                    play_again[i] = 1;
                }
            }

            if(play_again[0] && play_again[1]){
                // Reset board and continue
                init_board(board);
                current_player = 0;
                snprintf(buffer, BUFFER_SIZE, "Both players agreed to play again. Starting a new game!\n");
                for(int i=0; i < MAX_CLIENTS; i++) {
                    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clients[i], addrlen);
                }
                continue;
            }
            else{
                // Inform and close connections
                for(int i=0; i < MAX_CLIENTS; i++) {
                    if(play_again[i]){
                        snprintf(buffer, BUFFER_SIZE, "Opponent does not want to play again. Disconnecting.\n");
                        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clients[i], addrlen);
                    }
                    // UDP is connectionless; no need to close sockets for clients
                }
                printf("Game ended. No longer accepting new players.\n");
                break;
            }
        }

        // Check for draw
        if(check_draw(board)){
            board_to_string(board, board_str);
            for(int i=0; i < MAX_CLIENTS; i++) {
                sendto(sockfd, board_str, strlen(board_str), 0, (struct sockaddr*)&clients[i], addrlen);
            }
            snprintf(buffer, BUFFER_SIZE, "It's a Draw!\n");
            for(int i=0; i < MAX_CLIENTS; i++) {
                sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clients[i], addrlen);
            }

            // Ask to play again
            for(int i=0; i < MAX_CLIENTS; i++) {
                snprintf(buffer, BUFFER_SIZE, "Do you want to play again? (yes/no):\n");
                sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clients[i], addrlen);
            }

            // Receive responses
            for(int i=0; i < MAX_CLIENTS; i++) {
                memset(buffer, 0, BUFFER_SIZE);
                struct sockaddr_in response_addr;
                socklen_t response_addrlen = sizeof(response_addr);
                n = recvfrom(sockfd, buffer, BUFFER_SIZE-1, 0, (struct sockaddr*)&response_addr, &response_addrlen);
                if(n < 0){
                    perror("recvfrom");
                    play_again[i] = 0;
                    continue;
                }
                buffer[n] = '\0';

                // Verify response is from the correct client
                if(!compare_addr(response_addr, clients[i])){
                    // Optionally, handle unexpected responses
                    play_again[i] = 0;
                    continue;
                }

                if(strncmp(buffer, "no", 2) == 0){
                    play_again[i] = 0;
                }
                else{
                    play_again[i] = 1;
                }
            }

            if(play_again[0] && play_again[1]){
                // Reset board and continue
                init_board(board);
                current_player = 0;
                snprintf(buffer, BUFFER_SIZE, "Both players agreed to play again. Starting a new game!\n");
                for(int i=0; i < MAX_CLIENTS; i++) {
                    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clients[i], addrlen);
                }
                continue;
            }
            else{
                // Inform and close connections
                for(int i=0; i < MAX_CLIENTS; i++) {
                    if(play_again[i]){
                        snprintf(buffer, BUFFER_SIZE, "Opponent does not want to play again. Disconnecting.\n");
                        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clients[i], addrlen);
                    }
                    // UDP is connectionless; no need to close sockets for clients
                }
                printf("Game ended. No longer accepting new players.\n");
                break;
            }
        }

        // Switch turn
        current_player = 1 - current_player;
    }

    // Close server socket
    close(sockfd);
    return 0;
}
