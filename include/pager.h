#ifndef PAGER_H
#define PAGER_H

#include <stdio.h>
#include "database.h"

#define MAX_PAGES 1000

typedef struct {
    FILE* file;
    int file_length;
    int num_pages;
    Page* pages[MAX_PAGES];   // in-memory page cache
} Pager;

Pager* pager_open(const char* filename);
Page* pager_get_page(Pager* pager, int page_num);
void pager_flush(Pager* pager, int page_num);
void pager_close(Pager* pager);

#endif
