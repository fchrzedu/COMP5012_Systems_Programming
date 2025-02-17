#include <unistd.h>
#include <stdio.h>

void one(){
    for(int x=0; x<100; x++){
        printf("One.");
        fflush(stdout);
    }
}

void two(){
    for(int x=0;x<100;x++){
        printf("Two.");
        fflush(stdout);

    }
}

int main(){
    if(fork()){
        two();
    }else{
        one();
    }
}