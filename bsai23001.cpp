#include <iostream>
// #inlcude <stdio> // Just adding it, not sure if I would use it or not
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <iomanip>    // For formatting output

const int TOTAL_TRANSACTIONS = 1000000;

int main() {
    int p2c_pipe[2];
    int c2p_pipe[2];

    if (pipe(p2c_pipe) == -1 || pipe(c2p_pipe) == -1) {
        perror("Failed to create pipes");
        return 1;
    }

    pid_t child_pid = fork();

    if (child_pid< 0) {
        perror("Fork failed");
        return 1;
    }

    if (child_pid ==0) {
        // This process acts as an echo server.
        close(p2c_pipe[1]); // Close write-end of parent-to-child pipe

        close(c2p_pipe[0]); // Close read-end of child-to-parent pipe

        int message_buffer;
        for (int i = 0; i < TOTAL_TRANSACTIONS; ++i) {
            // Wait for a message from the parent
            if (read(p2c_pipe[0], &message_buffer, sizeof(int)) <= 0) break;
            // Send it back
            if (write(c2p_pipe[1], &message_buffer, sizeof(int)) <= 0) break;
        }

        // Cleanup
        close(p2c_pipe[0]);
        close(c2p_pipe[1]);
        return 0; // Child process exits

    } else {
        // --- Parent Process  Logic ---
        close(p2c_pipe[0]); // Close read-end of parent-to child pipe
        close(c2p_pipe [1]); // close write-end of child-to-parent pipe

        struct timeval start_time, end_time;
        gettimeofday(&start_time, NULL);

        for (int i = 0; i < TOTAL_TRANSACTIONS; ++i) {
            write(p2c_pipe[1], &i, sizeof(int));
            read(c2p_pipe[0], &i, sizeof(int));
        }

        gettimeofday(&end_time, NULL);

        // --- Performance Calculation ---
        long seconds = end_time.tv_sec  - start_time.tv_sec;
        long microseconds = end_time.tv_usec - start_time.tv_usec;
        double elapsed_seconds = seconds + microseconds * 1e-6;
        double elapsed_microseconds =  elapsed_seconds * 1e6;

        double throughput = TOTAL_TRANSACTIONS / elapsed_seconds;
        double avg_roundtrip_us = elapsed_microseconds /  TOTAL_TRANSACTIONS;

        double avg_one_way_latency_us = avg_roundtrip_us  / 2.0;

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "--- IPC Pipe Performance ---" << std::endl;
        std::cout << "Total Transactions:    " << TOTAL_TRANSACTIONS << std::endl;
        std::cout << "Total Elapsed Time:    " << elapsed_seconds << " s" << std::endl;
        std::cout << "Throughput (req/sec):  " << throughput << std::endl;
        std::cout << "Avg. Roundtrip Time:   " << avg_roundtrip_us << " us" << std::endl;
        std::cout << "Avg. One-Way Latency:  " << avg_one_way_latency_us << " us" << std::endl;

        // Cleanup
        close(p2c_pipe[1]);
        close(c2p_pipe[0]);
        wait(NULL); // Wait for child to finish
    }

    return 0;
}
