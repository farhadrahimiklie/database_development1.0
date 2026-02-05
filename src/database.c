#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"

static void init_db_if_needed(const char* filename){
    FILE* ptr = fopen(filename, "rb");
    if (ptr) {
        fclose(ptr);
        return;
    }
    ptr = fopen(filename, "wb");
    if (!ptr) {
        perror("Database File is not Created Yet.");
        exit(1);
    }

    DBHeader header;
    header.magic = DB_MAGIC;
    header.version = DB_VERSION;
    header.row_count = 0;

    fwrite(&header, sizeof(DBHeader), 1, ptr);

    // create empty page with 0 rows
    Page page;
    memset(&page, 0, sizeof(Page));
    page.header.page_row_count = 0;
    fwrite(&page, sizeof(Page), 1, ptr);
    fclose(ptr);
}

void insert_row(const char* filename, Row row){
    init_db_if_needed(filename); // Initialize the database file (header, page) first if not exist

    FILE* ptr = fopen(filename, "rb+"); // read and write to binary file
    if (!ptr) {
        perror("Database file cannot open for reading.");
        exit(1);
    }

    DBHeader header;

    if (fread(&header, sizeof(DBHeader), 1, ptr) != 1) {
        perror("Failed to read DB header");
        fclose(ptr);
        exit(1);
    }

    if (header.magic != DB_MAGIC) {
        printf("Invalid database file\n");
        fclose(ptr);
        exit(1);
    }

    // Calculate total_pages_in_db_file (1 header + N Pages)
    int rows_per_page = (PAGE_SIZE - sizeof(PageHeader)) / sizeof(Row);
    int total_pages = (header.row_count / rows_per_page) + (header.row_count % rows_per_page != 0);

    Page page;

    if (total_pages == 0) {
        // Should not happen, but just in case
        memset(&page, 0, sizeof(Page));
        page.header.page_row_count = 0;
        total_pages = 1;
    }

    // Read last page
    fseek(ptr, sizeof(DBHeader) + (total_pages - 1) * sizeof(Page), SEEK_SET);
    if (fread(&page, sizeof(Page), 1, ptr) != 1) {
        // If file ends here, create a new empty page
        memset(&page, 0, sizeof(Page));
        page.header.page_row_count = 0;
    }

    if (page.header.page_row_count < rows_per_page) {
        // insert row in this page
        page.rows[page.header.page_row_count] = row;
        page.header.page_row_count++;

        fseek(ptr, sizeof(DBHeader) + (total_pages -1) * sizeof(Page), SEEK_SET);
         if (fwrite(&page, sizeof(Page), 1, ptr) != 1) {
            perror("Failed to write page");
            fclose(ptr);
            exit(1);
        }
    }else {
        // create a new page
        memset(&page, 0, sizeof(Page));
        page.rows[0] = row;
        page.header.page_row_count = 1;
        fseek(ptr, 0, SEEK_END);

        if (fwrite(&page, sizeof(Page), 1, ptr) != 1) {
            perror("Failed to write new page");
            fclose(ptr);
            exit(1);
        }
        total_pages++;
    }

    // Update total row count in header
    header.row_count++;
    fseek(ptr, 0, SEEK_SET);

    if (fwrite(&header, sizeof(DBHeader), 1, ptr) != 1) {
        perror("Failed to update DB header");
        fclose(ptr);
        exit(1);
    }
    fclose(ptr);
}

void select_all(const char* filename){
    FILE* ptr = fopen(filename, "rb");
    if (!ptr) {
        perror("Database file cannot open.");
        return;
    }

    DBHeader header;

     if (fread(&header, sizeof(DBHeader), 1, ptr) != 1) {
        perror("Failed to read DB header");
        fclose(ptr);
        return;
    }
    
    if (header.magic != DB_MAGIC) {
        printf("Invalid database file \n");
        fclose(ptr);
        return;
    }

    int rows_per_page = (PAGE_SIZE - sizeof(PageHeader)) / sizeof(Row);
    int total_pages = (header.row_count / rows_per_page) + (header.row_count % rows_per_page != 0);

    for (int i = 0; i < total_pages; i++) {
        Page page;
        fseek(ptr, sizeof(DBHeader) + i * sizeof(Page), SEEK_SET);
        if (fread(&page, sizeof(Page), 1, ptr) != 1) {
            perror("Failed to read page");
            fclose(ptr);
            return;
        }

        for (int j = 0; j < page.header.page_row_count; j++) {
            Row row = page.rows[j];
            printf("%d | %s | %d\n", row.id, row.name, row.age);
        }
    }

    fclose(ptr);
}
