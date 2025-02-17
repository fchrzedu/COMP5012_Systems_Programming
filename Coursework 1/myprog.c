#include <stdio.h>
#include <stdlib.h>

/*
argc = number of things entered into command line when executing
argv = array that holds string values that were entered

*/
int main(int argc, char* argv[]){
    printf("Printing arguments...\n\n");

    for(int i = 1; i < argc; i++){
        printf("%s ", argv[i]);

    }
    printf("\n");


return 0;
}