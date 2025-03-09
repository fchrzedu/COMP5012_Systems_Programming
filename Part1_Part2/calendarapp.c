#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h> // used for isdigit 

typedef struct { // defines calendar entries
    int day;
    int month;
    int year;
    char description[100]; // 100 char limit
} CalEntry;

bool isValidFormat(const char* date){
    /*Check whether '/' is in appropriate location within format DD/MM/YYYY
    Check whether non slash characters are digits*/    
    if(date[2] != '/' || date[5] != '/'){return false;} // if slashes dont already exist where they are meant to
    if(strlen(date) != 10){return false;} 
    // if non slash characters are not numbers
    for(int i = 0; i < strlen(date); i++){
        if((i!=2 && i!= 5) && !isdigit(date[i])){
            return false;
        }
    }
    return true;
}
bool isValidDate(int d, int m, int y){
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    //Certain months have 30 or 31 days, and takes leap years into consideration
    if(y < 1000 || y > 3000){return false;}//ambigious bounds
    if(m<1||m>12){return false;}
    if((y%400==0)||(y%4 == 0 && y % 100 != 0) ){days[1] = 29;}else{days[1] = 28;} // check modulo of year for leap
    if(d < 1 || d > days[m - 1]){return false;} // check whether DD is in
    return true;

}
bool checkDupliate(const char* d){
    //No need to check whether file exists, already happend during boot.
    FILE *fp = fopen("entries.txt","r");
    char line[150];char check[11];    
    while(fgets(line, sizeof(line),fp)){        
        sscanf(line, "%10s", check); //extract the first 10 letters with slashes from text line
        if(strcmp(check,d) == 0){ // compare strings
            fclose(fp);
            return true;
        }
    }
    fclose(fp);return false;    
}
void readKeyboard() {
    CalEntry new_entry;
    char date_i[11];
    bool sanitised = false;

    printf("-->Please enter date for new calendar entry in format DD/MM/YYYY\n>>");

    while (!sanitised) {
        fgets(date_i, sizeof(date_i), stdin);
        date_i[strcspn(date_i, "\n")] = 0; // Remove newline 
        
        if (!isValidFormat(date_i)) {
            printf("-->Invalid format, enter date in format DD/MM/YYYY\n>>");        }
        
        else if (checkDupliate(date_i)) {
            printf("-->Entry already exists, enter new date in format DD/MM/YYYY\n>>");
        }
        // Logically if both are correct, proceed with scanning date
        else {
            sscanf(date_i, "%d/%d/%d", &new_entry.day, &new_entry.month, &new_entry.year);
            // Check if the date itself is valid
            if (!isValidDate(new_entry.day, new_entry.month, new_entry.year)) {
                printf("-->Incorrect date, try again.\n");
            } else {
                sanitised = true; // Set flag to exit loop
            }
        }
    }
    
    getchar();
    
    printf("-->Now please enter description for such date\n>>");
    fgets(new_entry.description, sizeof(new_entry.description), stdin); // get new entry, check its bound size and use stdin as keyboard stream    
   
    size_t len = strcspn(new_entry.description, "\n"); // get index of \n and save it as len
    if (new_entry.description[len] == '\n') {
    new_entry.description[len] = '\0';  // Replace newline with null terminator at len index
    }    
    FILE *fptr;
    fptr = fopen("entries.txt", "a"); // Open the file for appending
    

    fprintf(fptr, "%02d/%02d/%04d, %s\n", new_entry.day, new_entry.month, new_entry.year, new_entry.description); // 'print' to textfile at new line
    fclose(fptr); // Close the file after appending

    printf("-->Entry successfully added to the file.\n");
}




void readFile() {
    char line_buffer[150];
    bool is_empty = true;
    FILE *fptr;
    fptr = fopen("entries.txt", "r");
    if (fptr == NULL) {
        printf("-->File doesn't exist! Please create entries first.\n");    
    } 
    else{
        printf("------STORED CALENDAR ENTRIES------\n");
        while(fgets(line_buffer, sizeof(line_buffer), fptr)){
            is_empty = false;
            CalEntry entr;
            sscanf(line_buffer, "%d/%d/%d, %[^\n]", &entr.day, &entr.month, &entr.year, entr.description); //  without  %[^\n] I couldn't read whitespaces
            printf("%02d/%02d/%04d - %s\n", entr.day, entr.month, entr.year, entr.description); // Print entry
        }
    }
    if(is_empty){printf("-->No entries.\n");}
    fclose(fptr);

}

