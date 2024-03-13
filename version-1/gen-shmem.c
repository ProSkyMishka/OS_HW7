#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>

typedef struct {
    int number;
} Data;

int server_pid;
int shm_id;
char gen_object[] = "gen-memory";

void server_signal_handler(int signal) {
    if (signal == SIGINT) {
        printf("Received SIGINT in the client. Sending SIGINT to server...\n");
        kill(server_pid, SIGINT);
    }
    
    if(shm_id != -1) {
        shm_unlink(gen_object);
        close(shm_id);
    }
    
    exit(0);
}

int main() {
    printf("Gen PID: %d\n", getpid());
    printf("Input out PID: ");
    scanf("%d", &server_pid);  // Получить PID out от пользователя
    
    signal(SIGINT, server_signal_handler);
    
    if ((shm_id = shm_open(gen_object, O_CREAT|O_RDWR, 0666)) == -1) {
        perror("shm_open");
        return 1;
    } else {
        printf("Object is open: name = %s, id = 0x%x\n", gen_object, shm_id);
    }
    
    // Задание размера объекта памяти
    if (ftruncate(shm_id, sizeof(Data)) == -1) {
        perror("ftruncate");
        return 1;
    } else {
        printf("Memory size set and = %lu\n", sizeof(Data));
    }
    Data *shared_data = (Data *)mmap(NULL, sizeof(Data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
    if (shared_data == MAP_FAILED) {
      printf("Error getting pointer to shared memory\n");
      return 1;
    }
    
    srand(getpid()); // Инициализация генератора случайных чисел
    
    while (1) {
        sleep(1); // Подождать 1 секунду перед генерацией числа
        shared_data->number = rand() % 1000; // Генерация числа от 0 до 999
        
        printf("Отправлено число: %d\n", shared_data->number);
    }
    
    return 0;
}
