#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// A simple demonstration of Inter-Process Communication (IPC)
// The parent sends a number to the child, and the child sends it back.

int main() {
    int parent_to_child_pipe[2];
    int child_to_parent_pipe[2];

    // Create the communication channels
    if (pipe(parent_to_child_pipe) == -1 || pipe(child_to_parent_pipe) == -1) {
        perror("pipe creation failed");
        return 1;
    }

    pid_t process_id = fork();

    if (process_id < 0) {
        perror("fork failed");
        return 1;
    }

    if (process_id == 0) {
        // --- Child Process ---
        // Close unused pipe ends
        close(parent_to_child_pipe[1]); // Child doesn't write to this one
        close(child_to_parent_pipe[0]); // Child doesn't read from this one

        int received_val;
        read(parent_to_child_pipe[0], &received_val, sizeof(int));
        std::cout << "Child received: " << received_val << std::endl;

        // Send it back
        write(child_to_parent_pipe[1], &received_val, sizeof(int));
        
        // Close the pipes we used
        close(parent_to_child_pipe[0]);
        close(child_to_parent_pipe[1]);

    } else {
        // --- Parent Process ---
        // Close unused pipe ends
        close(parent_to_child_pipe[0]); // Parent doesn't read from this one
        close(child_to_parent_pipe[1]); // Parent doesn't write to this one

        int sent_val = 42;
        std::cout << "Parent sending: " << sent_val << std::endl;
        write(parent_to_child_pipe[1], &sent_val, sizeof(int));

        int returned_val;
        read(child_to_parent_pipe[0], &returned_val, sizeof(int));
        std::cout << "Parent got back: " << returned_val << std::endl;

        // Close the pipes we used
        close(parent_to_child_pipe[1]);
        close(child_to_parent_pipe[0]);
        
        // Wait for the child process to terminate
        wait(NULL);
    }

    return 0;
}
