/*
- Daemon for the program.
- It contains server side socket functionalities, a seperate client program exist
- Coursework wants 'daemon that LISTENS', therefore server-side resides here
- Client calls sharing lib, that sockes into daemon w/ server.
*/
/*
* lpedated.c - Simple timestamping daemon. From
* Linux Programming by Example by Kurt Wall
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

int main(void)
{
    pid_t pid, sid;
    time_t timebuf;
    int fd, len;
      
    /*
    Identify as lpedated - start log
    */
    openlog("daemondataserver", LOG_PID, LOG_DAEMON);
    
    /* Need a non parented PID */
    pid = fork();
    if(pid < 0) { 
        syslog(LOG_ERR, "%s\n", "perror"); /* priority, with message*/
        exit(EXIT_FAILURE);
    }
    if(pid > 0){exit(EXIT_SUCCESS);} /* Parent exists */      
        
        
    /* ------------In the child (orphaned)...------------- */

    if((sid = setsid()) < 0) { /* Create new unique session */
        syslog(LOG_ERR, "%s\n", "setsid"); /* Log error if failed to setsid */
        exit(EXIT_FAILURE);
    }
   
    /* Change to root directory  */
    if((chdir("/")) < 0) {
        syslog(LOG_ERR, "%s\n", "chdir");
        exit(EXIT_FAILURE);
    }

    /* Reset the file mode */
    umask(0); /* Protection */
    /* Close stdin, etc. */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    /* ----------------- Socket work below ---------------- */

    int server_sock, client_sock; /* Socket descriptors */
    /* Create socket
    - INET = IPv4 address, STREAM = TCP stream, 0 = automatic protocol */
    server_sock = socket(AF_INET, SOCK_STREAM, 0); /* Create socket */

    struct sockaddr_in server_address; /* Server address structure */
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002); // port num
    server_address.sin_addr.s_addr = INADDR_ANY; // accept any connections

    /* Binds serversock to address, associated with IPv4 address and its length */
    bind(server_sock, (struct sockaddr*) &server_address, sizeof(server_address));

    /* Listen for incoming connections, 1 connection at a time */
    listen(server_sock, 1);

    while(1){ /* Infinite loop, to constantly listen for new connections */  
        client_sock = accept(server_sock, NULL, NULL); /* Accept connection */
        if(client_sock < 0)        {
            syslog(LOG_ERR, "%s\n", "accept"); 
            exit(EXIT_FAILURE);
        }
        send(client_sock, "---Waiting on connection---\n", 30, 0); /* Send message to syslog */
        send(client_sock, "---Hello you have connected to the daemon!\n---",43,0 );
        close(server_sock);
        close(client_sock);
    }
    
    
    closelog();
    exit(EXIT_SUCCESS);
}
  
  
  