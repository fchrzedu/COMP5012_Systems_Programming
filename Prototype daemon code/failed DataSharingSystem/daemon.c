/*
- Daemon for the program.
- It contains server side socket functionalities, a seperate client program exist
- Coursework wants 'daemon that LISTENS', therefore server-side resides here
- Client calls sharing lib, that sockes into daemon w/ server.


- SIGISEV & memory dumping errors when using UNIX domain socket - had to use TCP with a loopback
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <syslog.h>
/* Socket headers \/*/
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9002
#define IP_ADDY "127.0.0.1"


void daemon_make(){
    pid_t pid, sid;   
    
    openlog("daemon_TCP_server", LOG_PID, LOG_DAEMON);
    
    pid = fork();
    if(pid < 0) { 
        syslog(LOG_ERR, "%s\n", "[-]Error forking\n"); 
        exit(EXIT_FAILURE);
    }
    if(pid > 0){exit(EXIT_SUCCESS);} /* Parent exists */      
    syslog(LOG_INFO, "[+]Succesfully forked()\n");
        
        
    /* ------------In the child (orphaned)...------------- */

    if((sid = setsid()) < 0) { /* Create new unique session */
        syslog(LOG_ERR, "%s\n", "[-]Setsid error\n"); /* Log error if failed to setsid */
        exit(EXIT_FAILURE);
    }
   
    /* Change to root directory  */
    if((chdir("/")) < 0) {
        syslog(LOG_ERR, "%s\n", "[-]Chdir error\n");
        exit(EXIT_FAILURE);
    }

    /* Reset the file mode */
    umask(0); /* Protection */
    /* Close stdin, etc. */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}
void server_socket_TCP(){
    
    /* ----------------- Socket work below ---------------- */

    int server_sock; /* Socket descriptors */
    /* Create socket
    - INET = IPv4 address, STREAM = TCP stream, 0 = automatic protocol */
    server_sock = socket(AF_INET, SOCK_STREAM, 0); /* Create socket */ 

    struct sockaddr_in server_address; /* Server address structure */
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT); // port num
    server_address.sin_addr.s_addr = INADDR_ANY; // accept any connections

    /* Binds serversock to address, associated with IPv4 address and its length */
    if(bind(server_sock, (struct sockaddr*) &server_address, sizeof(server_address))){
        syslog(LOG_ERR,"%s\n","[-]Bind() error\n");
        exit(EXIT_FAILURE);
    }
    
    /* Listen for incoming connections, 1 connection at a time */
    if(listen(server_sock, 2) < 0){
        syslog(LOG_ERR, "%s\n", "[-]Listen() error\n");       
        exit(EXIT_FAILURE);
    }
    
    int client_sock;

    while(1){ /* Infinite loop, to constantly listen for new connections */  
        client_sock = accept(server_sock, NULL, NULL); /* Accept connection */
        if(client_sock < 0)        {
            syslog(LOG_ERR, "%s\n", "[-]Accept() error\n"); 
            exit(EXIT_FAILURE);
        }
        send(client_sock, "Waiting on connection......\n", 30, 0);sleep(2);/* Send message to syslog */
        send(client_sock, "---Hello you have connected to the daemon!\n---",43,0 );   
        
    }
    close(server_sock);
    close(client_sock);
    closelog();
    exit(EXIT_SUCCESS);
}

int main(){
    daemon_make();
    server_socket_TCP();
    return 0;

}

  
  