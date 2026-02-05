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
    fread(&header, sizeof(DBHeader), 1, ptr);

    if (header.magic != DB_MAGIC) {
        printf("Invalid Database file \n");
        fclose(ptr);
        exit(1);
    }

    // Calculate total_pages_in_db_file (1 header + N Pages)
    int total_pages_in_db_file = (header.row_count / (PAGE_SIZE - sizeof(PageHeader)) / sizeof(Page)) + 1;

    // seek to last page
    fseek(ptr, sizeof(DBHeader) + (total_pages_in_db_file -1) * sizeof(Page), SEEK_SET);

    Page page;
    fread(&page, sizeof(Page), 1, ptr);

    // calculate rows per page (how much rows should exit in one page)

    int rows_per_page = (PAGE_SIZE - sizeof(PageHeader)) / sizeof(Row);

    if (page.header.page_row_count < rows_per_page) {
        // insert row in this page
        page.rows[page.header.page_row_count] = row;
        page.header.page_row_count++;

        fseek(ptr, sizeof(DBHeader) + (total_pages_in_db_file -1) * sizeof(Page), SEEK_SET);
        fwrite(&page, sizeof(Page), 1, ptr);
    }else {
        memset(&page, 0, sizeof(Page));
        page.rows[0] = row;
        page.header.page_row_count = 1;
        fseek(ptr, 0, SEEK_END);
        fwrite(&page, sizeof(Page), 1, ptr);
        total_pages_in_db_file++;
    }

    header.row_count++;
    fseek(ptr, 0, SEEK_SET);
    fwrite(&header, sizeof(DBHeader), 1, ptr);
    fclose(ptr);
}

void select_all(const char* filename){
    FILE* ptr = fopen(filename, "rb");
    if (!ptr) {
        perror("Database file cannot open.");
        return;
    }

    DBHeader header;

    fread(&header, sizeof(DBHeader), 1, ptr);
    if (header.magic != DB_MAGIC) {
        printf("Invalid database file \n");
        fclose(ptr);
        return;
    }

    int rows_per_page = (PAGE_SIZE - sizeof(PageHeader)) / sizeof(Row);
    int total_pages_in_db_file = (header.row_count / rows_per_page) + (header.row_count % rows_per_page != 0);

    for (int i = 0; i < total_pages_in_db_file; i++) {
        Page page;
        fread(&page, sizeof(Page), 1, ptr);

        for (int j = 0; j < page.header.page_row_count; j++) {
            Row row = page.rows[j];
            printf("%d | %s | %d\n", row.id, row.name, row.age);
        }
    }
    fclose(ptr);
}
