/*
- Daemon for the program.
- It contains server side socket functionalities, a seperate client program exist
- Coursework wants 'daemon that LISTENS', therefore server-side resides here
- Client calls sharing lib, that sockes into daemon w/ server.
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
#include <sys/un.h> /* for UNIX domain, sysnet used for TCP*/

#define SOCKETPATH "/tmp/daemon_socket_data"

int main(void)
{
    pid_t pid, sid;
      
    
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
    /* 
    * Due to using a UNIX domain socket rather than a TCP,
    * We have to close any terminal I/O, but re-open them not connected to terminals 
    * Essentially redirecting what goes to stdout (1) redirects to (2) stderr
    * */
    close(STDIN_FILENO); 
    close(STDOUT_FILENO); 
    close(STDERR_FILENO); 

   

    /*int nullfd = open("/dev/null", O_RDWR);
    if (nullfd == -1) {
        syslog(LOG_ERR, "Failed to open /dev/null: %s", "devnullerr");
        exit(EXIT_FAILURE);
    }
    dup2(nullfd, STDIN_FILENO);
    dup2(nullfd, STDOUT_FILENO);
    dup2(nullfd, STDERR_FILENO);*/

    /* ----------------- Socket work below ---------------- */

    /* Initialise server socket, not TCP but UNIX domain hence _un and AF_UNIX */
    unlink(SOCKETPATH);
    //potentially implement a clear previous sockets from path here

    int client_sock, server_sock ; /* socket descriptors */
    struct sockaddr_un server_address;
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if(server_sock<0){
        syslog(LOG_ERR,"Server socket creation failed!\n");
        exit(EXIT_FAILURE);
    }

    /*
    1. Clears server_address memory to zero
    2. Ensure family addressing is for UNIX, not TCP
    3. Copy the socket path into sun_path, where -1 represents \0
    */
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX; /* Stores path on sys as 'server IP' */
    strncpy(server_address.sun_path, SOCKETPATH, sizeof(server_address.sun_path) - 1);

    /* Bind socket to path and daemon */
    if (bind(server_sock, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        syslog(LOG_ERR, "%s\n", "binderr");
        close(server_sock); /* Close server socke and exit with fail, loggin err */
        exit(EXIT_FAILURE);
    }

    listen(server_sock,1);
    while(1){
        client_sock = accept(server_sock,NULL,NULL);
        if(client_sock <0){
            syslog(LOG_ERR, "%s\n", "accepterr");
            //close(client_sock); /* Close client socket after fail */
            //exit(EXIT_FAILURE);
        }

        const char *welcome_msg = "---You have succesfully connected to the daemon!\n";
        send(client_sock,welcome_msg,strlen(welcome_msg),0);
        close(client_sock);
    }
    close(server_sock);
    unlink(SOCKETPATH);
    closelog();
    exit(EXIT_SUCCESS);
    


    return 0;
}
  
  
  