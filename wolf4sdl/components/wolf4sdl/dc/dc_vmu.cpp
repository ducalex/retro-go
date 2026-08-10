//Wolf4SDL\DC
//dc_vmu.cpp
//2009 - Cyle Terry

#if defined(_arch_dreamcast)

#include <string.h>
#include "../wl_def.h"
#include "dc/maple.h"
#include "dc/maple/controller.h"
#include "dc/maple/vmu.h"
#include "dc/vmu_pkg.h"
#include "kos/fs.h"
#include "zlib/zlib.h"
#include "dc_vmu.h"

maple_device_t *vmu_lcd_addr[8];

void DC_StatusDrawLCD(int lcd) {
    const char *character;
    int x, y;
    int xi, xb;
    int i = 0;
    uint8 bitmap[48 * 32 / 8];
    maple_device_t *vmu_lcd_addr;

    memset(bitmap, 0, sizeof(bitmap));
    character = e_BJFaces[lcd - FACE1APIC];

    if(character) {
        for(y = 0; y < LCD_HEIGHT; y++) {
            for(x = 0; x < LCD_WIDTH; x++) {
                xi = x / 8;
                xb = 0x80 >> (x % 8);
                if(character[(31 - y) * 48 + (47 - x)] == '.')
                    bitmap[y * (48 / 8) + xi] |= xb;
            }
        }
    }

    while ((vmu_lcd_addr = maple_enum_type(i++, MAPLE_FUNC_LCD)))
        vmu_draw_lcd(vmu_lcd_addr, bitmap);
    vmu_shutdown ();
}

void DC_StatusClearLCD() {
    int x, y;
    int xi;
    int i = 0;
    uint8 bitmap[48 * 32 / 8];
    maple_device_t *vmu_lcd_addr;

    memset(bitmap, 0, sizeof(bitmap));
    for(y = 0; y < LCD_HEIGHT; y++) {
        for(x = 0; x < LCD_WIDTH; x++) {
            xi = x / 8;
            bitmap[y * (48 / 8) + xi] |= 0;
        }
    }

    while ((vmu_lcd_addr = maple_enum_type(i++, MAPLE_FUNC_LCD)))
        vmu_draw_lcd(vmu_lcd_addr, bitmap);
    vmu_shutdown ();
}


void DC_SaveToVMU(char *fname, char *str) {
    char destination[32];
    int filesize = 0;
    int vmu_package_size;
    unsigned long  zipsize = 0;
    unsigned char *vmu_package_out;
    unsigned char *data;
    unsigned char *zipdata;
    file_t file;
    vmu_pkg_t vmu_package;

    DiskFlopAnim(102, 85);

    strcpy(destination, "/vmu/a1/");
    strcat(destination, fname);
    file = fs_open(fname, O_RDONLY);
    filesize = fs_total(file);
    data = (unsigned char*)malloc(filesize);
    fs_read(file, data, filesize);
    fs_close(file);

    DiskFlopAnim(102, 85);

    zipsize = filesize * 2;
    zipdata = (unsigned char*)malloc(zipsize);
    compress(zipdata, &zipsize, data, filesize);

    DiskFlopAnim(102, 85);

#ifdef SPEAR
    strcpy(vmu_package.desc_short, "Sod4SDL\\DC");
    strcpy(vmu_package.app_id, "Sod4SDL\\DC");
#else
    strcpy(vmu_package.desc_short, "Wolf4SDL\\DC");
    strcpy(vmu_package.app_id, "Wolf4SDL\\DC");
#endif
    if(str == NULL)
        strcpy(vmu_package.desc_long, "Configuration");
    else {
        strcpy(vmu_package.desc_long, "Game Save - ");
        strcat(vmu_package.desc_long, str);
    }

    vmu_package.icon_cnt = 1;
    vmu_package.icon_anim_speed = 0;
    memcpy(&vmu_package.icon_pal[0], vmu_bios_save_icon, 32);
    vmu_package.icon_data = vmu_bios_save_icon + 32;
    vmu_package.eyecatch_type = VMUPKG_EC_NONE;
    vmu_package.data_len = zipsize;
    vmu_package.data = zipdata;
    vmu_pkg_build(&vmu_package, &vmu_package_out, &vmu_package_size);

    DiskFlopAnim(102, 85);

    fs_unlink(destination);
    file = fs_open(destination, O_WRONLY);
    fs_write(file, vmu_package_out, vmu_package_size);
    fs_close(file);

    DiskFlopAnim(102, 85);

    free(vmu_package_out);
    free(data);
    free(zipdata);
}


int DC_LoadFromVMU(char *fname) {
    char fpath[64];
    int file;
    int filesize;
    unsigned long unzipsize;
    unsigned char *data;
    unsigned char *unzipdata;
    vmu_pkg_t vmu_package;

    sprintf(fpath, "/vmu/a1/%s", fname);
    file = fs_open(fpath, O_RDONLY);
    if(file == 0) return 0;
    filesize = fs_total(file);
    if(filesize <= 0) return 0;
    data = (unsigned char*)malloc(filesize);
    fs_read(file, data, filesize);
    fs_close(file);

    if(!strcmp(fname, configname))
        DiskFlopAnim(102, 85);

    vmu_pkg_parse(data, &vmu_package);

    if(!strcmp(fname, configname))
        DiskFlopAnim(102, 85);

    unzipdata = (unsigned char*)malloc(65536);
    unzipsize = 65536;
    uncompress(unzipdata, &unzipsize, (unsigned char*)vmu_package.data, vmu_package.data_len);

    if(!strcmp(fname, configname))
        DiskFlopAnim(102, 85);

    fs_unlink(fname);
    file = fs_open(fname, O_WRONLY);
    fs_write(file, unzipdata, unzipsize);
    fs_close(file);

    if(!strcmp(fname, configname))
        DiskFlopAnim(102, 85);

    free(data);
    free(unzipdata);

    return 1;
}

#endif
