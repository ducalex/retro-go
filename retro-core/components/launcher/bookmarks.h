#include "applications.h"

typedef enum
{
    BOOK_TYPE_FAVORITE = 0,
    BOOK_TYPE_RECENT,
    BOOK_TYPE_CUSTOM1,
    BOOK_TYPE_CUSTOM2,
    BOOK_TYPE_COUNT,
} book_type_t;

typedef struct
{
    const char *name;
    const char *path;
    size_t capacity;
    size_t count;
    retro_file_t *items;
    tab_t *tab;
    bool initialized;
} book_t;

void bookmarks_init(void);
bool bookmark_add(book_type_t book, const retro_file_t *file);
bool bookmark_remove(book_type_t book, const retro_file_t *file);
bool bookmark_clear(book_type_t book);
bool bookmark_exists(book_type_t book, const retro_file_t *file);
retro_file_t *bookmark_find_by_app(book_type_t book, const retro_app_t *app);
