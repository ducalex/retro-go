#if 0
/*
    fileio.c --
    File management.
*/

#include "shared.h"
#include "unzip.h"


uint8 *loadFromZipByName(char *archive, char *filename, int *filesize)
{
    char name[PATH_MAX];
    unsigned char *buffer;

    int zerror = UNZ_OK;
    unzFile zhandle;
    unz_file_info zinfo;

    zhandle = unzOpen(archive);
    if(!zhandle) return (NULL);

    /* Seek to first file in archive */
    zerror = unzGoToFirstFile(zhandle);
    if(zerror != UNZ_OK)
    {
        unzClose(zhandle);
        return (NULL);
    }

    /* Get information about the file */
    unzGetCurrentFileInfo(zhandle, &zinfo, &name[0], 0xff, NULL, 0, NULL, 0);
    *filesize = zinfo.uncompressed_size;

    /* Error: file size is zero */
    if(*filesize <= 0)
    {
        unzClose(zhandle);
        return (NULL);
    }

    /* Open current file */
    zerror = unzOpenCurrentFile(zhandle);
    if(zerror != UNZ_OK)
    {
        unzClose(zhandle);
        return (NULL);
    }

    /* Allocate buffer and read in file */
    buffer = malloc(*filesize);
    if(!buffer) return (NULL);
    zerror = unzReadCurrentFile(zhandle, buffer, *filesize);

    /* Internal error: free buffer and close file */
    if(zerror < 0 || zerror != *filesize)
    {
        free(buffer);
        buffer = NULL;
        unzCloseCurrentFile(zhandle);
        unzClose(zhandle);
        return (NULL);
    }

    /* Close current file and archive file */
    unzCloseCurrentFile(zhandle);
    unzClose(zhandle);

    memcpy(filename, name, PATH_MAX);
    return (buffer);
}

/*
    Verifies if a file is a ZIP archive or not.
    Returns: 1= ZIP archive, 0= not a ZIP archive
*/
int check_zip(char *filename)
{
    uint8 buf[2];
    FILE *fd = NULL;
    fd = fopen(filename, "rb");
    if(!fd) return (0);
    fread(buf, 2, 1, fd);
    fclose(fd);
    if(memcmp(buf, "PK", 2) == 0) return (1);
    return (0);
}


/*
    Returns the size of a GZ compressed file.
*/
int gzsize(gzFile *gd)
{
    #define CHUNKSIZE   (0x10000)
    int size = 0, length = 0;
    unsigned char buffer[CHUNKSIZE];
    gzrewind(gd);
    do {
        size = gzread(gd, buffer, CHUNKSIZE);
        if(size <= 0) break;
        length += size;
    } while (!gzeof(gd));
    gzrewind(gd);
    return (length);
    #undef CHUNKSIZE
}
#endif
