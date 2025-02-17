#include <stdio.h>
#include <stdbool.h>


bool menu(){
    int choice = 0;
    printf("MENU FOR COMMANDS:\n");
    printf("1 read from keyboard\n");
    printf("2 read input from file\n");
    printf("3 modify input data\n");
    printf("0 exit\n");
    printf("Enter choice represented by it's integer> ");    
    scanf("%d",&choice);

    if(choice == 0){
        return true;
    }
    //switch case here for each menu
    else{
        switch(choice){
            case 1:
                break;
            case 2:
                break;
            default:

        }
    }
    return false;

}

int main(int argc, char* argv[])
{
    /*int num;

    printf("Enter integer number : ");

    scanf("%d", &num);

    printf("You have entered %d\n",num);
    
    */

    bool over = false;
    while(!over){
        over = menu();
    }
    printf("Exiting program...\n");
    return 0;
}
