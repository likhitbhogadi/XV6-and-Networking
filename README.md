# XV6 and Networking Project
OSN Monsoon 2024 Mini Project 2

## Project Structure
- XV6 modifications are implemented in `initial_xv6/src` directory
- Networking implementations are located in the `networks/` directory

## XV6 Kernel Modifications

### 1. System Call Counter (`getsyscount`)
Implemented a global counter array to track system call frequency:
- Added a protected global array in `syscall.c` to store call counts
- Used locks to prevent race conditions in multi-process environments
- Created `getsyscount` syscall that returns counts and resets counters
- Added user program (`syscount.c`) that uses fork/exec to measure syscall usage for any command

### 2. Timer Interrupts (`sigalarm`, `sigreturn`)
Implemented periodic timer interrupt handlers:
- Added process fields to track alarm intervals, handlers, and timer ticks
- Created `sigalarm` syscall to register a handler function for timer events
- Added `sigreturn` syscall that restores process state after handler execution
- Implemented reentrant prevention mechanism during handler execution

### 3. Lottery-based Scheduling
Implemented a probabilistic scheduler using ticket allocation:
- Added process attributes for tickets and arrival time in `proc` struct
- Used random number generator from `/user/grind.c`
- Implemented ticket inheritance during process forking
- Added selection algorithm that chooses processes based on ticket probability

### 4. Multi-Level Feedback Queue (MLFQ) Scheduler

#### Implementation Details:
- Added priority level, time slice tracking, and ticket counts to process structure
- Modified scheduler to always run the highest priority runnable process
- Implemented priority boosting every 48 ticks to prevent starvation
- Updated timer interrupt handling to manage time slices and priority adjustments
- Modified `yield` to reset process time slices on voluntary CPU relinquishment

#### MLFQ Analysis:
![MLFQ Scheduling Visualization](Screenshot%20from%202024-10-12%2021-36-47.png)

The visualization shows:
- 3 priority levels (queues 0-2) with 5 processes tracked over time
- Processes starting at highest priority (queue 0) and being demoted as they consume CPU time
- Priority boosting intervals (around ticks 716 and 771) returning all processes to queue 0
- Time-slice-based queue demotions working as expected
- Effective starvation prevention through periodic priority resets

## Networking Implementations

### 5. XOXO (Tic-Tac-Toe Game)

**Note:** Update the IP address in the code to match your network configuration.

#### UDP Implementation:
- **Client:**
    - Creates UDP socket and sends "JOIN" message to server
    - Uses `poll()` to simultaneously monitor user input and server responses
    - Handles game state updates and turn-based play through datagram exchange
    
- **Server:**
    - Manages game state for two players with symbols 'X' and 'O'
    - Validates moves and updates the board appropriately
    - Checks for win conditions across rows, columns, and diagonals
    - Supports rematch functionality after game completion

#### TCP Implementation:
- **Client:**
    - Establishes persistent connection to server
    - Uses `poll()` to handle both user input and server communication
    - Displays game state and responds to server prompts
    
- **Server:**
    - Accepts two TCP connections, assigning 'X' and 'O' to players
    - Maintains game state and enforces turn-based rules
    - Validates moves and checks for win/draw conditions
    - Supports rematch option after game completion

### 6. Reliable UDP Protocol

Implemented a TCP-like reliable protocol over UDP:

- **Server & Client Features:**
    - Message chunking with sequence numbers
    - Acknowledgment system for received chunks
    - Timeout-based retransmission for lost packets
    - Ordered packet reassembly at the receiver
    - Bidirectional communication with send/receive mode switching
    - Non-blocking I/O using `select()` for efficient timeout handling

## Performance Comparison

### Scheduling Performance Analysis

| Scheduler | CPUs | Average Runtime | Average Wait Time |
|-----------|------|----------------|------------------|
| Round Robin | 1 | 11 | 143-144 |
| Round Robin | 3 | 11-13 | 111-112 |
| Lottery-Based | 1 | 10 | 132-133 |
| MLFQ | 1 | 10-11 | 123-124 |

**Key Observations:**
- Runtime remains relatively consistent across schedulers as expected (all CPUs remain active)
- Wait times improve with more sophisticated scheduling policies
- MLFQ shows the best overall waiting time performance for single-CPU systems

## Scheduling Analysis

### 1. Arrival Time in Lottery Scheduling
Adding arrival time to lottery-based scheduling allows prioritization based on process readiness, improving responsiveness for newly arrived tasks and ensuring fair execution opportunity based on both ticket count and waiting time.

### 2. Potential Pitfalls
- Increased implementation complexity
- When multiple processes have equal tickets, the earliest-arrived process may dominate execution time, diminishing the randomness benefit of lottery scheduling

### 3. Equal Ticket Scenario
When all processes have identical ticket counts, the scheduler effectively reverts to a First-Come-First-Served (FCFS) model based on arrival time, potentially reducing the benefits of probabilistic scheduling for workload diversity.
