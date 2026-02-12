#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define PORT 6379
#define BUFFER_SIZE 1024
#define MAX_KEYS 100

typedef struct {
  char key[64];
  char value[256];
  time_t expires_at;
} Entry;

Entry store[MAX_KEYS];
int store_size = 0;

pthread_mutex_t store_mutex = PTHREAD_MUTEX_INITIALIZER;

int find_key(const char *key) {
   for(int i = 0; i < store_size; i++){
    if(strcmp(store[i].key, key) == 0)
        return i;
    }
       return -1;
   }


void *handle_client(void *arg){
  int client_fd = *(int *)arg;
  free(arg);

  char buffer[BUFFER_SIZE];

  while (1) {
    memset(buffer, 0, BUFFER_SIZE);
    int bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

    if (bytes <= 0){
        break;
    }

    char *command = strtok(buffer, " \n");

    if(!command) {
        send(client_fd,"ERROR", 6, 0);
        continue;
    }

    pthread_mutex_lock(&store_mutex);

    if (strcmp(command, "PING") == 0){
        send(client_fd, "PONG\n", 5, 0);
    }
    else if(strcmp(command,"SET") == 0){
        char *key = strtok(NULL, " \n");
        char *value = strtok(NULL, "\n");

    if(!key || !value){
        send(client_fd, "ERROR\n", 6, 0);
    } else {
        int idx = find_key(key);
        if (idx == -1 && store_size < MAX_KEYS){
            idx = store_size++;
        }
        strcpy(store[idx].key, key);
        strcpy(store[idx].value, value);
        store[idx].expires_at = 0;
        send(client_fd, "OK\n", 3, 0);
    }
   }
   else if (strcmp(command, "GET") == 0){
    char *key = strtok(NULL, " \n");

    int idx;
    if (key != NULL){
        idx = find_key(key);
    } else {
        idx = -1;
    }

    if (idx == -1){
        send(client_fd, "NOT_FOUND", 10, 0);
    } else {
         time_t now = time(NULL);
         if(store[idx].expires_at > 0 && now > store[idx].expires_at){
            send(client_fd, "NOT_FOUND\n", 10, 0);
         } else {
            char response[BUFFER_SIZE];
            snprintf(response, BUFFER_SIZE, "VALUE %s\n", store[idx].value);
            send(client_fd, response, strlen(response), 0);
         }
       }
     }

     else if(strcmp(command, "DEL") == 0){
        char *key = strtok(NULL, " \n");

        int idx;
        if (key != NULL){
          idx = find_key(key);
        } else {
          idx = -1;
        }

        if(idx == -1){
            send(client_fd, "NOT_FOUND\n", 10, 0);
        } else {
           store[idx] = store[store_size - 1];
           store_size--;
           send(client_fd, "DELETED\n", 8, 0);
        }

     }

     else if (strcmp(command, "SETEX") == 0){
        char *key = strtok(NULL, " \n");
        char *ttl_str = strtok(NULL, " \n");
        char *value = strtok(NULL, "\n");

        if(!key || !ttl_str || !value){
            send(client_fd, "ERROR\n", 6, 0);
        } else {
           int ttl = atoi(ttl_str);
           int idx = find_key(key);
           if (idx == -1 && store_size < MAX_KEYS){
               idx = store_size++;
           }

           strcpy(store[idx].key, key);
           strcpy(store[idx].value, value);
           store[idx].expires_at = time(NULL) + ttl;
           send(client_fd, "OK\n", 3, 0);
        }
     }

     else{
        send(client_fd, "UNKOWN_COMMAND\n", 16, 0);
     }

     pthread_mutex_unlock(&store_mutex);
  }

    close(client_fd);
    return NULL;
}

  int main() {
     int server_fd = socket(AF_INET, SOCK_STREAM, 0);
     if (server_fd < 0){
        perror("socket");
        exit(EXIT_FAILURE);
     }

     struct sockaddr_in address;
     address.sin_family = AF_INET;
     address.sin_addr.s_addr = INADDR_ANY;
     address.sin_port = htons(PORT);

     if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        perror("bind");
        exit(EXIT_FAILURE);
     }

     if (listen(server_fd, 10) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
     }

     printf("Server Listening On Port %d\n", PORT);


    while(1){
        struct sockaddr_in client_addr;
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);

        if (client_fd < 0){
            perror("accept");
            continue;
        }

       int *client_ptr = malloc(sizeof(int));
        *client_ptr = client_fd;

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, client_ptr);
        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}













