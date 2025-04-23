/*
- Client side socket program.
- Used to connect to the daemon through the shared library

*/

#define SERVER_PORT 9002 /* TCP */
#define SERVER_ADDR "127.0.0.1" /* Always connects on 127.0.0.1 to daemon*/
#def

int main(){
    char ID[256], secret[256], data[1024];
    
    if(daemonConnection(SERVER_ADDR, SERVER_PORT) != 0){
        /* if cannot connect to daemon through lib*/
        printf("\n");
        
    }
}