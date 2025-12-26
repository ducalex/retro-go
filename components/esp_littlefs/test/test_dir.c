#include "sdkconfig.h"

#ifdef CONFIG_VFS_SUPPORT_DIR

#include "test_littlefs_common.h"

//#define LOG_LOCAL_LEVEL 4

TEST_CASE("truncate", "[littlefs]")
{
    test_setup();

    FILE* f;
    char buf[10] = { 0 };
    const char fn[] = littlefs_base_path "/truncate.txt";

    f = fopen(fn, "w");
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL(11, fprintf(f, "0123456789\n"));
    TEST_ASSERT_EQUAL(0, fclose(f));

    TEST_ASSERT_EQUAL(0, truncate(fn, 3));

    f = fopen(fn, "r");
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL(3, fread(buf, 1, 8, f));
    TEST_ASSERT_EQUAL(0, fclose(f));
    TEST_ASSERT_EQUAL_STRING_LEN("012", buf, 8);

    test_teardown();
}

#ifdef ESP_LITTLEFS_ENABLE_FTRUNCATE
TEST_CASE("ftruncate", "[littlefs]")
{
    test_setup();

    int fd;
    FILE* f;
    char buf[10] = { 0 };
    const char fn[] = littlefs_base_path "/truncate.txt";

    f = fopen(fn, "w");
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL(11, fprintf(f, "0123456789\n"));
    TEST_ASSERT_EQUAL(0, fclose(f));

    fd = open(fn, O_RDWR);
    TEST_ASSERT_EQUAL(0, ftruncate(fd, 3));
    TEST_ASSERT_EQUAL(0, close(fd));


    f = fopen(fn, "r");
    TEST_ASSERT_NOT_NULL(f);
    TEST_ASSERT_EQUAL(3, fread(buf, 1, 8, f));
    TEST_ASSERT_EQUAL(0, fclose(f));
    TEST_ASSERT_EQUAL_STRING_LEN("012", buf, 8);

    test_teardown();
}
#endif // ESP_LITTLEFS_ENABLE_FTRUNCATE


static void test_littlefs_readdir_many_files(const char* dir_prefix)
{
    const int n_files = 40;
    const int n_folders = 4;
    unsigned char file_count[n_files * n_folders];
    memset(file_count, 0, sizeof(file_count)/sizeof(file_count[0]));
    char file_name[ESP_VFS_PATH_MAX + CONFIG_LITTLEFS_OBJ_NAME_LEN];

    /* clean stale files before the test */
    mkdir(dir_prefix, 0755);
    DIR* dir = opendir(dir_prefix);
    if (dir) {
        while (true) {
            struct dirent* de = readdir(dir);
            if (!de) {
                break;
            }
            int len = snprintf(file_name, sizeof(file_name), "%s/%s", dir_prefix, de->d_name);
            assert(len < sizeof(file_name));
            unlink(file_name);
        }
    }

    /* create files */
    for (int d = 0; d < n_folders; ++d) {
        printf("filling directory %d\n", d);
        snprintf(file_name, sizeof(file_name), "%s/%d", dir_prefix, d);
        mkdir( file_name, 0755 );
        for (int f = 0; f < n_files; ++f) {
            snprintf(file_name, sizeof(file_name), "%s/%d/%d.txt", dir_prefix, d, f);
            test_littlefs_create_file_with_text(file_name, file_name);
        }
    }

    /* list files */
    for (int d = 0; d < n_folders; ++d) {
        printf("listing files in directory %d\n", d);
        snprintf(file_name, sizeof(file_name), "%s/%d", dir_prefix, d);
        dir = opendir(file_name);
        TEST_ASSERT_NOT_NULL(dir);
        while (true) {
            struct dirent* de = readdir(dir);
            if (!de) {
                break;
            }
            int file_id;
            TEST_ASSERT_EQUAL(1, sscanf(de->d_name, "%d.txt", &file_id));
            file_count[file_id + d * n_files]++;
        }
        closedir(dir);
    }

    /* check that all created files have been seen */
    for (int d = 0; d < n_folders; ++d) {
        printf("checking that all files have been found in directory %d\n", d);
        for (int f = 0; f < n_files; ++f) {
            TEST_ASSERT_EQUAL(1, file_count[f + d * n_files]);
        }
    }
}

