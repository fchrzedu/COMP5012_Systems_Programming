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
int main(void)
{
    pid_t pid, sid;
    time_t timebuf;
    int fd, len;
      
    /* Open the system log */
    openlog("lpedated", LOG_PID, LOG_DAEMON);
    
    /* Fork our child process which will be the actual daemon */
    pid = fork();
    if(pid < 0) {
        syslog(LOG_ERR, "%s\n", "perror");
        exit(EXIT_FAILURE);
    }
    if(pid > 0)
        /* In the parent, let's bail */
        exit(EXIT_SUCCESS);
        
    /* In the child... */
    /* First, start a new session */
    if((sid = setsid()) < 0) {
        syslog(LOG_ERR, "%s\n", "setsid");
        exit(EXIT_FAILURE);
    }
   
    /* Next, make / the current directory */
    if((chdir("/")) < 0) {
        syslog(LOG_ERR, "%s\n", "chdir");
        exit(EXIT_FAILURE);
    }
    /* Reset the file mode */
    umask(0);
    /* Close stdin, etc. */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    /* Finally, do our work */
    /* Try converting a sample time to a string to see how much memory is used */
    time(&timebuf);
    len = strlen(ctime(&timebuf));
    /* Allocate that much memory */
    char *buf = malloc(sizeof(char) * (len + 1));
    if(buf == NULL) {
        syslog(LOG_ERR, "malloc");
        exit(EXIT_FAILURE);
    }       
    while(1) {
        /* Open our personal log file */    
        if((fd = open("/var/log/syslog", // changed to syslog
                      O_CREAT | O_WRONLY | O_APPEND, 0600)) < 0) {
            syslog(LOG_ERR, "open");
            exit(EXIT_FAILURE);
        }
        /* Get the time and convert it to a string, using the existing buffer */
        time(&timebuf);
        ctime_r(&timebuf, buf);
        /* Write the time to the file */
        write(fd, buf, len + 1);
        /* Close the file so that other processes can access it while we sleep */
        close(fd);
        /* Wait 60 seconds */ 
        sleep(60);
    }
    closelog();
    exit(EXIT_SUCCESS);
}
  
  