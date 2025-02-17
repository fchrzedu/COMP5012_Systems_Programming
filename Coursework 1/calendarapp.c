#include <stdio.h>
#include <stdbool.h>

typedef struct{
    int day;
    int month;
    int year;
    char description[100]; //100 char limit
} CalEntry;

void readKeyboard(){
    CalEntry new_entry;
    printf("Please enter date for new calendar entry in format DD/MM/YYYY> ");
    scanf("%d/%d/%d", &new_entry.day, &new_entry.month, &new_entry.year); //added space after delim to consume newline
    printf("\nEnter description for date entered above> ");

    gets(*new_entry.description);

    printf("The date entered is %d/%d/%d, whereas the desc is %s\n",new_entry.day,new_entry.month,new_entry.year,new_entry.description);

    
}
bool menu(){
    int choice = 0;
    printf("MENU FOR COMMANDS:\n");
    printf("1 Read from keyboard\n");
    printf("2 Read input from file\n");
    printf("4 View all calendar entries\n");
    printf("3 Modify input data\n");

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
                readKeyboard();
                break;
            case 2:
               // readFile();
                break;
            default:
            break;


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
