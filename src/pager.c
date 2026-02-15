#include <stdlib.h>
#include <string.h>
#include "pager.h"

Pager* pager_open(const char* filename) {
    FILE* file = fopen(filename, "rb+");
    if (!file) {
        file = fopen(filename, "wb+");
        if (!file) {
            perror("Unable to open database file");
            exit(1);
        }
    }

    fseek(file, 0, SEEK_END);
    int file_length = ftell(file);

    Pager* pager = malloc(sizeof(Pager));
    pager->file = file;
    pager->file_length = file_length;
    pager->num_pages = (file_length - sizeof(DBHeader)) / sizeof(Page);

    for (int i = 0; i < MAX_PAGES; i++) {
        pager->pages[i] = NULL;
    }

    return pager;
}

Page* pager_get_page(Pager* pager, int page_num) {
    if (page_num >= MAX_PAGES) {
        printf("Page number out of bounds\n");
        exit(1);
    }

    if (pager->pages[page_num] == NULL) {
        Page* page = malloc(sizeof(Page));
        memset(page, 0, sizeof(Page));

        if (page_num < pager->num_pages) {
            fseek(pager->file,
                  sizeof(DBHeader) + page_num * sizeof(Page),
                  SEEK_SET);
            fread(page, sizeof(Page), 1, pager->file);
        }

        pager->pages[page_num] = page;

        if (page_num >= pager->num_pages) {
            pager->num_pages = page_num + 1;
        }
    }

    return pager->pages[page_num];
}

void pager_flush(Pager* pager, int page_num) {
    if (pager->pages[page_num] == NULL) return;

    fseek(pager->file,
          sizeof(DBHeader) + page_num * sizeof(Page),
          SEEK_SET);
    fwrite(pager->pages[page_num], sizeof(Page), 1, pager->file);
}

void pager_close(Pager* pager) {
    for (int i = 0; i < pager->num_pages; i++) {
        if (pager->pages[i]) {
            pager_flush(pager, i);
            free(pager->pages[i]);
        }
    }

    fclose(pager->file);
    free(pager);
}
