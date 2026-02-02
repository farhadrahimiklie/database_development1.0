#ifndef DATABASE_H
#define  DATABASE_H

// declare macros for this projects
#define DB_MAGIC 0x54444B42 // TDKB
#define DB_VERSION 1

#define PAGE_SIZE 4096 // 4KB

#define NAME_MAX 64

typedef struct {
    int id;
    char name[NAME_MAX];
    int age;
}Row;

typedef  struct {
    int magic;
    int version;
    int row_count;
}DBHeader;

// PageHeader is track how many rows inside a page
typedef struct {
    int page_row_count;
}PageHeader;

typedef  struct {
    PageHeader header;
    Row rows[(PAGE_SIZE - sizeof(PageHeader)) / sizeof(Row)];
}Page;

void insert_row(const char* filename, Row row);
void select_all(const char* filename);

#endif