TEST_CASE("mkdir, rmdir", "[littlefs]")
{
    test_setup();
    const char filename_prefix[] = littlefs_base_path "/";

    char name_dir1[64];
    char name_dir2[64];
    char name_dir2_file[64];

    snprintf(name_dir1, sizeof(name_dir1), "%s1", filename_prefix);
    snprintf(name_dir2, sizeof(name_dir2), "%s2", filename_prefix);
    snprintf(name_dir2_file, sizeof(name_dir2_file), "%s2/1.txt", filename_prefix);

    TEST_ASSERT_EQUAL(0, mkdir(name_dir1, 0755));
    struct stat st;
    TEST_ASSERT_EQUAL(0, stat(name_dir1, &st));
    TEST_ASSERT_TRUE(st.st_mode & S_IFDIR);
    TEST_ASSERT_FALSE(st.st_mode & S_IFREG);
    TEST_ASSERT_EQUAL(0, rmdir(name_dir1));

    // Attempt to stat a removed directory
    TEST_ASSERT_EQUAL(-1, stat(name_dir1, &st));
    TEST_ASSERT_EQUAL(ENOENT, errno);

    TEST_ASSERT_EQUAL(0, mkdir(name_dir2, 0755));
    test_littlefs_create_file_with_text(name_dir2_file, "foo\n");
    TEST_ASSERT_EQUAL(0, stat(name_dir2, &st));
    TEST_ASSERT_TRUE(st.st_mode & S_IFDIR);
    TEST_ASSERT_FALSE(st.st_mode & S_IFREG);
    TEST_ASSERT_EQUAL(0, stat(name_dir2_file, &st));
    TEST_ASSERT_FALSE(st.st_mode & S_IFDIR);
    TEST_ASSERT_TRUE(st.st_mode & S_IFREG);

    // Can't remove directory, its not empty
    TEST_ASSERT_EQUAL(-1, rmdir(name_dir2));
    TEST_ASSERT_EQUAL(ENOTEMPTY, errno);

    TEST_ASSERT_EQUAL(0, unlink(name_dir2_file));
#if !CONFIG_LITTLEFS_SPIFFS_COMPAT
    /* this will have already been deleted */
    TEST_ASSERT_EQUAL(0, rmdir(name_dir2));
#endif

    test_teardown();
}

TEST_CASE("opendir, readdir, rewinddir, seekdir work as expected", "[littlefs]")
{
    test_setup();
    const char dir_prefix[] = littlefs_base_path "/dir";

    char name_dir_inner_file[64];
    char name_dir_inner[64];
    char name_dir_file3[64];
    char name_dir_file2[64];
    char name_dir_file1[64];

    snprintf(name_dir_inner_file, sizeof(name_dir_inner_file), "%s/inner/3.txt", dir_prefix);
    snprintf(name_dir_inner,      sizeof(name_dir_inner),      "%s/inner",       dir_prefix);
    snprintf(name_dir_file3,      sizeof(name_dir_file2),      "%s/boo.bin",     dir_prefix);
    snprintf(name_dir_file2,      sizeof(name_dir_file2),      "%s/2.txt",       dir_prefix);
    snprintf(name_dir_file1,      sizeof(name_dir_file1),      "%s/1.txt",       dir_prefix);

    /* Remove files/dirs that may exist */
    unlink(name_dir_inner_file);
    rmdir(name_dir_inner);
    unlink(name_dir_file1);
    unlink(name_dir_file2);
    unlink(name_dir_file3);
    rmdir(dir_prefix);

    /* Create the files */
    TEST_ASSERT_EQUAL(0, mkdir(dir_prefix, 0755));
    TEST_ASSERT_EQUAL(0, mkdir(name_dir_inner, 0755));
    test_littlefs_create_file_with_text(name_dir_file1, "1\n");
    test_littlefs_create_file_with_text(name_dir_file2, "2\n");
    test_littlefs_create_file_with_text(name_dir_file3, "\01\02\03");
    test_littlefs_create_file_with_text(name_dir_inner_file, "3\n");

    DIR* dir = opendir(dir_prefix);
    TEST_ASSERT_NOT_NULL(dir);
    int count = 0;
    const char* names[4];
    while( true ) {
        struct dirent* de = readdir(dir);
        if (!de) {
            break;
        }
        if (strcasecmp(de->d_name, "1.txt") == 0) {
            TEST_ASSERT_TRUE(de->d_type == DT_REG);
            names[count] = "1.txt";
            ++count;
        } else if (strcasecmp(de->d_name, "2.txt") == 0) {
            TEST_ASSERT_TRUE(de->d_type == DT_REG);
            names[count] = "2.txt";
            ++count;
        } else if (strcasecmp(de->d_name, "inner") == 0) {
            TEST_ASSERT_TRUE(de->d_type == DT_DIR);
            names[count] = "inner";
            ++count;
        } else if (strcasecmp(de->d_name, "boo.bin") == 0) {
            TEST_ASSERT_TRUE(de->d_type == DT_REG);
            names[count] = "boo.bin";
            ++count;
        } else {
            char buf[512] = { 0 };
            snprintf(buf, sizeof(buf), "unexpected directory entry \"%s\"", de->d_name);
            TEST_FAIL_MESSAGE(buf);
        }
    }
    TEST_ASSERT_EQUAL(count, 4);

    rewinddir(dir);
    struct dirent* de = readdir(dir);
    TEST_ASSERT_NOT_NULL(de);
    TEST_ASSERT_EQUAL(0, strcasecmp(de->d_name, names[0]));
    seekdir(dir, 3);
    de = readdir(dir);
    TEST_ASSERT_NOT_NULL(de);
    TEST_ASSERT_EQUAL(0, strcasecmp(de->d_name, names[3]));
    seekdir(dir, 1);
    de = readdir(dir);
    TEST_ASSERT_NOT_NULL(de);
    TEST_ASSERT_EQUAL(0, strcasecmp(de->d_name, names[1]));
    seekdir(dir, 2);
    de = readdir(dir);
    TEST_ASSERT_NOT_NULL(de);
    TEST_ASSERT_EQUAL(0, strcasecmp(de->d_name, names[2]));

    TEST_ASSERT_EQUAL(0, closedir(dir));

    test_teardown();
}

