#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>

typedef struct {
    int number;
    int flag;
} Data;

Data *shared_data;
int shm_id;
char gen_object[] = "gen-memory";

void client_signal_handler(int signal) {
    if (signal == SIGINT) {
        shared_data->flag = 1;
    }
    
    if(shm_id != -1) {
        shm_unlink(gen_object);
        close(shm_id);
    }
}

int main() {
    signal(SIGINT, client_signal_handler);
    
    //открыть объект
    if ( (shm_id = shm_open(gen_object, O_RDWR, 0666)) == -1 ) {
      printf("Opening error\n");
      perror("shm_open");
      return 1;
    } else {
      printf("Object is open: name = %s, id = 0x%x\n", gen_object, shm_id);
    }

    //получить доступ к памяти
    shared_data = (Data *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
    if (shared_data->number == (int)-1 ) {
      printf("Error getting pointer to shared memory\n");
      return 1;
    }
    
    while (1) {
        sleep(1); // Подождать 1 секунду перед чтением числа
        printf("Получено число: %d\n", shared_data->number);
        
        if (shared_data->flag == 1) {
            if(shm_id != -1) {
                shm_unlink(gen_object);
                close(shm_id);
            }
            exit(0);
        }
    }
    
    return 0;
}
