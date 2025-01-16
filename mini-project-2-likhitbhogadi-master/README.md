[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/JsZCPDRN)
# Intro to Xv6
OSN Monsoon 2024 mini project 2

## Some pointers
- main xv6 source code is present inside `initial_xv6/src` directory. This is where you will be making all the additions/modifications necessary for the xv6 part of the Mini Project. 
- work inside the `networks/` directory for the Networking part of the Mini Project.



- You are free to delete these instructions and add your report before submitting. 
## 1. getsyscount

        i kept a global array to store the no of times a system call at all times
        whenever a syscall is called, the syscall.c is called, so at that time i will update the count of that syscall
        i used locks for avoiding race conditions
        the syscall getsyscall will return this count whenever needed and reinitialize to 0 before returning
        i am calling it once before calling exec to reset 
        i use fork and exec in the syscount.c user prog to run the given function in the argument

## 2. sigalarm, sigreturn

        i am initialising new fields in proc struct like alarm interval, alarm handler for the arguments and ticks count of that process, saving state in trapframe, flag if alarm is active or not.
        in allocproc in proc.c, i am doing necessary initializations
        and creating 2 syscalls, sigalarm, sigreturn
        sigalarm when called, changes flag to 1, indicating that sigalarm is called, this is used in trap.c to identify it and switch to the alarm handler, also flag is set to 0 while handler is running to avoid reentrant alarm calls
        when done, sigreturn is called which sets flag to 1 again so that alarm can go on once again when timer requrement is met and also p->trapframe is set back to old state of the process calling sigalarm

## 3. lottery based scheduling

        i am using a random number generator , which is present in /user/grind.c of xv6
        i am creating new fields in proc struct in proc.h  tickets and arrival time
        these are initialised in allocproc
        in fork.c the child will inherit these
        the winner of ticket will be calculated and also checked if a better person is there and context switch is done
        using other functions like count tickets 
        it will loop thru all runnable processes and count the tickets

## 4. MLFQ 

## - MLFQ ANALYSIS

![alt text](<Screenshot from 2024-10-12 21-36-47.png>)

        The image represents a Multi-Level Feedback Queue (MLFQ) scheduler visualization for five processes, each with its unique color, plotting the queue number (y-axis) against time in ticks (x-axis):

    Queue Levels: There are 3 queue levels (0, 1, 2) where queue 0 represents the highest priority and queue 2 the lowest.
    Process Movement: The processes (P5, P6, P7, P8, P9) begin at queue 0, and over time, they are moved to lower priority queues (higher queue numbers) based on the scheduler's decision.
    Time Slices: The graph shows processes being demoted to lower-priority queues, after exhausting their time slice in the higher-priority queues.
    Priority Boosting: The processes periodically return to queue 0 (e.g., around ticks 716 and 771), which aligns with the priority boosting mechanism. This boost likely occurs after a set number of ticks (e.g., every 48 ticks).
    Idle/Waiting Times: Some processes drop to the lowest queue and seem to remain there (flat lines), indicating they are either waiting or have lower priority.
    Starvation Prevention: The regular boosting back to queue 0 suggests that the scheduler prevents starvation by resetting priorities periodically.

## - code implemenatation

        adding fields in proc struct priority, timeslice, total tickets
        The scheduler function now iterates through priority levels, always running the highest priority runnable process.
        i implemented priority boosting with a counter that resets all processes to the highest priority every 48 ticks.
        The allocproc function initializes new processes with the highest priority and appropriate time slice.
        i modified the usertrap function to handle time slices and priority changes when timer interrupts occur.
        The yield function was updated to reset the time slice when a process voluntarily relinquishes the CPU.

## 5. XOXO

- ip address needs to be changed in #define to the connected network

## UDP Client:

    The client sets up a UDP socket using socket(AF_INET, SOCK_DGRAM, 0) and defines the server address (localhost, port 9999). A "JOIN" message is sent to the server with sendto(). The program uses poll() to monitor both stdin (user input) and the server's responses, assigning fds[0] for stdin and fds[1] for the socket. Upon receiving data from the server (recvfrom()), it checks for prompts like "it's your turn" or "Do you want to play again?" and waits for user input (fgets()), which it sends back to the server using sendto(). The socket is closed when the game ends or an error occurs.

## UDP Server:

    The server uses a UDP socket to handle communication. It binds to PORT 9999 and waits for players to join by sending a "JOIN" message.
    A maximum of two clients (players) can join, and each player is assigned a symbol ('X' or 'O'). The game begins once both players have joined.
    The server sends the current board state to both players and prompts the current player to make a move, accepting input in the format row col.
    After receiving a move, it verifies the validity and updates the board. If the move is invalid, the player is prompted to try again.
    The server checks for a win or draw after each move. If a player wins, it announces the winner to both players; in case of a draw, it informs both players.
    After the game ends (either by win or draw), both players are asked if they want to play again. If both agree, the board is reset and a new game begins.
    The main loop alternates turns between players and updates the game state after each move.
    If either player does not wish to continue after a game, the server informs the players, and the game session ends.
    The recvfrom() function is used for receiving messages from clients, and sendto() is used to send messages to the respective clients.
    The server remains connectionless, as UDP does not require persistent connections with clients.

