#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


typedef struct {int day; int month; int year; char description[100];} CalEntry;


void readKeyboard(){
    CalEntry new_entry;
    printf("--Please enter date for new calendar entry in the format DD/MM/YYYY\n>>");
    scanf("%d/%d/%d", &new_entry.day,&new_entry.month,&new_entry.year);
    while (new_entry.day > 31 || new_entry.day < 1 || new_entry.month > 12 || new_entry.month < 1 || new_entry.year < 1000) { // boundary checking
        printf("--Incorrect date, try again.\n");
        printf("--Please enter date for new calendar entry in format DD/MM/YYYY\n>>");
        scanf("%d/%d/%d", &new_entry.day, &new_entry.month, &new_entry.year);
    }
    getchar();
    printf("--Enter description for %d/%d/%d\n>>",new_entry.day, new_entry.month, new_entry.year);
    fgets(new_entry.description,sizeof(new_entry.description),stdin);//stdin = keyboard I/O
    /*
    Get the size of the description and search for \n
    if newline is present at the end of the string, replace it with a null terminator!!
    */
    size_t len = strcspn(new_entry.description, "\n");
    if (new_entry.description[len] == '\n') {
    new_entry.description[len] = '\0';  
    }

    FILE *fptr = fopen("entries.txt","a");
    if(fptr != NULL){
        fprintf(fptr, "%02d/%02d/%04d, %s\n", new_entry.day, new_entry.month, new_entry.year, new_entry.description);     
        printf("Succesfully added new entry to file.\n");  
    }
    else{
        printf("--Error opening file to add new entry!\n");
    }
    fclose(fptr);
}

void readFile(){
    FILE *fptr;
    fptr = fopen("entries.txt","r");
    if(fptr==NULL){
        printf("File doesn't exist\n");
    }
}
bool menu(){
    int choice = 0;
    printf("\nMENU FOR COMMANDS:\n");
    printf("1 Create calendar entries\n");
    printf("2 View all stored entries\n");
    printf("3 Modify entries stored\n");
    printf("0 Exit\n");
    printf("Enter choice represented by its integer\n>> ");
    scanf("%d", &choice);

    switch (choice) {
        case 0:
            return true; // Exit
        case 1:
            readKeyboard();
            break;
        case 2:
            readFile();
            break;
        case 3:
            //modifyEntry();
            break;
        default:
            printf("Not a valid choice, try again\n");
            break;
    }
    return false; // Continue the program
    
}
bool openCheckFile(FILE **fptr) {
    *fptr = fopen("entries.txt", "r");
    if (*fptr == NULL) {
        printf("File doesn't exist, creating new file called entries.txt...\n");
        *fptr = fopen("entries.txt", "w");
        if (*fptr == NULL) {
            return false; // File could not be created
        }
        printf("entries.txt successfully created!\n");
    }
    fclose(*fptr); // Close the file after checking
    return true;
}
int main(int argc, char *argv[]) {
    FILE *fptr;
    if (!openCheckFile(&fptr)) {
        printf("Could not open or create the file!\n");
        return 1; // Exit the program if the file can't be created or opened
    }

    bool over = false;
    while (!over) {
        over = menu(); // Display the menu until the user exits
    }

    printf("Exiting program...\n");
    return 0;
}