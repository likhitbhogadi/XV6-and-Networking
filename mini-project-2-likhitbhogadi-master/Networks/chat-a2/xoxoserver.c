// xoxo_server.c
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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

int main() {
    int sockfd, clientfd[2]; // used to store fd of socket and 2 clients
    struct sockaddr_in address; // structure for ipv4 address
    socklen_t addrlen = sizeof(address); 
    char board[3][3]; 
    int current_player = 0; // 0 or 1
    char symbols[2] = {'X', 'O'};
    char buffer[BUFFER_SIZE];
    int play_again[2] = {0, 0};
    
    // Initialize board
    init_board(board);

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(sockfd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(sockfd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Accept two clients
    for(int i=0; i < MAX_CLIENTS; i++) {
        clientfd[i] = accept(sockfd, (struct sockaddr*)&address, &addrlen);
        if (clientfd[i] < 0) {
            perror("Accept failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        // Assign symbols
        snprintf(buffer, BUFFER_SIZE, "WELCOME Player %d! You are '%c'\n", i+1, symbols[i]);
        send(clientfd[i], buffer, strlen(buffer), 0);
    }

    // Notify both clients that the game is starting
    snprintf(buffer, BUFFER_SIZE, "Both players connected. Game starts!\n");
    for(int i=0; i < MAX_CLIENTS; i++) {
        send(clientfd[i], buffer, strlen(buffer), 0);
    }

    // Main game loop
    while(1) {
        // Send the current board
        char board_str[BUFFER_SIZE];
        board_to_string(board, board_str);
        for(int i=0; i < MAX_CLIENTS; i++) {
            send(clientfd[i], board_str, strlen(board_str), 0);
        }

        // Prompt the current player to make a move
        snprintf(buffer, BUFFER_SIZE, "Player %d ('%c'), it's your turn. Enter row and column (e.g., 1 1):\n", 
                 current_player+1, symbols[current_player]);
        send(clientfd[current_player], buffer, strlen(buffer), 0);

        // Receive move
        memset(buffer, 0, BUFFER_SIZE);
        int n = recv(clientfd[current_player], buffer, BUFFER_SIZE-1, 0);
        if(n <= 0){
            printf("Player %d disconnected.\n", current_player+1);
            // Inform the other player
            snprintf(buffer, BUFFER_SIZE, "Player %d has disconnected. Game over.\n", current_player+1);
            send(clientfd[1 - current_player], buffer, strlen(buffer), 0);
            break;
        }

        // Parse move
        int row, col;
        if(sscanf(buffer, "%d %d", &row, &col) != 2){
            snprintf(buffer, BUFFER_SIZE, "Invalid input. Please enter row and column numbers.\n");
            send(clientfd[current_player], buffer, strlen(buffer), 0);
            continue;
        }

        // Validate move
        if(row < 1 || row >3 || col <1 || col >3){
            snprintf(buffer, BUFFER_SIZE, "Invalid position. Rows and columns are 1-3.\n");
            send(clientfd[current_player], buffer, strlen(buffer), 0);
            continue;
        }

        row--; col--; // Convert to 0-indexed

        if(board[row][col] != ' '){
            snprintf(buffer, BUFFER_SIZE, "Cell already occupied. Choose another.\n");
            send(clientfd[current_player], buffer, strlen(buffer), 0);
            continue;
        }

        // Make the move
        board[row][col] = symbols[current_player];

        // Check for win
        if(check_win(board, symbols[current_player])){
            board_to_string(board, board_str);
            for(int i=0; i < MAX_CLIENTS; i++) {
                send(clientfd[i], board_str, strlen(board_str), 0);
            }
            snprintf(buffer, BUFFER_SIZE, "Player %d Wins!\n", current_player+1);
            for(int i=0; i < MAX_CLIENTS; i++) {
                send(clientfd[i], buffer, strlen(buffer), 0);
            }

            // Ask to play again
            for(int i=0; i < MAX_CLIENTS; i++) {
                snprintf(buffer, BUFFER_SIZE, "Do you want to play again? (yes/no):\n");
                send(clientfd[i], buffer, strlen(buffer), 0);
            }

            // Receive responses
            for(int i=0; i < MAX_CLIENTS; i++) {
                memset(buffer, 0, BUFFER_SIZE);
                n = recv(clientfd[i], buffer, BUFFER_SIZE-1, 0);
                if(n <= 0 || strncmp(buffer, "no", 2) == 0){
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
                continue;
            }
            else{
                // Inform and close connections
                for(int i=0; i < MAX_CLIENTS; i++) {
                    if(play_again[i]){
                        snprintf(buffer, BUFFER_SIZE, "Opponent does not want to play again. Disconnecting.\n");
                        send(clientfd[i], buffer, strlen(buffer), 0);
                    }
                    close(clientfd[i]);
                }
                break;
            }
        }

        // Check for draw
        if(check_draw(board)){
            board_to_string(board, board_str);
            for(int i=0; i < MAX_CLIENTS; i++) {
                send(clientfd[i], board_str, strlen(board_str), 0);
            }
            snprintf(buffer, BUFFER_SIZE, "It's a Draw!\n");
            for(int i=0; i < MAX_CLIENTS; i++) {
                send(clientfd[i], buffer, strlen(buffer), 0);
            }

            // Ask to play again
            for(int i=0; i < MAX_CLIENTS; i++) {
                snprintf(buffer, BUFFER_SIZE, "Do you want to play again? (yes/no):\n");
                send(clientfd[i], buffer, strlen(buffer), 0);
            }

            // Receive responses
            for(int i=0; i < MAX_CLIENTS; i++) {
                memset(buffer, 0, BUFFER_SIZE);
                n = recv(clientfd[i], buffer, BUFFER_SIZE-1, 0);
                if(n <= 0 || strncmp(buffer, "no", 2) == 0){
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
                continue;
            }
            else{
                // Inform and close connections
                
                for(int i=0; i < MAX_CLIENTS; i++) {
                    if(play_again[i]){
                        snprintf(buffer, BUFFER_SIZE, "Opponent does not want to play again. Disconnecting.\n");
                        send(clientfd[i], buffer, strlen(buffer), 0);
                    }
                    close(clientfd[i]);
                }
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
