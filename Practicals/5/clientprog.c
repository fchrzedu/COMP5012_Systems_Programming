#include <stdio.h>      // Standard input/output library (for printf, fgets)
#include <sys/socket.h> // Socket programming functions (socket, connect, send, recv)
#include <netinet/in.h> // Structures for handling internet addresses (sockaddr_in)
#include <arpa/inet.h>  // Functions for IP address conversion (inet_addr)
#include <unistd.h>     // System calls (close)
#include <string.h>     // String manipulation functions (strlen, memset, strcspn)

#define BUFFER_SIZE = 1024; // maxinimum buffer size const

int main(){
    int cs; // client socket
    struct sockaddr_in server_address; // server address
    char buffer[BUFFER_SIZE]; // buffer size array

    cs = socket(AF_INET, SOCK_STREAM, 0);
    /*
    AF_INET = using IPv4 
    SOCK_STREAM = using TCP both way
    0 = default TCP protocol
    */
    
    // client returns integer, 0 means connect, -1 = failure
    if(cs == -1){
        perror("Error creating socket.\n");
        exit(1);
    }

    if(connect(cs, (struct s)))
   
}