#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_STUDENTS 10
#define NUM_CHAIRS 3

pthread_mutex_t ta_mutex;
pthread_mutex_t chair_mutex;
sem_t students_sem;
sem_t ta_sem;

int num_waiting_students = 0;

void* student_thread(void* student_id) {
    int id = *(int*)student_id;

    while (1) {
        // Simulate programming
        printf("Student %d is programming.\n", id);
        sleep(rand() % 5);

        // Check if the TA is available
        pthread_mutex_lock(&ta_mutex);
        if (num_waiting_students < NUM_CHAIRS) {
            // Take a seat in the hallway
            pthread_mutex_lock(&chair_mutex);
            num_waiting_students++;
            printf("Student %d is waiting in the hallway.\n", id);
            pthread_mutex_unlock(&ta_mutex);
            pthread_mutex_unlock(&chair_mutex);
            
            // Signal the TA
            sem_post(&students_sem);
            
            // Wait for the TA to help
            sem_wait(&ta_sem);
        } else {
            // No available chairs, come back later
            pthread_mutex_unlock(&ta_mutex);
            printf("Student %d will come back later.\n", id);
        }
    }

    pthread_exit(NULL);
}

void* ta_thread(void* arg) {
    while (1) {
        // Wait for students to arrive
        sem_wait(&students_sem);

        // Help each student in turn
        while (1) {
            pthread_mutex_lock(&chair_mutex);
            if (num_waiting_students > 0) {
                num_waiting_students--;
                printf("TA is helping a student. Remaining students: %d\n", num_waiting_students);
                sem_post(&ta_sem);
                pthread_mutex_unlock(&chair_mutex);
                
                // Simulate helping the student
                sleep(rand() % 3);
            } else {
                pthread_mutex_unlock(&chair_mutex);
                break;
            }
        }
        
        // Nap if no more students are waiting
        printf("TA is napping.\n");
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t ta_tid, students_tid[NUM_STUDENTS];
    int student_ids[NUM_STUDENTS];

    srand(time(NULL));

    // Initialize mutexes and semaphores
    pthread_mutex_init(&ta_mutex, NULL);
    pthread_mutex_init(&chair_mutex, NULL);
    sem_init(&students_sem, 0, 0);
    sem_init(&ta_sem, 0, 0);

    // Create TA thread
    pthread_create(&ta_tid, NULL, ta_thread, NULL);

    // Create student threads
    for (int i = 0; i < NUM_STUDENTS; i++) {
        student_ids[i] = i + 1;
        pthread_create(&students_tid[i], NULL, student_thread, &student_ids[i]);
    }

    // Wait for TA and student threads to finish
    pthread_join(ta_tid, NULL);
    for (int i = 0; i < NUM_STUDENTS; i++) {
        pthread_join(students_tid[i], NULL);
    }

    // Destroy mutexes and semaphores
    pthread_mutex_destroy(&ta_mutex);
    pthread_mutex_destroy(&chair_mutex);
    sem_destroy(&students_sem);
    sem_destroy(&ta_sem);

    return 0;
}