## TCP server:

    The server initializes a Tic-Tac-Toe board using a 3x3 char array and sets up a TCP socket bound to port 9999. It listens for connections and accepts two clients, assigning them symbols 'X' and 'O'. A game loop follows where the current player is prompted to input their move (row and column), which is validated before updating the board. The server checks for a win by comparing rows, columns, and diagonals or a draw if the board is full. It communicates the game state (win, draw, or invalid moves) to both players. After each game, both players are asked if they want to play again, and the game resets or ends based on their responses.

## TCP client:

    The client creates a TCP socket and connects to the server on the specified IP and port (127.0.0.1:9999). It uses poll to monitor both stdin (user input) and the server socket for incoming data. Upon receiving messages from the server, the client displays them, such as board updates and prompts. If the server indicates it's the client's turn, the client reads the move (row and column) from the user and sends it back to the server. Similarly, when prompted for a rematch, the client responds with "yes" or "no". The game continues in a loop, receiving server updates and sending input until the server disconnects, at which point the client closes the socket and terminates.

## 6. fake it till you make it

## Server:

    This implementation sets up a server using UDP that alternates between sending and receiving data in chunks, mimicking TCP-like behavior. The send_data() function breaks a message into chunks, assigns sequence numbers, and sends each chunk to the client. After sending, the server listens for ACKs. If ACKs aren't received within a timeout period, it retransmits the unacknowledged chunks to ensure reliability. Each chunk is sent with a sequence number, and ACKs correspond to these numbers.

    The receive_data() function handles receiving packets from the client. It stores chunks in an array and sends an ACK for each received packet. Once all chunks are received, the message is reassembled and displayed. If chunks are missing or dropped, the sender retries sending those based on the ACK feedback. The communication alternates between sending and receiving using a sending_mode flag in a continuous loop.

    select() is used to monitor socket readiness for both sending and receiving, implementing timeouts for packet retransmissions and preventing blocking. The program supports bidirectional communication, where the server can switch between sending messages and receiving data.

## Client:

    This implementation uses UDP to mimic some TCP-like functionalities, including chunking, sequencing, ACKs, retransmissions, and timeouts. The send_data() function divides the message into chunks of size CHUNK_SIZE, with each chunk assigned a sequence number. These chunks are sent to the receiver, who responds with ACKs upon receiving them. If an ACK isn't received for a chunk within a defined timeout (TIMEOUT), the sender retransmits that chunk.

    The receiver, implemented in receive_data(), waits for incoming packets, stores received chunks in an array, and sends an ACK for each valid packet. It buffers the chunks and checks whether all chunks have been received based on the total chunk count provided in each packet. If chunks are missing, the sender retries transmission. To simulate a complete message transfer, both sending and receiving modes alternate using the sending_mode flag. The sender switches to receiving mode after transmission, and vice versa after data reception completes.

    The program uses select() to handle timeout mechanisms for retransmissions and ACK reception without blocking. The communication continues until all chunks are transmitted and acknowledged correctly.

## - SCHEDULING COMPARISION

## RR

### 1 cpu
- Average rtime 11
- wtime 144
- Average rtime 11
- wtime 143

### 3 cpus
- Average rtime 11
- wtime 112
- Average rtime 13
- wtime 111

## LBS

### 1 cpu

- Average rtime 10
- wtime 133
- Average rtime 10
- wtime 132

### 3 cpus
- ..
- ..

## MLFQ

### 1 cpu

- Average rtime 10 
- wtime 123
- Average rtime 11
- wtime 124

- runtime should be the same for all the schedulers which can be seen as above because whatever the scheduling policy is more or less it'll be running some process always 
- waittime changes as expected, more efficient as we go down in the list

- with default values in scheduler test (6 runs)

## - questions

## 1. implication of adding the arrival time in the lottery based scheduling policy? 

    Adding arrival time in lottery-based scheduling allows the scheduler to prioritize processes based on when they become runnable, enhancing responsiveness for newly arrived tasks. 
    This helps ensure that processes that are ready to run sooner get a fair chance to execute, improving overall system performance.

## 2. Are there any pitfalls to watch out for?

    one minor thing is slight increase in the complexity of the implementation and logic
    if a high no of processes have the same no. of tickets then any of the equal ticket holding processes winning the ticket would mean that ultimately the earliest arrived one of them will keep getting the tickets until its done thus reducing the "lottery" based randomness we wished for!

## 3. What happens if all processes have the same number of tickets?

    If all processes have the same number of tickets, they effectively have equal chances of being scheduled, leading to a fair allocation of CPU time. 
    However, this could result in inefficiencies if many processes compete for execution, as the scheduler will treat all processes equally, ignoring their individual needs or urgency.
    it is exactly equal to fcfs based on discussion in above question