#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"

#define INPUT_SIZE 256

int main(){
    char input[INPUT_SIZE];
    const char* db_file = "TDKB.dat";
    printf("TinyDB started...\n");
    printf("Type 'exit' to quit\n");

    while (1) {
        printf("tinydb > ");
        if (!fgets(input, INPUT_SIZE, stdin)) {
            break;
        }
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0) {
            break;
        }

        if (strncmp(input, "insert", 6) == 0) {
            Row row;
            int result = sscanf(input, "insert %d %s %d", &row.id, row.name, &row.age);

            if (result == 3) {
                insert_row(db_file, row);
                printf("Row Inserted Successfully.\n");
            }else {
                printf("Invalid Insert Format\n");
            }
        }else if(strcmp(input, "select") == 0){
            select_all(db_file);
        }else {
            printf("Unknown Commands");
        }
    }
    printf("Bye\n");
    return 0;
}
