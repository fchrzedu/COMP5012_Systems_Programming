#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h> // used for isdigit 
typedef struct {
    int day;
    int month;
    int year;
    char description[100]; // 100 char limit
} CalEntry;

bool isValidFormat(const char* date){
    /*Check whether '/' is in appropriate location within format DD/MM/YYYY
    Check whether non slash characters are digits*/    
    if(date[2] != '/' || date[5] != '/'){return false;}
    if(strlen(date) != 10){return false;}
    for(int i = 0; i < strlen(date); i++){
        if((i!=2 && i!= 5) && !isdigit(date[i])){
            return false;
        }
    }
    return true;
}
bool isValidDate(int d, int m, int y){
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    /*
    5   Certain months have 30 or 31 days, and takes leap years into consideration
    */
    if(y < 1000 || y > 3000){return false;}//ambigious bounds
    if(m<1||m>12){return false;}
    if((y%400==0)||(y%4 == 0 && y % 100 != 0) ){days[1] = 29;}else{days[1] = 28;}
    if(d < 1 || d > days[m - 1]){return false;}
    return true;

}
void readKeyboard() {
    CalEntry new_entry;
    char date_i[11];//Create array to store input

    printf("Please enter date for new calendar entry in format DD/MM/YYYY\n>>");
    fgets(date_i,sizeof(date_i),stdin);    
    date_i[strcspn(date_i, "\n")] = 0;//remove newline from fgets - stackoverf
    

    while(!isValidFormat(date_i)){ // calls func to check format
        printf("Invalid format, enter date in format DD/MM/YYYY\n>>");
        fgets(date_i,sizeof(date_i),stdin);
        date_i[strcspn(date_i, "\n")] = 0;        
    }
    sscanf(date_i, "%d/%d/%d", &new_entry.day, &new_entry.month, &new_entry.year);
    while(!isValidDate(new_entry.day, new_entry.month, new_entry.year))
    {
        printf("Incorrect date, try again.\n");
        printf("Please enter date for new calendar entry in format DD/MM/YYYY\n>>");
        fgets(date_i,sizeof(date_i),stdin);
        date_i[strcspn(date_i,"/n")] = 0; // remove newline
        sscanf(date_i, "%d/%d/%d", &new_entry.day, &new_entry.month, &new_entry.year);

    }
    getchar();
    printf("Now please enter description for such date\n>>");
    fgets(new_entry.description, sizeof(new_entry.description), stdin); // get new entry, check its bound size and use stdin as keyboard stream    
   
    /*
    Get the size of the description and search for \n
    if newline is present at the end of the string, replace it with a null terminator!!
    */
    size_t len = strcspn(new_entry.description, "\n");
    if (new_entry.description[len] == '\n') {
    new_entry.description[len] = '\0';  // Replace newline with null terminator
    }

    
    FILE *fptr;
    fptr = fopen("entries.txt", "a"); // Open the file for appending
    if (fptr == NULL) {
        printf("Error opening file for appending.\n");
        return;
    }

    fprintf(fptr, "%02d/%02d/%04d, %s\n", new_entry.day, new_entry.month, new_entry.year, new_entry.description);
    fclose(fptr); // Close the file after appending

    printf("Entry successfully added to the file.\n");
}




void readFile() {
    FILE *fptr;
    fptr = fopen("entries.txt", "r");
    if (fptr == NULL) {
        printf("File doesn't exist! Please create entries first.\n");
        return;
    }
    char line_buffer[150];

    
    bool is_empty = true;
    printf("------STORED CALENDAR ENTRIES------\n");
    while(fgets(line_buffer, sizeof(line_buffer), fptr)){
        is_empty = false;
        CalEntry entr;
        sscanf(line_buffer, "%d/%d/%d, %[^\n]", &entr.day, &entr.month, &entr.year, entr.description);
        printf("%02d/%02d/%04d - %s\n", entr.day, entr.month, entr.year, entr.description); // Print entry
    }
    if(is_empty){printf("No entries.\n");}

}

bool deleteEntry()[
    return false;
]
void modifyEntry() {
    char[1] choice;
    FILE *fptr;
    fptr = fopen("entries.txt", "r+"); // Open file for reading and writing
    if (fptr == NULL) {
        printf("File doesn't exist! Please create entries first.\n");
        return;
    }

    CalEntry entries[365]; // Assuming there will be no more a years of entries
    int entryCount = 0;
    char line_buffer[150];
    while (fgets(line_buffer, sizeof(line_buffer), fptr)) {
        sscanf(line_buffer, "%d/%d/%d, %[^\n]", &entries[entryCount].day, &entries[entryCount].month, &entries[entryCount].year, entries[entryCount].description);
        entryCount++;
    }

    fclose(fptr); // Close the file after reading

    printf("Do you wish to modify a date description (M) or delete an entry (D)?\n>>");

   

    printf("Enter the date (DD/MM/YYYY) of the entry you want to modify: ");
    int day, month, year;
    scanf("%d/%d/%d", &day, &month, &year);
    getchar(); // consume newline character

    bool found = false;
    for (int i = 0; i < entryCount; i++) {
        if (entries[i].day == day && entries[i].month == month && entries[i].year == year) {
            found = true;
            printf("Found entry: %02d/%02d/%04d - %s\n", entries[i].day, entries[i].month, entries[i].year, entries[i].description);
            printf("Enter new description: ");
            fgets(entries[i].description, sizeof(entries[i].description), stdin);
            // Remove trailing newline from description
            entries[i].description[strcspn(entries[i].description, "\n")] = '\0';
            break;
        }
    }

    if (!found) {
        printf("Entry not found.\n");
        return;
    }

    // Now overwrite the file with modified entries
    fptr = fopen("entries.txt", "w"); // Open the file in write mode to overwrite
    if (fptr == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }

    for (int i = 0; i < entryCount; i++) {
        fprintf(fptr, "%02d/%02d/%04d, %s\n", entries[i].day, entries[i].month, entries[i].year, entries[i].description);
    }

    fclose(fptr); // Close the file after writing
    printf("Entry modified successfully.\n");
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
            //stops feeding in '1' leftover and the newline
            while(getchar()!='\n');
            readKeyboard();
            break;
        case 2:
            readFile();
            break;
        case 3:
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