TEST_CASE("readdir with large number of files", "[littlefs][timeout=30]")
{
    test_setup();
    test_littlefs_readdir_many_files(littlefs_base_path "/dir2");
    test_teardown();
}

TEST_CASE("can opendir root directory of FS", "[littlefs]")
{
    test_setup();

    const char path[] = littlefs_base_path;

    char name_dir_file[64];
    const char * file_name = "test_opd.txt";
    snprintf(name_dir_file, sizeof(name_dir_file), "%s/%s", path, file_name);
    unlink(name_dir_file);
    test_littlefs_create_file_with_text(name_dir_file, "test_opendir\n");
    DIR* dir = opendir(path);
    TEST_ASSERT_NOT_NULL(dir);
    bool found = false;
    while (true) {
        struct dirent* de = readdir(dir);
        if (!de) {
            break;
        }
        if (strcasecmp(de->d_name, file_name) == 0) {
            found = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL(0, closedir(dir));
    unlink(name_dir_file);

    test_teardown();
}

TEST_CASE("unlink removes a file", "[littlefs]")
{
    test_setup();

    const char filename[] = littlefs_base_path "/unlink.txt";

    test_littlefs_create_file_with_text(filename, "unlink\n");
    TEST_ASSERT_EQUAL(0, unlink(filename));
    TEST_ASSERT_NULL(fopen(filename, "r"));

    test_teardown();
}

TEST_CASE("rename moves a file", "[littlefs]")
{
    test_setup();
    const char filename_prefix[] = littlefs_base_path "/move";

    char name_dst[64];
    char name_src[64];
    snprintf(name_dst, sizeof(name_dst), "%s_dst.txt", filename_prefix);
    snprintf(name_src, sizeof(name_src), "%s_src.txt", filename_prefix);

    unlink(name_dst);
    unlink(name_src);

    FILE* f = fopen(name_src, "w+");
    TEST_ASSERT_NOT_NULL(f);
    const char* str = "0123456789";
    for (int i = 0; i < 400; ++i) {
        TEST_ASSERT_NOT_EQUAL(EOF, fputs(str, f));
    }
    TEST_ASSERT_EQUAL(0, fclose(f));
    TEST_ASSERT_EQUAL(0, rename(name_src, name_dst));
    TEST_ASSERT_NULL(fopen(name_src, "r"));
    FILE* fdst = fopen(name_dst, "r");
    TEST_ASSERT_NOT_NULL(fdst);
    TEST_ASSERT_EQUAL(0, fseek(fdst, 0, SEEK_END));
    TEST_ASSERT_EQUAL(4000, ftell(fdst));
    TEST_ASSERT_EQUAL(0, fclose(fdst));

    test_teardown();
}


#endif