void modifyEntry() {
    char date[11];
    FILE *fptr;
    fptr = fopen("entries.txt", "r+"); // open file for reading and writing    

    if (fptr == NULL) {
        printf("-->Error opening file for reading.\n");
        return; // empty return not neccesarily modular, but want to handle error catch before contunuing; switch case '3' doesn't need a return function type too
    }

    CalEntry entries[365]; // assuming there will be no more than a year's worth of entries
    int entryCount = 0;
    char line_buffer[150];
    
    // read all entries into  array
    while (fgets(line_buffer, sizeof(line_buffer), fptr)) {
        sscanf(line_buffer, "%d/%d/%d, %[^\n]", &entries[entryCount].day, &entries[entryCount].month, &entries[entryCount].year, entries[entryCount].description);
        entryCount++;
    }

    fclose(fptr); // close the file after reading

    // user input
    printf("-->Enter the date (DD/MM/YYYY) of the entry you want to modify: ");
    int day, month, year;
    fgets(date, sizeof(date), stdin);
    date[strcspn(date, "\n")] = 0; 

    // Validate the date format
    bool validFormat = false;
    while (!validFormat) {
        if (isValidFormat(date)) {validFormat = true;} else {
            printf("-->Invalid format, enter date in format DD/MM/YYYY\n>>");
            fgets(date, sizeof(date), stdin);
            date[strcspn(date, "\n")] = 0; // remove newline from date at the end by searching for \n in date.
        }
    }
    sscanf(date, "%d/%d/%d", &day, &month, &year);   
    bool validDate = false;
    while (!validDate) {
        if (isValidDate(day, month, year)) {
            validDate = true;
        } else {
            printf("-->Incorrect date, try again.\n");
            printf("-->Please enter date to modify the entry in format DD/MM/YYYY\n>>");
            fgets(date, sizeof(date), stdin);
            date[strcspn(date, "\n")] = 0; // remove newline from date at the end by searching for \n in date.
            sscanf(date, "%d/%d/%d", &day, &month, &year);
        }
    }
    getchar();

    // flags for logic control
    bool entryFound = false;
    bool entryModified = false;

    // loop through all entries and try to find the one that matches
    for (int i = 0; i < entryCount; i++) {
        // checks whether entered date == in file
        if (entries[i].day == day && entries[i].month == month && entries[i].year == year) {            
            printf("-->Found entry: %02d/%02d/%04d - %s\n", entries[i].day, entries[i].month, entries[i].year, entries[i].description);
            entryFound = true;  
            
            
            printf("-->Enter new description: ");
            fgets(entries[i].description, sizeof(entries[i].description), stdin);
            entries[i].description[strcspn(entries[i].description, "\n")] = '\0'; 
            entryModified = true;  
        }
    }

    
    if (!entryFound) {printf("-->Entry not found.\n");}

    //write update entry back into file
    if (entryModified) {
        fptr = fopen("entries.txt", "w"); 

        if (fptr == NULL) {
            printf("-->Error opening file for writing.\n");
        } else {
            //needs to loop through each file entry and then print it back into
            for (int i = 0; i < entryCount; i++) {
                fprintf(fptr, "%02d/%02d/%04d, %s\n", entries[i].day, entries[i].month, entries[i].year, entries[i].description);
            }
            printf("-->Entry modified successfully.\n");
            fclose(fptr); 
        }
    }
}




bool openCheckFile(FILE **fptr) { //dereferencing due to passing
    *fptr = fopen("entries.txt", "r");
    if (*fptr == NULL) {
        printf("-->File doesn't exist, creating new file called entries.txt...\n");
        *fptr = fopen("entries.txt", "w");
        if (*fptr == NULL) {
            return false; // file could not be created
        }
        printf("entries.txt successfully created!\n");
    }
    fclose(*fptr);
    return true;
}

bool menu() {
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
            //stops feeding in '1' leftover and the newline - loops through int until newline is found
            while(getchar()!='\n');
            readKeyboard();
            break;
        case 2:
            readFile();
            break;
        case 3:
            while(getchar()!='\n');
            modifyEntry();
            break;
        default:
            printf("-->Not a valid choice, try again\n");
            break;
    }
    return false; // Continue the program
}

int main(int argc, char *argv[]) {
    FILE *fptr;
    if (!openCheckFile(&fptr)) {
        printf("-->Could not open or create the file!\n");
        exit(EXIT_SUCCESS); // EXIT LIBRARY TO TERMINATE PROGRAM TO PREVENT FUTURE FAILURE
    }
    bool over = false;
    while (!over) {
        over = menu(); // Display the menu until the user exits
    }

    printf("-->Exiting program...\n");
    return 0;
}