#ifndef TEST_LITTLEFS_COMMON_H__
#define TEST_LITTLEFS_COMMON_H__

#include "esp_littlefs.h"
#include "sdkconfig.h"

#include "errno.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_idf_version.h"
#include "esp_log.h"
#include "esp_partition.h"
#include "esp_rom_sys.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_vfs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "test_utils.h"
#include "unity.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/unistd.h>
#include <time.h>

#define littlefs_base_path "/littlefs"
extern const char littlefs_test_partition_label[];
extern const char littlefs_test_hello_str[];


void test_littlefs_create_file_with_text(const char* name, const char* text);
void test_littlefs_read_file(const char* filename);
void test_littlefs_read_file_with_content(const char* filename, const char* expected_content);

void test_setup();
void test_teardown();


#endif
