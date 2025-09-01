#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <iomanip>  

const int NUM_MESSAGES = 1000000;

int main() {
    int parent_to_child_pipe[2];
    int child_to_parent_pipe[2];

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
        close(parent_to_child_pipe[1]); 
        close(child_to_parent_pipe[0]);

        int buffer;
        for (int i = 0; i < NUM_MESSAGES; ++i) {
            read(parent_to_child_pipe[0], &buffer, sizeof(int));
            write(child_to_parent_pipe[1], &buffer, sizeof(int));
        }

        close(parent_to_child_pipe[0]);
        close(child_to_parent_pipe[1]);

    } else {
        close(parent_to_child_pipe[0]);
        close(child_to_parent_pipe[1]);

        struct timeval start_time, end_time;
        gettimeofday(&start_time, NULL); // Start the timer

        for (int i = 0; i < NUM_MESSAGES; ++i) {
            int message = i;
            write(parent_to_child_pipe[1], &message, sizeof(int));
            read(child_to_parent_pipe[0], &message, sizeof(int));
        }

        gettimeofday(&end_time, NULL); // Stop the timer

        long seconds = end_time.tv_sec - start_time.tv_sec;
        long microseconds = end_time.tv_usec - start_time.tv_usec;
        double total_elapsed_time = seconds + microseconds * 1e-6;

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "Total time for " << NUM_MESSAGES << " roundtrips: " << total_elapsed_time << " seconds" << std::endl;

        close(parent_to_child_pipe[1]);
        close(child_to_parent_pipe[0]);
        wait(NULL);
    }

    return 0;
}
