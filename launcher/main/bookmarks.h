#include "emulators.h"

typedef enum
{
    BOOK_TYPE_FAVORITE = 0,
    BOOK_TYPE_RECENT,
    BOOK_TYPE_CUSTOM1,
    BOOK_TYPE_CUSTOM2,
    BOOK_TYPE_COUNT,
} book_type_t;

typedef retro_emulator_file_t bookmark_t;

typedef struct
{
    char name[64];
    char path[168];
    bool initialized;
    long capacity;
    long count;
    retro_emulator_file_t *items;
    tab_t *tab;
} book_t;

void bookmarks_init(void);
bool bookmark_add(book_type_t book, retro_emulator_file_t *file);
bool bookmark_remove(book_type_t book, retro_emulator_file_t *file);
bool bookmark_clear(book_type_t book);
retro_emulator_file_t *bookmark_find(book_type_t book, retro_emulator_file_t *file);
