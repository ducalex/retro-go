/*
 * Copyright 2013, Alexander von Gluck, All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *              Alexander von Gluck IV <kallisti5@unixzen.com>
 */


#include "zipmgr.h"

#if 0

#include <zip.h>
#include <sys/stat.h>

#include "debug.h"


uint32
zipmgr_probe_file(char* zipFilename, char* foundGameFile)
{
	struct zip* zipHandle = zip_open(zipFilename, 0, NULL);
	if (zipHandle == NULL) {
		MESSAGE_ERROR("Zip %s error: %s\n", zipFilename,
			zip_strerror(zipHandle));
		return ZIP_ERROR;
	}

	zip_int64_t numberOfFiles = zip_get_num_entries(zipHandle,
		ZIP_FL_UNCHANGED);

	zip_int64_t position;
	// First scan through entire zip for an HCD
	for (position = 0; position < numberOfFiles; position++) {
		const char* filename = zip_get_name(zipHandle, position, 0);
		if (strcasestr(filename, ".HCD")) {
			MESSAGE_INFO("Found a valid cd game definition within zip file: "
				"%s\n", filename);
			strncpy(foundGameFile, filename, PATH_MAX);
			zip_close(zipHandle);
			return ZIP_HAS_HCD;
		}
	}

	// Then scan zip for PCE or ISO
	for (position = 0; position < numberOfFiles; position++) {
		const char* filename = zip_get_name(zipHandle, position, 0);
		if (strcasestr(filename, ".PCE")) {
			MESSAGE_INFO("Found a valid rom within zip file: %s\n",
				filename);
			strncpy(foundGameFile, filename, PATH_MAX);
			zip_close(zipHandle);
			return ZIP_HAS_PCE;
		}
		if (strcasestr(filename, ".ISO")) {
			MESSAGE_INFO("Found an ISO within the zip file: %s\n",
				filename);
			strncpy(foundGameFile, filename, PATH_MAX);
			zip_close(zipHandle);
			return ZIP_HAS_ISO;
		}
	}
	
	zip_close(zipHandle);
	return ZIP_HAS_NONE;
}


static void
safe_create_dir(const char *dir)
{
	if (mkdir(dir, 0755) < 0) {
		if (errno != EEXIST) {
			perror(dir);
			exit(1);
		}
	}
}


uint32
zipmgr_extract_to_disk(char* zipFilename, char* destination)
{
	struct zip* zipHandle = zip_open(zipFilename, 0, NULL);
	if (zipHandle == NULL) {
		MESSAGE_ERROR("Zip %s error: %s\n", zipFilename,
			zip_strerror(zipHandle));
		return ZIP_ERROR;
	}

	safe_create_dir(destination);
	printf("Extracting.");
	int i = 0;
	for (i = 0; i < zip_get_num_entries(zipHandle, 0); i++) {
		struct zip_stat sb;
		if (zip_stat_index(zipHandle, i, 0, &sb) == 0) {
			char outputFile[PATH_MAX];
			snprintf(outputFile, PATH_MAX, "%s%s%s", destination, PATH_SLASH, sb.name);
			int len = strlen(outputFile);
			if (outputFile[len - 1] == '/') {
				safe_create_dir(outputFile);
			} else {
				struct zip_file* zf = zip_fopen_index(zipHandle, i, 0);
				if (!zf) {
					fprintf(stderr, "boese, boese/n");
					return 1;
				}
				int fd = open(outputFile, O_RDWR | O_TRUNC | O_CREAT, 0644);
				if (fd < 0) {
					fprintf(stderr, "boese, boese/n");
					return 1;
				}
 
				long long sum = 0;
				while (sum != sb.size) {
					char buf[100];
					len = zip_fread(zf, buf, 100);
					if (len < 0) {
						fprintf(stderr, "boese, boese/n");
						return 1;
					}
					write(fd, buf, len);
					sum += len;
				}
				close(fd);
				zip_fclose(zf);
			}
			printf(".");
		} else {
			printf("File[%s] Line[%d]/n", __FILE__, __LINE__);
		}
	}
	printf("\n");
	return 0;
}


char*
zipmgr_extract_to_memory(char* zipFilename, char* cartFilename,
	size_t* cartSize)
{
	struct zip* zipHandle = zip_open(zipFilename, 0, NULL);
	if (zipHandle == NULL) {
		MESSAGE_ERROR("Zip %s error: %s\n", zipFilename,
			zip_strerror(zipHandle));
		return NULL;
	}

	zip_int64_t fileIndex = zip_name_locate(zipHandle, cartFilename, 0);
	if (fileIndex < 0) {
		MESSAGE_ERROR("Zip %s error: %s\n", zipFilename,
			zip_strerror(zipHandle));
		return NULL;
	}

	struct zip_stat fileStat;
	zip_stat_index(zipHandle, fileIndex, 0, &fileStat);

	struct zip_file* cartHandle = zip_fopen_index(zipHandle, fileIndex, 0);
	if (cartHandle == NULL) {
		MESSAGE_ERROR("Zip %s error: %s\n", zipFilename,
			zip_strerror(zipHandle));
		zip_close(zipHandle);
		return NULL;
	}

	size_t alignedSize = fileStat.size + (fileStat.size % 16);
	char* extractedCart = calloc(alignedSize / 16, 16);

	if (extractedCart == NULL) {
		MESSAGE_ERROR("Memory allocation error!\n");
		zip_fclose(cartHandle);
		zip_close(zipHandle);
		return NULL;
	}

	size_t bytesExtracted = zip_fread(cartHandle, extractedCart, fileStat.size);

	if (bytesExtracted != fileStat.size) {
		MESSAGE_ERROR("Error reading file from zip archive!\n");
		zip_fclose(cartHandle);
		zip_close(zipHandle);
		free(extractedCart);
		return NULL;
	} else
		*cartSize = bytesExtracted;

	zip_fclose(cartHandle);
	zip_close(zipHandle);
	return extractedCart;
}
#else
uint32
zipmgr_probe_file(char* zipFilename, char* foundGameFile)
{
    return 0;
}

char*
zipmgr_extract_to_memory(char* zipFilename, char* cartFilename,
    size_t* cartSize)
{
    return NULL;
}

uint32
zipmgr_extract_to_disk(char* zipFilename, char* destination)
{
    return 0;
}
#endif
