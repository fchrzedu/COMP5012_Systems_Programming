#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 1024
int main() {
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0); // TCp/IP socket creation

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    listen(server_socket, 1);

   
    int client_socket;
    client_socket = accept(server_socket, NULL, NULL);

    const char *q = ">Do you like Marvel Rivals? (yes/no) \n";
    send(client_socket, q, strlen(q),0); // feeds in question to display in console

    char response[BUFFER_SIZE]; 
    memset(response,0,sizeof(response));
    recv(client_socket, response, sizeof(response),0);


    const char *yes_r = ">Great me too!\n";
    const char *no_r = "What a shame!\n";
    const char *error = "Incorrect input, please use either (yes) or (no).\n";

    if(strncmp(response, "yes", 3) == 0){
        send(client_socket, yes_r, strlen(yes_r),0);
    }else if (strncmp(response, "no", 2) == 0){
        send(client_socket, no_r, strlen(no_r),0);
    }else{
        send(client_socket, error, strlen(error),0);
    }
    
   
    

    close(client_socket);
    close(server_socket);
}