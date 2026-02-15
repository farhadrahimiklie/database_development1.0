#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "database.h"
#include "pager.h"

/*
    Initialize database file if it does not exist.
    Only creates DBHeader.
*/
static void init_db_if_needed(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file) {
        fclose(file);
        return;
    }

    file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to create database file");
        exit(1);
    }

    DBHeader header;
    header.magic = DB_MAGIC;
    header.version = DB_VERSION;
    header.row_count = 0;

    fwrite(&header, sizeof(DBHeader), 1, file);
    fclose(file);
}

/*
    Insert a row using Pager
*/
void insert_row(const char* filename, Row row) {
    init_db_if_needed(filename);

    Pager* pager = pager_open(filename);

    /* Read DB header */
    DBHeader header;
    fseek(pager->file, 0, SEEK_SET);
    fread(&header, sizeof(DBHeader), 1, pager->file);

    if (header.magic != DB_MAGIC) {
        printf("Invalid database file\n");
        pager_close(pager);
        exit(1);
    }

    int rows_per_page =
        (PAGE_SIZE - sizeof(PageHeader)) / sizeof(Row);

    int page_num = header.row_count / rows_per_page;

    Page* page = pager_get_page(pager, page_num);

    /* Insert row */
    page->rows[page->header.page_row_count] = row;
    page->header.page_row_count++;

    /* Update header */
    header.row_count++;

    fseek(pager->file, 0, SEEK_SET);
    fwrite(&header, sizeof(DBHeader), 1, pager->file);

    pager_close(pager);
}

/*
    Sequential scan (SELECT *)
*/
void select_all(const char* filename) {
    Pager* pager = pager_open(filename);

    DBHeader header;
    fseek(pager->file, 0, SEEK_SET);
    fread(&header, sizeof(DBHeader), 1, pager->file);

    if (header.magic != DB_MAGIC) {
        printf("Invalid database file\n");
        pager_close(pager);
        return;
    }

    int rows_per_page =
        (PAGE_SIZE - sizeof(PageHeader)) / sizeof(Row);

    int total_pages =
        (header.row_count + rows_per_page - 1) / rows_per_page;

    for (int i = 0; i < total_pages; i++) {
        Page* page = pager_get_page(pager, i);

        for (int j = 0; j < page->header.page_row_count; j++) {
            Row r = page->rows[j];
            printf("%d | %s | %d\n", r.id, r.name, r.age);
        }
    }

    pager_close(pager);
}
