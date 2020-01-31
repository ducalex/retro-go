/****************************************************************************
 *  config.c
 *
 *  SMS Plus GX configuration file support
 *
 *  code by Eke-Eke (2008)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ***************************************************************************/
#include "shared.h"

#ifdef HW_RVL
#define CONFIG_VERSION "SMSPLUS 1.3.2W"
#else
#define CONFIG_VERSION "SMSPLUS 1.3.2G"
#endif

void config_save()
{
//   char pathname[MAXPATHLEN];
//
//   if (!fat_enabled) return;
//
//   /* first check if directory exist */
//   sprintf (pathname, DEFAULT_PATH);
//   DIR_ITER *dir = diropen(pathname);
//   if (dir == NULL) mkdir(pathname,S_IRWXU);
//   else dirclose(dir);
//
//   /* open configuration file */
//   sprintf (pathname, "%s/config.ini", pathname);
//   FILE *fp = fopen(pathname, "wb");
//   if (fp == NULL) return;
//
//   /* save options */
//   fwrite(&option, sizeof(option), 1, fp);
//
//   /* save key mapping */
//   fwrite(pad_keymap, sizeof(pad_keymap), 1, fp);
// #ifdef HW_RVL
//   fwrite(wpad_keymap, sizeof(wpad_keymap), 1, fp);
// #endif
//
//   fclose(fp);
}

void config_load()
{
//   char pathname[MAXPATHLEN];
//
//   /* open configuration file */
//   sprintf (pathname, "%s/config.ini", DEFAULT_PATH);
//   FILE *fp = fopen(pathname, "rb");
//   if (fp == NULL) return;
//
//   /* read version */
//   char version[15];
//   fread(version, 15, 1, fp);
//   fclose(fp);
//   if (strcmp(version,CONFIG_VERSION)) return;
//
//   /* read file */
//   fp = fopen(pathname, "rb");
//   fread(&option, sizeof(option), 1, fp);
//
//  /* load key mapping */
//   fread(pad_keymap, sizeof(pad_keymap), 1, fp);
// #ifdef HW_RVL
//   fread(wpad_keymap, sizeof(wpad_keymap), 1, fp);
// #endif
//   fclose(fp);
}

void set_option_defaults ()
{
  /* version TAG */
  strncpy(option.version,CONFIG_VERSION,15);

  option.sndrate      = 48000;
  option.country      = 0;
  option.console      = 0;
  option.fm           = SND_NONE;
  option.overscan     = 1;
  option.xshift       = 0;
  option.yshift       = 0;
  option.xscale       = 0;
  option.yscale       = 0;
  option.aspect       = 1;
  option.render       = 0;
  option.ntsc         = 0;
  option.bilinear     = 1;
  option.tv_mode      = 0;
  option.sms_pal      = 1;
  option.tms_pal      = 0;
  option.autofreeze   = -1;
  option.spritelimit  = 1;
  option.extra_gg     = 0;
}
