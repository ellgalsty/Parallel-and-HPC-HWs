#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


void* print_message(void* arg) {
    char* message = (char*)arg;
    printf("%s\n", message);
    return NULL;
}

int main() {
    pthread_t thread1, thread2, thread3;
    char* message1 = "thread 1 is running";
    char* message2 = "thread 2 is running";
    char* message3 = "thread 3 is running";

    if (pthread_create(&thread1, NULL, print_message, (void*)message1) != 0) {
        perror("failed to create thread 1");
        return 1;
    }

    if (pthread_create(&thread2, NULL, print_message, (void*)message2) != 0) {
        perror("failed to create thread 2");
        return 1;
    }

    if (pthread_create(&thread3, NULL, print_message, (void*) message3) != 0) {
        perror("failed to create thread 3");
        return 1;
    }

    if (pthread_join(thread1, NULL) != 0) {
        perror("failed to join thread 1");
        return 1;
    }

    if (pthread_join(thread2, NULL) != 0) {
        perror("failed to join thread 2");
        return 1;
    }

    if (pthread_join(thread3, NULL) != 0) {
        perror("failed to join thread 3");
        return 1;
    }

    printf("end of the program exexcution\n");

    return 0;
}