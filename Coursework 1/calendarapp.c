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
    scanf("%d/%d/%d", &new_entry.day, &new_entry.month, &new_entry.year); 
    while(new_entry.day > 31 || new_entry.day < 00 || new_entry.month > 12 || new_entry.month < 01 || new_entry.year < 0000){ // bound checking
        printf("Incorrect date, try again.\n");
        printf("Please enter date for new calendar entry in format DD/MM/YYYY> ");
        scanf("%d/%d/%d", &new_entry.day, &new_entry.month, &new_entry.year); 
    }
    getchar(); //using fgets, need to consume newline char
    printf("Now please enter description for such date> \n");
    fgets(new_entry.description, sizeof(new_entry.description),stdin);
    
    printf("Appending %d/%d/%d '%s' to entries.txt...", new_entry.day, new_entry.month, new_entry.year, new_entry.description);
    //add file append here
}
void createFile(){
    FILE *f = fopen("entries.txt","w");
    if(f==NULL){printf("Unable to create file.\n");}
    else{printf("Created entries.txt succesfully.\n");fclose(f);}
}
void readFile(){
    FILE *fptr = fopen("entries.txt", "r"); 
    if(fptr == NULL){
        printf("File entries.txt not found, creating a new one...\n");
        createFile();
        return;
    }
    // File exists, proceed to read
    fclose(fptr);
}
bool menu(){
    int choice = 0;
    printf("\nMENU FOR COMMANDS:\n"); // done - needs bound checking
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
                readFile();
                break;
            default:
            printf("Not a valid choice, try again\n");
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
