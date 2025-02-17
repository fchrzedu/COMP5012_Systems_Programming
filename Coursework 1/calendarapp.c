#include <stdio.h>
#include <stdbool.h>

typedef struct{
    int day;
    int month;
    int year;
    char description[50];
} CalEntry;

void readKeyboard(){
    CalEntry new_entry;
    printf("Please enter date for new calendar entry in format DD/MM/YYYY> ");
    scanf("%d/%d/%d", &new_entry.day, &new_entry.month, &new_entry.year);

    getchar();  // To consume the newline character left by scanf after reading the date

    printf("\nPlease now enter description for such date> ");
    fgets(new_entry.description, sizeof(new_entry.description), stdin); 

    // Print the new entry
    printf("Event on %02d/%02d/%04d: %s\n", new_entry.day, new_entry.month, new_entry.year, new_entry.description);

    
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
