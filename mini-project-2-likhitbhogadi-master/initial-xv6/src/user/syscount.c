#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fcntl.h"

int main(int argc, char *argv[])
{
    // for debugging
    // Define an array of syscall names (must match kernel/syscall.h)
    // char *syscall_names[] = {
    //     "fork",
    //     "exit",
    //     "wait",
    //     "pipe",
    //     "read",
    //     "kill",
    //     "exec",
    //     "fstat",
    //     "chdir",
    //     "dup",
    //     "getpid",
    //     "sbrk",
    //     "sleep",
    //     "uptime",
    //     "open",
    //     "write",
    //     "mknod",
    //     "unlink",
    //     "link",
    //     "mkdir",
    //     "close",
    //     "waitx",
    //     "hello",
    //     "getSysCount",
    //     // Add more syscall names as needed, ensuring alignment with syscall numbers
    // };
    // int num_syscalls = sizeof(syscall_names)/sizeof(syscall_names[0]);
    // char *syscall_name = "unknown";
    // for(int i=0; i<num_syscalls; i++){
    //     syscall_name = syscall_names[i];
    //     printf("PID %d called %s %d times.\n", getpid(), syscall_name, getSysCount(1<<(i+1)));
    // }

    if(argc < 3){
        fprintf(2, "Usage: syscount <mask> <command> [args...]\n"); // why fprintf instead of printf ?? because stderr
        exit(1); // why exit instead of return 0?
    }

    int mask = atoi(argv[1]); // try wrong inputs/bugs


    // Fork a child to execute the command
    int pid = fork();

    // call getsyscount before running exec to initialize count to 0 
    getSysCount(mask);

    if(pid < 0){
        fprintf(2, "syscount: fork failed\n");
        exit(1);
    }

    if(pid == 0){
        // In child: execute the command
        exec(argv[2], &argv[2]);
        // If exec fails
        fprintf(2, "syscount: exec %s failed\n", argv[2]);
        exit(1);
    } else {
        // In parent: wait for the child to finish
        wait(0); // why the argument 0? does it mean NULL?

        // Get the syscall count
        int count = getSysCount(mask);

        // Identify the syscall number based on the mask
        int syscall_num = -1;
        for(int i = 1; i < 33; i++) {
            if(mask & (1 << i)){
                syscall_num = i;
                break;
            }
        }

        if(syscall_num == -1){
            fprintf(2, "syscount: invalid mask %d\n", mask);
            exit(1);
        }

        // Define an array of syscall names (must match kernel/syscall.h)
        char *syscall_names[] = {
            "fork",
            "exit",
            "wait",
            "pipe",
            "read",
            "kill",
            "exec",
            "fstat",
            "chdir",
            "dup",
            "getpid",
            "sbrk",
            "sleep",
            "uptime",
            "open",
            "write",
            "mknod",
            "unlink",
            "link",
            "mkdir",
            "close",
            "waitx",
            "hello",
            "getSysCount",
            // Add more syscall names as needed, ensuring alignment with syscall numbers
        };

        // Ensure syscall_num is within range
        char *syscall_name = "unknown";
        int num_syscalls = sizeof(syscall_names)/sizeof(syscall_names[0]);
        if(syscall_num < num_syscalls){
            syscall_name = syscall_names[syscall_num-1];
        }

        printf("PID %d called %s %d times.\n", getpid(), syscall_name, count);

        // for debugging
        for(int i=0; i<num_syscalls; i++){
            syscall_name = syscall_names[i];
            printf("PID %d called %s %d times.\n", getpid(), syscall_name, getSysCount(1<<(i+1)));
        }
    }

    exit(0);
}
