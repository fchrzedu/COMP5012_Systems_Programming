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

#define SOCKET_PATH "/tmp/unixdomainsocket"

int main(){
    pid_t pid,sid;

    openlog("{daemon_log}",LOG_PID,LOG_DAEMON);

    pid = fork();
    if(pid<0){
        syslog(LOG_ERR, "%s\n", "[-]forking error\n"); 
    }
    if(pid>0){exit(EXIT_SUCCESS);}
    syslog(LOG_NOTICE,"[+]Child orph\n");

    if((sid = setsid()) < 0) { /* Create new unique session */
        syslog(LOG_ERR, "%s\n", "[-]setsid"); /* Log error if failed to setsid */
        exit(EXIT_FAILURE);
    }
    syslog(LOG_NOTICE,"[+]Made Session\n");
    /* Change to root directory  */
    if((chdir("/")) < 0) {
        syslog(LOG_ERR, "%s\n", "[-]chdir");
        exit(EXIT_FAILURE);
    }

    syslog(LOG_NOTICE,"[+]Changed dir\n");
    umask(0);    
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    syslog(LOG_NOTICE,"[+]Closed std - daemon deattached\n");

    
    int client_sock, server_sock ; /* socket descriptors */
    struct sockaddr_un server_address;
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if(server_sock<0){
        syslog(LOG_ERR,"[-]Server socket creation failed!\n");
        exit(EXIT_FAILURE);
    }
    syslog(LOG_NOTICE,"[+]socket() done");

    char buff[108]; /* exact size of sun_path */
    memset(buff, 0, sizeof(buff));
    strncpy(buff, SOCKET_PATH, sizeof(buff) - 1);
    syslog(LOG_NOTICE, "testbuf = %s", buff);

    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX; /* Stores path on sys as 'server IP' */
    strncpy(server_address.sun_path, SOCKET_PATH, 25);
    syslog(LOG_NOTICE,"[+]Memset worked");

    unlink("/tmp/unixdaemonsocket");

    /* Bind socket to path and daemon */
    if (bind(server_sock, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        syslog(LOG_ERR, "%s\n", "[-]Bind()error");
        close(server_sock); /* Close server socket and exit with fail, loggin err */
        exit(EXIT_FAILURE);
    }
    syslog(LOG_NOTICE,"[+]Bind() success\n");


    listen(server_sock,1);
    while(1){
        /* Accept() below was throwing sigsegv errors */
        struct sockaddr_un client_address;
        socklen_t client_len = sizeof(client_address);
        /*
        -> 1st = server socket that is listening to connections
        -> 2nd = ptr to socket of address struct sockaddr* (so client)
            casted to know the socket type (which original accept() was missing)
        */
        client_sock = accept(server_sock, (struct sockaddr*)&client_address, &client_len);

        if(client_sock <0){
            syslog(LOG_ERR, "%s\n", "[-]Accept() error");
            close(client_sock); 
            exit(EXIT_FAILURE);
        }
        syslog(LOG_NOTICE,"[+]Accept() success\n");
        const char *connecting_msg = ">>Connecting....\n";
        const char *welcome_msg = "---You have succesfully connected to the daemon!\n";
        send(client_sock,connecting_msg,strlen(connecting_msg),0);
        
        send(client_sock,welcome_msg,strlen(welcome_msg),0);
        close(client_sock);
    }
    close(server_sock);
    unlink(SOCKET_PATH);
    closelog();
    exit(EXIT_SUCCESS);




    return 0;
}