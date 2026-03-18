#ifndef LIBRETRO_CORE_OPTIONS_INTL_H__
#define LIBRETRO_CORE_OPTIONS_INTL_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1500 && _MSC_VER < 1900)
/* https://support.microsoft.com/en-us/kb/980263 */
#pragma execution_character_set("utf-8")
#pragma warning(disable:4566)
#endif

#include <libretro.h>

/*
 ********************************
 * VERSION: 2.0
 ********************************
 *
 * - 2.0: Add support for core options v2 interface
 * - 1.3: Move translations to libretro_core_options_intl.h
 *        - libretro_core_options_intl.h includes BOM and utf-8
 *          fix for MSVC 2010-2013
 *        - Added HAVE_NO_LANGEXTRA flag to disable translations
 *          on platforms/compilers without BOM support
 * - 1.2: Use core options v1 interface when
 *        RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION is >= 1
 *        (previously required RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION == 1)
 * - 1.1: Support generation of core options v0 retro_core_option_value
 *        arrays containing options with a single value
 * - 1.0: First commit
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
 ********************************
 * Core Option Definitions
 ********************************
*/
/* RETRO_LANGUAGE_AR */

#define CATEGORY_SYSTEM_LABEL_AR NULL
#define CATEGORY_SYSTEM_INFO_0_AR NULL
#define CATEGORY_VIDEO_LABEL_AR "فيديو"
#define CATEGORY_VIDEO_INFO_0_AR NULL
#define CATEGORY_VIDEO_INFO_1_AR NULL
#define CATEGORY_AUDIO_LABEL_AR "نظام تشغيل الصوت"
#define CATEGORY_AUDIO_INFO_0_AR NULL
#define CATEGORY_INPUT_LABEL_AR "الإدخال"
#define CATEGORY_INPUT_INFO_0_AR NULL
#define CATEGORY_HACKS_LABEL_AR NULL
#define CATEGORY_HACKS_INFO_0_AR NULL
#define CATEGORY_DIP_SWITCH_LABEL_AR NULL
#define CATEGORY_DIP_SWITCH_INFO_0_AR NULL
#define FCEUMM_REGION_LABEL_AR "المنطقة"
#define FCEUMM_REGION_INFO_0_AR NULL
#define OPTION_VAL_AUTO_AR "تلقائي"
#define OPTION_VAL_NTSC_AR NULL
#define OPTION_VAL_PAL_AR NULL
#define OPTION_VAL_DENDY_AR NULL
#define FCEUMM_GAME_GENIE_LABEL_AR NULL
#define FCEUMM_GAME_GENIE_INFO_0_AR NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_AR NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_AR NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_AR NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_AR NULL
#define FCEUMM_ASPECT_LABEL_AR "نسبة الجانب"
#define FCEUMM_ASPECT_INFO_0_AR NULL
#define OPTION_VAL_8_7_PAR_AR NULL
#define OPTION_VAL_4_3_AR NULL
#define OPTION_VAL_PP_AR NULL
#define FCEUMM_OVERSCAN_LABEL_AR NULL
#define FCEUMM_OVERSCAN_INFO_0_AR NULL
#define FCEUMM_OVERSCAN_H_LABEL_AR NULL
#define FCEUMM_OVERSCAN_H_INFO_0_AR NULL
#define FCEUMM_OVERSCAN_V_LABEL_AR NULL
#define FCEUMM_OVERSCAN_V_INFO_0_AR NULL
#define FCEUMM_PALETTE_LABEL_AR NULL
#define FCEUMM_PALETTE_INFO_0_AR NULL
#define OPTION_VAL_DEFAULT_AR NULL
#define OPTION_VAL_ASQREALC_AR NULL
#define OPTION_VAL_NINTENDO_VC_AR NULL
#define OPTION_VAL_RGB_AR NULL
#define OPTION_VAL_YUV_V3_AR NULL
#define OPTION_VAL_UNSATURATED_FINAL_AR NULL
#define OPTION_VAL_SONY_CXA2025AS_US_AR NULL
#define OPTION_VAL_BMF_FINAL2_AR NULL
#define OPTION_VAL_BMF_FINAL3_AR NULL
#define OPTION_VAL_SMOOTH_FBX_AR NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_AR NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_AR NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_AR NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_AR NULL
#define OPTION_VAL_NESCAP_AR NULL
#define OPTION_VAL_WAVEBEAM_AR NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_AR NULL
#define OPTION_VAL_MAGNUM_FBX_AR NULL
#define OPTION_VAL_SMOOTH_V2_FBX_AR NULL
#define OPTION_VAL_NES_CLASSIC_FBX_AR NULL
#define OPTION_VAL_RAW_AR NULL
#define OPTION_VAL_CUSTOM_AR "مخصص"
#define FCEUMM_NTSC_FILTER_LABEL_AR NULL
#define FCEUMM_NTSC_FILTER_INFO_0_AR NULL
#define OPTION_VAL_COMPOSITE_AR NULL
#define OPTION_VAL_SVIDEO_AR NULL
#define OPTION_VAL_RGB_O9_AR NULL
#define OPTION_VAL_MONOCHROME_AR "أحادي اللون"
#define FCEUMM_SNDQUALITY_LABEL_AR NULL
#define FCEUMM_SNDQUALITY_INFO_0_AR NULL
#define OPTION_VAL_LOW_AR NULL
#define OPTION_VAL_HIGH_AR NULL
#define OPTION_VAL_VERY_HIGH_AR NULL
#define FCEUMM_SNDLOWPASS_LABEL_AR NULL
#define FCEUMM_SNDLOWPASS_INFO_0_AR NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_AR NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_AR NULL
#define OPTION_VAL_01_MS_DELAY_AR NULL
#define OPTION_VAL_02_MS_DELAY_AR NULL
#define OPTION_VAL_03_MS_DELAY_AR NULL
#define OPTION_VAL_04_MS_DELAY_AR NULL
#define OPTION_VAL_05_MS_DELAY_AR NULL
#define OPTION_VAL_06_MS_DELAY_AR NULL
#define OPTION_VAL_07_MS_DELAY_AR NULL
#define OPTION_VAL_08_MS_DELAY_AR NULL
#define OPTION_VAL_09_MS_DELAY_AR NULL
#define OPTION_VAL_10_MS_DELAY_AR NULL
#define OPTION_VAL_11_MS_DELAY_AR NULL
#define OPTION_VAL_12_MS_DELAY_AR NULL
#define OPTION_VAL_13_MS_DELAY_AR NULL
#define OPTION_VAL_14_MS_DELAY_AR NULL
#define OPTION_VAL_15_MS_DELAY_AR NULL
#define OPTION_VAL_16_MS_DELAY_AR NULL
#define OPTION_VAL_17_MS_DELAY_AR NULL
#define OPTION_VAL_18_MS_DELAY_AR NULL
#define OPTION_VAL_19_MS_DELAY_AR NULL
#define OPTION_VAL_20_MS_DELAY_AR NULL
#define OPTION_VAL_21_MS_DELAY_AR NULL
#define OPTION_VAL_22_MS_DELAY_AR NULL
#define OPTION_VAL_23_MS_DELAY_AR NULL
#define OPTION_VAL_24_MS_DELAY_AR NULL
#define OPTION_VAL_25_MS_DELAY_AR NULL
#define OPTION_VAL_26_MS_DELAY_AR NULL
#define OPTION_VAL_27_MS_DELAY_AR NULL
#define OPTION_VAL_28_MS_DELAY_AR NULL
#define OPTION_VAL_29_MS_DELAY_AR NULL
#define OPTION_VAL_30_MS_DELAY_AR NULL
#define OPTION_VAL_31_MS_DELAY_AR NULL
#define OPTION_VAL_32_MS_DELAY_AR NULL
#define FCEUMM_SWAPDUTY_LABEL_AR NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_AR NULL
#define FCEUMM_SWAPDUTY_INFO_0_AR NULL
#define FCEUMM_SNDVOLUME_LABEL_AR NULL
#define FCEUMM_SNDVOLUME_INFO_0_AR NULL
#define OPTION_VAL_0_AR NULL
#define OPTION_VAL_1_AR NULL
#define OPTION_VAL_2_AR NULL
#define OPTION_VAL_3_AR NULL
#define OPTION_VAL_4_AR NULL
#define OPTION_VAL_5_AR NULL
#define OPTION_VAL_6_AR NULL
#define OPTION_VAL_7_AR NULL
#define OPTION_VAL_8_AR NULL
#define OPTION_VAL_9_AR NULL
#define OPTION_VAL_10_AR NULL
#define FCEUMM_APU_1_LABEL_AR NULL
#define FCEUMM_APU_1_LABEL_CAT_AR NULL
#define FCEUMM_APU_1_INFO_0_AR NULL
#define FCEUMM_APU_2_LABEL_AR NULL
#define FCEUMM_APU_2_LABEL_CAT_AR NULL
#define FCEUMM_APU_2_INFO_0_AR NULL
#define FCEUMM_APU_3_LABEL_AR NULL
#define FCEUMM_APU_3_LABEL_CAT_AR NULL
#define FCEUMM_APU_3_INFO_0_AR NULL
#define FCEUMM_APU_4_LABEL_AR NULL
#define FCEUMM_APU_4_LABEL_CAT_AR NULL
#define FCEUMM_APU_4_INFO_0_AR NULL
#define FCEUMM_APU_5_LABEL_AR NULL
#define FCEUMM_APU_5_LABEL_CAT_AR NULL
#define FCEUMM_APU_5_INFO_0_AR NULL
#define FCEUMM_TURBO_ENABLE_LABEL_AR NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_AR NULL
#define OPTION_VAL_NONE_AR "لاشيء"
#define OPTION_VAL_PLAYER_1_AR NULL
#define OPTION_VAL_PLAYER_2_AR NULL
#define OPTION_VAL_BOTH_AR NULL
#define FCEUMM_TURBO_DELAY_LABEL_AR NULL
#define FCEUMM_TURBO_DELAY_INFO_0_AR NULL
#define FCEUMM_ZAPPER_MODE_LABEL_AR NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_AR NULL
#define OPTION_VAL_CLIGHTGUN_AR NULL
#define OPTION_VAL_STLIGHTGUN_AR NULL
#define OPTION_VAL_TOUCHSCREEN_AR NULL
#define OPTION_VAL_MOUSE_AR NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_AR NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_AR NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_AR NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_AR NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_AR NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_AR NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_AR NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_AR NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_AR NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_AR NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_AR NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_AR NULL
#define FCEUMM_OVERCLOCKING_LABEL_AR NULL
#define FCEUMM_OVERCLOCKING_INFO_0_AR NULL
#define OPTION_VAL_2X_POSTRENDER_AR NULL
#define OPTION_VAL_2X_VBLANK_AR NULL
#define FCEUMM_RAMSTATE_LABEL_AR NULL
#define FCEUMM_RAMSTATE_INFO_0_AR NULL
#define OPTION_VAL_FILL_FF_AR NULL
#define OPTION_VAL_FILL_00_AR NULL
#define OPTION_VAL_RANDOM_AR NULL

struct retro_core_option_v2_category option_cats_ar[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_AR,
      CATEGORY_SYSTEM_INFO_0_AR
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_AR,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_AR
#else
      CATEGORY_VIDEO_INFO_1_AR
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_AR,
      CATEGORY_AUDIO_INFO_0_AR
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_AR,
      CATEGORY_INPUT_INFO_0_AR
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_AR,
      CATEGORY_HACKS_INFO_0_AR
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_AR,
      CATEGORY_DIP_SWITCH_INFO_0_AR
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_ar[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_AR,
      NULL,
      FCEUMM_REGION_INFO_0_AR,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_AR },
         { "NTSC",  OPTION_VAL_NTSC_AR },
         { "PAL",   OPTION_VAL_PAL_AR },
         { "Dendy", OPTION_VAL_DENDY_AR },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_AR,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_AR,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_AR,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_AR,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_AR,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_AR,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_AR,
      NULL,
      FCEUMM_ASPECT_INFO_0_AR,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_AR },
         { "4:3",     OPTION_VAL_4_3_AR },
         { "PP",      OPTION_VAL_PP_AR },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_AR,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_AR,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_AR,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_AR,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_AR,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_AR,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_AR,
      NULL,
      FCEUMM_PALETTE_INFO_0_AR,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_AR },
         { "asqrealc",             OPTION_VAL_ASQREALC_AR },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_AR },
         { "rgb",                  OPTION_VAL_RGB_AR },
         { "yuv-v3",               OPTION_VAL_YUV_V3_AR },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_AR },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_AR },
         { "pal",                  OPTION_VAL_PAL_AR },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_AR },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_AR },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_AR },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_AR },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_AR },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_AR },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_AR },
         { "nescap",               OPTION_VAL_NESCAP_AR },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_AR },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_AR },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_AR },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_AR },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_AR },
         { "raw",                  OPTION_VAL_RAW_AR },
         { "custom",               OPTION_VAL_CUSTOM_AR },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_AR,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_AR,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_AR },
         { "svideo",     OPTION_VAL_SVIDEO_AR },
         { "rgb",        OPTION_VAL_RGB_O9_AR },
         { "monochrome", OPTION_VAL_MONOCHROME_AR },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_AR,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_AR,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_AR },
         { "High",      OPTION_VAL_HIGH_AR },
         { "Very High", OPTION_VAL_VERY_HIGH_AR },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_AR,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_AR,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_AR,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_AR,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_AR },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_AR },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_AR },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_AR },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_AR },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_AR },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_AR },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_AR },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_AR },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_AR },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_AR },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_AR },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_AR },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_AR },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_AR },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_AR },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_AR },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_AR },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_AR },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_AR },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_AR },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_AR },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_AR },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_AR },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_AR },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_AR },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_AR },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_AR },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_AR },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_AR },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_AR },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_AR },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_AR,
      FCEUMM_SWAPDUTY_LABEL_CAT_AR,
      FCEUMM_SWAPDUTY_INFO_0_AR,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_AR,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_AR,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_AR },
         { "1",  OPTION_VAL_1_AR },
         { "2",  OPTION_VAL_2_AR },
         { "3",  OPTION_VAL_3_AR },
         { "4",  OPTION_VAL_4_AR },
         { "5",  OPTION_VAL_5_AR },
         { "6",  OPTION_VAL_6_AR },
         { "7",  OPTION_VAL_7_AR },
         { "8",  OPTION_VAL_8_AR },
         { "9",  OPTION_VAL_9_AR },
         { "10", OPTION_VAL_10_AR },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_AR,
      FCEUMM_APU_1_LABEL_CAT_AR,
      FCEUMM_APU_1_INFO_0_AR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_AR,
      FCEUMM_APU_2_LABEL_CAT_AR,
      FCEUMM_APU_2_INFO_0_AR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_AR,
      FCEUMM_APU_3_LABEL_CAT_AR,
      FCEUMM_APU_3_INFO_0_AR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_AR,
      FCEUMM_APU_4_LABEL_CAT_AR,
      FCEUMM_APU_4_INFO_0_AR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_AR,
      FCEUMM_APU_5_LABEL_CAT_AR,
      FCEUMM_APU_5_INFO_0_AR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_AR,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_AR,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_AR },
         { "Player 1", OPTION_VAL_PLAYER_1_AR },
         { "Player 2", OPTION_VAL_PLAYER_2_AR },
         { "Both",     OPTION_VAL_BOTH_AR },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_AR,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_AR,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_AR,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_AR,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_AR },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_AR },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_AR },
         { "mouse",       OPTION_VAL_MOUSE_AR },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_AR,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_AR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_AR,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_AR,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_AR,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_AR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_AR,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_AR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_AR,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_AR,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_AR,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_AR,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_AR,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_AR,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_AR },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_AR },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_AR,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_AR,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_AR },
         { "fill $00", OPTION_VAL_FILL_00_AR },
         { "random",   OPTION_VAL_RANDOM_AR },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_ar = {
   option_cats_ar,
   option_defs_ar
};

/* RETRO_LANGUAGE_AST */

#define CATEGORY_SYSTEM_LABEL_AST "Sistema"
#define CATEGORY_SYSTEM_INFO_0_AST NULL
#define CATEGORY_VIDEO_LABEL_AST "Videu"
#define CATEGORY_VIDEO_INFO_0_AST NULL
#define CATEGORY_VIDEO_INFO_1_AST NULL
#define CATEGORY_AUDIO_LABEL_AST "Audiu"
#define CATEGORY_AUDIO_INFO_0_AST NULL
#define CATEGORY_INPUT_LABEL_AST "Entrada"
#define CATEGORY_INPUT_INFO_0_AST NULL
#define CATEGORY_HACKS_LABEL_AST "Hacks de la emulación"
#define CATEGORY_HACKS_INFO_0_AST NULL
#define CATEGORY_DIP_SWITCH_LABEL_AST NULL
#define CATEGORY_DIP_SWITCH_INFO_0_AST NULL
#define FCEUMM_REGION_LABEL_AST "Rexón"
#define FCEUMM_REGION_INFO_0_AST NULL
#define OPTION_VAL_AUTO_AST NULL
#define OPTION_VAL_NTSC_AST NULL
#define OPTION_VAL_PAL_AST NULL
#define OPTION_VAL_DENDY_AST NULL
#define FCEUMM_GAME_GENIE_LABEL_AST NULL
#define FCEUMM_GAME_GENIE_INFO_0_AST NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_AST NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_AST NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_AST NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_AST NULL
#define FCEUMM_ASPECT_LABEL_AST "Proporción d'aspeutu"
#define FCEUMM_ASPECT_INFO_0_AST NULL
#define OPTION_VAL_8_7_PAR_AST NULL
#define OPTION_VAL_4_3_AST NULL
#define OPTION_VAL_PP_AST NULL
#define FCEUMM_OVERSCAN_LABEL_AST NULL
#define FCEUMM_OVERSCAN_INFO_0_AST NULL
#define FCEUMM_OVERSCAN_H_LABEL_AST NULL
#define FCEUMM_OVERSCAN_H_INFO_0_AST NULL
#define FCEUMM_OVERSCAN_V_LABEL_AST NULL
#define FCEUMM_OVERSCAN_V_INFO_0_AST NULL
#define FCEUMM_PALETTE_LABEL_AST NULL
#define FCEUMM_PALETTE_INFO_0_AST NULL
#define OPTION_VAL_DEFAULT_AST NULL
#define OPTION_VAL_ASQREALC_AST NULL
#define OPTION_VAL_NINTENDO_VC_AST NULL
#define OPTION_VAL_RGB_AST NULL
#define OPTION_VAL_YUV_V3_AST "YUV-V3 de FBX"
#define OPTION_VAL_UNSATURATED_FINAL_AST NULL
#define OPTION_VAL_SONY_CXA2025AS_US_AST NULL
#define OPTION_VAL_BMF_FINAL2_AST NULL
#define OPTION_VAL_BMF_FINAL3_AST NULL
#define OPTION_VAL_SMOOTH_FBX_AST NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_AST NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_AST NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_AST NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_AST NULL
#define OPTION_VAL_NESCAP_AST NULL
#define OPTION_VAL_WAVEBEAM_AST NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_AST NULL
#define OPTION_VAL_MAGNUM_FBX_AST NULL
#define OPTION_VAL_SMOOTH_V2_FBX_AST NULL
#define OPTION_VAL_NES_CLASSIC_FBX_AST NULL
#define OPTION_VAL_RAW_AST "En bruto"
#define OPTION_VAL_CUSTOM_AST NULL
#define FCEUMM_NTSC_FILTER_LABEL_AST NULL
#define FCEUMM_NTSC_FILTER_INFO_0_AST NULL
#define OPTION_VAL_COMPOSITE_AST NULL
#define OPTION_VAL_SVIDEO_AST NULL
#define OPTION_VAL_RGB_O9_AST NULL
#define OPTION_VAL_MONOCHROME_AST NULL
#define FCEUMM_SNDQUALITY_LABEL_AST "Calidá del soníu"
#define FCEUMM_SNDQUALITY_INFO_0_AST NULL
#define OPTION_VAL_LOW_AST NULL
#define OPTION_VAL_HIGH_AST NULL
#define OPTION_VAL_VERY_HIGH_AST NULL
#define FCEUMM_SNDLOWPASS_LABEL_AST NULL
#define FCEUMM_SNDLOWPASS_INFO_0_AST NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_AST NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_AST NULL
#define OPTION_VAL_01_MS_DELAY_AST NULL
#define OPTION_VAL_02_MS_DELAY_AST NULL
#define OPTION_VAL_03_MS_DELAY_AST NULL
#define OPTION_VAL_04_MS_DELAY_AST NULL
#define OPTION_VAL_05_MS_DELAY_AST NULL
#define OPTION_VAL_06_MS_DELAY_AST NULL
#define OPTION_VAL_07_MS_DELAY_AST NULL
#define OPTION_VAL_08_MS_DELAY_AST NULL
#define OPTION_VAL_09_MS_DELAY_AST NULL
#define OPTION_VAL_10_MS_DELAY_AST NULL
#define OPTION_VAL_11_MS_DELAY_AST NULL
#define OPTION_VAL_12_MS_DELAY_AST NULL
#define OPTION_VAL_13_MS_DELAY_AST NULL
#define OPTION_VAL_14_MS_DELAY_AST NULL
#define OPTION_VAL_15_MS_DELAY_AST NULL
#define OPTION_VAL_16_MS_DELAY_AST NULL
#define OPTION_VAL_17_MS_DELAY_AST NULL
#define OPTION_VAL_18_MS_DELAY_AST NULL
#define OPTION_VAL_19_MS_DELAY_AST NULL
#define OPTION_VAL_20_MS_DELAY_AST NULL
#define OPTION_VAL_21_MS_DELAY_AST NULL
#define OPTION_VAL_22_MS_DELAY_AST NULL
#define OPTION_VAL_23_MS_DELAY_AST NULL
#define OPTION_VAL_24_MS_DELAY_AST NULL
#define OPTION_VAL_25_MS_DELAY_AST NULL
#define OPTION_VAL_26_MS_DELAY_AST NULL
#define OPTION_VAL_27_MS_DELAY_AST NULL
#define OPTION_VAL_28_MS_DELAY_AST NULL
#define OPTION_VAL_29_MS_DELAY_AST NULL
#define OPTION_VAL_30_MS_DELAY_AST NULL
#define OPTION_VAL_31_MS_DELAY_AST NULL
#define OPTION_VAL_32_MS_DELAY_AST NULL
#define FCEUMM_SWAPDUTY_LABEL_AST NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_AST NULL
#define FCEUMM_SWAPDUTY_INFO_0_AST NULL
#define FCEUMM_SNDVOLUME_LABEL_AST NULL
#define FCEUMM_SNDVOLUME_INFO_0_AST NULL
#define OPTION_VAL_0_AST NULL
#define OPTION_VAL_1_AST NULL
#define OPTION_VAL_2_AST NULL
#define OPTION_VAL_3_AST NULL
#define OPTION_VAL_4_AST NULL
#define OPTION_VAL_5_AST NULL
#define OPTION_VAL_6_AST NULL
#define OPTION_VAL_7_AST NULL
#define OPTION_VAL_8_AST NULL
#define OPTION_VAL_9_AST NULL
#define OPTION_VAL_10_AST NULL
#define FCEUMM_APU_1_LABEL_AST NULL
#define FCEUMM_APU_1_LABEL_CAT_AST NULL
#define FCEUMM_APU_1_INFO_0_AST NULL
#define FCEUMM_APU_2_LABEL_AST NULL
#define FCEUMM_APU_2_LABEL_CAT_AST NULL
#define FCEUMM_APU_2_INFO_0_AST NULL
#define FCEUMM_APU_3_LABEL_AST NULL
#define FCEUMM_APU_3_LABEL_CAT_AST NULL
#define FCEUMM_APU_3_INFO_0_AST NULL
#define FCEUMM_APU_4_LABEL_AST NULL
#define FCEUMM_APU_4_LABEL_CAT_AST NULL
#define FCEUMM_APU_4_INFO_0_AST NULL
#define FCEUMM_APU_5_LABEL_AST NULL
#define FCEUMM_APU_5_LABEL_CAT_AST "Canal 5 (PCM)"
#define FCEUMM_APU_5_INFO_0_AST NULL
#define FCEUMM_TURBO_ENABLE_LABEL_AST NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_AST NULL
#define OPTION_VAL_NONE_AST NULL
#define OPTION_VAL_PLAYER_1_AST "Xugador 1"
#define OPTION_VAL_PLAYER_2_AST "Xugador 2"
#define OPTION_VAL_BOTH_AST NULL
#define FCEUMM_TURBO_DELAY_LABEL_AST NULL
#define FCEUMM_TURBO_DELAY_INFO_0_AST NULL
#define FCEUMM_ZAPPER_MODE_LABEL_AST NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_AST NULL
#define OPTION_VAL_CLIGHTGUN_AST NULL
#define OPTION_VAL_STLIGHTGUN_AST NULL
#define OPTION_VAL_TOUCHSCREEN_AST "Pantalla táctil"
#define OPTION_VAL_MOUSE_AST "Mur"
#define FCEUMM_SHOW_CROSSHAIR_LABEL_AST NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_AST NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_AST NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_AST NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_AST NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_AST NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_AST NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_AST NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_AST "Permitir les direiciones opuestes"
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_AST NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_AST NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_AST NULL
#define FCEUMM_OVERCLOCKING_LABEL_AST NULL
#define FCEUMM_OVERCLOCKING_INFO_0_AST NULL
#define OPTION_VAL_2X_POSTRENDER_AST NULL
#define OPTION_VAL_2X_VBLANK_AST NULL
#define FCEUMM_RAMSTATE_LABEL_AST NULL
#define FCEUMM_RAMSTATE_INFO_0_AST NULL
#define OPTION_VAL_FILL_FF_AST NULL
#define OPTION_VAL_FILL_00_AST NULL
#define OPTION_VAL_RANDOM_AST NULL

struct retro_core_option_v2_category option_cats_ast[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_AST,
      CATEGORY_SYSTEM_INFO_0_AST
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_AST,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_AST
#else
      CATEGORY_VIDEO_INFO_1_AST
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_AST,
      CATEGORY_AUDIO_INFO_0_AST
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_AST,
      CATEGORY_INPUT_INFO_0_AST
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_AST,
      CATEGORY_HACKS_INFO_0_AST
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_AST,
      CATEGORY_DIP_SWITCH_INFO_0_AST
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_ast[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_AST,
      NULL,
      FCEUMM_REGION_INFO_0_AST,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_AST },
         { "NTSC",  OPTION_VAL_NTSC_AST },
         { "PAL",   OPTION_VAL_PAL_AST },
         { "Dendy", OPTION_VAL_DENDY_AST },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_AST,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_AST,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_AST,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_AST,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_AST,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_AST,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_AST,
      NULL,
      FCEUMM_ASPECT_INFO_0_AST,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_AST },
         { "4:3",     OPTION_VAL_4_3_AST },
         { "PP",      OPTION_VAL_PP_AST },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_AST,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_AST,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_AST,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_AST,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_AST,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_AST,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_AST,
      NULL,
      FCEUMM_PALETTE_INFO_0_AST,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_AST },
         { "asqrealc",             OPTION_VAL_ASQREALC_AST },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_AST },
         { "rgb",                  OPTION_VAL_RGB_AST },
         { "yuv-v3",               OPTION_VAL_YUV_V3_AST },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_AST },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_AST },
         { "pal",                  OPTION_VAL_PAL_AST },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_AST },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_AST },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_AST },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_AST },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_AST },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_AST },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_AST },
         { "nescap",               OPTION_VAL_NESCAP_AST },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_AST },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_AST },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_AST },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_AST },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_AST },
         { "raw",                  OPTION_VAL_RAW_AST },
         { "custom",               OPTION_VAL_CUSTOM_AST },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_AST,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_AST,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_AST },
         { "svideo",     OPTION_VAL_SVIDEO_AST },
         { "rgb",        OPTION_VAL_RGB_O9_AST },
         { "monochrome", OPTION_VAL_MONOCHROME_AST },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_AST,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_AST,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_AST },
         { "High",      OPTION_VAL_HIGH_AST },
         { "Very High", OPTION_VAL_VERY_HIGH_AST },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_AST,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_AST,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_AST,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_AST,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_AST },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_AST },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_AST },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_AST },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_AST },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_AST },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_AST },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_AST },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_AST },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_AST },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_AST },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_AST },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_AST },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_AST },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_AST },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_AST },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_AST },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_AST },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_AST },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_AST },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_AST },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_AST },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_AST },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_AST },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_AST },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_AST },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_AST },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_AST },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_AST },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_AST },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_AST },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_AST },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_AST,
      FCEUMM_SWAPDUTY_LABEL_CAT_AST,
      FCEUMM_SWAPDUTY_INFO_0_AST,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_AST,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_AST,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_AST },
         { "1",  OPTION_VAL_1_AST },
         { "2",  OPTION_VAL_2_AST },
         { "3",  OPTION_VAL_3_AST },
         { "4",  OPTION_VAL_4_AST },
         { "5",  OPTION_VAL_5_AST },
         { "6",  OPTION_VAL_6_AST },
         { "7",  OPTION_VAL_7_AST },
         { "8",  OPTION_VAL_8_AST },
         { "9",  OPTION_VAL_9_AST },
         { "10", OPTION_VAL_10_AST },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_AST,
      FCEUMM_APU_1_LABEL_CAT_AST,
      FCEUMM_APU_1_INFO_0_AST,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_AST,
      FCEUMM_APU_2_LABEL_CAT_AST,
      FCEUMM_APU_2_INFO_0_AST,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_AST,
      FCEUMM_APU_3_LABEL_CAT_AST,
      FCEUMM_APU_3_INFO_0_AST,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_AST,
      FCEUMM_APU_4_LABEL_CAT_AST,
      FCEUMM_APU_4_INFO_0_AST,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_AST,
      FCEUMM_APU_5_LABEL_CAT_AST,
      FCEUMM_APU_5_INFO_0_AST,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_AST,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_AST,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_AST },
         { "Player 1", OPTION_VAL_PLAYER_1_AST },
         { "Player 2", OPTION_VAL_PLAYER_2_AST },
         { "Both",     OPTION_VAL_BOTH_AST },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_AST,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_AST,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_AST,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_AST,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_AST },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_AST },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_AST },
         { "mouse",       OPTION_VAL_MOUSE_AST },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_AST,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_AST,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_AST,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_AST,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_AST,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_AST,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_AST,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_AST,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_AST,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_AST,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_AST,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_AST,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_AST,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_AST,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_AST },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_AST },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_AST,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_AST,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_AST },
         { "fill $00", OPTION_VAL_FILL_00_AST },
         { "random",   OPTION_VAL_RANDOM_AST },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_ast = {
   option_cats_ast,
   option_defs_ast
};

/* RETRO_LANGUAGE_CA */

#define CATEGORY_SYSTEM_LABEL_CA "Sistema"
#define CATEGORY_SYSTEM_INFO_0_CA NULL
#define CATEGORY_VIDEO_LABEL_CA "Vídeo"
#define CATEGORY_VIDEO_INFO_0_CA NULL
#define CATEGORY_VIDEO_INFO_1_CA NULL
#define CATEGORY_AUDIO_LABEL_CA "Àudio"
#define CATEGORY_AUDIO_INFO_0_CA NULL
#define CATEGORY_INPUT_LABEL_CA "Dispositius de joc"
#define CATEGORY_INPUT_INFO_0_CA NULL
#define CATEGORY_HACKS_LABEL_CA NULL
#define CATEGORY_HACKS_INFO_0_CA NULL
#define CATEGORY_DIP_SWITCH_LABEL_CA NULL
#define CATEGORY_DIP_SWITCH_INFO_0_CA NULL
#define FCEUMM_REGION_LABEL_CA "Regió"
#define FCEUMM_REGION_INFO_0_CA NULL
#define OPTION_VAL_AUTO_CA "Automàtic"
#define OPTION_VAL_NTSC_CA NULL
#define OPTION_VAL_PAL_CA NULL
#define OPTION_VAL_DENDY_CA NULL
#define FCEUMM_GAME_GENIE_LABEL_CA NULL
#define FCEUMM_GAME_GENIE_INFO_0_CA NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_CA NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_CA NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_CA NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_CA NULL
#define FCEUMM_ASPECT_LABEL_CA "Relació d'aspecte"
#define FCEUMM_ASPECT_INFO_0_CA NULL
#define OPTION_VAL_8_7_PAR_CA NULL
#define OPTION_VAL_4_3_CA NULL
#define OPTION_VAL_PP_CA NULL
#define FCEUMM_OVERSCAN_LABEL_CA NULL
#define FCEUMM_OVERSCAN_INFO_0_CA NULL
#define FCEUMM_OVERSCAN_H_LABEL_CA NULL
#define FCEUMM_OVERSCAN_H_INFO_0_CA NULL
#define FCEUMM_OVERSCAN_V_LABEL_CA NULL
#define FCEUMM_OVERSCAN_V_INFO_0_CA NULL
#define FCEUMM_PALETTE_LABEL_CA NULL
#define FCEUMM_PALETTE_INFO_0_CA NULL
#define OPTION_VAL_DEFAULT_CA NULL
#define OPTION_VAL_ASQREALC_CA NULL
#define OPTION_VAL_NINTENDO_VC_CA NULL
#define OPTION_VAL_RGB_CA NULL
#define OPTION_VAL_YUV_V3_CA NULL
#define OPTION_VAL_UNSATURATED_FINAL_CA NULL
#define OPTION_VAL_SONY_CXA2025AS_US_CA NULL
#define OPTION_VAL_BMF_FINAL2_CA NULL
#define OPTION_VAL_BMF_FINAL3_CA NULL
#define OPTION_VAL_SMOOTH_FBX_CA NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_CA NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_CA NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_CA NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_CA NULL
#define OPTION_VAL_NESCAP_CA NULL
#define OPTION_VAL_WAVEBEAM_CA NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_CA NULL
#define OPTION_VAL_MAGNUM_FBX_CA NULL
#define OPTION_VAL_SMOOTH_V2_FBX_CA NULL
#define OPTION_VAL_NES_CLASSIC_FBX_CA NULL
#define OPTION_VAL_RAW_CA NULL
#define OPTION_VAL_CUSTOM_CA "Personalitzat"
#define FCEUMM_NTSC_FILTER_LABEL_CA NULL
#define FCEUMM_NTSC_FILTER_INFO_0_CA NULL
#define OPTION_VAL_COMPOSITE_CA NULL
#define OPTION_VAL_SVIDEO_CA NULL
#define OPTION_VAL_RGB_O9_CA NULL
#define OPTION_VAL_MONOCHROME_CA "Monocrom"
#define FCEUMM_SNDQUALITY_LABEL_CA NULL
#define FCEUMM_SNDQUALITY_INFO_0_CA NULL
#define OPTION_VAL_LOW_CA NULL
#define OPTION_VAL_HIGH_CA NULL
#define OPTION_VAL_VERY_HIGH_CA NULL
#define FCEUMM_SNDLOWPASS_LABEL_CA NULL
#define FCEUMM_SNDLOWPASS_INFO_0_CA NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_CA NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_CA NULL
#define OPTION_VAL_01_MS_DELAY_CA NULL
#define OPTION_VAL_02_MS_DELAY_CA NULL
#define OPTION_VAL_03_MS_DELAY_CA NULL
#define OPTION_VAL_04_MS_DELAY_CA NULL
#define OPTION_VAL_05_MS_DELAY_CA NULL
#define OPTION_VAL_06_MS_DELAY_CA NULL
#define OPTION_VAL_07_MS_DELAY_CA NULL
#define OPTION_VAL_08_MS_DELAY_CA NULL
#define OPTION_VAL_09_MS_DELAY_CA NULL
#define OPTION_VAL_10_MS_DELAY_CA NULL
#define OPTION_VAL_11_MS_DELAY_CA NULL
#define OPTION_VAL_12_MS_DELAY_CA NULL
#define OPTION_VAL_13_MS_DELAY_CA NULL
#define OPTION_VAL_14_MS_DELAY_CA NULL
#define OPTION_VAL_15_MS_DELAY_CA NULL
#define OPTION_VAL_16_MS_DELAY_CA NULL
#define OPTION_VAL_17_MS_DELAY_CA NULL
#define OPTION_VAL_18_MS_DELAY_CA NULL
#define OPTION_VAL_19_MS_DELAY_CA NULL
#define OPTION_VAL_20_MS_DELAY_CA NULL
#define OPTION_VAL_21_MS_DELAY_CA NULL
#define OPTION_VAL_22_MS_DELAY_CA NULL
#define OPTION_VAL_23_MS_DELAY_CA NULL
#define OPTION_VAL_24_MS_DELAY_CA NULL
#define OPTION_VAL_25_MS_DELAY_CA NULL
#define OPTION_VAL_26_MS_DELAY_CA NULL
#define OPTION_VAL_27_MS_DELAY_CA NULL
#define OPTION_VAL_28_MS_DELAY_CA NULL
#define OPTION_VAL_29_MS_DELAY_CA NULL
#define OPTION_VAL_30_MS_DELAY_CA NULL
#define OPTION_VAL_31_MS_DELAY_CA NULL
#define OPTION_VAL_32_MS_DELAY_CA NULL
#define FCEUMM_SWAPDUTY_LABEL_CA NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_CA NULL
#define FCEUMM_SWAPDUTY_INFO_0_CA NULL
#define FCEUMM_SNDVOLUME_LABEL_CA NULL
#define FCEUMM_SNDVOLUME_INFO_0_CA NULL
#define OPTION_VAL_0_CA NULL
#define OPTION_VAL_1_CA NULL
#define OPTION_VAL_2_CA NULL
#define OPTION_VAL_3_CA NULL
#define OPTION_VAL_4_CA NULL
#define OPTION_VAL_5_CA NULL
#define OPTION_VAL_6_CA NULL
#define OPTION_VAL_7_CA NULL
#define OPTION_VAL_8_CA NULL
#define OPTION_VAL_9_CA NULL
#define OPTION_VAL_10_CA NULL
#define FCEUMM_APU_1_LABEL_CA NULL
#define FCEUMM_APU_1_LABEL_CAT_CA NULL
#define FCEUMM_APU_1_INFO_0_CA NULL
#define FCEUMM_APU_2_LABEL_CA NULL
#define FCEUMM_APU_2_LABEL_CAT_CA NULL
#define FCEUMM_APU_2_INFO_0_CA NULL
#define FCEUMM_APU_3_LABEL_CA NULL
#define FCEUMM_APU_3_LABEL_CAT_CA NULL
#define FCEUMM_APU_3_INFO_0_CA NULL
#define FCEUMM_APU_4_LABEL_CA NULL
#define FCEUMM_APU_4_LABEL_CAT_CA NULL
#define FCEUMM_APU_4_INFO_0_CA NULL
#define FCEUMM_APU_5_LABEL_CA NULL
#define FCEUMM_APU_5_LABEL_CAT_CA NULL
#define FCEUMM_APU_5_INFO_0_CA NULL
#define FCEUMM_TURBO_ENABLE_LABEL_CA NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_CA NULL
#define OPTION_VAL_NONE_CA "Cap"
#define OPTION_VAL_PLAYER_1_CA NULL
#define OPTION_VAL_PLAYER_2_CA NULL
#define OPTION_VAL_BOTH_CA NULL
#define FCEUMM_TURBO_DELAY_LABEL_CA NULL
#define FCEUMM_TURBO_DELAY_INFO_0_CA NULL
#define FCEUMM_ZAPPER_MODE_LABEL_CA NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_CA NULL
#define OPTION_VAL_CLIGHTGUN_CA NULL
#define OPTION_VAL_STLIGHTGUN_CA NULL
#define OPTION_VAL_TOUCHSCREEN_CA NULL
#define OPTION_VAL_MOUSE_CA NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_CA NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_CA NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_CA NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_CA NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_CA NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_CA NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_CA NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_CA NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_CA NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_CA NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_CA NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_CA NULL
#define FCEUMM_OVERCLOCKING_LABEL_CA NULL
#define FCEUMM_OVERCLOCKING_INFO_0_CA NULL
#define OPTION_VAL_2X_POSTRENDER_CA NULL
#define OPTION_VAL_2X_VBLANK_CA NULL
#define FCEUMM_RAMSTATE_LABEL_CA NULL
#define FCEUMM_RAMSTATE_INFO_0_CA NULL
#define OPTION_VAL_FILL_FF_CA NULL
#define OPTION_VAL_FILL_00_CA NULL
#define OPTION_VAL_RANDOM_CA NULL

struct retro_core_option_v2_category option_cats_ca[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_CA,
      CATEGORY_SYSTEM_INFO_0_CA
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_CA,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_CA
#else
      CATEGORY_VIDEO_INFO_1_CA
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_CA,
      CATEGORY_AUDIO_INFO_0_CA
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_CA,
      CATEGORY_INPUT_INFO_0_CA
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_CA,
      CATEGORY_HACKS_INFO_0_CA
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_CA,
      CATEGORY_DIP_SWITCH_INFO_0_CA
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_ca[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_CA,
      NULL,
      FCEUMM_REGION_INFO_0_CA,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_CA },
         { "NTSC",  OPTION_VAL_NTSC_CA },
         { "PAL",   OPTION_VAL_PAL_CA },
         { "Dendy", OPTION_VAL_DENDY_CA },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_CA,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_CA,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_CA,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_CA,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_CA,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_CA,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_CA,
      NULL,
      FCEUMM_ASPECT_INFO_0_CA,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_CA },
         { "4:3",     OPTION_VAL_4_3_CA },
         { "PP",      OPTION_VAL_PP_CA },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_CA,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_CA,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_CA,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_CA,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_CA,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_CA,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_CA,
      NULL,
      FCEUMM_PALETTE_INFO_0_CA,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_CA },
         { "asqrealc",             OPTION_VAL_ASQREALC_CA },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_CA },
         { "rgb",                  OPTION_VAL_RGB_CA },
         { "yuv-v3",               OPTION_VAL_YUV_V3_CA },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_CA },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_CA },
         { "pal",                  OPTION_VAL_PAL_CA },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_CA },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_CA },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_CA },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_CA },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_CA },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_CA },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_CA },
         { "nescap",               OPTION_VAL_NESCAP_CA },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_CA },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_CA },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_CA },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_CA },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_CA },
         { "raw",                  OPTION_VAL_RAW_CA },
         { "custom",               OPTION_VAL_CUSTOM_CA },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_CA,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_CA,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_CA },
         { "svideo",     OPTION_VAL_SVIDEO_CA },
         { "rgb",        OPTION_VAL_RGB_O9_CA },
         { "monochrome", OPTION_VAL_MONOCHROME_CA },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_CA,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_CA,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_CA },
         { "High",      OPTION_VAL_HIGH_CA },
         { "Very High", OPTION_VAL_VERY_HIGH_CA },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_CA,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_CA,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_CA,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_CA,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_CA },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_CA },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_CA },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_CA },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_CA },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_CA },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_CA },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_CA },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_CA },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_CA },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_CA },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_CA },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_CA },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_CA },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_CA },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_CA },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_CA },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_CA },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_CA },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_CA },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_CA },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_CA },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_CA },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_CA },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_CA },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_CA },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_CA },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_CA },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_CA },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_CA },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_CA },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_CA },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_CA,
      FCEUMM_SWAPDUTY_LABEL_CAT_CA,
      FCEUMM_SWAPDUTY_INFO_0_CA,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_CA,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_CA,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_CA },
         { "1",  OPTION_VAL_1_CA },
         { "2",  OPTION_VAL_2_CA },
         { "3",  OPTION_VAL_3_CA },
         { "4",  OPTION_VAL_4_CA },
         { "5",  OPTION_VAL_5_CA },
         { "6",  OPTION_VAL_6_CA },
         { "7",  OPTION_VAL_7_CA },
         { "8",  OPTION_VAL_8_CA },
         { "9",  OPTION_VAL_9_CA },
         { "10", OPTION_VAL_10_CA },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_CA,
      FCEUMM_APU_1_LABEL_CAT_CA,
      FCEUMM_APU_1_INFO_0_CA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_CA,
      FCEUMM_APU_2_LABEL_CAT_CA,
      FCEUMM_APU_2_INFO_0_CA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_CA,
      FCEUMM_APU_3_LABEL_CAT_CA,
      FCEUMM_APU_3_INFO_0_CA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_CA,
      FCEUMM_APU_4_LABEL_CAT_CA,
      FCEUMM_APU_4_INFO_0_CA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_CA,
      FCEUMM_APU_5_LABEL_CAT_CA,
      FCEUMM_APU_5_INFO_0_CA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_CA,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_CA,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_CA },
         { "Player 1", OPTION_VAL_PLAYER_1_CA },
         { "Player 2", OPTION_VAL_PLAYER_2_CA },
         { "Both",     OPTION_VAL_BOTH_CA },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_CA,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_CA,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_CA,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_CA,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_CA },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_CA },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_CA },
         { "mouse",       OPTION_VAL_MOUSE_CA },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_CA,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_CA,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_CA,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_CA,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_CA,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_CA,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_CA,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_CA,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_CA,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_CA,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_CA,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_CA,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_CA,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_CA,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_CA },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_CA },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_CA,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_CA,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_CA },
         { "fill $00", OPTION_VAL_FILL_00_CA },
         { "random",   OPTION_VAL_RANDOM_CA },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_ca = {
   option_cats_ca,
   option_defs_ca
};

/* RETRO_LANGUAGE_CHS */

#define CATEGORY_SYSTEM_LABEL_CHS "系统"
#define CATEGORY_SYSTEM_INFO_0_CHS NULL
#define CATEGORY_VIDEO_LABEL_CHS "视频"
#define CATEGORY_VIDEO_INFO_0_CHS NULL
#define CATEGORY_VIDEO_INFO_1_CHS NULL
#define CATEGORY_AUDIO_LABEL_CHS "音频"
#define CATEGORY_AUDIO_INFO_0_CHS NULL
#define CATEGORY_INPUT_LABEL_CHS "输入"
#define CATEGORY_INPUT_INFO_0_CHS NULL
#define CATEGORY_HACKS_LABEL_CHS "模拟修改"
#define CATEGORY_HACKS_INFO_0_CHS NULL
#define CATEGORY_DIP_SWITCH_LABEL_CHS NULL
#define CATEGORY_DIP_SWITCH_INFO_0_CHS NULL
#define FCEUMM_REGION_LABEL_CHS "地区"
#define FCEUMM_REGION_INFO_0_CHS NULL
#define OPTION_VAL_AUTO_CHS "自动"
#define OPTION_VAL_NTSC_CHS NULL
#define OPTION_VAL_PAL_CHS NULL
#define OPTION_VAL_DENDY_CHS NULL
#define FCEUMM_GAME_GENIE_LABEL_CHS NULL
#define FCEUMM_GAME_GENIE_INFO_0_CHS NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_CHS NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_CHS NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_CHS NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_CHS NULL
#define FCEUMM_ASPECT_LABEL_CHS "纵横比"
#define FCEUMM_ASPECT_INFO_0_CHS NULL
#define OPTION_VAL_8_7_PAR_CHS NULL
#define OPTION_VAL_4_3_CHS NULL
#define OPTION_VAL_PP_CHS NULL
#define FCEUMM_OVERSCAN_LABEL_CHS NULL
#define FCEUMM_OVERSCAN_INFO_0_CHS NULL
#define FCEUMM_OVERSCAN_H_LABEL_CHS NULL
#define FCEUMM_OVERSCAN_H_INFO_0_CHS NULL
#define FCEUMM_OVERSCAN_V_LABEL_CHS NULL
#define FCEUMM_OVERSCAN_V_INFO_0_CHS NULL
#define FCEUMM_PALETTE_LABEL_CHS "色板"
#define FCEUMM_PALETTE_INFO_0_CHS NULL
#define OPTION_VAL_DEFAULT_CHS "默认"
#define OPTION_VAL_ASQREALC_CHS NULL
#define OPTION_VAL_NINTENDO_VC_CHS NULL
#define OPTION_VAL_RGB_CHS NULL
#define OPTION_VAL_YUV_V3_CHS NULL
#define OPTION_VAL_UNSATURATED_FINAL_CHS NULL
#define OPTION_VAL_SONY_CXA2025AS_US_CHS NULL
#define OPTION_VAL_BMF_FINAL2_CHS NULL
#define OPTION_VAL_BMF_FINAL3_CHS NULL
#define OPTION_VAL_SMOOTH_FBX_CHS NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_CHS NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_CHS NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_CHS NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_CHS NULL
#define OPTION_VAL_NESCAP_CHS NULL
#define OPTION_VAL_WAVEBEAM_CHS NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_CHS NULL
#define OPTION_VAL_MAGNUM_FBX_CHS NULL
#define OPTION_VAL_SMOOTH_V2_FBX_CHS NULL
#define OPTION_VAL_NES_CLASSIC_FBX_CHS NULL
#define OPTION_VAL_RAW_CHS NULL
#define OPTION_VAL_CUSTOM_CHS "自定义"
#define FCEUMM_NTSC_FILTER_LABEL_CHS NULL
#define FCEUMM_NTSC_FILTER_INFO_0_CHS NULL
#define OPTION_VAL_COMPOSITE_CHS "合成"
#define OPTION_VAL_SVIDEO_CHS "S-视频"
#define OPTION_VAL_RGB_O9_CHS NULL
#define OPTION_VAL_MONOCHROME_CHS "黑白"
#define FCEUMM_SNDQUALITY_LABEL_CHS NULL
#define FCEUMM_SNDQUALITY_INFO_0_CHS NULL
#define OPTION_VAL_LOW_CHS "低"
#define OPTION_VAL_HIGH_CHS "高"
#define OPTION_VAL_VERY_HIGH_CHS NULL
#define FCEUMM_SNDLOWPASS_LABEL_CHS NULL
#define FCEUMM_SNDLOWPASS_INFO_0_CHS NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_CHS NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_CHS NULL
#define OPTION_VAL_01_MS_DELAY_CHS NULL
#define OPTION_VAL_02_MS_DELAY_CHS NULL
#define OPTION_VAL_03_MS_DELAY_CHS NULL
#define OPTION_VAL_04_MS_DELAY_CHS NULL
#define OPTION_VAL_05_MS_DELAY_CHS NULL
#define OPTION_VAL_06_MS_DELAY_CHS NULL
#define OPTION_VAL_07_MS_DELAY_CHS NULL
#define OPTION_VAL_08_MS_DELAY_CHS NULL
#define OPTION_VAL_09_MS_DELAY_CHS NULL
#define OPTION_VAL_10_MS_DELAY_CHS NULL
#define OPTION_VAL_11_MS_DELAY_CHS NULL
#define OPTION_VAL_12_MS_DELAY_CHS NULL
#define OPTION_VAL_13_MS_DELAY_CHS NULL
#define OPTION_VAL_14_MS_DELAY_CHS NULL
#define OPTION_VAL_15_MS_DELAY_CHS NULL
#define OPTION_VAL_16_MS_DELAY_CHS NULL
#define OPTION_VAL_17_MS_DELAY_CHS NULL
#define OPTION_VAL_18_MS_DELAY_CHS NULL
#define OPTION_VAL_19_MS_DELAY_CHS NULL
#define OPTION_VAL_20_MS_DELAY_CHS NULL
#define OPTION_VAL_21_MS_DELAY_CHS NULL
#define OPTION_VAL_22_MS_DELAY_CHS NULL
#define OPTION_VAL_23_MS_DELAY_CHS NULL
#define OPTION_VAL_24_MS_DELAY_CHS NULL
#define OPTION_VAL_25_MS_DELAY_CHS NULL
#define OPTION_VAL_26_MS_DELAY_CHS NULL
#define OPTION_VAL_27_MS_DELAY_CHS NULL
#define OPTION_VAL_28_MS_DELAY_CHS NULL
#define OPTION_VAL_29_MS_DELAY_CHS NULL
#define OPTION_VAL_30_MS_DELAY_CHS NULL
#define OPTION_VAL_31_MS_DELAY_CHS NULL
#define OPTION_VAL_32_MS_DELAY_CHS NULL
#define FCEUMM_SWAPDUTY_LABEL_CHS NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_CHS NULL
#define FCEUMM_SWAPDUTY_INFO_0_CHS NULL
#define FCEUMM_SNDVOLUME_LABEL_CHS "主音量"
#define FCEUMM_SNDVOLUME_INFO_0_CHS NULL
#define OPTION_VAL_0_CHS NULL
#define OPTION_VAL_1_CHS NULL
#define OPTION_VAL_2_CHS NULL
#define OPTION_VAL_3_CHS NULL
#define OPTION_VAL_4_CHS NULL
#define OPTION_VAL_5_CHS NULL
#define OPTION_VAL_6_CHS NULL
#define OPTION_VAL_7_CHS NULL
#define OPTION_VAL_8_CHS NULL
#define OPTION_VAL_9_CHS NULL
#define OPTION_VAL_10_CHS NULL
#define FCEUMM_APU_1_LABEL_CHS NULL
#define FCEUMM_APU_1_LABEL_CAT_CHS NULL
#define FCEUMM_APU_1_INFO_0_CHS NULL
#define FCEUMM_APU_2_LABEL_CHS NULL
#define FCEUMM_APU_2_LABEL_CAT_CHS NULL
#define FCEUMM_APU_2_INFO_0_CHS NULL
#define FCEUMM_APU_3_LABEL_CHS NULL
#define FCEUMM_APU_3_LABEL_CAT_CHS NULL
#define FCEUMM_APU_3_INFO_0_CHS NULL
#define FCEUMM_APU_4_LABEL_CHS NULL
#define FCEUMM_APU_4_LABEL_CAT_CHS NULL
#define FCEUMM_APU_4_INFO_0_CHS NULL
#define FCEUMM_APU_5_LABEL_CHS NULL
#define FCEUMM_APU_5_LABEL_CAT_CHS NULL
#define FCEUMM_APU_5_INFO_0_CHS NULL
#define FCEUMM_TURBO_ENABLE_LABEL_CHS NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_CHS NULL
#define OPTION_VAL_NONE_CHS "无"
#define OPTION_VAL_PLAYER_1_CHS "玩家甲"
#define OPTION_VAL_PLAYER_2_CHS "玩家乙"
#define OPTION_VAL_BOTH_CHS "全部"
#define FCEUMM_TURBO_DELAY_LABEL_CHS NULL
#define FCEUMM_TURBO_DELAY_INFO_0_CHS NULL
#define FCEUMM_ZAPPER_MODE_LABEL_CHS NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_CHS NULL
#define OPTION_VAL_CLIGHTGUN_CHS NULL
#define OPTION_VAL_STLIGHTGUN_CHS NULL
#define OPTION_VAL_TOUCHSCREEN_CHS NULL
#define OPTION_VAL_MOUSE_CHS "鼠标"
#define FCEUMM_SHOW_CROSSHAIR_LABEL_CHS NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_CHS NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_CHS NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_CHS NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_CHS NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_CHS NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_CHS NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_CHS NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_CHS NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_CHS NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_CHS NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_CHS NULL
#define FCEUMM_OVERCLOCKING_LABEL_CHS NULL
#define FCEUMM_OVERCLOCKING_INFO_0_CHS NULL
#define OPTION_VAL_2X_POSTRENDER_CHS NULL
#define OPTION_VAL_2X_VBLANK_CHS NULL
#define FCEUMM_RAMSTATE_LABEL_CHS NULL
#define FCEUMM_RAMSTATE_INFO_0_CHS NULL
#define OPTION_VAL_FILL_FF_CHS NULL
#define OPTION_VAL_FILL_00_CHS NULL
#define OPTION_VAL_RANDOM_CHS "随机"

struct retro_core_option_v2_category option_cats_chs[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_CHS,
      CATEGORY_SYSTEM_INFO_0_CHS
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_CHS,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_CHS
#else
      CATEGORY_VIDEO_INFO_1_CHS
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_CHS,
      CATEGORY_AUDIO_INFO_0_CHS
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_CHS,
      CATEGORY_INPUT_INFO_0_CHS
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_CHS,
      CATEGORY_HACKS_INFO_0_CHS
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_CHS,
      CATEGORY_DIP_SWITCH_INFO_0_CHS
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_chs[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_CHS,
      NULL,
      FCEUMM_REGION_INFO_0_CHS,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_CHS },
         { "NTSC",  OPTION_VAL_NTSC_CHS },
         { "PAL",   OPTION_VAL_PAL_CHS },
         { "Dendy", OPTION_VAL_DENDY_CHS },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_CHS,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_CHS,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_CHS,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_CHS,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_CHS,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_CHS,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_CHS,
      NULL,
      FCEUMM_ASPECT_INFO_0_CHS,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_CHS },
         { "4:3",     OPTION_VAL_4_3_CHS },
         { "PP",      OPTION_VAL_PP_CHS },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_CHS,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_CHS,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_CHS,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_CHS,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_CHS,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_CHS,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_CHS,
      NULL,
      FCEUMM_PALETTE_INFO_0_CHS,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_CHS },
         { "asqrealc",             OPTION_VAL_ASQREALC_CHS },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_CHS },
         { "rgb",                  OPTION_VAL_RGB_CHS },
         { "yuv-v3",               OPTION_VAL_YUV_V3_CHS },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_CHS },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_CHS },
         { "pal",                  OPTION_VAL_PAL_CHS },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_CHS },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_CHS },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_CHS },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_CHS },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_CHS },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_CHS },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_CHS },
         { "nescap",               OPTION_VAL_NESCAP_CHS },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_CHS },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_CHS },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_CHS },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_CHS },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_CHS },
         { "raw",                  OPTION_VAL_RAW_CHS },
         { "custom",               OPTION_VAL_CUSTOM_CHS },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_CHS,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_CHS,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_CHS },
         { "svideo",     OPTION_VAL_SVIDEO_CHS },
         { "rgb",        OPTION_VAL_RGB_O9_CHS },
         { "monochrome", OPTION_VAL_MONOCHROME_CHS },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_CHS,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_CHS,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_CHS },
         { "High",      OPTION_VAL_HIGH_CHS },
         { "Very High", OPTION_VAL_VERY_HIGH_CHS },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_CHS,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_CHS,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_CHS,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_CHS,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_CHS },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_CHS },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_CHS },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_CHS },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_CHS },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_CHS },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_CHS },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_CHS },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_CHS },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_CHS },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_CHS },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_CHS },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_CHS },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_CHS },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_CHS },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_CHS },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_CHS },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_CHS },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_CHS },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_CHS },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_CHS },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_CHS },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_CHS },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_CHS },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_CHS },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_CHS },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_CHS },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_CHS },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_CHS },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_CHS },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_CHS },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_CHS },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_CHS,
      FCEUMM_SWAPDUTY_LABEL_CAT_CHS,
      FCEUMM_SWAPDUTY_INFO_0_CHS,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_CHS,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_CHS,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_CHS },
         { "1",  OPTION_VAL_1_CHS },
         { "2",  OPTION_VAL_2_CHS },
         { "3",  OPTION_VAL_3_CHS },
         { "4",  OPTION_VAL_4_CHS },
         { "5",  OPTION_VAL_5_CHS },
         { "6",  OPTION_VAL_6_CHS },
         { "7",  OPTION_VAL_7_CHS },
         { "8",  OPTION_VAL_8_CHS },
         { "9",  OPTION_VAL_9_CHS },
         { "10", OPTION_VAL_10_CHS },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_CHS,
      FCEUMM_APU_1_LABEL_CAT_CHS,
      FCEUMM_APU_1_INFO_0_CHS,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_CHS,
      FCEUMM_APU_2_LABEL_CAT_CHS,
      FCEUMM_APU_2_INFO_0_CHS,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_CHS,
      FCEUMM_APU_3_LABEL_CAT_CHS,
      FCEUMM_APU_3_INFO_0_CHS,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_CHS,
      FCEUMM_APU_4_LABEL_CAT_CHS,
      FCEUMM_APU_4_INFO_0_CHS,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_CHS,
      FCEUMM_APU_5_LABEL_CAT_CHS,
      FCEUMM_APU_5_INFO_0_CHS,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_CHS,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_CHS,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_CHS },
         { "Player 1", OPTION_VAL_PLAYER_1_CHS },
         { "Player 2", OPTION_VAL_PLAYER_2_CHS },
         { "Both",     OPTION_VAL_BOTH_CHS },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_CHS,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_CHS,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_CHS,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_CHS,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_CHS },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_CHS },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_CHS },
         { "mouse",       OPTION_VAL_MOUSE_CHS },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_CHS,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_CHS,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_CHS,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_CHS,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_CHS,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_CHS,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_CHS,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_CHS,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_CHS,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_CHS,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_CHS,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_CHS,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_CHS,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_CHS,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_CHS },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_CHS },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_CHS,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_CHS,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_CHS },
         { "fill $00", OPTION_VAL_FILL_00_CHS },
         { "random",   OPTION_VAL_RANDOM_CHS },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_chs = {
   option_cats_chs,
   option_defs_chs
};

/* RETRO_LANGUAGE_CHT */

#define CATEGORY_SYSTEM_LABEL_CHT "系統"
#define CATEGORY_SYSTEM_INFO_0_CHT NULL
#define CATEGORY_VIDEO_LABEL_CHT "顯示"
#define CATEGORY_VIDEO_INFO_0_CHT NULL
#define CATEGORY_VIDEO_INFO_1_CHT NULL
#define CATEGORY_AUDIO_LABEL_CHT "音效"
#define CATEGORY_AUDIO_INFO_0_CHT NULL
#define CATEGORY_INPUT_LABEL_CHT "輸入"
#define CATEGORY_INPUT_INFO_0_CHT NULL
#define CATEGORY_HACKS_LABEL_CHT "進階"
#define CATEGORY_HACKS_INFO_0_CHT "變更模擬器進階破解的設定。"
#define CATEGORY_DIP_SWITCH_LABEL_CHT NULL
#define CATEGORY_DIP_SWITCH_INFO_0_CHT NULL
#define FCEUMM_REGION_LABEL_CHT "地區"
#define FCEUMM_REGION_INFO_0_CHT NULL
#define OPTION_VAL_AUTO_CHT "自動"
#define OPTION_VAL_NTSC_CHT NULL
#define OPTION_VAL_PAL_CHT NULL
#define OPTION_VAL_DENDY_CHT NULL
#define FCEUMM_GAME_GENIE_LABEL_CHT NULL
#define FCEUMM_GAME_GENIE_INFO_0_CHT NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_CHT NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_CHT NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_CHT NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_CHT NULL
#define FCEUMM_ASPECT_LABEL_CHT "寬高比"
#define FCEUMM_ASPECT_INFO_0_CHT NULL
#define OPTION_VAL_8_7_PAR_CHT "8:7 標準"
#define OPTION_VAL_4_3_CHT NULL
#define OPTION_VAL_PP_CHT NULL
#define FCEUMM_OVERSCAN_LABEL_CHT NULL
#define FCEUMM_OVERSCAN_INFO_0_CHT NULL
#define FCEUMM_OVERSCAN_H_LABEL_CHT NULL
#define FCEUMM_OVERSCAN_H_INFO_0_CHT NULL
#define FCEUMM_OVERSCAN_V_LABEL_CHT NULL
#define FCEUMM_OVERSCAN_V_INFO_0_CHT NULL
#define FCEUMM_PALETTE_LABEL_CHT "影像配色"
#define FCEUMM_PALETTE_INFO_0_CHT NULL
#define OPTION_VAL_DEFAULT_CHT "預設"
#define OPTION_VAL_ASQREALC_CHT NULL
#define OPTION_VAL_NINTENDO_VC_CHT NULL
#define OPTION_VAL_RGB_CHT NULL
#define OPTION_VAL_YUV_V3_CHT NULL
#define OPTION_VAL_UNSATURATED_FINAL_CHT NULL
#define OPTION_VAL_SONY_CXA2025AS_US_CHT NULL
#define OPTION_VAL_BMF_FINAL2_CHT NULL
#define OPTION_VAL_BMF_FINAL3_CHT NULL
#define OPTION_VAL_SMOOTH_FBX_CHT NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_CHT NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_CHT NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_CHT NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_CHT NULL
#define OPTION_VAL_NESCAP_CHT NULL
#define OPTION_VAL_WAVEBEAM_CHT NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_CHT NULL
#define OPTION_VAL_MAGNUM_FBX_CHT NULL
#define OPTION_VAL_SMOOTH_V2_FBX_CHT NULL
#define OPTION_VAL_NES_CLASSIC_FBX_CHT NULL
#define OPTION_VAL_RAW_CHT NULL
#define OPTION_VAL_CUSTOM_CHT "自訂"
#define FCEUMM_NTSC_FILTER_LABEL_CHT NULL
#define FCEUMM_NTSC_FILTER_INFO_0_CHT NULL
#define OPTION_VAL_COMPOSITE_CHT "合成"
#define OPTION_VAL_SVIDEO_CHT NULL
#define OPTION_VAL_RGB_O9_CHT "三原色"
#define OPTION_VAL_MONOCHROME_CHT "單色白 (Monochrome)"
#define FCEUMM_SNDQUALITY_LABEL_CHT NULL
#define FCEUMM_SNDQUALITY_INFO_0_CHT NULL
#define OPTION_VAL_LOW_CHT "低"
#define OPTION_VAL_HIGH_CHT "高"
#define OPTION_VAL_VERY_HIGH_CHT NULL
#define FCEUMM_SNDLOWPASS_LABEL_CHT NULL
#define FCEUMM_SNDLOWPASS_INFO_0_CHT NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_CHT NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_CHT NULL
#define OPTION_VAL_01_MS_DELAY_CHT NULL
#define OPTION_VAL_02_MS_DELAY_CHT NULL
#define OPTION_VAL_03_MS_DELAY_CHT NULL
#define OPTION_VAL_04_MS_DELAY_CHT NULL
#define OPTION_VAL_05_MS_DELAY_CHT NULL
#define OPTION_VAL_06_MS_DELAY_CHT NULL
#define OPTION_VAL_07_MS_DELAY_CHT NULL
#define OPTION_VAL_08_MS_DELAY_CHT NULL
#define OPTION_VAL_09_MS_DELAY_CHT NULL
#define OPTION_VAL_10_MS_DELAY_CHT NULL
#define OPTION_VAL_11_MS_DELAY_CHT NULL
#define OPTION_VAL_12_MS_DELAY_CHT NULL
#define OPTION_VAL_13_MS_DELAY_CHT NULL
#define OPTION_VAL_14_MS_DELAY_CHT NULL
#define OPTION_VAL_15_MS_DELAY_CHT NULL
#define OPTION_VAL_16_MS_DELAY_CHT NULL
#define OPTION_VAL_17_MS_DELAY_CHT NULL
#define OPTION_VAL_18_MS_DELAY_CHT NULL
#define OPTION_VAL_19_MS_DELAY_CHT NULL
#define OPTION_VAL_20_MS_DELAY_CHT NULL
#define OPTION_VAL_21_MS_DELAY_CHT NULL
#define OPTION_VAL_22_MS_DELAY_CHT NULL
#define OPTION_VAL_23_MS_DELAY_CHT NULL
#define OPTION_VAL_24_MS_DELAY_CHT NULL
#define OPTION_VAL_25_MS_DELAY_CHT NULL
#define OPTION_VAL_26_MS_DELAY_CHT NULL
#define OPTION_VAL_27_MS_DELAY_CHT NULL
#define OPTION_VAL_28_MS_DELAY_CHT NULL
#define OPTION_VAL_29_MS_DELAY_CHT NULL
#define OPTION_VAL_30_MS_DELAY_CHT NULL
#define OPTION_VAL_31_MS_DELAY_CHT NULL
#define OPTION_VAL_32_MS_DELAY_CHT NULL
#define FCEUMM_SWAPDUTY_LABEL_CHT NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_CHT NULL
#define FCEUMM_SWAPDUTY_INFO_0_CHT NULL
#define FCEUMM_SNDVOLUME_LABEL_CHT NULL
#define FCEUMM_SNDVOLUME_INFO_0_CHT NULL
#define OPTION_VAL_0_CHT NULL
#define OPTION_VAL_1_CHT NULL
#define OPTION_VAL_2_CHT NULL
#define OPTION_VAL_3_CHT NULL
#define OPTION_VAL_4_CHT NULL
#define OPTION_VAL_5_CHT NULL
#define OPTION_VAL_6_CHT NULL
#define OPTION_VAL_7_CHT NULL
#define OPTION_VAL_8_CHT NULL
#define OPTION_VAL_9_CHT NULL
#define OPTION_VAL_10_CHT NULL
#define FCEUMM_APU_1_LABEL_CHT NULL
#define FCEUMM_APU_1_LABEL_CAT_CHT NULL
#define FCEUMM_APU_1_INFO_0_CHT NULL
#define FCEUMM_APU_2_LABEL_CHT NULL
#define FCEUMM_APU_2_LABEL_CAT_CHT NULL
#define FCEUMM_APU_2_INFO_0_CHT NULL
#define FCEUMM_APU_3_LABEL_CHT NULL
#define FCEUMM_APU_3_LABEL_CAT_CHT NULL
#define FCEUMM_APU_3_INFO_0_CHT NULL
#define FCEUMM_APU_4_LABEL_CHT NULL
#define FCEUMM_APU_4_LABEL_CAT_CHT NULL
#define FCEUMM_APU_4_INFO_0_CHT NULL
#define FCEUMM_APU_5_LABEL_CHT NULL
#define FCEUMM_APU_5_LABEL_CAT_CHT NULL
#define FCEUMM_APU_5_INFO_0_CHT NULL
#define FCEUMM_TURBO_ENABLE_LABEL_CHT NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_CHT NULL
#define OPTION_VAL_NONE_CHT "無"
#define OPTION_VAL_PLAYER_1_CHT "玩家 1"
#define OPTION_VAL_PLAYER_2_CHT "玩家 2"
#define OPTION_VAL_BOTH_CHT NULL
#define FCEUMM_TURBO_DELAY_LABEL_CHT "連發延遲"
#define FCEUMM_TURBO_DELAY_INFO_0_CHT "調整連發間隔延遲時間(幀數)。"
#define FCEUMM_ZAPPER_MODE_LABEL_CHT NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_CHT NULL
#define OPTION_VAL_CLIGHTGUN_CHT NULL
#define OPTION_VAL_STLIGHTGUN_CHT NULL
#define OPTION_VAL_TOUCHSCREEN_CHT "觸控螢幕"
#define OPTION_VAL_MOUSE_CHT NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_CHT NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_CHT NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_CHT NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_CHT NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_CHT NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_CHT NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_CHT NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_CHT NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_CHT "允許反向操作"
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_CHT "允許反方向快速操作，可同時按下左右或上下的方向。可能導致遊戲的移動架構出錯。"
#define FCEUMM_NOSPRITELIMIT_LABEL_CHT "沒有素材限制"
#define FCEUMM_NOSPRITELIMIT_INFO_0_CHT NULL
#define FCEUMM_OVERCLOCKING_LABEL_CHT NULL
#define FCEUMM_OVERCLOCKING_INFO_0_CHT NULL
#define OPTION_VAL_2X_POSTRENDER_CHT NULL
#define OPTION_VAL_2X_VBLANK_CHT NULL
#define FCEUMM_RAMSTATE_LABEL_CHT NULL
#define FCEUMM_RAMSTATE_INFO_0_CHT NULL
#define OPTION_VAL_FILL_FF_CHT NULL
#define OPTION_VAL_FILL_00_CHT NULL
#define OPTION_VAL_RANDOM_CHT NULL

struct retro_core_option_v2_category option_cats_cht[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_CHT,
      CATEGORY_SYSTEM_INFO_0_CHT
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_CHT,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_CHT
#else
      CATEGORY_VIDEO_INFO_1_CHT
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_CHT,
      CATEGORY_AUDIO_INFO_0_CHT
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_CHT,
      CATEGORY_INPUT_INFO_0_CHT
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_CHT,
      CATEGORY_HACKS_INFO_0_CHT
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_CHT,
      CATEGORY_DIP_SWITCH_INFO_0_CHT
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_cht[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_CHT,
      NULL,
      FCEUMM_REGION_INFO_0_CHT,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_CHT },
         { "NTSC",  OPTION_VAL_NTSC_CHT },
         { "PAL",   OPTION_VAL_PAL_CHT },
         { "Dendy", OPTION_VAL_DENDY_CHT },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_CHT,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_CHT,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_CHT,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_CHT,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_CHT,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_CHT,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_CHT,
      NULL,
      FCEUMM_ASPECT_INFO_0_CHT,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_CHT },
         { "4:3",     OPTION_VAL_4_3_CHT },
         { "PP",      OPTION_VAL_PP_CHT },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_CHT,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_CHT,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_CHT,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_CHT,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_CHT,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_CHT,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_CHT,
      NULL,
      FCEUMM_PALETTE_INFO_0_CHT,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_CHT },
         { "asqrealc",             OPTION_VAL_ASQREALC_CHT },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_CHT },
         { "rgb",                  OPTION_VAL_RGB_CHT },
         { "yuv-v3",               OPTION_VAL_YUV_V3_CHT },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_CHT },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_CHT },
         { "pal",                  OPTION_VAL_PAL_CHT },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_CHT },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_CHT },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_CHT },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_CHT },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_CHT },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_CHT },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_CHT },
         { "nescap",               OPTION_VAL_NESCAP_CHT },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_CHT },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_CHT },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_CHT },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_CHT },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_CHT },
         { "raw",                  OPTION_VAL_RAW_CHT },
         { "custom",               OPTION_VAL_CUSTOM_CHT },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_CHT,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_CHT,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_CHT },
         { "svideo",     OPTION_VAL_SVIDEO_CHT },
         { "rgb",        OPTION_VAL_RGB_O9_CHT },
         { "monochrome", OPTION_VAL_MONOCHROME_CHT },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_CHT,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_CHT,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_CHT },
         { "High",      OPTION_VAL_HIGH_CHT },
         { "Very High", OPTION_VAL_VERY_HIGH_CHT },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_CHT,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_CHT,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_CHT,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_CHT,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_CHT },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_CHT },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_CHT },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_CHT },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_CHT },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_CHT },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_CHT },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_CHT },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_CHT },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_CHT },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_CHT },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_CHT },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_CHT },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_CHT },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_CHT },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_CHT },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_CHT },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_CHT },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_CHT },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_CHT },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_CHT },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_CHT },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_CHT },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_CHT },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_CHT },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_CHT },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_CHT },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_CHT },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_CHT },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_CHT },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_CHT },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_CHT },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_CHT,
      FCEUMM_SWAPDUTY_LABEL_CAT_CHT,
      FCEUMM_SWAPDUTY_INFO_0_CHT,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_CHT,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_CHT,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_CHT },
         { "1",  OPTION_VAL_1_CHT },
         { "2",  OPTION_VAL_2_CHT },
         { "3",  OPTION_VAL_3_CHT },
         { "4",  OPTION_VAL_4_CHT },
         { "5",  OPTION_VAL_5_CHT },
         { "6",  OPTION_VAL_6_CHT },
         { "7",  OPTION_VAL_7_CHT },
         { "8",  OPTION_VAL_8_CHT },
         { "9",  OPTION_VAL_9_CHT },
         { "10", OPTION_VAL_10_CHT },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_CHT,
      FCEUMM_APU_1_LABEL_CAT_CHT,
      FCEUMM_APU_1_INFO_0_CHT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_CHT,
      FCEUMM_APU_2_LABEL_CAT_CHT,
      FCEUMM_APU_2_INFO_0_CHT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_CHT,
      FCEUMM_APU_3_LABEL_CAT_CHT,
      FCEUMM_APU_3_INFO_0_CHT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_CHT,
      FCEUMM_APU_4_LABEL_CAT_CHT,
      FCEUMM_APU_4_INFO_0_CHT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_CHT,
      FCEUMM_APU_5_LABEL_CAT_CHT,
      FCEUMM_APU_5_INFO_0_CHT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_CHT,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_CHT,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_CHT },
         { "Player 1", OPTION_VAL_PLAYER_1_CHT },
         { "Player 2", OPTION_VAL_PLAYER_2_CHT },
         { "Both",     OPTION_VAL_BOTH_CHT },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_CHT,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_CHT,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_CHT,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_CHT,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_CHT },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_CHT },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_CHT },
         { "mouse",       OPTION_VAL_MOUSE_CHT },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_CHT,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_CHT,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_CHT,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_CHT,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_CHT,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_CHT,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_CHT,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_CHT,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_CHT,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_CHT,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_CHT,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_CHT,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_CHT,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_CHT,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_CHT },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_CHT },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_CHT,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_CHT,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_CHT },
         { "fill $00", OPTION_VAL_FILL_00_CHT },
         { "random",   OPTION_VAL_RANDOM_CHT },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_cht = {
   option_cats_cht,
   option_defs_cht
};

/* RETRO_LANGUAGE_CS */

#define CATEGORY_SYSTEM_LABEL_CS "Systém"
#define CATEGORY_SYSTEM_INFO_0_CS NULL
#define CATEGORY_VIDEO_LABEL_CS NULL
#define CATEGORY_VIDEO_INFO_0_CS NULL
#define CATEGORY_VIDEO_INFO_1_CS NULL
#define CATEGORY_AUDIO_LABEL_CS "Zvuk"
#define CATEGORY_AUDIO_INFO_0_CS NULL
#define CATEGORY_INPUT_LABEL_CS "Vstup"
#define CATEGORY_INPUT_INFO_0_CS NULL
#define CATEGORY_HACKS_LABEL_CS NULL
#define CATEGORY_HACKS_INFO_0_CS "Změna nastavení přetaktování procesoru a přesnosti emulace ovlivňující výkon a kompatibilitu na nízké úrovni."
#define CATEGORY_DIP_SWITCH_LABEL_CS "Přepínače DIP"
#define CATEGORY_DIP_SWITCH_INFO_0_CS NULL
#define FCEUMM_REGION_LABEL_CS NULL
#define FCEUMM_REGION_INFO_0_CS "Vynucení jádra k použití časování pro regiony NTSC, PAL nebo Dendy."
#define OPTION_VAL_AUTO_CS NULL
#define OPTION_VAL_NTSC_CS NULL
#define OPTION_VAL_PAL_CS NULL
#define OPTION_VAL_DENDY_CS NULL
#define FCEUMM_GAME_GENIE_LABEL_CS "Doplněk Game Genie (Nutný Restart)"
#define FCEUMM_GAME_GENIE_INFO_0_CS "Umožňuje emulaci přídavné kazety Game Genie, která umožňuje zadávat cheatovací kódy při spouštění her. Soubor ROM Game Genie 'gamegenie.nes' musí být přítomen v systémovém adresáři frontendu. Nevztahuje se na FDS nebo arkádový obsah."
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_CS "Zobrazit Rozšířené Možnosti Systému"
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_CS "Zobrazit pokročilé možnosti systému a vylepšení."
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_CS "Zobrazit Rozšířené Možnosti Zvuku"
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_CS "Zobrazit pokročilé možnosti zvuku a vylepšení."
#define FCEUMM_ASPECT_LABEL_CS "Poměr Stran"
#define FCEUMM_ASPECT_INFO_0_CS "Zvolte preferovaný poměr stran."
#define OPTION_VAL_8_7_PAR_CS NULL
#define OPTION_VAL_4_3_CS NULL
#define OPTION_VAL_PP_CS "Perfektní pixely"
#define FCEUMM_OVERSCAN_LABEL_CS "Overscan Oříznutí"
#define FCEUMM_OVERSCAN_INFO_0_CS "Odstraní 8 pixelů overscanu ze všech stran obrazovky."
#define FCEUMM_OVERSCAN_H_LABEL_CS "Oříznutí Horizontálního Přesvícení"
#define FCEUMM_OVERSCAN_H_INFO_0_CS "Odstraní 8 pixelů z levé a pravé strany obrazovky a simuluje tak overscan, který se vyskytuje u standardních CRT televizorů."
#define FCEUMM_OVERSCAN_V_LABEL_CS "Oříznutí Vertikálního Přesahu"
#define FCEUMM_OVERSCAN_V_INFO_0_CS "Odstraní 8 pixelů z horní a dolní části obrazovky a simuluje tak overscan, který se vyskytuje u standardních CRT televizorů."
#define FCEUMM_PALETTE_LABEL_CS "Paleta barev"
#define FCEUMM_PALETTE_INFO_0_CS "Můžete si vybrat z předgenerovaných palet, vlastní palety 64x3 ze souboru nebo surového formátu (je třeba použít shader nes-decoder)."
#define OPTION_VAL_DEFAULT_CS "Výchozí"
#define OPTION_VAL_ASQREALC_CS NULL
#define OPTION_VAL_NINTENDO_VC_CS NULL
#define OPTION_VAL_RGB_CS NULL
#define OPTION_VAL_YUV_V3_CS NULL
#define OPTION_VAL_UNSATURATED_FINAL_CS NULL
#define OPTION_VAL_SONY_CXA2025AS_US_CS NULL
#define OPTION_VAL_BMF_FINAL2_CS NULL
#define OPTION_VAL_BMF_FINAL3_CS NULL
#define OPTION_VAL_SMOOTH_FBX_CS NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_CS NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_CS NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_CS NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_CS NULL
#define OPTION_VAL_NESCAP_CS NULL
#define OPTION_VAL_WAVEBEAM_CS NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_CS NULL
#define OPTION_VAL_MAGNUM_FBX_CS NULL
#define OPTION_VAL_SMOOTH_V2_FBX_CS NULL
#define OPTION_VAL_NES_CLASSIC_FBX_CS NULL
#define OPTION_VAL_RAW_CS "Nezpracovaná Data"
#define OPTION_VAL_CUSTOM_CS "Vlastní"
#define FCEUMM_NTSC_FILTER_LABEL_CS "NTSC Filtr"
#define FCEUMM_NTSC_FILTER_INFO_0_CS NULL
#define OPTION_VAL_COMPOSITE_CS "Kompozitní"
#define OPTION_VAL_SVIDEO_CS NULL
#define OPTION_VAL_RGB_O9_CS NULL
#define OPTION_VAL_MONOCHROME_CS NULL
#define FCEUMM_SNDQUALITY_LABEL_CS "Kvalita Zvuku"
#define FCEUMM_SNDQUALITY_INFO_0_CS "Povolte vyšší kvalitu zvuku. Zvyšuje požadavky na výkon."
#define OPTION_VAL_LOW_CS "Nízká"
#define OPTION_VAL_HIGH_CS "Vysoká"
#define OPTION_VAL_VERY_HIGH_CS "Velmi Vysoká"
#define FCEUMM_SNDLOWPASS_LABEL_CS NULL
#define FCEUMM_SNDLOWPASS_INFO_0_CS NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_CS NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_CS NULL
#define OPTION_VAL_01_MS_DELAY_CS NULL
#define OPTION_VAL_02_MS_DELAY_CS NULL
#define OPTION_VAL_03_MS_DELAY_CS NULL
#define OPTION_VAL_04_MS_DELAY_CS NULL
#define OPTION_VAL_05_MS_DELAY_CS NULL
#define OPTION_VAL_06_MS_DELAY_CS NULL
#define OPTION_VAL_07_MS_DELAY_CS NULL
#define OPTION_VAL_08_MS_DELAY_CS NULL
#define OPTION_VAL_09_MS_DELAY_CS NULL
#define OPTION_VAL_10_MS_DELAY_CS NULL
#define OPTION_VAL_11_MS_DELAY_CS NULL
#define OPTION_VAL_12_MS_DELAY_CS NULL
#define OPTION_VAL_13_MS_DELAY_CS NULL
#define OPTION_VAL_14_MS_DELAY_CS NULL
#define OPTION_VAL_15_MS_DELAY_CS NULL
#define OPTION_VAL_16_MS_DELAY_CS NULL
#define OPTION_VAL_17_MS_DELAY_CS NULL
#define OPTION_VAL_18_MS_DELAY_CS NULL
#define OPTION_VAL_19_MS_DELAY_CS NULL
#define OPTION_VAL_20_MS_DELAY_CS NULL
#define OPTION_VAL_21_MS_DELAY_CS NULL
#define OPTION_VAL_22_MS_DELAY_CS NULL
#define OPTION_VAL_23_MS_DELAY_CS NULL
#define OPTION_VAL_24_MS_DELAY_CS NULL
#define OPTION_VAL_25_MS_DELAY_CS NULL
#define OPTION_VAL_26_MS_DELAY_CS NULL
#define OPTION_VAL_27_MS_DELAY_CS NULL
#define OPTION_VAL_28_MS_DELAY_CS NULL
#define OPTION_VAL_29_MS_DELAY_CS NULL
#define OPTION_VAL_30_MS_DELAY_CS NULL
#define OPTION_VAL_31_MS_DELAY_CS NULL
#define OPTION_VAL_32_MS_DELAY_CS NULL
#define FCEUMM_SWAPDUTY_LABEL_CS "Výměna Pracovních Cyklů Zvuku"
#define FCEUMM_SWAPDUTY_LABEL_CAT_CS "Výměna Pracovních Cyklů"
#define FCEUMM_SWAPDUTY_INFO_0_CS "Simuluje zvuk z famiclonů, které mají obrácené bity pracovního cyklu kanálů pulzní vlny."
#define FCEUMM_SNDVOLUME_LABEL_CS "Hlavní Hlasitost"
#define FCEUMM_SNDVOLUME_INFO_0_CS "Změnit hlavní úroveň hlasitosti."
#define OPTION_VAL_0_CS NULL
#define OPTION_VAL_1_CS NULL
#define OPTION_VAL_2_CS NULL
#define OPTION_VAL_3_CS NULL
#define OPTION_VAL_4_CS NULL
#define OPTION_VAL_5_CS NULL
#define OPTION_VAL_6_CS NULL
#define OPTION_VAL_7_CS NULL
#define OPTION_VAL_8_CS NULL
#define OPTION_VAL_9_CS NULL
#define OPTION_VAL_10_CS NULL
#define FCEUMM_APU_1_LABEL_CS "Zvukový Kanál 1 (Square 1)"
#define FCEUMM_APU_1_LABEL_CAT_CS "Kanál 1 (Square 1)"
#define FCEUMM_APU_1_INFO_0_CS "Povolí nebo zakáže zvukový výstup generátoru pulsních vln 1."
#define FCEUMM_APU_2_LABEL_CS "Zvukový Kanál 2 (Square 2)"
#define FCEUMM_APU_2_LABEL_CAT_CS "Kanál 2 (Square 2)"
#define FCEUMM_APU_2_INFO_0_CS "Povolí nebo zakáže zvukový výstup generátoru pulsních vln 2."
#define FCEUMM_APU_3_LABEL_CS "Zvukový Kanál 3 (Triangle)"
#define FCEUMM_APU_3_LABEL_CAT_CS "Kanál 3 (Triangle)"
#define FCEUMM_APU_3_INFO_0_CS "Povolí nebo zakáže zvukový výstup generátoru trojúhelníkových vln."
#define FCEUMM_APU_4_LABEL_CS "Zvukový Kanál 4 (Noise)"
#define FCEUMM_APU_4_LABEL_CAT_CS "Kanál 4 (Noise)"
#define FCEUMM_APU_4_INFO_0_CS "Povolí nebo zakáže zvukový výstup generátoru šumu."
#define FCEUMM_APU_5_LABEL_CS "Zvukový Kanál 5 (PCM)"
#define FCEUMM_APU_5_LABEL_CAT_CS "Kanál 5 (PCM)"
#define FCEUMM_APU_5_INFO_0_CS "Povolí nebo zakáže zvukový výstup delta modulačního kanálu."
#define FCEUMM_TURBO_ENABLE_LABEL_CS "Povolení Turba"
#define FCEUMM_TURBO_ENABLE_INFO_0_CS "Povolí nebo zakáže turbo tlačítka."
#define OPTION_VAL_NONE_CS "Žádné"
#define OPTION_VAL_PLAYER_1_CS NULL
#define OPTION_VAL_PLAYER_2_CS NULL
#define OPTION_VAL_BOTH_CS "Obojí"
#define FCEUMM_TURBO_DELAY_LABEL_CS "Zpoždění Turba (ve snímcích/s)"
#define FCEUMM_TURBO_DELAY_INFO_0_CS "Rychlost opakování turbo tlačítek ve snímcích/s."
#define FCEUMM_ZAPPER_MODE_LABEL_CS "Režim Zapperu"
#define FCEUMM_ZAPPER_MODE_INFO_0_CS NULL
#define OPTION_VAL_CLIGHTGUN_CS NULL
#define OPTION_VAL_STLIGHTGUN_CS NULL
#define OPTION_VAL_TOUCHSCREEN_CS "Dotyková Obrazovka"
#define OPTION_VAL_MOUSE_CS "Myš"
#define FCEUMM_SHOW_CROSSHAIR_LABEL_CS "Zobrazit Zapper Kříž"
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_CS NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_CS "Dovolená Odchylka Zapperu"
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_CS "Nastaví kolik pixelů z cílové oblasti je na cíli."
#define FCEUMM_ZAPPER_TRIGGER_LABEL_CS NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_CS NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_CS NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_CS NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_CS NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_CS "Umožňuje současné kombinace tlačítek NAHORU+DOLŮ nebo VLEVO+VPRAVO, které mohou v některých hrách vytvářet různé efekty."
#define FCEUMM_NOSPRITELIMIT_LABEL_CS "Žádný Limit Spritu"
#define FCEUMM_NOSPRITELIMIT_INFO_0_CS "Odstraní hardwarový limit 8 skenovacích řádků. Tím se sníží blikání spritu, ale v některých hrách to může způsobit chybu, protože některé hry to používají pro efekty."
#define FCEUMM_OVERCLOCKING_LABEL_CS "Přetaktování"
#define FCEUMM_OVERCLOCKING_INFO_0_CS "Povoluje nebo zakazuje přetaktování, které může snížit zpomalení v některých hrách. Metoda Postrender je kompatibilnější s každou hrou, Vblank je účinnější pro hry jako Contra Force."
#define OPTION_VAL_2X_POSTRENDER_CS NULL
#define OPTION_VAL_2X_VBLANK_CS NULL
#define FCEUMM_RAMSTATE_LABEL_CS "Vyplnění Paměti RAM při Zapnutí (Nutný Restart)"
#define FCEUMM_RAMSTATE_INFO_0_CS "Hodnoty paměti RAM při zapnutí. Některé hry se například spoléhají na počáteční hodnoty RAM při generování náhodných čísel."
#define OPTION_VAL_FILL_FF_CS NULL
#define OPTION_VAL_FILL_00_CS NULL
#define OPTION_VAL_RANDOM_CS "Náhodná"

struct retro_core_option_v2_category option_cats_cs[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_CS,
      CATEGORY_SYSTEM_INFO_0_CS
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_CS,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_CS
#else
      CATEGORY_VIDEO_INFO_1_CS
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_CS,
      CATEGORY_AUDIO_INFO_0_CS
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_CS,
      CATEGORY_INPUT_INFO_0_CS
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_CS,
      CATEGORY_HACKS_INFO_0_CS
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_CS,
      CATEGORY_DIP_SWITCH_INFO_0_CS
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_cs[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_CS,
      NULL,
      FCEUMM_REGION_INFO_0_CS,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_CS },
         { "NTSC",  OPTION_VAL_NTSC_CS },
         { "PAL",   OPTION_VAL_PAL_CS },
         { "Dendy", OPTION_VAL_DENDY_CS },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_CS,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_CS,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_CS,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_CS,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_CS,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_CS,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_CS,
      NULL,
      FCEUMM_ASPECT_INFO_0_CS,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_CS },
         { "4:3",     OPTION_VAL_4_3_CS },
         { "PP",      OPTION_VAL_PP_CS },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_CS,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_CS,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_CS,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_CS,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_CS,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_CS,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_CS,
      NULL,
      FCEUMM_PALETTE_INFO_0_CS,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_CS },
         { "asqrealc",             OPTION_VAL_ASQREALC_CS },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_CS },
         { "rgb",                  OPTION_VAL_RGB_CS },
         { "yuv-v3",               OPTION_VAL_YUV_V3_CS },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_CS },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_CS },
         { "pal",                  OPTION_VAL_PAL_CS },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_CS },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_CS },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_CS },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_CS },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_CS },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_CS },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_CS },
         { "nescap",               OPTION_VAL_NESCAP_CS },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_CS },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_CS },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_CS },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_CS },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_CS },
         { "raw",                  OPTION_VAL_RAW_CS },
         { "custom",               OPTION_VAL_CUSTOM_CS },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_CS,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_CS,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_CS },
         { "svideo",     OPTION_VAL_SVIDEO_CS },
         { "rgb",        OPTION_VAL_RGB_O9_CS },
         { "monochrome", OPTION_VAL_MONOCHROME_CS },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_CS,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_CS,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_CS },
         { "High",      OPTION_VAL_HIGH_CS },
         { "Very High", OPTION_VAL_VERY_HIGH_CS },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_CS,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_CS,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_CS,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_CS,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_CS },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_CS },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_CS },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_CS },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_CS },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_CS },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_CS },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_CS },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_CS },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_CS },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_CS },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_CS },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_CS },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_CS },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_CS },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_CS },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_CS },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_CS },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_CS },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_CS },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_CS },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_CS },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_CS },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_CS },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_CS },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_CS },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_CS },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_CS },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_CS },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_CS },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_CS },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_CS },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_CS,
      FCEUMM_SWAPDUTY_LABEL_CAT_CS,
      FCEUMM_SWAPDUTY_INFO_0_CS,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_CS,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_CS,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_CS },
         { "1",  OPTION_VAL_1_CS },
         { "2",  OPTION_VAL_2_CS },
         { "3",  OPTION_VAL_3_CS },
         { "4",  OPTION_VAL_4_CS },
         { "5",  OPTION_VAL_5_CS },
         { "6",  OPTION_VAL_6_CS },
         { "7",  OPTION_VAL_7_CS },
         { "8",  OPTION_VAL_8_CS },
         { "9",  OPTION_VAL_9_CS },
         { "10", OPTION_VAL_10_CS },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_CS,
      FCEUMM_APU_1_LABEL_CAT_CS,
      FCEUMM_APU_1_INFO_0_CS,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_CS,
      FCEUMM_APU_2_LABEL_CAT_CS,
      FCEUMM_APU_2_INFO_0_CS,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_CS,
      FCEUMM_APU_3_LABEL_CAT_CS,
      FCEUMM_APU_3_INFO_0_CS,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_CS,
      FCEUMM_APU_4_LABEL_CAT_CS,
      FCEUMM_APU_4_INFO_0_CS,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_CS,
      FCEUMM_APU_5_LABEL_CAT_CS,
      FCEUMM_APU_5_INFO_0_CS,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_CS,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_CS,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_CS },
         { "Player 1", OPTION_VAL_PLAYER_1_CS },
         { "Player 2", OPTION_VAL_PLAYER_2_CS },
         { "Both",     OPTION_VAL_BOTH_CS },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_CS,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_CS,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_CS,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_CS,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_CS },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_CS },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_CS },
         { "mouse",       OPTION_VAL_MOUSE_CS },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_CS,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_CS,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_CS,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_CS,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_CS,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_CS,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_CS,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_CS,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_CS,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_CS,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_CS,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_CS,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_CS,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_CS,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_CS },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_CS },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_CS,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_CS,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_CS },
         { "fill $00", OPTION_VAL_FILL_00_CS },
         { "random",   OPTION_VAL_RANDOM_CS },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_cs = {
   option_cats_cs,
   option_defs_cs
};

/* RETRO_LANGUAGE_CY */

#define CATEGORY_SYSTEM_LABEL_CY NULL
#define CATEGORY_SYSTEM_INFO_0_CY NULL
#define CATEGORY_VIDEO_LABEL_CY NULL
#define CATEGORY_VIDEO_INFO_0_CY NULL
#define CATEGORY_VIDEO_INFO_1_CY NULL
#define CATEGORY_AUDIO_LABEL_CY NULL
#define CATEGORY_AUDIO_INFO_0_CY NULL
#define CATEGORY_INPUT_LABEL_CY "Mewnbwn"
#define CATEGORY_INPUT_INFO_0_CY NULL
#define CATEGORY_HACKS_LABEL_CY NULL
#define CATEGORY_HACKS_INFO_0_CY NULL
#define CATEGORY_DIP_SWITCH_LABEL_CY NULL
#define CATEGORY_DIP_SWITCH_INFO_0_CY NULL
#define FCEUMM_REGION_LABEL_CY NULL
#define FCEUMM_REGION_INFO_0_CY NULL
#define OPTION_VAL_AUTO_CY NULL
#define OPTION_VAL_NTSC_CY NULL
#define OPTION_VAL_PAL_CY NULL
#define OPTION_VAL_DENDY_CY NULL
#define FCEUMM_GAME_GENIE_LABEL_CY NULL
#define FCEUMM_GAME_GENIE_INFO_0_CY NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_CY NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_CY NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_CY NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_CY NULL
#define FCEUMM_ASPECT_LABEL_CY NULL
#define FCEUMM_ASPECT_INFO_0_CY NULL
#define OPTION_VAL_8_7_PAR_CY NULL
#define OPTION_VAL_4_3_CY NULL
#define OPTION_VAL_PP_CY NULL
#define FCEUMM_OVERSCAN_LABEL_CY NULL
#define FCEUMM_OVERSCAN_INFO_0_CY NULL
#define FCEUMM_OVERSCAN_H_LABEL_CY NULL
#define FCEUMM_OVERSCAN_H_INFO_0_CY NULL
#define FCEUMM_OVERSCAN_V_LABEL_CY NULL
#define FCEUMM_OVERSCAN_V_INFO_0_CY NULL
#define FCEUMM_PALETTE_LABEL_CY NULL
#define FCEUMM_PALETTE_INFO_0_CY NULL
#define OPTION_VAL_DEFAULT_CY NULL
#define OPTION_VAL_ASQREALC_CY NULL
#define OPTION_VAL_NINTENDO_VC_CY NULL
#define OPTION_VAL_RGB_CY NULL
#define OPTION_VAL_YUV_V3_CY NULL
#define OPTION_VAL_UNSATURATED_FINAL_CY NULL
#define OPTION_VAL_SONY_CXA2025AS_US_CY NULL
#define OPTION_VAL_BMF_FINAL2_CY NULL
#define OPTION_VAL_BMF_FINAL3_CY NULL
#define OPTION_VAL_SMOOTH_FBX_CY NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_CY NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_CY NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_CY NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_CY NULL
#define OPTION_VAL_NESCAP_CY NULL
#define OPTION_VAL_WAVEBEAM_CY NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_CY NULL
#define OPTION_VAL_MAGNUM_FBX_CY NULL
#define OPTION_VAL_SMOOTH_V2_FBX_CY NULL
#define OPTION_VAL_NES_CLASSIC_FBX_CY NULL
#define OPTION_VAL_RAW_CY NULL
#define OPTION_VAL_CUSTOM_CY NULL
#define FCEUMM_NTSC_FILTER_LABEL_CY NULL
#define FCEUMM_NTSC_FILTER_INFO_0_CY NULL
#define OPTION_VAL_COMPOSITE_CY NULL
#define OPTION_VAL_SVIDEO_CY NULL
#define OPTION_VAL_RGB_O9_CY NULL
#define OPTION_VAL_MONOCHROME_CY NULL
#define FCEUMM_SNDQUALITY_LABEL_CY NULL
#define FCEUMM_SNDQUALITY_INFO_0_CY NULL
#define OPTION_VAL_LOW_CY NULL
#define OPTION_VAL_HIGH_CY NULL
#define OPTION_VAL_VERY_HIGH_CY NULL
#define FCEUMM_SNDLOWPASS_LABEL_CY NULL
#define FCEUMM_SNDLOWPASS_INFO_0_CY NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_CY NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_CY NULL
#define OPTION_VAL_01_MS_DELAY_CY NULL
#define OPTION_VAL_02_MS_DELAY_CY NULL
#define OPTION_VAL_03_MS_DELAY_CY NULL
#define OPTION_VAL_04_MS_DELAY_CY NULL
#define OPTION_VAL_05_MS_DELAY_CY NULL
#define OPTION_VAL_06_MS_DELAY_CY NULL
#define OPTION_VAL_07_MS_DELAY_CY NULL
#define OPTION_VAL_08_MS_DELAY_CY NULL
#define OPTION_VAL_09_MS_DELAY_CY NULL
#define OPTION_VAL_10_MS_DELAY_CY NULL
#define OPTION_VAL_11_MS_DELAY_CY NULL
#define OPTION_VAL_12_MS_DELAY_CY NULL
#define OPTION_VAL_13_MS_DELAY_CY NULL
#define OPTION_VAL_14_MS_DELAY_CY NULL
#define OPTION_VAL_15_MS_DELAY_CY NULL
#define OPTION_VAL_16_MS_DELAY_CY NULL
#define OPTION_VAL_17_MS_DELAY_CY NULL
#define OPTION_VAL_18_MS_DELAY_CY NULL
#define OPTION_VAL_19_MS_DELAY_CY NULL
#define OPTION_VAL_20_MS_DELAY_CY NULL
#define OPTION_VAL_21_MS_DELAY_CY NULL
#define OPTION_VAL_22_MS_DELAY_CY NULL
#define OPTION_VAL_23_MS_DELAY_CY NULL
#define OPTION_VAL_24_MS_DELAY_CY NULL
#define OPTION_VAL_25_MS_DELAY_CY NULL
#define OPTION_VAL_26_MS_DELAY_CY NULL
#define OPTION_VAL_27_MS_DELAY_CY NULL
#define OPTION_VAL_28_MS_DELAY_CY NULL
#define OPTION_VAL_29_MS_DELAY_CY NULL
#define OPTION_VAL_30_MS_DELAY_CY NULL
#define OPTION_VAL_31_MS_DELAY_CY NULL
#define OPTION_VAL_32_MS_DELAY_CY NULL
#define FCEUMM_SWAPDUTY_LABEL_CY NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_CY NULL
#define FCEUMM_SWAPDUTY_INFO_0_CY NULL
#define FCEUMM_SNDVOLUME_LABEL_CY NULL
#define FCEUMM_SNDVOLUME_INFO_0_CY NULL
#define OPTION_VAL_0_CY NULL
#define OPTION_VAL_1_CY NULL
#define OPTION_VAL_2_CY NULL
#define OPTION_VAL_3_CY NULL
#define OPTION_VAL_4_CY NULL
#define OPTION_VAL_5_CY NULL
#define OPTION_VAL_6_CY NULL
#define OPTION_VAL_7_CY NULL
#define OPTION_VAL_8_CY NULL
#define OPTION_VAL_9_CY NULL
#define OPTION_VAL_10_CY NULL
#define FCEUMM_APU_1_LABEL_CY NULL
#define FCEUMM_APU_1_LABEL_CAT_CY NULL
#define FCEUMM_APU_1_INFO_0_CY NULL
#define FCEUMM_APU_2_LABEL_CY NULL
#define FCEUMM_APU_2_LABEL_CAT_CY NULL
#define FCEUMM_APU_2_INFO_0_CY NULL
#define FCEUMM_APU_3_LABEL_CY NULL
#define FCEUMM_APU_3_LABEL_CAT_CY NULL
#define FCEUMM_APU_3_INFO_0_CY NULL
#define FCEUMM_APU_4_LABEL_CY NULL
#define FCEUMM_APU_4_LABEL_CAT_CY NULL
#define FCEUMM_APU_4_INFO_0_CY NULL
#define FCEUMM_APU_5_LABEL_CY NULL
#define FCEUMM_APU_5_LABEL_CAT_CY NULL
#define FCEUMM_APU_5_INFO_0_CY NULL
#define FCEUMM_TURBO_ENABLE_LABEL_CY NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_CY NULL
#define OPTION_VAL_NONE_CY "Dim"
#define OPTION_VAL_PLAYER_1_CY NULL
#define OPTION_VAL_PLAYER_2_CY NULL
#define OPTION_VAL_BOTH_CY NULL
#define FCEUMM_TURBO_DELAY_LABEL_CY NULL
#define FCEUMM_TURBO_DELAY_INFO_0_CY NULL
#define FCEUMM_ZAPPER_MODE_LABEL_CY NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_CY NULL
#define OPTION_VAL_CLIGHTGUN_CY NULL
#define OPTION_VAL_STLIGHTGUN_CY NULL
#define OPTION_VAL_TOUCHSCREEN_CY NULL
#define OPTION_VAL_MOUSE_CY NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_CY NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_CY NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_CY NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_CY NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_CY NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_CY NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_CY NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_CY NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_CY NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_CY NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_CY NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_CY NULL
#define FCEUMM_OVERCLOCKING_LABEL_CY NULL
#define FCEUMM_OVERCLOCKING_INFO_0_CY NULL
#define OPTION_VAL_2X_POSTRENDER_CY NULL
#define OPTION_VAL_2X_VBLANK_CY NULL
#define FCEUMM_RAMSTATE_LABEL_CY NULL
#define FCEUMM_RAMSTATE_INFO_0_CY NULL
#define OPTION_VAL_FILL_FF_CY NULL
#define OPTION_VAL_FILL_00_CY NULL
#define OPTION_VAL_RANDOM_CY NULL

struct retro_core_option_v2_category option_cats_cy[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_CY,
      CATEGORY_SYSTEM_INFO_0_CY
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_CY,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_CY
#else
      CATEGORY_VIDEO_INFO_1_CY
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_CY,
      CATEGORY_AUDIO_INFO_0_CY
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_CY,
      CATEGORY_INPUT_INFO_0_CY
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_CY,
      CATEGORY_HACKS_INFO_0_CY
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_CY,
      CATEGORY_DIP_SWITCH_INFO_0_CY
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_cy[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_CY,
      NULL,
      FCEUMM_REGION_INFO_0_CY,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_CY },
         { "NTSC",  OPTION_VAL_NTSC_CY },
         { "PAL",   OPTION_VAL_PAL_CY },
         { "Dendy", OPTION_VAL_DENDY_CY },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_CY,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_CY,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_CY,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_CY,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_CY,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_CY,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_CY,
      NULL,
      FCEUMM_ASPECT_INFO_0_CY,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_CY },
         { "4:3",     OPTION_VAL_4_3_CY },
         { "PP",      OPTION_VAL_PP_CY },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_CY,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_CY,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_CY,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_CY,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_CY,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_CY,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_CY,
      NULL,
      FCEUMM_PALETTE_INFO_0_CY,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_CY },
         { "asqrealc",             OPTION_VAL_ASQREALC_CY },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_CY },
         { "rgb",                  OPTION_VAL_RGB_CY },
         { "yuv-v3",               OPTION_VAL_YUV_V3_CY },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_CY },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_CY },
         { "pal",                  OPTION_VAL_PAL_CY },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_CY },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_CY },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_CY },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_CY },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_CY },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_CY },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_CY },
         { "nescap",               OPTION_VAL_NESCAP_CY },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_CY },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_CY },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_CY },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_CY },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_CY },
         { "raw",                  OPTION_VAL_RAW_CY },
         { "custom",               OPTION_VAL_CUSTOM_CY },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_CY,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_CY,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_CY },
         { "svideo",     OPTION_VAL_SVIDEO_CY },
         { "rgb",        OPTION_VAL_RGB_O9_CY },
         { "monochrome", OPTION_VAL_MONOCHROME_CY },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_CY,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_CY,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_CY },
         { "High",      OPTION_VAL_HIGH_CY },
         { "Very High", OPTION_VAL_VERY_HIGH_CY },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_CY,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_CY,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_CY,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_CY,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_CY },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_CY },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_CY },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_CY },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_CY },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_CY },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_CY },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_CY },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_CY },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_CY },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_CY },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_CY },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_CY },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_CY },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_CY },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_CY },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_CY },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_CY },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_CY },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_CY },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_CY },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_CY },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_CY },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_CY },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_CY },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_CY },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_CY },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_CY },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_CY },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_CY },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_CY },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_CY },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_CY,
      FCEUMM_SWAPDUTY_LABEL_CAT_CY,
      FCEUMM_SWAPDUTY_INFO_0_CY,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_CY,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_CY,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_CY },
         { "1",  OPTION_VAL_1_CY },
         { "2",  OPTION_VAL_2_CY },
         { "3",  OPTION_VAL_3_CY },
         { "4",  OPTION_VAL_4_CY },
         { "5",  OPTION_VAL_5_CY },
         { "6",  OPTION_VAL_6_CY },
         { "7",  OPTION_VAL_7_CY },
         { "8",  OPTION_VAL_8_CY },
         { "9",  OPTION_VAL_9_CY },
         { "10", OPTION_VAL_10_CY },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_CY,
      FCEUMM_APU_1_LABEL_CAT_CY,
      FCEUMM_APU_1_INFO_0_CY,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_CY,
      FCEUMM_APU_2_LABEL_CAT_CY,
      FCEUMM_APU_2_INFO_0_CY,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_CY,
      FCEUMM_APU_3_LABEL_CAT_CY,
      FCEUMM_APU_3_INFO_0_CY,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_CY,
      FCEUMM_APU_4_LABEL_CAT_CY,
      FCEUMM_APU_4_INFO_0_CY,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_CY,
      FCEUMM_APU_5_LABEL_CAT_CY,
      FCEUMM_APU_5_INFO_0_CY,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_CY,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_CY,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_CY },
         { "Player 1", OPTION_VAL_PLAYER_1_CY },
         { "Player 2", OPTION_VAL_PLAYER_2_CY },
         { "Both",     OPTION_VAL_BOTH_CY },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_CY,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_CY,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_CY,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_CY,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_CY },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_CY },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_CY },
         { "mouse",       OPTION_VAL_MOUSE_CY },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_CY,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_CY,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_CY,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_CY,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_CY,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_CY,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_CY,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_CY,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_CY,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_CY,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_CY,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_CY,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_CY,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_CY,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_CY },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_CY },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_CY,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_CY,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_CY },
         { "fill $00", OPTION_VAL_FILL_00_CY },
         { "random",   OPTION_VAL_RANDOM_CY },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_cy = {
   option_cats_cy,
   option_defs_cy
};

/* RETRO_LANGUAGE_DA */

#define CATEGORY_SYSTEM_LABEL_DA NULL
#define CATEGORY_SYSTEM_INFO_0_DA NULL
#define CATEGORY_VIDEO_LABEL_DA NULL
#define CATEGORY_VIDEO_INFO_0_DA NULL
#define CATEGORY_VIDEO_INFO_1_DA NULL
#define CATEGORY_AUDIO_LABEL_DA "Lyd"
#define CATEGORY_AUDIO_INFO_0_DA NULL
#define CATEGORY_INPUT_LABEL_DA NULL
#define CATEGORY_INPUT_INFO_0_DA NULL
#define CATEGORY_HACKS_LABEL_DA NULL
#define CATEGORY_HACKS_INFO_0_DA NULL
#define CATEGORY_DIP_SWITCH_LABEL_DA NULL
#define CATEGORY_DIP_SWITCH_INFO_0_DA NULL
#define FCEUMM_REGION_LABEL_DA NULL
#define FCEUMM_REGION_INFO_0_DA NULL
#define OPTION_VAL_AUTO_DA NULL
#define OPTION_VAL_NTSC_DA NULL
#define OPTION_VAL_PAL_DA NULL
#define OPTION_VAL_DENDY_DA NULL
#define FCEUMM_GAME_GENIE_LABEL_DA NULL
#define FCEUMM_GAME_GENIE_INFO_0_DA NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_DA NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_DA NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_DA NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_DA NULL
#define FCEUMM_ASPECT_LABEL_DA NULL
#define FCEUMM_ASPECT_INFO_0_DA NULL
#define OPTION_VAL_8_7_PAR_DA NULL
#define OPTION_VAL_4_3_DA NULL
#define OPTION_VAL_PP_DA NULL
#define FCEUMM_OVERSCAN_LABEL_DA NULL
#define FCEUMM_OVERSCAN_INFO_0_DA NULL
#define FCEUMM_OVERSCAN_H_LABEL_DA NULL
#define FCEUMM_OVERSCAN_H_INFO_0_DA NULL
#define FCEUMM_OVERSCAN_V_LABEL_DA NULL
#define FCEUMM_OVERSCAN_V_INFO_0_DA NULL
#define FCEUMM_PALETTE_LABEL_DA NULL
#define FCEUMM_PALETTE_INFO_0_DA NULL
#define OPTION_VAL_DEFAULT_DA NULL
#define OPTION_VAL_ASQREALC_DA NULL
#define OPTION_VAL_NINTENDO_VC_DA NULL
#define OPTION_VAL_RGB_DA NULL
#define OPTION_VAL_YUV_V3_DA NULL
#define OPTION_VAL_UNSATURATED_FINAL_DA NULL
#define OPTION_VAL_SONY_CXA2025AS_US_DA NULL
#define OPTION_VAL_BMF_FINAL2_DA NULL
#define OPTION_VAL_BMF_FINAL3_DA NULL
#define OPTION_VAL_SMOOTH_FBX_DA NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_DA NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_DA NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_DA NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_DA NULL
#define OPTION_VAL_NESCAP_DA NULL
#define OPTION_VAL_WAVEBEAM_DA NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_DA NULL
#define OPTION_VAL_MAGNUM_FBX_DA NULL
#define OPTION_VAL_SMOOTH_V2_FBX_DA NULL
#define OPTION_VAL_NES_CLASSIC_FBX_DA NULL
#define OPTION_VAL_RAW_DA NULL
#define OPTION_VAL_CUSTOM_DA NULL
#define FCEUMM_NTSC_FILTER_LABEL_DA NULL
#define FCEUMM_NTSC_FILTER_INFO_0_DA NULL
#define OPTION_VAL_COMPOSITE_DA NULL
#define OPTION_VAL_SVIDEO_DA NULL
#define OPTION_VAL_RGB_O9_DA NULL
#define OPTION_VAL_MONOCHROME_DA NULL
#define FCEUMM_SNDQUALITY_LABEL_DA NULL
#define FCEUMM_SNDQUALITY_INFO_0_DA NULL
#define OPTION_VAL_LOW_DA NULL
#define OPTION_VAL_HIGH_DA NULL
#define OPTION_VAL_VERY_HIGH_DA NULL
#define FCEUMM_SNDLOWPASS_LABEL_DA NULL
#define FCEUMM_SNDLOWPASS_INFO_0_DA NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_DA NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_DA NULL
#define OPTION_VAL_01_MS_DELAY_DA NULL
#define OPTION_VAL_02_MS_DELAY_DA NULL
#define OPTION_VAL_03_MS_DELAY_DA NULL
#define OPTION_VAL_04_MS_DELAY_DA NULL
#define OPTION_VAL_05_MS_DELAY_DA NULL
#define OPTION_VAL_06_MS_DELAY_DA NULL
#define OPTION_VAL_07_MS_DELAY_DA NULL
#define OPTION_VAL_08_MS_DELAY_DA NULL
#define OPTION_VAL_09_MS_DELAY_DA NULL
#define OPTION_VAL_10_MS_DELAY_DA NULL
#define OPTION_VAL_11_MS_DELAY_DA NULL
#define OPTION_VAL_12_MS_DELAY_DA NULL
#define OPTION_VAL_13_MS_DELAY_DA NULL
#define OPTION_VAL_14_MS_DELAY_DA NULL
#define OPTION_VAL_15_MS_DELAY_DA NULL
#define OPTION_VAL_16_MS_DELAY_DA NULL
#define OPTION_VAL_17_MS_DELAY_DA NULL
#define OPTION_VAL_18_MS_DELAY_DA NULL
#define OPTION_VAL_19_MS_DELAY_DA NULL
#define OPTION_VAL_20_MS_DELAY_DA NULL
#define OPTION_VAL_21_MS_DELAY_DA NULL
#define OPTION_VAL_22_MS_DELAY_DA NULL
#define OPTION_VAL_23_MS_DELAY_DA NULL
#define OPTION_VAL_24_MS_DELAY_DA NULL
#define OPTION_VAL_25_MS_DELAY_DA NULL
#define OPTION_VAL_26_MS_DELAY_DA NULL
#define OPTION_VAL_27_MS_DELAY_DA NULL
#define OPTION_VAL_28_MS_DELAY_DA NULL
#define OPTION_VAL_29_MS_DELAY_DA NULL
#define OPTION_VAL_30_MS_DELAY_DA NULL
#define OPTION_VAL_31_MS_DELAY_DA NULL
#define OPTION_VAL_32_MS_DELAY_DA NULL
#define FCEUMM_SWAPDUTY_LABEL_DA NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_DA NULL
#define FCEUMM_SWAPDUTY_INFO_0_DA NULL
#define FCEUMM_SNDVOLUME_LABEL_DA NULL
#define FCEUMM_SNDVOLUME_INFO_0_DA NULL
#define OPTION_VAL_0_DA NULL
#define OPTION_VAL_1_DA NULL
#define OPTION_VAL_2_DA NULL
#define OPTION_VAL_3_DA NULL
#define OPTION_VAL_4_DA NULL
#define OPTION_VAL_5_DA NULL
#define OPTION_VAL_6_DA NULL
#define OPTION_VAL_7_DA NULL
#define OPTION_VAL_8_DA NULL
#define OPTION_VAL_9_DA NULL
#define OPTION_VAL_10_DA NULL
#define FCEUMM_APU_1_LABEL_DA NULL
#define FCEUMM_APU_1_LABEL_CAT_DA NULL
#define FCEUMM_APU_1_INFO_0_DA NULL
#define FCEUMM_APU_2_LABEL_DA NULL
#define FCEUMM_APU_2_LABEL_CAT_DA NULL
#define FCEUMM_APU_2_INFO_0_DA NULL
#define FCEUMM_APU_3_LABEL_DA NULL
#define FCEUMM_APU_3_LABEL_CAT_DA NULL
#define FCEUMM_APU_3_INFO_0_DA NULL
#define FCEUMM_APU_4_LABEL_DA NULL
#define FCEUMM_APU_4_LABEL_CAT_DA NULL
#define FCEUMM_APU_4_INFO_0_DA NULL
#define FCEUMM_APU_5_LABEL_DA NULL
#define FCEUMM_APU_5_LABEL_CAT_DA NULL
#define FCEUMM_APU_5_INFO_0_DA NULL
#define FCEUMM_TURBO_ENABLE_LABEL_DA NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_DA NULL
#define OPTION_VAL_NONE_DA NULL
#define OPTION_VAL_PLAYER_1_DA NULL
#define OPTION_VAL_PLAYER_2_DA NULL
#define OPTION_VAL_BOTH_DA NULL
#define FCEUMM_TURBO_DELAY_LABEL_DA NULL
#define FCEUMM_TURBO_DELAY_INFO_0_DA NULL
#define FCEUMM_ZAPPER_MODE_LABEL_DA NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_DA NULL
#define OPTION_VAL_CLIGHTGUN_DA NULL
#define OPTION_VAL_STLIGHTGUN_DA NULL
#define OPTION_VAL_TOUCHSCREEN_DA NULL
#define OPTION_VAL_MOUSE_DA NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_DA NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_DA NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_DA NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_DA NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_DA NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_DA NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_DA NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_DA NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_DA NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_DA NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_DA NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_DA NULL
#define FCEUMM_OVERCLOCKING_LABEL_DA NULL
#define FCEUMM_OVERCLOCKING_INFO_0_DA NULL
#define OPTION_VAL_2X_POSTRENDER_DA NULL
#define OPTION_VAL_2X_VBLANK_DA NULL
#define FCEUMM_RAMSTATE_LABEL_DA NULL
#define FCEUMM_RAMSTATE_INFO_0_DA NULL
#define OPTION_VAL_FILL_FF_DA NULL
#define OPTION_VAL_FILL_00_DA NULL
#define OPTION_VAL_RANDOM_DA NULL

struct retro_core_option_v2_category option_cats_da[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_DA,
      CATEGORY_SYSTEM_INFO_0_DA
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_DA,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_DA
#else
      CATEGORY_VIDEO_INFO_1_DA
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_DA,
      CATEGORY_AUDIO_INFO_0_DA
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_DA,
      CATEGORY_INPUT_INFO_0_DA
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_DA,
      CATEGORY_HACKS_INFO_0_DA
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_DA,
      CATEGORY_DIP_SWITCH_INFO_0_DA
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_da[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_DA,
      NULL,
      FCEUMM_REGION_INFO_0_DA,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_DA },
         { "NTSC",  OPTION_VAL_NTSC_DA },
         { "PAL",   OPTION_VAL_PAL_DA },
         { "Dendy", OPTION_VAL_DENDY_DA },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_DA,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_DA,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_DA,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_DA,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_DA,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_DA,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_DA,
      NULL,
      FCEUMM_ASPECT_INFO_0_DA,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_DA },
         { "4:3",     OPTION_VAL_4_3_DA },
         { "PP",      OPTION_VAL_PP_DA },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_DA,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_DA,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_DA,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_DA,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_DA,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_DA,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_DA,
      NULL,
      FCEUMM_PALETTE_INFO_0_DA,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_DA },
         { "asqrealc",             OPTION_VAL_ASQREALC_DA },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_DA },
         { "rgb",                  OPTION_VAL_RGB_DA },
         { "yuv-v3",               OPTION_VAL_YUV_V3_DA },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_DA },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_DA },
         { "pal",                  OPTION_VAL_PAL_DA },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_DA },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_DA },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_DA },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_DA },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_DA },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_DA },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_DA },
         { "nescap",               OPTION_VAL_NESCAP_DA },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_DA },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_DA },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_DA },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_DA },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_DA },
         { "raw",                  OPTION_VAL_RAW_DA },
         { "custom",               OPTION_VAL_CUSTOM_DA },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_DA,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_DA,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_DA },
         { "svideo",     OPTION_VAL_SVIDEO_DA },
         { "rgb",        OPTION_VAL_RGB_O9_DA },
         { "monochrome", OPTION_VAL_MONOCHROME_DA },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_DA,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_DA,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_DA },
         { "High",      OPTION_VAL_HIGH_DA },
         { "Very High", OPTION_VAL_VERY_HIGH_DA },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_DA,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_DA,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_DA,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_DA,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_DA },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_DA },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_DA },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_DA },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_DA },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_DA },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_DA },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_DA },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_DA },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_DA },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_DA },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_DA },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_DA },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_DA },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_DA },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_DA },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_DA },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_DA },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_DA },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_DA },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_DA },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_DA },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_DA },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_DA },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_DA },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_DA },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_DA },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_DA },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_DA },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_DA },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_DA },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_DA },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_DA,
      FCEUMM_SWAPDUTY_LABEL_CAT_DA,
      FCEUMM_SWAPDUTY_INFO_0_DA,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_DA,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_DA,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_DA },
         { "1",  OPTION_VAL_1_DA },
         { "2",  OPTION_VAL_2_DA },
         { "3",  OPTION_VAL_3_DA },
         { "4",  OPTION_VAL_4_DA },
         { "5",  OPTION_VAL_5_DA },
         { "6",  OPTION_VAL_6_DA },
         { "7",  OPTION_VAL_7_DA },
         { "8",  OPTION_VAL_8_DA },
         { "9",  OPTION_VAL_9_DA },
         { "10", OPTION_VAL_10_DA },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_DA,
      FCEUMM_APU_1_LABEL_CAT_DA,
      FCEUMM_APU_1_INFO_0_DA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_DA,
      FCEUMM_APU_2_LABEL_CAT_DA,
      FCEUMM_APU_2_INFO_0_DA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_DA,
      FCEUMM_APU_3_LABEL_CAT_DA,
      FCEUMM_APU_3_INFO_0_DA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_DA,
      FCEUMM_APU_4_LABEL_CAT_DA,
      FCEUMM_APU_4_INFO_0_DA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_DA,
      FCEUMM_APU_5_LABEL_CAT_DA,
      FCEUMM_APU_5_INFO_0_DA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_DA,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_DA,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_DA },
         { "Player 1", OPTION_VAL_PLAYER_1_DA },
         { "Player 2", OPTION_VAL_PLAYER_2_DA },
         { "Both",     OPTION_VAL_BOTH_DA },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_DA,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_DA,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_DA,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_DA,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_DA },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_DA },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_DA },
         { "mouse",       OPTION_VAL_MOUSE_DA },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_DA,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_DA,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_DA,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_DA,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_DA,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_DA,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_DA,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_DA,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_DA,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_DA,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_DA,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_DA,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_DA,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_DA,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_DA },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_DA },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_DA,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_DA,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_DA },
         { "fill $00", OPTION_VAL_FILL_00_DA },
         { "random",   OPTION_VAL_RANDOM_DA },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_da = {
   option_cats_da,
   option_defs_da
};

/* RETRO_LANGUAGE_DE */

#define CATEGORY_SYSTEM_LABEL_DE NULL
#define CATEGORY_SYSTEM_INFO_0_DE NULL
#define CATEGORY_VIDEO_LABEL_DE NULL
#define CATEGORY_VIDEO_INFO_0_DE NULL
#define CATEGORY_VIDEO_INFO_1_DE NULL
#define CATEGORY_AUDIO_LABEL_DE NULL
#define CATEGORY_AUDIO_INFO_0_DE NULL
#define CATEGORY_INPUT_LABEL_DE NULL
#define CATEGORY_INPUT_INFO_0_DE NULL
#define CATEGORY_HACKS_LABEL_DE "Emulations-Hacks"
#define CATEGORY_HACKS_INFO_0_DE "Parameter für Prozessorübertaktung und Emulationsgenauigkeit, die sich auf die Low-Level-Leistung und Kompatibilität auswirken, ändern."
#define CATEGORY_DIP_SWITCH_LABEL_DE "DIP-Schalter"
#define CATEGORY_DIP_SWITCH_INFO_0_DE NULL
#define FCEUMM_REGION_LABEL_DE NULL
#define FCEUMM_REGION_INFO_0_DE "Erzwingen, dass der Kern NTSC, PAL oder Dendy Region Timings verwendet."
#define OPTION_VAL_AUTO_DE NULL
#define OPTION_VAL_NTSC_DE NULL
#define OPTION_VAL_PAL_DE NULL
#define OPTION_VAL_DENDY_DE NULL
#define FCEUMM_GAME_GENIE_LABEL_DE "Game Genie Add-On (Neustart erforderlich)"
#define FCEUMM_GAME_GENIE_INFO_0_DE "Aktiviere Emulation einer Game Genie Add-on Cartridge, die es erlaubt, Cheat-Codes einzugeben, wenn Spiele gestartet werden. Die Game Genie ROM Datei 'gamegenie.nes' muss im Frontend's Systemverzeichnis vorhanden sein. Gilt nicht für FDS oder Arcade-Inhalte."
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_DE "Erweiterte System Optionen anzeigen"
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_DE "Erweiterte System Optionen und Tweaks anzeigen."
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_DE "Erweiterte Ton Optionen anzeigen"
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_DE "Erweiterte Ton Optionen und Tweaks anzeigen."
#define FCEUMM_ASPECT_LABEL_DE "Seitenverhältnis"
#define FCEUMM_ASPECT_INFO_0_DE "Wähle bevorzugtes Seitenverhältnis."
#define OPTION_VAL_8_7_PAR_DE NULL
#define OPTION_VAL_4_3_DE NULL
#define OPTION_VAL_PP_DE "Pixelgenau"
#define FCEUMM_OVERSCAN_LABEL_DE "Overscan zuschneiden"
#define FCEUMM_OVERSCAN_INFO_0_DE "Entfernt 8 Pixel Overscan von allen Seiten des Bildschirms."
#define FCEUMM_OVERSCAN_H_LABEL_DE "Horizontalen Overscan zuschneiden"
#define FCEUMM_OVERSCAN_H_INFO_0_DE "Entfernt 8 Pixel von der linken und rechten Seite des Bildschirms, um den Overscan zu simulieren, der auf Standard CRT-Fernsehern gesehen wird."
#define FCEUMM_OVERSCAN_V_LABEL_DE "Vertikalen Overscan zuschneiden"
#define FCEUMM_OVERSCAN_V_INFO_0_DE "Entfernt 8 Pixel von oben und unten auf dem Bildschirm, um den Overscan zu simulieren, der auf Standard CRT-Fernsehern gesehen wird."
#define FCEUMM_PALETTE_LABEL_DE "Farbpalette"
#define FCEUMM_PALETTE_INFO_0_DE "Wähle aus vorgenerierten Paletten, einer benutzerdefinierten 64x3 Palette aus einer Datei- oder Rohformat (benötigt nutzung eines Nes-Decoder Shader)."
#define OPTION_VAL_DEFAULT_DE "Standard"
#define OPTION_VAL_ASQREALC_DE NULL
#define OPTION_VAL_NINTENDO_VC_DE "Nintendo Virtuelle Konsole"
#define OPTION_VAL_RGB_DE NULL
#define OPTION_VAL_YUV_V3_DE NULL
#define OPTION_VAL_UNSATURATED_FINAL_DE "FBX's Ungesättigt-Final"
#define OPTION_VAL_SONY_CXA2025AS_US_DE NULL
#define OPTION_VAL_BMF_FINAL2_DE NULL
#define OPTION_VAL_BMF_FINAL3_DE NULL
#define OPTION_VAL_SMOOTH_FBX_DE "FBX's Glatt"
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_DE "FBX's Komposit Direkt"
#define OPTION_VAL_PVM_STYLE_D93_FBX_DE "FBX's PVM-Stil D93"
#define OPTION_VAL_NTSC_HARDWARE_FBX_DE NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_DE NULL
#define OPTION_VAL_NESCAP_DE NULL
#define OPTION_VAL_WAVEBEAM_DE "nakedarthur's Wellenstrahl"
#define OPTION_VAL_DIGITAL_PRIME_FBX_DE NULL
#define OPTION_VAL_MAGNUM_FBX_DE NULL
#define OPTION_VAL_SMOOTH_V2_FBX_DE NULL
#define OPTION_VAL_NES_CLASSIC_FBX_DE NULL
#define OPTION_VAL_RAW_DE "Roh"
#define OPTION_VAL_CUSTOM_DE "Benutzerdefiniert"
#define FCEUMM_NTSC_FILTER_LABEL_DE "NTSC-Filter"
#define FCEUMM_NTSC_FILTER_INFO_0_DE NULL
#define OPTION_VAL_COMPOSITE_DE NULL
#define OPTION_VAL_SVIDEO_DE NULL
#define OPTION_VAL_RGB_O9_DE NULL
#define OPTION_VAL_MONOCHROME_DE "Monochrom"
#define FCEUMM_SNDQUALITY_LABEL_DE "Tonqualität"
#define FCEUMM_SNDQUALITY_INFO_0_DE "Höhere Klangqualität aktivieren. Erhöht die Leistungsanforderungen."
#define OPTION_VAL_LOW_DE "Niedrig"
#define OPTION_VAL_HIGH_DE "Hoch"
#define OPTION_VAL_VERY_HIGH_DE "Sehr hoch"
#define FCEUMM_SNDLOWPASS_LABEL_DE NULL
#define FCEUMM_SNDLOWPASS_INFO_0_DE NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_DE NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_DE NULL
#define OPTION_VAL_01_MS_DELAY_DE NULL
#define OPTION_VAL_02_MS_DELAY_DE NULL
#define OPTION_VAL_03_MS_DELAY_DE NULL
#define OPTION_VAL_04_MS_DELAY_DE NULL
#define OPTION_VAL_05_MS_DELAY_DE NULL
#define OPTION_VAL_06_MS_DELAY_DE NULL
#define OPTION_VAL_07_MS_DELAY_DE NULL
#define OPTION_VAL_08_MS_DELAY_DE NULL
#define OPTION_VAL_09_MS_DELAY_DE NULL
#define OPTION_VAL_10_MS_DELAY_DE NULL
#define OPTION_VAL_11_MS_DELAY_DE NULL
#define OPTION_VAL_12_MS_DELAY_DE NULL
#define OPTION_VAL_13_MS_DELAY_DE NULL
#define OPTION_VAL_14_MS_DELAY_DE NULL
#define OPTION_VAL_15_MS_DELAY_DE NULL
#define OPTION_VAL_16_MS_DELAY_DE NULL
#define OPTION_VAL_17_MS_DELAY_DE NULL
#define OPTION_VAL_18_MS_DELAY_DE NULL
#define OPTION_VAL_19_MS_DELAY_DE NULL
#define OPTION_VAL_20_MS_DELAY_DE NULL
#define OPTION_VAL_21_MS_DELAY_DE NULL
#define OPTION_VAL_22_MS_DELAY_DE NULL
#define OPTION_VAL_23_MS_DELAY_DE NULL
#define OPTION_VAL_24_MS_DELAY_DE NULL
#define OPTION_VAL_25_MS_DELAY_DE NULL
#define OPTION_VAL_26_MS_DELAY_DE NULL
#define OPTION_VAL_27_MS_DELAY_DE NULL
#define OPTION_VAL_28_MS_DELAY_DE NULL
#define OPTION_VAL_29_MS_DELAY_DE NULL
#define OPTION_VAL_30_MS_DELAY_DE NULL
#define OPTION_VAL_31_MS_DELAY_DE NULL
#define OPTION_VAL_32_MS_DELAY_DE NULL
#define FCEUMM_SWAPDUTY_LABEL_DE "Audio Tastgrad tauschen"
#define FCEUMM_SWAPDUTY_LABEL_CAT_DE "Tastgrad tauschen"
#define FCEUMM_SWAPDUTY_INFO_0_DE "Simuliert den Klang von Famiclonen, die die Pulswellenkanäle umkehren."
#define FCEUMM_SNDVOLUME_LABEL_DE "Gesamtlautstärke"
#define FCEUMM_SNDVOLUME_INFO_0_DE "Gesamtlautstärkepegel ändern."
#define OPTION_VAL_0_DE "0 %"
#define OPTION_VAL_1_DE "10 %"
#define OPTION_VAL_2_DE "20 %"
#define OPTION_VAL_3_DE "30 %"
#define OPTION_VAL_4_DE "40 %"
#define OPTION_VAL_5_DE "50 %"
#define OPTION_VAL_6_DE "60 %"
#define OPTION_VAL_7_DE "70 %"
#define OPTION_VAL_8_DE "80 %"
#define OPTION_VAL_9_DE "90 %"
#define OPTION_VAL_10_DE "100 %"
#define FCEUMM_APU_1_LABEL_DE "Audiokanal 1 (Square 1)"
#define FCEUMM_APU_1_LABEL_CAT_DE "Kanal 1 (Square 1)"
#define FCEUMM_APU_1_INFO_0_DE "Aktiviert oder deaktiviert den Impulswellengenerator Audio-Ausgang 1."
#define FCEUMM_APU_2_LABEL_DE "Audiokanal 2 (Square 2)"
#define FCEUMM_APU_2_LABEL_CAT_DE "Kanal 2 (Square 2)"
#define FCEUMM_APU_2_INFO_0_DE "Aktiviert oder deaktiviert den Impulswellengenerator Audio-Ausgang 2."
#define FCEUMM_APU_3_LABEL_DE "Audio-Kanal 3 (Dreieck)"
#define FCEUMM_APU_3_LABEL_CAT_DE "Kanal 3 (Dreieck)"
#define FCEUMM_APU_3_INFO_0_DE "Aktiviert oder deaktiviert die Audioausgabe des Dreieck Wave Generators."
#define FCEUMM_APU_4_LABEL_DE "Audiokanal 4 (Noise)"
#define FCEUMM_APU_4_LABEL_CAT_DE "Kanal 4 (Noise)"
#define FCEUMM_APU_4_INFO_0_DE "Aktiviert oder deaktiviert die Rauschgenerator-Audioausgabe."
#define FCEUMM_APU_5_LABEL_DE "Audiokanal 5 (PCM)"
#define FCEUMM_APU_5_LABEL_CAT_DE "Kanal 5 (PCM)"
#define FCEUMM_APU_5_INFO_0_DE "Aktiviert oder deaktiviert die Audio-Ausgabe des Delta-Modulationskanals."
#define FCEUMM_TURBO_ENABLE_LABEL_DE "Turbo aktivieren"
#define FCEUMM_TURBO_ENABLE_INFO_0_DE "Turboknöpfe ein- oder ausschalten."
#define OPTION_VAL_NONE_DE "Keine"
#define OPTION_VAL_PLAYER_1_DE "Spieler 1"
#define OPTION_VAL_PLAYER_2_DE "Spieler 2"
#define OPTION_VAL_BOTH_DE "Beide"
#define FCEUMM_TURBO_DELAY_LABEL_DE "Turboverzögerung (in Frames)"
#define FCEUMM_TURBO_DELAY_INFO_0_DE "Wiederholungsrate von Turbo-Tasten in Bildern."
#define FCEUMM_ZAPPER_MODE_LABEL_DE "Zapper-Modus"
#define FCEUMM_ZAPPER_MODE_INFO_0_DE NULL
#define OPTION_VAL_CLIGHTGUN_DE NULL
#define OPTION_VAL_STLIGHTGUN_DE NULL
#define OPTION_VAL_TOUCHSCREEN_DE NULL
#define OPTION_VAL_MOUSE_DE "Maus"
#define FCEUMM_SHOW_CROSSHAIR_LABEL_DE "Zapper-Fadenkreuz anzeigen"
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_DE NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_DE "Zapper-Toleranz"
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_DE "Legt fest, wie viele Pixel außerhalb des Zielbereichs im Ziel liegen."
#define FCEUMM_ZAPPER_TRIGGER_LABEL_DE NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_DE NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_DE NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_DE NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_DE "Entgegengesetzte Richtungen zulassen"
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_DE "Erlaubt gleichzeitige OBEN+UNTEN oder LINKS+RECHTS-Tasten-Kombinationen, was in manchen Spielen unterschiedliche Effekte erzeugen kann."
#define FCEUMM_NOSPRITELIMIT_LABEL_DE "Kein Sprite-Limit"
#define FCEUMM_NOSPRITELIMIT_INFO_0_DE "Entfernt das Hardware-Limit von 8 Sprites pro Scanline. Dies reduziert Sprite-Flackern, kann aber bei einigen Spielen zu Fehlern führen, da manche dies für Effekte verwenden."
#define FCEUMM_OVERCLOCKING_LABEL_DE "Übertakten"
#define FCEUMM_OVERCLOCKING_INFO_0_DE "Aktiviert oder deaktiviert Übertaktung, was die Verlangsamung einiger Spiele reduzieren kann. Nachrendern-Methode ist kompatibel mit jedem Spiel, Abtastlücke ist effektiver für Spiele wie Contra Force."
#define OPTION_VAL_2X_POSTRENDER_DE "2x-Nachrendern"
#define OPTION_VAL_2X_VBLANK_DE "2x-Austastlücke"
#define FCEUMM_RAMSTATE_LABEL_DE "RAM füllen beim Einschalten (Neustart erforderlich)"
#define FCEUMM_RAMSTATE_INFO_0_DE "RAM-Werte beim Einschalten. Einige Spiele verlassen sich beispielsweise bei der Generierung von Zufallszahlen auf die anfänglichen RAM-Werte."
#define OPTION_VAL_FILL_FF_DE NULL
#define OPTION_VAL_FILL_00_DE NULL
#define OPTION_VAL_RANDOM_DE "Zufällig"

struct retro_core_option_v2_category option_cats_de[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_DE,
      CATEGORY_SYSTEM_INFO_0_DE
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_DE,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_DE
#else
      CATEGORY_VIDEO_INFO_1_DE
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_DE,
      CATEGORY_AUDIO_INFO_0_DE
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_DE,
      CATEGORY_INPUT_INFO_0_DE
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_DE,
      CATEGORY_HACKS_INFO_0_DE
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_DE,
      CATEGORY_DIP_SWITCH_INFO_0_DE
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_de[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_DE,
      NULL,
      FCEUMM_REGION_INFO_0_DE,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_DE },
         { "NTSC",  OPTION_VAL_NTSC_DE },
         { "PAL",   OPTION_VAL_PAL_DE },
         { "Dendy", OPTION_VAL_DENDY_DE },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_DE,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_DE,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_DE,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_DE,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_DE,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_DE,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_DE,
      NULL,
      FCEUMM_ASPECT_INFO_0_DE,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_DE },
         { "4:3",     OPTION_VAL_4_3_DE },
         { "PP",      OPTION_VAL_PP_DE },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_DE,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_DE,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_DE,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_DE,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_DE,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_DE,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_DE,
      NULL,
      FCEUMM_PALETTE_INFO_0_DE,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_DE },
         { "asqrealc",             OPTION_VAL_ASQREALC_DE },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_DE },
         { "rgb",                  OPTION_VAL_RGB_DE },
         { "yuv-v3",               OPTION_VAL_YUV_V3_DE },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_DE },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_DE },
         { "pal",                  OPTION_VAL_PAL_DE },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_DE },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_DE },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_DE },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_DE },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_DE },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_DE },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_DE },
         { "nescap",               OPTION_VAL_NESCAP_DE },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_DE },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_DE },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_DE },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_DE },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_DE },
         { "raw",                  OPTION_VAL_RAW_DE },
         { "custom",               OPTION_VAL_CUSTOM_DE },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_DE,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_DE,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_DE },
         { "svideo",     OPTION_VAL_SVIDEO_DE },
         { "rgb",        OPTION_VAL_RGB_O9_DE },
         { "monochrome", OPTION_VAL_MONOCHROME_DE },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_DE,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_DE,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_DE },
         { "High",      OPTION_VAL_HIGH_DE },
         { "Very High", OPTION_VAL_VERY_HIGH_DE },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_DE,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_DE,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_DE,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_DE,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_DE },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_DE },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_DE },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_DE },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_DE },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_DE },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_DE },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_DE },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_DE },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_DE },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_DE },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_DE },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_DE },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_DE },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_DE },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_DE },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_DE },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_DE },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_DE },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_DE },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_DE },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_DE },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_DE },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_DE },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_DE },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_DE },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_DE },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_DE },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_DE },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_DE },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_DE },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_DE },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_DE,
      FCEUMM_SWAPDUTY_LABEL_CAT_DE,
      FCEUMM_SWAPDUTY_INFO_0_DE,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_DE,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_DE,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_DE },
         { "1",  OPTION_VAL_1_DE },
         { "2",  OPTION_VAL_2_DE },
         { "3",  OPTION_VAL_3_DE },
         { "4",  OPTION_VAL_4_DE },
         { "5",  OPTION_VAL_5_DE },
         { "6",  OPTION_VAL_6_DE },
         { "7",  OPTION_VAL_7_DE },
         { "8",  OPTION_VAL_8_DE },
         { "9",  OPTION_VAL_9_DE },
         { "10", OPTION_VAL_10_DE },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_DE,
      FCEUMM_APU_1_LABEL_CAT_DE,
      FCEUMM_APU_1_INFO_0_DE,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_DE,
      FCEUMM_APU_2_LABEL_CAT_DE,
      FCEUMM_APU_2_INFO_0_DE,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_DE,
      FCEUMM_APU_3_LABEL_CAT_DE,
      FCEUMM_APU_3_INFO_0_DE,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_DE,
      FCEUMM_APU_4_LABEL_CAT_DE,
      FCEUMM_APU_4_INFO_0_DE,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_DE,
      FCEUMM_APU_5_LABEL_CAT_DE,
      FCEUMM_APU_5_INFO_0_DE,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_DE,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_DE,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_DE },
         { "Player 1", OPTION_VAL_PLAYER_1_DE },
         { "Player 2", OPTION_VAL_PLAYER_2_DE },
         { "Both",     OPTION_VAL_BOTH_DE },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_DE,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_DE,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_DE,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_DE,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_DE },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_DE },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_DE },
         { "mouse",       OPTION_VAL_MOUSE_DE },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_DE,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_DE,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_DE,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_DE,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_DE,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_DE,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_DE,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_DE,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_DE,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_DE,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_DE,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_DE,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_DE,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_DE,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_DE },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_DE },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_DE,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_DE,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_DE },
         { "fill $00", OPTION_VAL_FILL_00_DE },
         { "random",   OPTION_VAL_RANDOM_DE },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_de = {
   option_cats_de,
   option_defs_de
};

/* RETRO_LANGUAGE_EL */

#define CATEGORY_SYSTEM_LABEL_EL "Σύστημα"
#define CATEGORY_SYSTEM_INFO_0_EL NULL
#define CATEGORY_VIDEO_LABEL_EL "Οδηγός Βίντεο"
#define CATEGORY_VIDEO_INFO_0_EL NULL
#define CATEGORY_VIDEO_INFO_1_EL NULL
#define CATEGORY_AUDIO_LABEL_EL "Οδηγός Ήχου"
#define CATEGORY_AUDIO_INFO_0_EL NULL
#define CATEGORY_INPUT_LABEL_EL "Οδηγός Εισαγωγής"
#define CATEGORY_INPUT_INFO_0_EL NULL
#define CATEGORY_HACKS_LABEL_EL NULL
#define CATEGORY_HACKS_INFO_0_EL NULL
#define CATEGORY_DIP_SWITCH_LABEL_EL NULL
#define CATEGORY_DIP_SWITCH_INFO_0_EL NULL
#define FCEUMM_REGION_LABEL_EL "Περιοχή"
#define FCEUMM_REGION_INFO_0_EL NULL
#define OPTION_VAL_AUTO_EL "Αυτόματο"
#define OPTION_VAL_NTSC_EL NULL
#define OPTION_VAL_PAL_EL NULL
#define OPTION_VAL_DENDY_EL NULL
#define FCEUMM_GAME_GENIE_LABEL_EL NULL
#define FCEUMM_GAME_GENIE_INFO_0_EL NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_EL NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_EL NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_EL NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_EL NULL
#define FCEUMM_ASPECT_LABEL_EL "Αναλογία Οθόνης"
#define FCEUMM_ASPECT_INFO_0_EL NULL
#define OPTION_VAL_8_7_PAR_EL NULL
#define OPTION_VAL_4_3_EL NULL
#define OPTION_VAL_PP_EL NULL
#define FCEUMM_OVERSCAN_LABEL_EL NULL
#define FCEUMM_OVERSCAN_INFO_0_EL NULL
#define FCEUMM_OVERSCAN_H_LABEL_EL NULL
#define FCEUMM_OVERSCAN_H_INFO_0_EL NULL
#define FCEUMM_OVERSCAN_V_LABEL_EL NULL
#define FCEUMM_OVERSCAN_V_INFO_0_EL NULL
#define FCEUMM_PALETTE_LABEL_EL "Παλέτα Χρωμάτων"
#define FCEUMM_PALETTE_INFO_0_EL NULL
#define OPTION_VAL_DEFAULT_EL "Προκαθορισμένο"
#define OPTION_VAL_ASQREALC_EL NULL
#define OPTION_VAL_NINTENDO_VC_EL NULL
#define OPTION_VAL_RGB_EL NULL
#define OPTION_VAL_YUV_V3_EL NULL
#define OPTION_VAL_UNSATURATED_FINAL_EL NULL
#define OPTION_VAL_SONY_CXA2025AS_US_EL NULL
#define OPTION_VAL_BMF_FINAL2_EL NULL
#define OPTION_VAL_BMF_FINAL3_EL NULL
#define OPTION_VAL_SMOOTH_FBX_EL NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_EL NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_EL NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_EL NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_EL NULL
#define OPTION_VAL_NESCAP_EL NULL
#define OPTION_VAL_WAVEBEAM_EL NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_EL NULL
#define OPTION_VAL_MAGNUM_FBX_EL NULL
#define OPTION_VAL_SMOOTH_V2_FBX_EL NULL
#define OPTION_VAL_NES_CLASSIC_FBX_EL NULL
#define OPTION_VAL_RAW_EL NULL
#define OPTION_VAL_CUSTOM_EL NULL
#define FCEUMM_NTSC_FILTER_LABEL_EL NULL
#define FCEUMM_NTSC_FILTER_INFO_0_EL NULL
#define OPTION_VAL_COMPOSITE_EL NULL
#define OPTION_VAL_SVIDEO_EL NULL
#define OPTION_VAL_RGB_O9_EL NULL
#define OPTION_VAL_MONOCHROME_EL "Μονόχρωμο"
#define FCEUMM_SNDQUALITY_LABEL_EL NULL
#define FCEUMM_SNDQUALITY_INFO_0_EL NULL
#define OPTION_VAL_LOW_EL NULL
#define OPTION_VAL_HIGH_EL NULL
#define OPTION_VAL_VERY_HIGH_EL NULL
#define FCEUMM_SNDLOWPASS_LABEL_EL NULL
#define FCEUMM_SNDLOWPASS_INFO_0_EL NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_EL NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_EL NULL
#define OPTION_VAL_01_MS_DELAY_EL NULL
#define OPTION_VAL_02_MS_DELAY_EL NULL
#define OPTION_VAL_03_MS_DELAY_EL NULL
#define OPTION_VAL_04_MS_DELAY_EL NULL
#define OPTION_VAL_05_MS_DELAY_EL NULL
#define OPTION_VAL_06_MS_DELAY_EL NULL
#define OPTION_VAL_07_MS_DELAY_EL NULL
#define OPTION_VAL_08_MS_DELAY_EL NULL
#define OPTION_VAL_09_MS_DELAY_EL NULL
#define OPTION_VAL_10_MS_DELAY_EL NULL
#define OPTION_VAL_11_MS_DELAY_EL NULL
#define OPTION_VAL_12_MS_DELAY_EL NULL
#define OPTION_VAL_13_MS_DELAY_EL NULL
#define OPTION_VAL_14_MS_DELAY_EL NULL
#define OPTION_VAL_15_MS_DELAY_EL NULL
#define OPTION_VAL_16_MS_DELAY_EL NULL
#define OPTION_VAL_17_MS_DELAY_EL NULL
#define OPTION_VAL_18_MS_DELAY_EL NULL
#define OPTION_VAL_19_MS_DELAY_EL NULL
#define OPTION_VAL_20_MS_DELAY_EL NULL
#define OPTION_VAL_21_MS_DELAY_EL NULL
#define OPTION_VAL_22_MS_DELAY_EL NULL
#define OPTION_VAL_23_MS_DELAY_EL NULL
#define OPTION_VAL_24_MS_DELAY_EL NULL
#define OPTION_VAL_25_MS_DELAY_EL NULL
#define OPTION_VAL_26_MS_DELAY_EL NULL
#define OPTION_VAL_27_MS_DELAY_EL NULL
#define OPTION_VAL_28_MS_DELAY_EL NULL
#define OPTION_VAL_29_MS_DELAY_EL NULL
#define OPTION_VAL_30_MS_DELAY_EL NULL
#define OPTION_VAL_31_MS_DELAY_EL NULL
#define OPTION_VAL_32_MS_DELAY_EL NULL
#define FCEUMM_SWAPDUTY_LABEL_EL NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_EL NULL
#define FCEUMM_SWAPDUTY_INFO_0_EL NULL
#define FCEUMM_SNDVOLUME_LABEL_EL NULL
#define FCEUMM_SNDVOLUME_INFO_0_EL NULL
#define OPTION_VAL_0_EL NULL
#define OPTION_VAL_1_EL NULL
#define OPTION_VAL_2_EL NULL
#define OPTION_VAL_3_EL NULL
#define OPTION_VAL_4_EL NULL
#define OPTION_VAL_5_EL NULL
#define OPTION_VAL_6_EL NULL
#define OPTION_VAL_7_EL NULL
#define OPTION_VAL_8_EL "το 80%"
#define OPTION_VAL_9_EL NULL
#define OPTION_VAL_10_EL NULL
#define FCEUMM_APU_1_LABEL_EL NULL
#define FCEUMM_APU_1_LABEL_CAT_EL NULL
#define FCEUMM_APU_1_INFO_0_EL NULL
#define FCEUMM_APU_2_LABEL_EL NULL
#define FCEUMM_APU_2_LABEL_CAT_EL NULL
#define FCEUMM_APU_2_INFO_0_EL NULL
#define FCEUMM_APU_3_LABEL_EL NULL
#define FCEUMM_APU_3_LABEL_CAT_EL NULL
#define FCEUMM_APU_3_INFO_0_EL NULL
#define FCEUMM_APU_4_LABEL_EL NULL
#define FCEUMM_APU_4_LABEL_CAT_EL NULL
#define FCEUMM_APU_4_INFO_0_EL NULL
#define FCEUMM_APU_5_LABEL_EL NULL
#define FCEUMM_APU_5_LABEL_CAT_EL NULL
#define FCEUMM_APU_5_INFO_0_EL NULL
#define FCEUMM_TURBO_ENABLE_LABEL_EL "Ενεργοποίηση Turbo"
#define FCEUMM_TURBO_ENABLE_INFO_0_EL NULL
#define OPTION_VAL_NONE_EL "Κανείς"
#define OPTION_VAL_PLAYER_1_EL "Παίκτης 1"
#define OPTION_VAL_PLAYER_2_EL "Παίκτης 2"
#define OPTION_VAL_BOTH_EL NULL
#define FCEUMM_TURBO_DELAY_LABEL_EL NULL
#define FCEUMM_TURBO_DELAY_INFO_0_EL NULL
#define FCEUMM_ZAPPER_MODE_LABEL_EL NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_EL NULL
#define OPTION_VAL_CLIGHTGUN_EL NULL
#define OPTION_VAL_STLIGHTGUN_EL NULL
#define OPTION_VAL_TOUCHSCREEN_EL NULL
#define OPTION_VAL_MOUSE_EL NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_EL NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_EL NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_EL NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_EL NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_EL NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_EL NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_EL NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_EL NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_EL NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_EL NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_EL NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_EL NULL
#define FCEUMM_OVERCLOCKING_LABEL_EL NULL
#define FCEUMM_OVERCLOCKING_INFO_0_EL NULL
#define OPTION_VAL_2X_POSTRENDER_EL NULL
#define OPTION_VAL_2X_VBLANK_EL NULL
#define FCEUMM_RAMSTATE_LABEL_EL NULL
#define FCEUMM_RAMSTATE_INFO_0_EL NULL
#define OPTION_VAL_FILL_FF_EL NULL
#define OPTION_VAL_FILL_00_EL NULL
#define OPTION_VAL_RANDOM_EL NULL

struct retro_core_option_v2_category option_cats_el[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_EL,
      CATEGORY_SYSTEM_INFO_0_EL
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_EL,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_EL
#else
      CATEGORY_VIDEO_INFO_1_EL
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_EL,
      CATEGORY_AUDIO_INFO_0_EL
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_EL,
      CATEGORY_INPUT_INFO_0_EL
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_EL,
      CATEGORY_HACKS_INFO_0_EL
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_EL,
      CATEGORY_DIP_SWITCH_INFO_0_EL
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_el[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_EL,
      NULL,
      FCEUMM_REGION_INFO_0_EL,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_EL },
         { "NTSC",  OPTION_VAL_NTSC_EL },
         { "PAL",   OPTION_VAL_PAL_EL },
         { "Dendy", OPTION_VAL_DENDY_EL },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_EL,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_EL,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_EL,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_EL,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_EL,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_EL,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_EL,
      NULL,
      FCEUMM_ASPECT_INFO_0_EL,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_EL },
         { "4:3",     OPTION_VAL_4_3_EL },
         { "PP",      OPTION_VAL_PP_EL },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_EL,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_EL,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_EL,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_EL,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_EL,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_EL,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_EL,
      NULL,
      FCEUMM_PALETTE_INFO_0_EL,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_EL },
         { "asqrealc",             OPTION_VAL_ASQREALC_EL },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_EL },
         { "rgb",                  OPTION_VAL_RGB_EL },
         { "yuv-v3",               OPTION_VAL_YUV_V3_EL },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_EL },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_EL },
         { "pal",                  OPTION_VAL_PAL_EL },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_EL },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_EL },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_EL },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_EL },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_EL },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_EL },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_EL },
         { "nescap",               OPTION_VAL_NESCAP_EL },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_EL },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_EL },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_EL },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_EL },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_EL },
         { "raw",                  OPTION_VAL_RAW_EL },
         { "custom",               OPTION_VAL_CUSTOM_EL },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_EL,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_EL,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_EL },
         { "svideo",     OPTION_VAL_SVIDEO_EL },
         { "rgb",        OPTION_VAL_RGB_O9_EL },
         { "monochrome", OPTION_VAL_MONOCHROME_EL },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_EL,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_EL,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_EL },
         { "High",      OPTION_VAL_HIGH_EL },
         { "Very High", OPTION_VAL_VERY_HIGH_EL },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_EL,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_EL,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_EL,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_EL,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_EL },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_EL },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_EL },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_EL },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_EL },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_EL },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_EL },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_EL },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_EL },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_EL },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_EL },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_EL },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_EL },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_EL },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_EL },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_EL },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_EL },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_EL },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_EL },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_EL },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_EL },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_EL },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_EL },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_EL },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_EL },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_EL },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_EL },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_EL },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_EL },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_EL },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_EL },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_EL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_EL,
      FCEUMM_SWAPDUTY_LABEL_CAT_EL,
      FCEUMM_SWAPDUTY_INFO_0_EL,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_EL,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_EL,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_EL },
         { "1",  OPTION_VAL_1_EL },
         { "2",  OPTION_VAL_2_EL },
         { "3",  OPTION_VAL_3_EL },
         { "4",  OPTION_VAL_4_EL },
         { "5",  OPTION_VAL_5_EL },
         { "6",  OPTION_VAL_6_EL },
         { "7",  OPTION_VAL_7_EL },
         { "8",  OPTION_VAL_8_EL },
         { "9",  OPTION_VAL_9_EL },
         { "10", OPTION_VAL_10_EL },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_EL,
      FCEUMM_APU_1_LABEL_CAT_EL,
      FCEUMM_APU_1_INFO_0_EL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_EL,
      FCEUMM_APU_2_LABEL_CAT_EL,
      FCEUMM_APU_2_INFO_0_EL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_EL,
      FCEUMM_APU_3_LABEL_CAT_EL,
      FCEUMM_APU_3_INFO_0_EL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_EL,
      FCEUMM_APU_4_LABEL_CAT_EL,
      FCEUMM_APU_4_INFO_0_EL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_EL,
      FCEUMM_APU_5_LABEL_CAT_EL,
      FCEUMM_APU_5_INFO_0_EL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_EL,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_EL,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_EL },
         { "Player 1", OPTION_VAL_PLAYER_1_EL },
         { "Player 2", OPTION_VAL_PLAYER_2_EL },
         { "Both",     OPTION_VAL_BOTH_EL },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_EL,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_EL,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_EL,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_EL,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_EL },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_EL },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_EL },
         { "mouse",       OPTION_VAL_MOUSE_EL },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_EL,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_EL,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_EL,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_EL,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_EL,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_EL,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_EL,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_EL,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_EL,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_EL,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_EL,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_EL,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_EL,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_EL,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_EL },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_EL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_EL,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_EL,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_EL },
         { "fill $00", OPTION_VAL_FILL_00_EL },
         { "random",   OPTION_VAL_RANDOM_EL },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_el = {
   option_cats_el,
   option_defs_el
};

/* RETRO_LANGUAGE_EN */

#define CATEGORY_SYSTEM_LABEL_EN NULL
#define CATEGORY_SYSTEM_INFO_0_EN NULL
#define CATEGORY_VIDEO_LABEL_EN NULL
#define CATEGORY_VIDEO_INFO_0_EN NULL
#define CATEGORY_VIDEO_INFO_1_EN NULL
#define CATEGORY_AUDIO_LABEL_EN NULL
#define CATEGORY_AUDIO_INFO_0_EN NULL
#define CATEGORY_INPUT_LABEL_EN NULL
#define CATEGORY_INPUT_INFO_0_EN NULL
#define CATEGORY_HACKS_LABEL_EN NULL
#define CATEGORY_HACKS_INFO_0_EN "Change processor over-clocking and emulation accuracy settings affecting low-level performance and compatibility."
#define CATEGORY_DIP_SWITCH_LABEL_EN NULL
#define CATEGORY_DIP_SWITCH_INFO_0_EN NULL
#define FCEUMM_REGION_LABEL_EN NULL
#define FCEUMM_REGION_INFO_0_EN NULL
#define OPTION_VAL_AUTO_EN NULL
#define OPTION_VAL_NTSC_EN NULL
#define OPTION_VAL_PAL_EN NULL
#define OPTION_VAL_DENDY_EN NULL
#define FCEUMM_GAME_GENIE_LABEL_EN NULL
#define FCEUMM_GAME_GENIE_INFO_0_EN "Enable emulation of a Game Genie add-on cartridge, allowing cheat codes to be entered when launching games. The Game Genie ROM file 'gamegenie.nes' must be present in the front-end's system directory. Does not apply to FDS or arcade content."
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_EN NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_EN NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_EN NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_EN NULL
#define FCEUMM_ASPECT_LABEL_EN NULL
#define FCEUMM_ASPECT_INFO_0_EN NULL
#define OPTION_VAL_8_7_PAR_EN NULL
#define OPTION_VAL_4_3_EN NULL
#define OPTION_VAL_PP_EN NULL
#define FCEUMM_OVERSCAN_LABEL_EN NULL
#define FCEUMM_OVERSCAN_INFO_0_EN NULL
#define FCEUMM_OVERSCAN_H_LABEL_EN NULL
#define FCEUMM_OVERSCAN_H_INFO_0_EN NULL
#define FCEUMM_OVERSCAN_V_LABEL_EN NULL
#define FCEUMM_OVERSCAN_V_INFO_0_EN NULL
#define FCEUMM_PALETTE_LABEL_EN "Colour Palette"
#define FCEUMM_PALETTE_INFO_0_EN NULL
#define OPTION_VAL_DEFAULT_EN NULL
#define OPTION_VAL_ASQREALC_EN NULL
#define OPTION_VAL_NINTENDO_VC_EN NULL
#define OPTION_VAL_RGB_EN NULL
#define OPTION_VAL_YUV_V3_EN NULL
#define OPTION_VAL_UNSATURATED_FINAL_EN NULL
#define OPTION_VAL_SONY_CXA2025AS_US_EN NULL
#define OPTION_VAL_BMF_FINAL2_EN NULL
#define OPTION_VAL_BMF_FINAL3_EN NULL
#define OPTION_VAL_SMOOTH_FBX_EN NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_EN NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_EN NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_EN NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_EN NULL
#define OPTION_VAL_NESCAP_EN NULL
#define OPTION_VAL_WAVEBEAM_EN NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_EN NULL
#define OPTION_VAL_MAGNUM_FBX_EN NULL
#define OPTION_VAL_SMOOTH_V2_FBX_EN NULL
#define OPTION_VAL_NES_CLASSIC_FBX_EN NULL
#define OPTION_VAL_RAW_EN NULL
#define OPTION_VAL_CUSTOM_EN NULL
#define FCEUMM_NTSC_FILTER_LABEL_EN NULL
#define FCEUMM_NTSC_FILTER_INFO_0_EN NULL
#define OPTION_VAL_COMPOSITE_EN NULL
#define OPTION_VAL_SVIDEO_EN NULL
#define OPTION_VAL_RGB_O9_EN NULL
#define OPTION_VAL_MONOCHROME_EN NULL
#define FCEUMM_SNDQUALITY_LABEL_EN NULL
#define FCEUMM_SNDQUALITY_INFO_0_EN NULL
#define OPTION_VAL_LOW_EN NULL
#define OPTION_VAL_HIGH_EN NULL
#define OPTION_VAL_VERY_HIGH_EN NULL
#define FCEUMM_SNDLOWPASS_LABEL_EN NULL
#define FCEUMM_SNDLOWPASS_INFO_0_EN NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_EN NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_EN NULL
#define OPTION_VAL_01_MS_DELAY_EN NULL
#define OPTION_VAL_02_MS_DELAY_EN NULL
#define OPTION_VAL_03_MS_DELAY_EN NULL
#define OPTION_VAL_04_MS_DELAY_EN NULL
#define OPTION_VAL_05_MS_DELAY_EN NULL
#define OPTION_VAL_06_MS_DELAY_EN NULL
#define OPTION_VAL_07_MS_DELAY_EN NULL
#define OPTION_VAL_08_MS_DELAY_EN NULL
#define OPTION_VAL_09_MS_DELAY_EN NULL
#define OPTION_VAL_10_MS_DELAY_EN NULL
#define OPTION_VAL_11_MS_DELAY_EN NULL
#define OPTION_VAL_12_MS_DELAY_EN NULL
#define OPTION_VAL_13_MS_DELAY_EN NULL
#define OPTION_VAL_14_MS_DELAY_EN NULL
#define OPTION_VAL_15_MS_DELAY_EN NULL
#define OPTION_VAL_16_MS_DELAY_EN NULL
#define OPTION_VAL_17_MS_DELAY_EN NULL
#define OPTION_VAL_18_MS_DELAY_EN NULL
#define OPTION_VAL_19_MS_DELAY_EN NULL
#define OPTION_VAL_20_MS_DELAY_EN NULL
#define OPTION_VAL_21_MS_DELAY_EN NULL
#define OPTION_VAL_22_MS_DELAY_EN NULL
#define OPTION_VAL_23_MS_DELAY_EN NULL
#define OPTION_VAL_24_MS_DELAY_EN NULL
#define OPTION_VAL_25_MS_DELAY_EN NULL
#define OPTION_VAL_26_MS_DELAY_EN NULL
#define OPTION_VAL_27_MS_DELAY_EN NULL
#define OPTION_VAL_28_MS_DELAY_EN NULL
#define OPTION_VAL_29_MS_DELAY_EN NULL
#define OPTION_VAL_30_MS_DELAY_EN NULL
#define OPTION_VAL_31_MS_DELAY_EN NULL
#define OPTION_VAL_32_MS_DELAY_EN NULL
#define FCEUMM_SWAPDUTY_LABEL_EN NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_EN NULL
#define FCEUMM_SWAPDUTY_INFO_0_EN NULL
#define FCEUMM_SNDVOLUME_LABEL_EN NULL
#define FCEUMM_SNDVOLUME_INFO_0_EN NULL
#define OPTION_VAL_0_EN NULL
#define OPTION_VAL_1_EN NULL
#define OPTION_VAL_2_EN NULL
#define OPTION_VAL_3_EN NULL
#define OPTION_VAL_4_EN NULL
#define OPTION_VAL_5_EN NULL
#define OPTION_VAL_6_EN NULL
#define OPTION_VAL_7_EN NULL
#define OPTION_VAL_8_EN NULL
#define OPTION_VAL_9_EN NULL
#define OPTION_VAL_10_EN NULL
#define FCEUMM_APU_1_LABEL_EN NULL
#define FCEUMM_APU_1_LABEL_CAT_EN NULL
#define FCEUMM_APU_1_INFO_0_EN NULL
#define FCEUMM_APU_2_LABEL_EN NULL
#define FCEUMM_APU_2_LABEL_CAT_EN NULL
#define FCEUMM_APU_2_INFO_0_EN NULL
#define FCEUMM_APU_3_LABEL_EN NULL
#define FCEUMM_APU_3_LABEL_CAT_EN NULL
#define FCEUMM_APU_3_INFO_0_EN NULL
#define FCEUMM_APU_4_LABEL_EN NULL
#define FCEUMM_APU_4_LABEL_CAT_EN NULL
#define FCEUMM_APU_4_INFO_0_EN NULL
#define FCEUMM_APU_5_LABEL_EN NULL
#define FCEUMM_APU_5_LABEL_CAT_EN NULL
#define FCEUMM_APU_5_INFO_0_EN NULL
#define FCEUMM_TURBO_ENABLE_LABEL_EN NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_EN NULL
#define OPTION_VAL_NONE_EN NULL
#define OPTION_VAL_PLAYER_1_EN NULL
#define OPTION_VAL_PLAYER_2_EN NULL
#define OPTION_VAL_BOTH_EN NULL
#define FCEUMM_TURBO_DELAY_LABEL_EN NULL
#define FCEUMM_TURBO_DELAY_INFO_0_EN NULL
#define FCEUMM_ZAPPER_MODE_LABEL_EN NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_EN NULL
#define OPTION_VAL_CLIGHTGUN_EN NULL
#define OPTION_VAL_STLIGHTGUN_EN NULL
#define OPTION_VAL_TOUCHSCREEN_EN NULL
#define OPTION_VAL_MOUSE_EN NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_EN NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_EN NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_EN NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_EN NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_EN NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_EN NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_EN NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_EN NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_EN NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_EN NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_EN NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_EN "Removes the 8-per-scan-line hardware limit. This reduces sprite flickering but can cause some games to glitch since some use this for effects."
#define FCEUMM_OVERCLOCKING_LABEL_EN "Over-clock"
#define FCEUMM_OVERCLOCKING_INFO_0_EN "Enables or disables over-clocking, which can reduce slowdowns in some games. Post-render method is more compatible with every game, V-blank is more effective for games like Contra Force."
#define OPTION_VAL_2X_POSTRENDER_EN NULL
#define OPTION_VAL_2X_VBLANK_EN "2x-V-Blank"
#define FCEUMM_RAMSTATE_LABEL_EN NULL
#define FCEUMM_RAMSTATE_INFO_0_EN NULL
#define OPTION_VAL_FILL_FF_EN NULL
#define OPTION_VAL_FILL_00_EN NULL
#define OPTION_VAL_RANDOM_EN NULL

struct retro_core_option_v2_category option_cats_en[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_EN,
      CATEGORY_SYSTEM_INFO_0_EN
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_EN,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_EN
#else
      CATEGORY_VIDEO_INFO_1_EN
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_EN,
      CATEGORY_AUDIO_INFO_0_EN
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_EN,
      CATEGORY_INPUT_INFO_0_EN
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_EN,
      CATEGORY_HACKS_INFO_0_EN
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_EN,
      CATEGORY_DIP_SWITCH_INFO_0_EN
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_en[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_EN,
      NULL,
      FCEUMM_REGION_INFO_0_EN,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_EN },
         { "NTSC",  OPTION_VAL_NTSC_EN },
         { "PAL",   OPTION_VAL_PAL_EN },
         { "Dendy", OPTION_VAL_DENDY_EN },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_EN,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_EN,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_EN,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_EN,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_EN,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_EN,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_EN,
      NULL,
      FCEUMM_ASPECT_INFO_0_EN,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_EN },
         { "4:3",     OPTION_VAL_4_3_EN },
         { "PP",      OPTION_VAL_PP_EN },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_EN,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_EN,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_EN,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_EN,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_EN,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_EN,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_EN,
      NULL,
      FCEUMM_PALETTE_INFO_0_EN,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_EN },
         { "asqrealc",             OPTION_VAL_ASQREALC_EN },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_EN },
         { "rgb",                  OPTION_VAL_RGB_EN },
         { "yuv-v3",               OPTION_VAL_YUV_V3_EN },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_EN },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_EN },
         { "pal",                  OPTION_VAL_PAL_EN },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_EN },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_EN },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_EN },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_EN },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_EN },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_EN },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_EN },
         { "nescap",               OPTION_VAL_NESCAP_EN },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_EN },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_EN },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_EN },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_EN },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_EN },
         { "raw",                  OPTION_VAL_RAW_EN },
         { "custom",               OPTION_VAL_CUSTOM_EN },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_EN,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_EN,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_EN },
         { "svideo",     OPTION_VAL_SVIDEO_EN },
         { "rgb",        OPTION_VAL_RGB_O9_EN },
         { "monochrome", OPTION_VAL_MONOCHROME_EN },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_EN,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_EN,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_EN },
         { "High",      OPTION_VAL_HIGH_EN },
         { "Very High", OPTION_VAL_VERY_HIGH_EN },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_EN,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_EN,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_EN,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_EN,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_EN },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_EN },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_EN },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_EN },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_EN },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_EN },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_EN },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_EN },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_EN },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_EN },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_EN },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_EN },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_EN },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_EN },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_EN },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_EN },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_EN },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_EN },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_EN },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_EN },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_EN },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_EN },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_EN },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_EN },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_EN },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_EN },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_EN },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_EN },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_EN },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_EN },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_EN },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_EN },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_EN,
      FCEUMM_SWAPDUTY_LABEL_CAT_EN,
      FCEUMM_SWAPDUTY_INFO_0_EN,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_EN,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_EN,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_EN },
         { "1",  OPTION_VAL_1_EN },
         { "2",  OPTION_VAL_2_EN },
         { "3",  OPTION_VAL_3_EN },
         { "4",  OPTION_VAL_4_EN },
         { "5",  OPTION_VAL_5_EN },
         { "6",  OPTION_VAL_6_EN },
         { "7",  OPTION_VAL_7_EN },
         { "8",  OPTION_VAL_8_EN },
         { "9",  OPTION_VAL_9_EN },
         { "10", OPTION_VAL_10_EN },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_EN,
      FCEUMM_APU_1_LABEL_CAT_EN,
      FCEUMM_APU_1_INFO_0_EN,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_EN,
      FCEUMM_APU_2_LABEL_CAT_EN,
      FCEUMM_APU_2_INFO_0_EN,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_EN,
      FCEUMM_APU_3_LABEL_CAT_EN,
      FCEUMM_APU_3_INFO_0_EN,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_EN,
      FCEUMM_APU_4_LABEL_CAT_EN,
      FCEUMM_APU_4_INFO_0_EN,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_EN,
      FCEUMM_APU_5_LABEL_CAT_EN,
      FCEUMM_APU_5_INFO_0_EN,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_EN,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_EN,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_EN },
         { "Player 1", OPTION_VAL_PLAYER_1_EN },
         { "Player 2", OPTION_VAL_PLAYER_2_EN },
         { "Both",     OPTION_VAL_BOTH_EN },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_EN,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_EN,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_EN,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_EN,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_EN },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_EN },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_EN },
         { "mouse",       OPTION_VAL_MOUSE_EN },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_EN,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_EN,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_EN,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_EN,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_EN,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_EN,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_EN,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_EN,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_EN,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_EN,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_EN,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_EN,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_EN,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_EN,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_EN },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_EN },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_EN,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_EN,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_EN },
         { "fill $00", OPTION_VAL_FILL_00_EN },
         { "random",   OPTION_VAL_RANDOM_EN },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_en = {
   option_cats_en,
   option_defs_en
};

/* RETRO_LANGUAGE_EO */

#define CATEGORY_SYSTEM_LABEL_EO NULL
#define CATEGORY_SYSTEM_INFO_0_EO NULL
#define CATEGORY_VIDEO_LABEL_EO "Video Driver"
#define CATEGORY_VIDEO_INFO_0_EO NULL
#define CATEGORY_VIDEO_INFO_1_EO NULL
#define CATEGORY_AUDIO_LABEL_EO "Audio Driver"
#define CATEGORY_AUDIO_INFO_0_EO NULL
#define CATEGORY_INPUT_LABEL_EO "Input Driver"
#define CATEGORY_INPUT_INFO_0_EO NULL
#define CATEGORY_HACKS_LABEL_EO NULL
#define CATEGORY_HACKS_INFO_0_EO NULL
#define CATEGORY_DIP_SWITCH_LABEL_EO NULL
#define CATEGORY_DIP_SWITCH_INFO_0_EO NULL
#define FCEUMM_REGION_LABEL_EO NULL
#define FCEUMM_REGION_INFO_0_EO NULL
#define OPTION_VAL_AUTO_EO NULL
#define OPTION_VAL_NTSC_EO NULL
#define OPTION_VAL_PAL_EO NULL
#define OPTION_VAL_DENDY_EO NULL
#define FCEUMM_GAME_GENIE_LABEL_EO NULL
#define FCEUMM_GAME_GENIE_INFO_0_EO NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_EO NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_EO NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_EO NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_EO NULL
#define FCEUMM_ASPECT_LABEL_EO NULL
#define FCEUMM_ASPECT_INFO_0_EO NULL
#define OPTION_VAL_8_7_PAR_EO NULL
#define OPTION_VAL_4_3_EO NULL
#define OPTION_VAL_PP_EO NULL
#define FCEUMM_OVERSCAN_LABEL_EO NULL
#define FCEUMM_OVERSCAN_INFO_0_EO NULL
#define FCEUMM_OVERSCAN_H_LABEL_EO NULL
#define FCEUMM_OVERSCAN_H_INFO_0_EO NULL
#define FCEUMM_OVERSCAN_V_LABEL_EO NULL
#define FCEUMM_OVERSCAN_V_INFO_0_EO NULL
#define FCEUMM_PALETTE_LABEL_EO NULL
#define FCEUMM_PALETTE_INFO_0_EO NULL
#define OPTION_VAL_DEFAULT_EO NULL
#define OPTION_VAL_ASQREALC_EO NULL
#define OPTION_VAL_NINTENDO_VC_EO NULL
#define OPTION_VAL_RGB_EO NULL
#define OPTION_VAL_YUV_V3_EO NULL
#define OPTION_VAL_UNSATURATED_FINAL_EO NULL
#define OPTION_VAL_SONY_CXA2025AS_US_EO NULL
#define OPTION_VAL_BMF_FINAL2_EO NULL
#define OPTION_VAL_BMF_FINAL3_EO NULL
#define OPTION_VAL_SMOOTH_FBX_EO NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_EO NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_EO NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_EO NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_EO NULL
#define OPTION_VAL_NESCAP_EO NULL
#define OPTION_VAL_WAVEBEAM_EO NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_EO NULL
#define OPTION_VAL_MAGNUM_FBX_EO NULL
#define OPTION_VAL_SMOOTH_V2_FBX_EO NULL
#define OPTION_VAL_NES_CLASSIC_FBX_EO NULL
#define OPTION_VAL_RAW_EO NULL
#define OPTION_VAL_CUSTOM_EO NULL
#define FCEUMM_NTSC_FILTER_LABEL_EO NULL
#define FCEUMM_NTSC_FILTER_INFO_0_EO NULL
#define OPTION_VAL_COMPOSITE_EO NULL
#define OPTION_VAL_SVIDEO_EO NULL
#define OPTION_VAL_RGB_O9_EO NULL
#define OPTION_VAL_MONOCHROME_EO NULL
#define FCEUMM_SNDQUALITY_LABEL_EO NULL
#define FCEUMM_SNDQUALITY_INFO_0_EO NULL
#define OPTION_VAL_LOW_EO NULL
#define OPTION_VAL_HIGH_EO NULL
#define OPTION_VAL_VERY_HIGH_EO NULL
#define FCEUMM_SNDLOWPASS_LABEL_EO NULL
#define FCEUMM_SNDLOWPASS_INFO_0_EO NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_EO NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_EO NULL
#define OPTION_VAL_01_MS_DELAY_EO NULL
#define OPTION_VAL_02_MS_DELAY_EO NULL
#define OPTION_VAL_03_MS_DELAY_EO NULL
#define OPTION_VAL_04_MS_DELAY_EO NULL
#define OPTION_VAL_05_MS_DELAY_EO NULL
#define OPTION_VAL_06_MS_DELAY_EO NULL
#define OPTION_VAL_07_MS_DELAY_EO NULL
#define OPTION_VAL_08_MS_DELAY_EO NULL
#define OPTION_VAL_09_MS_DELAY_EO NULL
#define OPTION_VAL_10_MS_DELAY_EO NULL
#define OPTION_VAL_11_MS_DELAY_EO NULL
#define OPTION_VAL_12_MS_DELAY_EO NULL
#define OPTION_VAL_13_MS_DELAY_EO NULL
#define OPTION_VAL_14_MS_DELAY_EO NULL
#define OPTION_VAL_15_MS_DELAY_EO NULL
#define OPTION_VAL_16_MS_DELAY_EO NULL
#define OPTION_VAL_17_MS_DELAY_EO NULL
#define OPTION_VAL_18_MS_DELAY_EO NULL
#define OPTION_VAL_19_MS_DELAY_EO NULL
#define OPTION_VAL_20_MS_DELAY_EO NULL
#define OPTION_VAL_21_MS_DELAY_EO NULL
#define OPTION_VAL_22_MS_DELAY_EO NULL
#define OPTION_VAL_23_MS_DELAY_EO NULL
#define OPTION_VAL_24_MS_DELAY_EO NULL
#define OPTION_VAL_25_MS_DELAY_EO NULL
#define OPTION_VAL_26_MS_DELAY_EO NULL
#define OPTION_VAL_27_MS_DELAY_EO NULL
#define OPTION_VAL_28_MS_DELAY_EO NULL
#define OPTION_VAL_29_MS_DELAY_EO NULL
#define OPTION_VAL_30_MS_DELAY_EO NULL
#define OPTION_VAL_31_MS_DELAY_EO NULL
#define OPTION_VAL_32_MS_DELAY_EO NULL
#define FCEUMM_SWAPDUTY_LABEL_EO NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_EO NULL
#define FCEUMM_SWAPDUTY_INFO_0_EO NULL
#define FCEUMM_SNDVOLUME_LABEL_EO NULL
#define FCEUMM_SNDVOLUME_INFO_0_EO NULL
#define OPTION_VAL_0_EO NULL
#define OPTION_VAL_1_EO NULL
#define OPTION_VAL_2_EO NULL
#define OPTION_VAL_3_EO NULL
#define OPTION_VAL_4_EO NULL
#define OPTION_VAL_5_EO NULL
#define OPTION_VAL_6_EO NULL
#define OPTION_VAL_7_EO NULL
#define OPTION_VAL_8_EO NULL
#define OPTION_VAL_9_EO NULL
#define OPTION_VAL_10_EO NULL
#define FCEUMM_APU_1_LABEL_EO NULL
#define FCEUMM_APU_1_LABEL_CAT_EO NULL
#define FCEUMM_APU_1_INFO_0_EO NULL
#define FCEUMM_APU_2_LABEL_EO NULL
#define FCEUMM_APU_2_LABEL_CAT_EO NULL
#define FCEUMM_APU_2_INFO_0_EO NULL
#define FCEUMM_APU_3_LABEL_EO NULL
#define FCEUMM_APU_3_LABEL_CAT_EO NULL
#define FCEUMM_APU_3_INFO_0_EO NULL
#define FCEUMM_APU_4_LABEL_EO NULL
#define FCEUMM_APU_4_LABEL_CAT_EO NULL
#define FCEUMM_APU_4_INFO_0_EO NULL
#define FCEUMM_APU_5_LABEL_EO NULL
#define FCEUMM_APU_5_LABEL_CAT_EO NULL
#define FCEUMM_APU_5_INFO_0_EO NULL
#define FCEUMM_TURBO_ENABLE_LABEL_EO NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_EO NULL
#define OPTION_VAL_NONE_EO NULL
#define OPTION_VAL_PLAYER_1_EO NULL
#define OPTION_VAL_PLAYER_2_EO NULL
#define OPTION_VAL_BOTH_EO NULL
#define FCEUMM_TURBO_DELAY_LABEL_EO NULL
#define FCEUMM_TURBO_DELAY_INFO_0_EO NULL
#define FCEUMM_ZAPPER_MODE_LABEL_EO NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_EO NULL
#define OPTION_VAL_CLIGHTGUN_EO NULL
#define OPTION_VAL_STLIGHTGUN_EO NULL
#define OPTION_VAL_TOUCHSCREEN_EO NULL
#define OPTION_VAL_MOUSE_EO NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_EO NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_EO NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_EO NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_EO NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_EO NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_EO NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_EO NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_EO NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_EO NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_EO NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_EO NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_EO NULL
#define FCEUMM_OVERCLOCKING_LABEL_EO NULL
#define FCEUMM_OVERCLOCKING_INFO_0_EO NULL
#define OPTION_VAL_2X_POSTRENDER_EO NULL
#define OPTION_VAL_2X_VBLANK_EO NULL
#define FCEUMM_RAMSTATE_LABEL_EO NULL
#define FCEUMM_RAMSTATE_INFO_0_EO NULL
#define OPTION_VAL_FILL_FF_EO NULL
#define OPTION_VAL_FILL_00_EO NULL
#define OPTION_VAL_RANDOM_EO NULL

struct retro_core_option_v2_category option_cats_eo[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_EO,
      CATEGORY_SYSTEM_INFO_0_EO
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_EO,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_EO
#else
      CATEGORY_VIDEO_INFO_1_EO
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_EO,
      CATEGORY_AUDIO_INFO_0_EO
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_EO,
      CATEGORY_INPUT_INFO_0_EO
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_EO,
      CATEGORY_HACKS_INFO_0_EO
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_EO,
      CATEGORY_DIP_SWITCH_INFO_0_EO
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_eo[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_EO,
      NULL,
      FCEUMM_REGION_INFO_0_EO,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_EO },
         { "NTSC",  OPTION_VAL_NTSC_EO },
         { "PAL",   OPTION_VAL_PAL_EO },
         { "Dendy", OPTION_VAL_DENDY_EO },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_EO,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_EO,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_EO,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_EO,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_EO,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_EO,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_EO,
      NULL,
      FCEUMM_ASPECT_INFO_0_EO,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_EO },
         { "4:3",     OPTION_VAL_4_3_EO },
         { "PP",      OPTION_VAL_PP_EO },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_EO,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_EO,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_EO,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_EO,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_EO,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_EO,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_EO,
      NULL,
      FCEUMM_PALETTE_INFO_0_EO,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_EO },
         { "asqrealc",             OPTION_VAL_ASQREALC_EO },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_EO },
         { "rgb",                  OPTION_VAL_RGB_EO },
         { "yuv-v3",               OPTION_VAL_YUV_V3_EO },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_EO },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_EO },
         { "pal",                  OPTION_VAL_PAL_EO },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_EO },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_EO },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_EO },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_EO },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_EO },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_EO },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_EO },
         { "nescap",               OPTION_VAL_NESCAP_EO },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_EO },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_EO },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_EO },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_EO },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_EO },
         { "raw",                  OPTION_VAL_RAW_EO },
         { "custom",               OPTION_VAL_CUSTOM_EO },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_EO,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_EO,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_EO },
         { "svideo",     OPTION_VAL_SVIDEO_EO },
         { "rgb",        OPTION_VAL_RGB_O9_EO },
         { "monochrome", OPTION_VAL_MONOCHROME_EO },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_EO,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_EO,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_EO },
         { "High",      OPTION_VAL_HIGH_EO },
         { "Very High", OPTION_VAL_VERY_HIGH_EO },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_EO,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_EO,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_EO,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_EO,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_EO },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_EO },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_EO },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_EO },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_EO },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_EO },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_EO },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_EO },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_EO },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_EO },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_EO },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_EO },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_EO },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_EO },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_EO },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_EO },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_EO },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_EO },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_EO },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_EO },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_EO },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_EO },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_EO },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_EO },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_EO },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_EO },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_EO },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_EO },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_EO },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_EO },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_EO },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_EO },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_EO,
      FCEUMM_SWAPDUTY_LABEL_CAT_EO,
      FCEUMM_SWAPDUTY_INFO_0_EO,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_EO,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_EO,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_EO },
         { "1",  OPTION_VAL_1_EO },
         { "2",  OPTION_VAL_2_EO },
         { "3",  OPTION_VAL_3_EO },
         { "4",  OPTION_VAL_4_EO },
         { "5",  OPTION_VAL_5_EO },
         { "6",  OPTION_VAL_6_EO },
         { "7",  OPTION_VAL_7_EO },
         { "8",  OPTION_VAL_8_EO },
         { "9",  OPTION_VAL_9_EO },
         { "10", OPTION_VAL_10_EO },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_EO,
      FCEUMM_APU_1_LABEL_CAT_EO,
      FCEUMM_APU_1_INFO_0_EO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_EO,
      FCEUMM_APU_2_LABEL_CAT_EO,
      FCEUMM_APU_2_INFO_0_EO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_EO,
      FCEUMM_APU_3_LABEL_CAT_EO,
      FCEUMM_APU_3_INFO_0_EO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_EO,
      FCEUMM_APU_4_LABEL_CAT_EO,
      FCEUMM_APU_4_INFO_0_EO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_EO,
      FCEUMM_APU_5_LABEL_CAT_EO,
      FCEUMM_APU_5_INFO_0_EO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_EO,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_EO,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_EO },
         { "Player 1", OPTION_VAL_PLAYER_1_EO },
         { "Player 2", OPTION_VAL_PLAYER_2_EO },
         { "Both",     OPTION_VAL_BOTH_EO },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_EO,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_EO,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_EO,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_EO,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_EO },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_EO },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_EO },
         { "mouse",       OPTION_VAL_MOUSE_EO },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_EO,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_EO,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_EO,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_EO,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_EO,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_EO,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_EO,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_EO,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_EO,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_EO,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_EO,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_EO,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_EO,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_EO,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_EO },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_EO },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_EO,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_EO,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_EO },
         { "fill $00", OPTION_VAL_FILL_00_EO },
         { "random",   OPTION_VAL_RANDOM_EO },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_eo = {
   option_cats_eo,
   option_defs_eo
};

/* RETRO_LANGUAGE_ES */

#define CATEGORY_SYSTEM_LABEL_ES "Sistema"
#define CATEGORY_SYSTEM_INFO_0_ES NULL
#define CATEGORY_VIDEO_LABEL_ES "Vídeo"
#define CATEGORY_VIDEO_INFO_0_ES NULL
#define CATEGORY_VIDEO_INFO_1_ES NULL
#define CATEGORY_AUDIO_LABEL_ES NULL
#define CATEGORY_AUDIO_INFO_0_ES NULL
#define CATEGORY_INPUT_LABEL_ES "Entrada"
#define CATEGORY_INPUT_INFO_0_ES NULL
#define CATEGORY_HACKS_LABEL_ES "Arreglos de emulación"
#define CATEGORY_HACKS_INFO_0_ES "Cambia las opciones de velocidad del procesador y de precisión de la emulación que afectan al rendimiento a bajo nivel y a la compatibilidad."
#define CATEGORY_DIP_SWITCH_LABEL_ES "Interruptores DIP"
#define CATEGORY_DIP_SWITCH_INFO_0_ES NULL
#define FCEUMM_REGION_LABEL_ES "Región"
#define FCEUMM_REGION_INFO_0_ES "Fuerza al núcleo a utilizar los intervalos de las regiones NTSC, PAL o de la consola Dendy."
#define OPTION_VAL_AUTO_ES "Selección automática"
#define OPTION_VAL_NTSC_ES NULL
#define OPTION_VAL_PAL_ES NULL
#define OPTION_VAL_DENDY_ES NULL
#define FCEUMM_GAME_GENIE_LABEL_ES "Módulo Game Genie (es necesario reiniciar)"
#define FCEUMM_GAME_GENIE_INFO_0_ES "Habilita la emulación de un cartucho de ampliación Game Genie para poder introducir códigos de trucos al ejecutar juegos. Es necesario que esté presente el archivo ROM de Game Genie, gamegenie.nes, en el directorio de sistema del front-end. No se aplicará a los contenidos arcade o de FDS."
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_ES "Mostrar opciones avanzadas del sistema"
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_ES "Muestra los ajustes avanzados del sistema."
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_ES "Mostrar opciones avanzadas de sonido"
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_ES "Muestra los ajustes avanzados del sonido."
#define FCEUMM_ASPECT_LABEL_ES "Relación de aspecto"
#define FCEUMM_ASPECT_INFO_0_ES "Selecciona la relación de aspecto."
#define OPTION_VAL_8_7_PAR_ES "Relación de aspecto de píxeles (PAR) 8:7"
#define OPTION_VAL_4_3_ES NULL
#define OPTION_VAL_PP_ES "Píxeles perfectos"
#define FCEUMM_OVERSCAN_LABEL_ES "Recortar sobrebarrido"
#define FCEUMM_OVERSCAN_INFO_0_ES "Elimina los 8 píxeles del sobrebarrido en todos los lados de la pantalla."
#define FCEUMM_OVERSCAN_H_LABEL_ES "Recortar sobrebarrido horizontal"
#define FCEUMM_OVERSCAN_H_INFO_0_ES "Elimina 8 píxeles en los laterales izquierdo y derecho de la pantalla para simular el sobrebarrido que no mostraría un monitor de tubo catódico (CRT) estándar."
#define FCEUMM_OVERSCAN_V_LABEL_ES "Recortar sobrebarrido vertical"
#define FCEUMM_OVERSCAN_V_INFO_0_ES "Elimina 8 píxeles de las partes superior e inferior de la pantalla para simular el sobrebarrido que no mostraría un monitor de tubo catódico (CRT) estándar."
#define FCEUMM_PALETTE_LABEL_ES "Paleta de colores"
#define FCEUMM_PALETTE_INFO_0_ES "Selecciona una paleta de entre una lista de paletas pregeneradas, una paleta 64 × 3 personalizada de un archivo o un formato en bruto (necesita un shader del tipo nes-decoder)."
#define OPTION_VAL_DEFAULT_ES "Valor predeterminado"
#define OPTION_VAL_ASQREALC_ES "Real de AspiringSquire"
#define OPTION_VAL_NINTENDO_VC_ES "Consola virtual de Nintendo"
#define OPTION_VAL_RGB_ES "PPU RGB de Nintendo"
#define OPTION_VAL_YUV_V3_ES "YUV-V3 de FBX"
#define OPTION_VAL_UNSATURATED_FINAL_ES "Final sin saturación de FBX"
#define OPTION_VAL_SONY_CXA2025AS_US_ES "Sony CXA2025AS de EE. UU."
#define OPTION_VAL_BMF_FINAL2_ES "Final 2 de BMF"
#define OPTION_VAL_BMF_FINAL3_ES "Final 3 de BMF"
#define OPTION_VAL_SMOOTH_FBX_ES "Suavizada de FBX"
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_ES "Señal directa de vídeo compuesto de FBX"
#define OPTION_VAL_PVM_STYLE_D93_FBX_ES "PVM estilo D93 de FBX"
#define OPTION_VAL_NTSC_HARDWARE_FBX_ES "Hardware NTSC de FBX"
#define OPTION_VAL_NES_CLASSIC_FBX_FS_ES NULL
#define OPTION_VAL_NESCAP_ES "NESCAP de RGBSource"
#define OPTION_VAL_WAVEBEAM_ES "Wavebeam de nakedarthur"
#define OPTION_VAL_DIGITAL_PRIME_FBX_ES NULL
#define OPTION_VAL_MAGNUM_FBX_ES NULL
#define OPTION_VAL_SMOOTH_V2_FBX_ES NULL
#define OPTION_VAL_NES_CLASSIC_FBX_ES NULL
#define OPTION_VAL_RAW_ES "Datos en bruto"
#define OPTION_VAL_CUSTOM_ES "Personalizar"
#define FCEUMM_NTSC_FILTER_LABEL_ES "Filtro tipo NTSC"
#define FCEUMM_NTSC_FILTER_INFO_0_ES NULL
#define OPTION_VAL_COMPOSITE_ES "Vídeo compuesto"
#define OPTION_VAL_SVIDEO_ES NULL
#define OPTION_VAL_RGB_O9_ES NULL
#define OPTION_VAL_MONOCHROME_ES "Monocromo"
#define FCEUMM_SNDQUALITY_LABEL_ES "Calidad del sonido"
#define FCEUMM_SNDQUALITY_INFO_0_ES "Activa un sonido de alta calidad, aumentando los requisitos de rendimiento."
#define OPTION_VAL_LOW_ES "Baja"
#define OPTION_VAL_HIGH_ES "Alta"
#define OPTION_VAL_VERY_HIGH_ES "Muy alta"
#define FCEUMM_SNDLOWPASS_LABEL_ES NULL
#define FCEUMM_SNDLOWPASS_INFO_0_ES NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_ES NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_ES NULL
#define OPTION_VAL_01_MS_DELAY_ES NULL
#define OPTION_VAL_02_MS_DELAY_ES NULL
#define OPTION_VAL_03_MS_DELAY_ES NULL
#define OPTION_VAL_04_MS_DELAY_ES NULL
#define OPTION_VAL_05_MS_DELAY_ES NULL
#define OPTION_VAL_06_MS_DELAY_ES NULL
#define OPTION_VAL_07_MS_DELAY_ES NULL
#define OPTION_VAL_08_MS_DELAY_ES NULL
#define OPTION_VAL_09_MS_DELAY_ES NULL
#define OPTION_VAL_10_MS_DELAY_ES NULL
#define OPTION_VAL_11_MS_DELAY_ES NULL
#define OPTION_VAL_12_MS_DELAY_ES NULL
#define OPTION_VAL_13_MS_DELAY_ES NULL
#define OPTION_VAL_14_MS_DELAY_ES NULL
#define OPTION_VAL_15_MS_DELAY_ES NULL
#define OPTION_VAL_16_MS_DELAY_ES NULL
#define OPTION_VAL_17_MS_DELAY_ES NULL
#define OPTION_VAL_18_MS_DELAY_ES NULL
#define OPTION_VAL_19_MS_DELAY_ES NULL
#define OPTION_VAL_20_MS_DELAY_ES NULL
#define OPTION_VAL_21_MS_DELAY_ES NULL
#define OPTION_VAL_22_MS_DELAY_ES NULL
#define OPTION_VAL_23_MS_DELAY_ES NULL
#define OPTION_VAL_24_MS_DELAY_ES NULL
#define OPTION_VAL_25_MS_DELAY_ES NULL
#define OPTION_VAL_26_MS_DELAY_ES NULL
#define OPTION_VAL_27_MS_DELAY_ES NULL
#define OPTION_VAL_28_MS_DELAY_ES NULL
#define OPTION_VAL_29_MS_DELAY_ES NULL
#define OPTION_VAL_30_MS_DELAY_ES NULL
#define OPTION_VAL_31_MS_DELAY_ES NULL
#define OPTION_VAL_32_MS_DELAY_ES NULL
#define FCEUMM_SWAPDUTY_LABEL_ES "Intercambiar los ciclos de trabajo del audio"
#define FCEUMM_SWAPDUTY_LABEL_CAT_ES "Intercambiar los ciclos de trabajo"
#define FCEUMM_SWAPDUTY_INFO_0_ES "Simula el sonido de los famiclones que tienen invertidos los bits de los ciclos de trabajo de los canales de ondas de pulso."
#define FCEUMM_SNDVOLUME_LABEL_ES "Volumen general"
#define FCEUMM_SNDVOLUME_INFO_0_ES "Cambia el nivel de volumen general."
#define OPTION_VAL_0_ES "0 %"
#define OPTION_VAL_1_ES "10 %"
#define OPTION_VAL_2_ES "20 %"
#define OPTION_VAL_3_ES "30 %"
#define OPTION_VAL_4_ES "40 %"
#define OPTION_VAL_5_ES "50 %"
#define OPTION_VAL_6_ES "60 %"
#define OPTION_VAL_7_ES "70 %"
#define OPTION_VAL_8_ES "80 %"
#define OPTION_VAL_9_ES "90 %"
#define OPTION_VAL_10_ES "100 %"
#define FCEUMM_APU_1_LABEL_ES "Canal de audio 1 (cuadradas 1)"
#define FCEUMM_APU_1_LABEL_CAT_ES "Canal 1 (cuadradas 1)"
#define FCEUMM_APU_1_INFO_0_ES "Activa o desactiva la salida de audio del generador de ondas de pulso 1."
#define FCEUMM_APU_2_LABEL_ES "Canal de audio 2 (cuadradas 2)"
#define FCEUMM_APU_2_LABEL_CAT_ES "Canal 2 (cuadradas 2)"
#define FCEUMM_APU_2_INFO_0_ES "Activa o desactiva la salida de audio del generador de ondas de pulso 2."
#define FCEUMM_APU_3_LABEL_ES "Canal de audio 1 (triangulares)"
#define FCEUMM_APU_3_LABEL_CAT_ES "Canal 3 (triangulares)"
#define FCEUMM_APU_3_INFO_0_ES "Activa o desactiva la salida de audio del generador de ondas triangulares."
#define FCEUMM_APU_4_LABEL_ES "Canal de audio 4 (ruido)"
#define FCEUMM_APU_4_LABEL_CAT_ES "Canal 4 (ruido)"
#define FCEUMM_APU_4_INFO_0_ES "Activa o desactiva la salida de audio del generador de ruido."
#define FCEUMM_APU_5_LABEL_ES "Canal de audio 5 (PCM)"
#define FCEUMM_APU_5_LABEL_CAT_ES "Canal 5 (PCM)"
#define FCEUMM_APU_5_INFO_0_ES "Activa o desactiva la salida de audio del canal de modulación delta."
#define FCEUMM_TURBO_ENABLE_LABEL_ES "Botones turbo"
#define FCEUMM_TURBO_ENABLE_INFO_0_ES "Activa o desactiva los botones con turbo."
#define OPTION_VAL_NONE_ES "Ninguno"
#define OPTION_VAL_PLAYER_1_ES "Jugador 1"
#define OPTION_VAL_PLAYER_2_ES "Jugador 2"
#define OPTION_VAL_BOTH_ES "Ambos"
#define FCEUMM_TURBO_DELAY_LABEL_ES "Retraso del turbo (en fotogramas)"
#define FCEUMM_TURBO_DELAY_INFO_0_ES "Indica la velocidad de repetición de los botones con turbo en fotogramas."
#define FCEUMM_ZAPPER_MODE_LABEL_ES "Modo Zapper"
#define FCEUMM_ZAPPER_MODE_INFO_0_ES NULL
#define OPTION_VAL_CLIGHTGUN_ES NULL
#define OPTION_VAL_STLIGHTGUN_ES NULL
#define OPTION_VAL_TOUCHSCREEN_ES "Pantalla táctil"
#define OPTION_VAL_MOUSE_ES "Ratón"
#define FCEUMM_SHOW_CROSSHAIR_LABEL_ES "Mostrar punto de mira de la Zapper"
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_ES NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_ES "Tolerancia de la Zapper"
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_ES "Establece la cantidad de píxeles fuera de un objetivo que se consideran como parte del mismo."
#define FCEUMM_ZAPPER_TRIGGER_LABEL_ES NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_ES NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_ES NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_ES NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_ES "Permitir direcciones opuestas"
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_ES "Permite pulsar a la vez las combinaciones de botones Arriba+Abajo o Izquierda+Derecha, que pueden provocar todo tipo de efectos en algunos juegos."
#define FCEUMM_NOSPRITELIMIT_LABEL_ES "Desactivar límite de sprites"
#define FCEUMM_NOSPRITELIMIT_INFO_0_ES "Elimina el límite del hardware original de 8 sprites por línea de barrido. Reduce el parpadeo de los sprites, pero algunos juegos pueden mostrar defectos gráficos, ya que aprovechan este límite para mostrar ciertos efectos."
#define FCEUMM_OVERCLOCKING_LABEL_ES "Velocidad de procesamiento"
#define FCEUMM_OVERCLOCKING_INFO_0_ES "Aumenta o disminuye la velocidad del procesador de la consola, lo que podría reducir las ralentizaciones en algunos juegos. El método posrenderizado es más compatible con todos los juegos y el método de intervalos verticales es más eficaz para juegos como Contra Force."
#define OPTION_VAL_2X_POSTRENDER_ES "2x (posrenderizado)"
#define OPTION_VAL_2X_VBLANK_ES "2x (intervalos verticales)"
#define FCEUMM_RAMSTATE_LABEL_ES "Llenado de la RAM al arrancar (es necesario reiniciar)"
#define FCEUMM_RAMSTATE_INFO_0_ES "Indica los valores que tendrá la RAM al arrancar. Algunos juegos necesitan un valor concreto en la RAM, por ejemplo, para generar números aleatorios."
#define OPTION_VAL_FILL_FF_ES NULL
#define OPTION_VAL_FILL_00_ES NULL
#define OPTION_VAL_RANDOM_ES "Al azar"

struct retro_core_option_v2_category option_cats_es[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_ES,
      CATEGORY_SYSTEM_INFO_0_ES
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_ES,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_ES
#else
      CATEGORY_VIDEO_INFO_1_ES
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_ES,
      CATEGORY_AUDIO_INFO_0_ES
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_ES,
      CATEGORY_INPUT_INFO_0_ES
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_ES,
      CATEGORY_HACKS_INFO_0_ES
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_ES,
      CATEGORY_DIP_SWITCH_INFO_0_ES
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_es[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_ES,
      NULL,
      FCEUMM_REGION_INFO_0_ES,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_ES },
         { "NTSC",  OPTION_VAL_NTSC_ES },
         { "PAL",   OPTION_VAL_PAL_ES },
         { "Dendy", OPTION_VAL_DENDY_ES },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_ES,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_ES,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_ES,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_ES,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_ES,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_ES,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_ES,
      NULL,
      FCEUMM_ASPECT_INFO_0_ES,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_ES },
         { "4:3",     OPTION_VAL_4_3_ES },
         { "PP",      OPTION_VAL_PP_ES },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_ES,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_ES,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_ES,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_ES,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_ES,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_ES,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_ES,
      NULL,
      FCEUMM_PALETTE_INFO_0_ES,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_ES },
         { "asqrealc",             OPTION_VAL_ASQREALC_ES },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_ES },
         { "rgb",                  OPTION_VAL_RGB_ES },
         { "yuv-v3",               OPTION_VAL_YUV_V3_ES },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_ES },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_ES },
         { "pal",                  OPTION_VAL_PAL_ES },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_ES },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_ES },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_ES },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_ES },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_ES },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_ES },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_ES },
         { "nescap",               OPTION_VAL_NESCAP_ES },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_ES },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_ES },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_ES },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_ES },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_ES },
         { "raw",                  OPTION_VAL_RAW_ES },
         { "custom",               OPTION_VAL_CUSTOM_ES },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_ES,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_ES,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_ES },
         { "svideo",     OPTION_VAL_SVIDEO_ES },
         { "rgb",        OPTION_VAL_RGB_O9_ES },
         { "monochrome", OPTION_VAL_MONOCHROME_ES },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_ES,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_ES,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_ES },
         { "High",      OPTION_VAL_HIGH_ES },
         { "Very High", OPTION_VAL_VERY_HIGH_ES },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_ES,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_ES,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_ES,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_ES,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_ES },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_ES },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_ES },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_ES },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_ES },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_ES },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_ES },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_ES },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_ES },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_ES },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_ES },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_ES },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_ES },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_ES },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_ES },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_ES },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_ES },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_ES },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_ES },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_ES },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_ES },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_ES },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_ES },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_ES },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_ES },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_ES },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_ES },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_ES },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_ES },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_ES },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_ES },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_ES },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_ES,
      FCEUMM_SWAPDUTY_LABEL_CAT_ES,
      FCEUMM_SWAPDUTY_INFO_0_ES,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_ES,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_ES,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_ES },
         { "1",  OPTION_VAL_1_ES },
         { "2",  OPTION_VAL_2_ES },
         { "3",  OPTION_VAL_3_ES },
         { "4",  OPTION_VAL_4_ES },
         { "5",  OPTION_VAL_5_ES },
         { "6",  OPTION_VAL_6_ES },
         { "7",  OPTION_VAL_7_ES },
         { "8",  OPTION_VAL_8_ES },
         { "9",  OPTION_VAL_9_ES },
         { "10", OPTION_VAL_10_ES },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_ES,
      FCEUMM_APU_1_LABEL_CAT_ES,
      FCEUMM_APU_1_INFO_0_ES,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_ES,
      FCEUMM_APU_2_LABEL_CAT_ES,
      FCEUMM_APU_2_INFO_0_ES,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_ES,
      FCEUMM_APU_3_LABEL_CAT_ES,
      FCEUMM_APU_3_INFO_0_ES,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_ES,
      FCEUMM_APU_4_LABEL_CAT_ES,
      FCEUMM_APU_4_INFO_0_ES,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_ES,
      FCEUMM_APU_5_LABEL_CAT_ES,
      FCEUMM_APU_5_INFO_0_ES,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_ES,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_ES,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_ES },
         { "Player 1", OPTION_VAL_PLAYER_1_ES },
         { "Player 2", OPTION_VAL_PLAYER_2_ES },
         { "Both",     OPTION_VAL_BOTH_ES },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_ES,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_ES,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_ES,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_ES,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_ES },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_ES },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_ES },
         { "mouse",       OPTION_VAL_MOUSE_ES },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_ES,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_ES,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_ES,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_ES,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_ES,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_ES,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_ES,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_ES,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_ES,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_ES,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_ES,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_ES,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_ES,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_ES,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_ES },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_ES },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_ES,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_ES,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_ES },
         { "fill $00", OPTION_VAL_FILL_00_ES },
         { "random",   OPTION_VAL_RANDOM_ES },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_es = {
   option_cats_es,
   option_defs_es
};

/* RETRO_LANGUAGE_FA */

#define CATEGORY_SYSTEM_LABEL_FA "سیستم"
#define CATEGORY_SYSTEM_INFO_0_FA NULL
#define CATEGORY_VIDEO_LABEL_FA "ویدیو"
#define CATEGORY_VIDEO_INFO_0_FA NULL
#define CATEGORY_VIDEO_INFO_1_FA NULL
#define CATEGORY_AUDIO_LABEL_FA "صدا"
#define CATEGORY_AUDIO_INFO_0_FA NULL
#define CATEGORY_INPUT_LABEL_FA "داده"
#define CATEGORY_INPUT_INFO_0_FA NULL
#define CATEGORY_HACKS_LABEL_FA NULL
#define CATEGORY_HACKS_INFO_0_FA NULL
#define CATEGORY_DIP_SWITCH_LABEL_FA NULL
#define CATEGORY_DIP_SWITCH_INFO_0_FA NULL
#define FCEUMM_REGION_LABEL_FA "منطقه"
#define FCEUMM_REGION_INFO_0_FA NULL
#define OPTION_VAL_AUTO_FA "خودکار"
#define OPTION_VAL_NTSC_FA NULL
#define OPTION_VAL_PAL_FA NULL
#define OPTION_VAL_DENDY_FA NULL
#define FCEUMM_GAME_GENIE_LABEL_FA NULL
#define FCEUMM_GAME_GENIE_INFO_0_FA NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_FA NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_FA NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_FA NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_FA NULL
#define FCEUMM_ASPECT_LABEL_FA "نسبت تصویر"
#define FCEUMM_ASPECT_INFO_0_FA NULL
#define OPTION_VAL_8_7_PAR_FA NULL
#define OPTION_VAL_4_3_FA NULL
#define OPTION_VAL_PP_FA NULL
#define FCEUMM_OVERSCAN_LABEL_FA NULL
#define FCEUMM_OVERSCAN_INFO_0_FA NULL
#define FCEUMM_OVERSCAN_H_LABEL_FA NULL
#define FCEUMM_OVERSCAN_H_INFO_0_FA NULL
#define FCEUMM_OVERSCAN_V_LABEL_FA NULL
#define FCEUMM_OVERSCAN_V_INFO_0_FA NULL
#define FCEUMM_PALETTE_LABEL_FA NULL
#define FCEUMM_PALETTE_INFO_0_FA NULL
#define OPTION_VAL_DEFAULT_FA NULL
#define OPTION_VAL_ASQREALC_FA NULL
#define OPTION_VAL_NINTENDO_VC_FA NULL
#define OPTION_VAL_RGB_FA NULL
#define OPTION_VAL_YUV_V3_FA NULL
#define OPTION_VAL_UNSATURATED_FINAL_FA NULL
#define OPTION_VAL_SONY_CXA2025AS_US_FA NULL
#define OPTION_VAL_BMF_FINAL2_FA NULL
#define OPTION_VAL_BMF_FINAL3_FA NULL
#define OPTION_VAL_SMOOTH_FBX_FA NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_FA NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_FA NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_FA NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_FA NULL
#define OPTION_VAL_NESCAP_FA NULL
#define OPTION_VAL_WAVEBEAM_FA NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_FA NULL
#define OPTION_VAL_MAGNUM_FBX_FA NULL
#define OPTION_VAL_SMOOTH_V2_FBX_FA NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FA NULL
#define OPTION_VAL_RAW_FA NULL
#define OPTION_VAL_CUSTOM_FA "سفارشی"
#define FCEUMM_NTSC_FILTER_LABEL_FA NULL
#define FCEUMM_NTSC_FILTER_INFO_0_FA NULL
#define OPTION_VAL_COMPOSITE_FA NULL
#define OPTION_VAL_SVIDEO_FA NULL
#define OPTION_VAL_RGB_O9_FA NULL
#define OPTION_VAL_MONOCHROME_FA NULL
#define FCEUMM_SNDQUALITY_LABEL_FA NULL
#define FCEUMM_SNDQUALITY_INFO_0_FA NULL
#define OPTION_VAL_LOW_FA NULL
#define OPTION_VAL_HIGH_FA NULL
#define OPTION_VAL_VERY_HIGH_FA NULL
#define FCEUMM_SNDLOWPASS_LABEL_FA NULL
#define FCEUMM_SNDLOWPASS_INFO_0_FA NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_FA NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_FA NULL
#define OPTION_VAL_01_MS_DELAY_FA NULL
#define OPTION_VAL_02_MS_DELAY_FA NULL
#define OPTION_VAL_03_MS_DELAY_FA NULL
#define OPTION_VAL_04_MS_DELAY_FA NULL
#define OPTION_VAL_05_MS_DELAY_FA NULL
#define OPTION_VAL_06_MS_DELAY_FA NULL
#define OPTION_VAL_07_MS_DELAY_FA NULL
#define OPTION_VAL_08_MS_DELAY_FA NULL
#define OPTION_VAL_09_MS_DELAY_FA NULL
#define OPTION_VAL_10_MS_DELAY_FA NULL
#define OPTION_VAL_11_MS_DELAY_FA NULL
#define OPTION_VAL_12_MS_DELAY_FA NULL
#define OPTION_VAL_13_MS_DELAY_FA NULL
#define OPTION_VAL_14_MS_DELAY_FA NULL
#define OPTION_VAL_15_MS_DELAY_FA NULL
#define OPTION_VAL_16_MS_DELAY_FA NULL
#define OPTION_VAL_17_MS_DELAY_FA NULL
#define OPTION_VAL_18_MS_DELAY_FA NULL
#define OPTION_VAL_19_MS_DELAY_FA NULL
#define OPTION_VAL_20_MS_DELAY_FA NULL
#define OPTION_VAL_21_MS_DELAY_FA NULL
#define OPTION_VAL_22_MS_DELAY_FA NULL
#define OPTION_VAL_23_MS_DELAY_FA NULL
#define OPTION_VAL_24_MS_DELAY_FA NULL
#define OPTION_VAL_25_MS_DELAY_FA NULL
#define OPTION_VAL_26_MS_DELAY_FA NULL
#define OPTION_VAL_27_MS_DELAY_FA NULL
#define OPTION_VAL_28_MS_DELAY_FA NULL
#define OPTION_VAL_29_MS_DELAY_FA NULL
#define OPTION_VAL_30_MS_DELAY_FA NULL
#define OPTION_VAL_31_MS_DELAY_FA NULL
#define OPTION_VAL_32_MS_DELAY_FA NULL
#define FCEUMM_SWAPDUTY_LABEL_FA NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_FA NULL
#define FCEUMM_SWAPDUTY_INFO_0_FA NULL
#define FCEUMM_SNDVOLUME_LABEL_FA NULL
#define FCEUMM_SNDVOLUME_INFO_0_FA NULL
#define OPTION_VAL_0_FA NULL
#define OPTION_VAL_1_FA NULL
#define OPTION_VAL_2_FA NULL
#define OPTION_VAL_3_FA NULL
#define OPTION_VAL_4_FA NULL
#define OPTION_VAL_5_FA NULL
#define OPTION_VAL_6_FA NULL
#define OPTION_VAL_7_FA NULL
#define OPTION_VAL_8_FA NULL
#define OPTION_VAL_9_FA NULL
#define OPTION_VAL_10_FA NULL
#define FCEUMM_APU_1_LABEL_FA NULL
#define FCEUMM_APU_1_LABEL_CAT_FA NULL
#define FCEUMM_APU_1_INFO_0_FA NULL
#define FCEUMM_APU_2_LABEL_FA NULL
#define FCEUMM_APU_2_LABEL_CAT_FA NULL
#define FCEUMM_APU_2_INFO_0_FA NULL
#define FCEUMM_APU_3_LABEL_FA NULL
#define FCEUMM_APU_3_LABEL_CAT_FA NULL
#define FCEUMM_APU_3_INFO_0_FA NULL
#define FCEUMM_APU_4_LABEL_FA NULL
#define FCEUMM_APU_4_LABEL_CAT_FA NULL
#define FCEUMM_APU_4_INFO_0_FA NULL
#define FCEUMM_APU_5_LABEL_FA NULL
#define FCEUMM_APU_5_LABEL_CAT_FA NULL
#define FCEUMM_APU_5_INFO_0_FA NULL
#define FCEUMM_TURBO_ENABLE_LABEL_FA NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_FA NULL
#define OPTION_VAL_NONE_FA "هیچکدام"
#define OPTION_VAL_PLAYER_1_FA NULL
#define OPTION_VAL_PLAYER_2_FA NULL
#define OPTION_VAL_BOTH_FA NULL
#define FCEUMM_TURBO_DELAY_LABEL_FA NULL
#define FCEUMM_TURBO_DELAY_INFO_0_FA NULL
#define FCEUMM_ZAPPER_MODE_LABEL_FA NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_FA NULL
#define OPTION_VAL_CLIGHTGUN_FA NULL
#define OPTION_VAL_STLIGHTGUN_FA NULL
#define OPTION_VAL_TOUCHSCREEN_FA NULL
#define OPTION_VAL_MOUSE_FA NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_FA NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_FA NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_FA NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_FA NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_FA NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_FA NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_FA NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_FA NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_FA NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_FA NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_FA NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_FA NULL
#define FCEUMM_OVERCLOCKING_LABEL_FA NULL
#define FCEUMM_OVERCLOCKING_INFO_0_FA NULL
#define OPTION_VAL_2X_POSTRENDER_FA NULL
#define OPTION_VAL_2X_VBLANK_FA NULL
#define FCEUMM_RAMSTATE_LABEL_FA NULL
#define FCEUMM_RAMSTATE_INFO_0_FA NULL
#define OPTION_VAL_FILL_FF_FA NULL
#define OPTION_VAL_FILL_00_FA NULL
#define OPTION_VAL_RANDOM_FA NULL

struct retro_core_option_v2_category option_cats_fa[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_FA,
      CATEGORY_SYSTEM_INFO_0_FA
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_FA,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_FA
#else
      CATEGORY_VIDEO_INFO_1_FA
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_FA,
      CATEGORY_AUDIO_INFO_0_FA
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_FA,
      CATEGORY_INPUT_INFO_0_FA
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_FA,
      CATEGORY_HACKS_INFO_0_FA
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_FA,
      CATEGORY_DIP_SWITCH_INFO_0_FA
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_fa[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_FA,
      NULL,
      FCEUMM_REGION_INFO_0_FA,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_FA },
         { "NTSC",  OPTION_VAL_NTSC_FA },
         { "PAL",   OPTION_VAL_PAL_FA },
         { "Dendy", OPTION_VAL_DENDY_FA },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_FA,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_FA,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_FA,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_FA,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_FA,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_FA,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_FA,
      NULL,
      FCEUMM_ASPECT_INFO_0_FA,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_FA },
         { "4:3",     OPTION_VAL_4_3_FA },
         { "PP",      OPTION_VAL_PP_FA },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_FA,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_FA,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_FA,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_FA,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_FA,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_FA,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_FA,
      NULL,
      FCEUMM_PALETTE_INFO_0_FA,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_FA },
         { "asqrealc",             OPTION_VAL_ASQREALC_FA },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_FA },
         { "rgb",                  OPTION_VAL_RGB_FA },
         { "yuv-v3",               OPTION_VAL_YUV_V3_FA },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_FA },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_FA },
         { "pal",                  OPTION_VAL_PAL_FA },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_FA },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_FA },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_FA },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_FA },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_FA },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_FA },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_FA },
         { "nescap",               OPTION_VAL_NESCAP_FA },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_FA },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_FA },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_FA },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_FA },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_FA },
         { "raw",                  OPTION_VAL_RAW_FA },
         { "custom",               OPTION_VAL_CUSTOM_FA },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_FA,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_FA,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_FA },
         { "svideo",     OPTION_VAL_SVIDEO_FA },
         { "rgb",        OPTION_VAL_RGB_O9_FA },
         { "monochrome", OPTION_VAL_MONOCHROME_FA },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_FA,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_FA,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_FA },
         { "High",      OPTION_VAL_HIGH_FA },
         { "Very High", OPTION_VAL_VERY_HIGH_FA },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_FA,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_FA,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_FA,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_FA,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_FA },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_FA },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_FA },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_FA },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_FA },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_FA },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_FA },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_FA },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_FA },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_FA },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_FA },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_FA },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_FA },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_FA },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_FA },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_FA },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_FA },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_FA },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_FA },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_FA },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_FA },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_FA },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_FA },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_FA },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_FA },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_FA },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_FA },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_FA },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_FA },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_FA },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_FA },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_FA },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_FA,
      FCEUMM_SWAPDUTY_LABEL_CAT_FA,
      FCEUMM_SWAPDUTY_INFO_0_FA,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_FA,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_FA,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_FA },
         { "1",  OPTION_VAL_1_FA },
         { "2",  OPTION_VAL_2_FA },
         { "3",  OPTION_VAL_3_FA },
         { "4",  OPTION_VAL_4_FA },
         { "5",  OPTION_VAL_5_FA },
         { "6",  OPTION_VAL_6_FA },
         { "7",  OPTION_VAL_7_FA },
         { "8",  OPTION_VAL_8_FA },
         { "9",  OPTION_VAL_9_FA },
         { "10", OPTION_VAL_10_FA },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_FA,
      FCEUMM_APU_1_LABEL_CAT_FA,
      FCEUMM_APU_1_INFO_0_FA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_FA,
      FCEUMM_APU_2_LABEL_CAT_FA,
      FCEUMM_APU_2_INFO_0_FA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_FA,
      FCEUMM_APU_3_LABEL_CAT_FA,
      FCEUMM_APU_3_INFO_0_FA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_FA,
      FCEUMM_APU_4_LABEL_CAT_FA,
      FCEUMM_APU_4_INFO_0_FA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_FA,
      FCEUMM_APU_5_LABEL_CAT_FA,
      FCEUMM_APU_5_INFO_0_FA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_FA,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_FA,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_FA },
         { "Player 1", OPTION_VAL_PLAYER_1_FA },
         { "Player 2", OPTION_VAL_PLAYER_2_FA },
         { "Both",     OPTION_VAL_BOTH_FA },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_FA,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_FA,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_FA,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_FA,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_FA },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_FA },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_FA },
         { "mouse",       OPTION_VAL_MOUSE_FA },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_FA,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_FA,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_FA,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_FA,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_FA,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_FA,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_FA,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_FA,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_FA,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_FA,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_FA,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_FA,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_FA,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_FA,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_FA },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_FA },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_FA,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_FA,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_FA },
         { "fill $00", OPTION_VAL_FILL_00_FA },
         { "random",   OPTION_VAL_RANDOM_FA },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_fa = {
   option_cats_fa,
   option_defs_fa
};

/* RETRO_LANGUAGE_FI */

#define CATEGORY_SYSTEM_LABEL_FI "Järjestelmä"
#define CATEGORY_SYSTEM_INFO_0_FI NULL
#define CATEGORY_VIDEO_LABEL_FI NULL
#define CATEGORY_VIDEO_INFO_0_FI NULL
#define CATEGORY_VIDEO_INFO_1_FI NULL
#define CATEGORY_AUDIO_LABEL_FI "Ääni"
#define CATEGORY_AUDIO_INFO_0_FI NULL
#define CATEGORY_INPUT_LABEL_FI "Syöte"
#define CATEGORY_INPUT_INFO_0_FI NULL
#define CATEGORY_HACKS_LABEL_FI "Emulointikikat"
#define CATEGORY_HACKS_INFO_0_FI "Muuta prosessorin ylikellotus- ja emulointitarkkuusasetuksia, jotka vaikuttavat matalan tason suorituskykyyn ja yhteensopivuuteen."
#define CATEGORY_DIP_SWITCH_LABEL_FI NULL
#define CATEGORY_DIP_SWITCH_INFO_0_FI NULL
#define FCEUMM_REGION_LABEL_FI "Alue"
#define FCEUMM_REGION_INFO_0_FI "Pakota ydin käyttämään NTSC-, PAL- tai Dendy-alueen ajoituksia."
#define OPTION_VAL_AUTO_FI "Automaattinen"
#define OPTION_VAL_NTSC_FI NULL
#define OPTION_VAL_PAL_FI NULL
#define OPTION_VAL_DENDY_FI NULL
#define FCEUMM_GAME_GENIE_LABEL_FI "Game Genie lisäosa (Uudelleenkäynnistys vaaditaan)"
#define FCEUMM_GAME_GENIE_INFO_0_FI NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_FI NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_FI NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_FI NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_FI NULL
#define FCEUMM_ASPECT_LABEL_FI "Kuvasuhde"
#define FCEUMM_ASPECT_INFO_0_FI "Valitse ensisijainen kuvasuhde."
#define OPTION_VAL_8_7_PAR_FI NULL
#define OPTION_VAL_4_3_FI NULL
#define OPTION_VAL_PP_FI NULL
#define FCEUMM_OVERSCAN_LABEL_FI NULL
#define FCEUMM_OVERSCAN_INFO_0_FI NULL
#define FCEUMM_OVERSCAN_H_LABEL_FI NULL
#define FCEUMM_OVERSCAN_H_INFO_0_FI NULL
#define FCEUMM_OVERSCAN_V_LABEL_FI NULL
#define FCEUMM_OVERSCAN_V_INFO_0_FI NULL
#define FCEUMM_PALETTE_LABEL_FI "Väripaletti"
#define FCEUMM_PALETTE_INFO_0_FI NULL
#define OPTION_VAL_DEFAULT_FI "Oletus"
#define OPTION_VAL_ASQREALC_FI NULL
#define OPTION_VAL_NINTENDO_VC_FI NULL
#define OPTION_VAL_RGB_FI NULL
#define OPTION_VAL_YUV_V3_FI NULL
#define OPTION_VAL_UNSATURATED_FINAL_FI NULL
#define OPTION_VAL_SONY_CXA2025AS_US_FI NULL
#define OPTION_VAL_BMF_FINAL2_FI NULL
#define OPTION_VAL_BMF_FINAL3_FI NULL
#define OPTION_VAL_SMOOTH_FBX_FI NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_FI NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_FI NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_FI "FBX:n NTSC laitteisto"
#define OPTION_VAL_NES_CLASSIC_FBX_FS_FI NULL
#define OPTION_VAL_NESCAP_FI NULL
#define OPTION_VAL_WAVEBEAM_FI NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_FI NULL
#define OPTION_VAL_MAGNUM_FBX_FI NULL
#define OPTION_VAL_SMOOTH_V2_FBX_FI NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FI NULL
#define OPTION_VAL_RAW_FI "Tavallinen"
#define OPTION_VAL_CUSTOM_FI "Mukautettu"
#define FCEUMM_NTSC_FILTER_LABEL_FI "NTSC-suodatin"
#define FCEUMM_NTSC_FILTER_INFO_0_FI NULL
#define OPTION_VAL_COMPOSITE_FI "Komposiitti"
#define OPTION_VAL_SVIDEO_FI NULL
#define OPTION_VAL_RGB_O9_FI NULL
#define OPTION_VAL_MONOCHROME_FI "Yksivärinen"
#define FCEUMM_SNDQUALITY_LABEL_FI "Äänenlaatu"
#define FCEUMM_SNDQUALITY_INFO_0_FI "Ota käyttöön laadukkaampi ääni. Lisää suorituskykyä koskevia vaatimuksia."
#define OPTION_VAL_LOW_FI "Matala"
#define OPTION_VAL_HIGH_FI "Korkea"
#define OPTION_VAL_VERY_HIGH_FI "Erittäin korkea"
#define FCEUMM_SNDLOWPASS_LABEL_FI NULL
#define FCEUMM_SNDLOWPASS_INFO_0_FI NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_FI NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_FI NULL
#define OPTION_VAL_01_MS_DELAY_FI NULL
#define OPTION_VAL_02_MS_DELAY_FI NULL
#define OPTION_VAL_03_MS_DELAY_FI NULL
#define OPTION_VAL_04_MS_DELAY_FI NULL
#define OPTION_VAL_05_MS_DELAY_FI NULL
#define OPTION_VAL_06_MS_DELAY_FI NULL
#define OPTION_VAL_07_MS_DELAY_FI NULL
#define OPTION_VAL_08_MS_DELAY_FI NULL
#define OPTION_VAL_09_MS_DELAY_FI NULL
#define OPTION_VAL_10_MS_DELAY_FI NULL
#define OPTION_VAL_11_MS_DELAY_FI NULL
#define OPTION_VAL_12_MS_DELAY_FI NULL
#define OPTION_VAL_13_MS_DELAY_FI NULL
#define OPTION_VAL_14_MS_DELAY_FI NULL
#define OPTION_VAL_15_MS_DELAY_FI NULL
#define OPTION_VAL_16_MS_DELAY_FI NULL
#define OPTION_VAL_17_MS_DELAY_FI NULL
#define OPTION_VAL_18_MS_DELAY_FI NULL
#define OPTION_VAL_19_MS_DELAY_FI NULL
#define OPTION_VAL_20_MS_DELAY_FI NULL
#define OPTION_VAL_21_MS_DELAY_FI NULL
#define OPTION_VAL_22_MS_DELAY_FI NULL
#define OPTION_VAL_23_MS_DELAY_FI NULL
#define OPTION_VAL_24_MS_DELAY_FI NULL
#define OPTION_VAL_25_MS_DELAY_FI NULL
#define OPTION_VAL_26_MS_DELAY_FI NULL
#define OPTION_VAL_27_MS_DELAY_FI NULL
#define OPTION_VAL_28_MS_DELAY_FI NULL
#define OPTION_VAL_29_MS_DELAY_FI NULL
#define OPTION_VAL_30_MS_DELAY_FI NULL
#define OPTION_VAL_31_MS_DELAY_FI NULL
#define OPTION_VAL_32_MS_DELAY_FI NULL
#define FCEUMM_SWAPDUTY_LABEL_FI NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_FI NULL
#define FCEUMM_SWAPDUTY_INFO_0_FI NULL
#define FCEUMM_SNDVOLUME_LABEL_FI NULL
#define FCEUMM_SNDVOLUME_INFO_0_FI NULL
#define OPTION_VAL_0_FI "0 %"
#define OPTION_VAL_1_FI "10 %"
#define OPTION_VAL_2_FI "20 %"
#define OPTION_VAL_3_FI "30 %"
#define OPTION_VAL_4_FI "40 %"
#define OPTION_VAL_5_FI "50 %"
#define OPTION_VAL_6_FI "60 %"
#define OPTION_VAL_7_FI "70 %"
#define OPTION_VAL_8_FI "80 %"
#define OPTION_VAL_9_FI "90 %"
#define OPTION_VAL_10_FI "100 %"
#define FCEUMM_APU_1_LABEL_FI NULL
#define FCEUMM_APU_1_LABEL_CAT_FI NULL
#define FCEUMM_APU_1_INFO_0_FI NULL
#define FCEUMM_APU_2_LABEL_FI NULL
#define FCEUMM_APU_2_LABEL_CAT_FI NULL
#define FCEUMM_APU_2_INFO_0_FI NULL
#define FCEUMM_APU_3_LABEL_FI NULL
#define FCEUMM_APU_3_LABEL_CAT_FI NULL
#define FCEUMM_APU_3_INFO_0_FI NULL
#define FCEUMM_APU_4_LABEL_FI NULL
#define FCEUMM_APU_4_LABEL_CAT_FI NULL
#define FCEUMM_APU_4_INFO_0_FI NULL
#define FCEUMM_APU_5_LABEL_FI NULL
#define FCEUMM_APU_5_LABEL_CAT_FI NULL
#define FCEUMM_APU_5_INFO_0_FI NULL
#define FCEUMM_TURBO_ENABLE_LABEL_FI "Ota turbo käyttöön"
#define FCEUMM_TURBO_ENABLE_INFO_0_FI "Ota käyttöön tai poista käytöstä turbo painikkeet."
#define OPTION_VAL_NONE_FI "Ei mitään"
#define OPTION_VAL_PLAYER_1_FI "Pelaaja 1"
#define OPTION_VAL_PLAYER_2_FI "Pelaaja 2"
#define OPTION_VAL_BOTH_FI "Molemmat"
#define FCEUMM_TURBO_DELAY_LABEL_FI NULL
#define FCEUMM_TURBO_DELAY_INFO_0_FI NULL
#define FCEUMM_ZAPPER_MODE_LABEL_FI "Zapper-tila"
#define FCEUMM_ZAPPER_MODE_INFO_0_FI NULL
#define OPTION_VAL_CLIGHTGUN_FI NULL
#define OPTION_VAL_STLIGHTGUN_FI NULL
#define OPTION_VAL_TOUCHSCREEN_FI "Kosketusnäyttö"
#define OPTION_VAL_MOUSE_FI "Hiiri"
#define FCEUMM_SHOW_CROSSHAIR_LABEL_FI "Näytä zapperin tähtäin"
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_FI NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_FI NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_FI NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_FI NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_FI NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_FI NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_FI NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_FI "Salli vastakkaiset suunnat"
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_FI NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_FI "Poista sprite-rajoitus"
#define FCEUMM_NOSPRITELIMIT_INFO_0_FI NULL
#define FCEUMM_OVERCLOCKING_LABEL_FI "Ylikellotus"
#define FCEUMM_OVERCLOCKING_INFO_0_FI NULL
#define OPTION_VAL_2X_POSTRENDER_FI NULL
#define OPTION_VAL_2X_VBLANK_FI NULL
#define FCEUMM_RAMSTATE_LABEL_FI NULL
#define FCEUMM_RAMSTATE_INFO_0_FI "RAM-arvot käynnistyksessä. Jotkin pelit tarvitsevat alustavat RAM-arvot joidenkin satunnaisten numeroiden luomiseen."
#define OPTION_VAL_FILL_FF_FI NULL
#define OPTION_VAL_FILL_00_FI NULL
#define OPTION_VAL_RANDOM_FI "Satunnainen"

struct retro_core_option_v2_category option_cats_fi[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_FI,
      CATEGORY_SYSTEM_INFO_0_FI
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_FI,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_FI
#else
      CATEGORY_VIDEO_INFO_1_FI
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_FI,
      CATEGORY_AUDIO_INFO_0_FI
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_FI,
      CATEGORY_INPUT_INFO_0_FI
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_FI,
      CATEGORY_HACKS_INFO_0_FI
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_FI,
      CATEGORY_DIP_SWITCH_INFO_0_FI
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_fi[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_FI,
      NULL,
      FCEUMM_REGION_INFO_0_FI,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_FI },
         { "NTSC",  OPTION_VAL_NTSC_FI },
         { "PAL",   OPTION_VAL_PAL_FI },
         { "Dendy", OPTION_VAL_DENDY_FI },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_FI,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_FI,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_FI,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_FI,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_FI,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_FI,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_FI,
      NULL,
      FCEUMM_ASPECT_INFO_0_FI,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_FI },
         { "4:3",     OPTION_VAL_4_3_FI },
         { "PP",      OPTION_VAL_PP_FI },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_FI,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_FI,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_FI,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_FI,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_FI,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_FI,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_FI,
      NULL,
      FCEUMM_PALETTE_INFO_0_FI,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_FI },
         { "asqrealc",             OPTION_VAL_ASQREALC_FI },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_FI },
         { "rgb",                  OPTION_VAL_RGB_FI },
         { "yuv-v3",               OPTION_VAL_YUV_V3_FI },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_FI },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_FI },
         { "pal",                  OPTION_VAL_PAL_FI },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_FI },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_FI },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_FI },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_FI },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_FI },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_FI },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_FI },
         { "nescap",               OPTION_VAL_NESCAP_FI },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_FI },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_FI },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_FI },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_FI },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_FI },
         { "raw",                  OPTION_VAL_RAW_FI },
         { "custom",               OPTION_VAL_CUSTOM_FI },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_FI,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_FI,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_FI },
         { "svideo",     OPTION_VAL_SVIDEO_FI },
         { "rgb",        OPTION_VAL_RGB_O9_FI },
         { "monochrome", OPTION_VAL_MONOCHROME_FI },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_FI,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_FI,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_FI },
         { "High",      OPTION_VAL_HIGH_FI },
         { "Very High", OPTION_VAL_VERY_HIGH_FI },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_FI,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_FI,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_FI,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_FI,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_FI },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_FI },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_FI },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_FI },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_FI },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_FI },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_FI },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_FI },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_FI },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_FI },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_FI },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_FI },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_FI },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_FI },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_FI },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_FI },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_FI },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_FI },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_FI },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_FI },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_FI },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_FI },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_FI },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_FI },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_FI },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_FI },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_FI },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_FI },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_FI },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_FI },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_FI },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_FI },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_FI,
      FCEUMM_SWAPDUTY_LABEL_CAT_FI,
      FCEUMM_SWAPDUTY_INFO_0_FI,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_FI,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_FI,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_FI },
         { "1",  OPTION_VAL_1_FI },
         { "2",  OPTION_VAL_2_FI },
         { "3",  OPTION_VAL_3_FI },
         { "4",  OPTION_VAL_4_FI },
         { "5",  OPTION_VAL_5_FI },
         { "6",  OPTION_VAL_6_FI },
         { "7",  OPTION_VAL_7_FI },
         { "8",  OPTION_VAL_8_FI },
         { "9",  OPTION_VAL_9_FI },
         { "10", OPTION_VAL_10_FI },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_FI,
      FCEUMM_APU_1_LABEL_CAT_FI,
      FCEUMM_APU_1_INFO_0_FI,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_FI,
      FCEUMM_APU_2_LABEL_CAT_FI,
      FCEUMM_APU_2_INFO_0_FI,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_FI,
      FCEUMM_APU_3_LABEL_CAT_FI,
      FCEUMM_APU_3_INFO_0_FI,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_FI,
      FCEUMM_APU_4_LABEL_CAT_FI,
      FCEUMM_APU_4_INFO_0_FI,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_FI,
      FCEUMM_APU_5_LABEL_CAT_FI,
      FCEUMM_APU_5_INFO_0_FI,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_FI,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_FI,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_FI },
         { "Player 1", OPTION_VAL_PLAYER_1_FI },
         { "Player 2", OPTION_VAL_PLAYER_2_FI },
         { "Both",     OPTION_VAL_BOTH_FI },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_FI,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_FI,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_FI,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_FI,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_FI },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_FI },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_FI },
         { "mouse",       OPTION_VAL_MOUSE_FI },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_FI,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_FI,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_FI,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_FI,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_FI,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_FI,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_FI,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_FI,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_FI,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_FI,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_FI,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_FI,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_FI,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_FI,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_FI },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_FI },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_FI,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_FI,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_FI },
         { "fill $00", OPTION_VAL_FILL_00_FI },
         { "random",   OPTION_VAL_RANDOM_FI },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_fi = {
   option_cats_fi,
   option_defs_fi
};

/* RETRO_LANGUAGE_FR */

#define CATEGORY_SYSTEM_LABEL_FR "Système"
#define CATEGORY_SYSTEM_INFO_0_FR NULL
#define CATEGORY_VIDEO_LABEL_FR "Vidéo"
#define CATEGORY_VIDEO_INFO_0_FR NULL
#define CATEGORY_VIDEO_INFO_1_FR NULL
#define CATEGORY_AUDIO_LABEL_FR NULL
#define CATEGORY_AUDIO_INFO_0_FR NULL
#define CATEGORY_INPUT_LABEL_FR "Entrées"
#define CATEGORY_INPUT_INFO_0_FR NULL
#define CATEGORY_HACKS_LABEL_FR "Hacks d'émulation"
#define CATEGORY_HACKS_INFO_0_FR "Modifier les réglages d'overclocking du processeur et de la précision d'émulation qui affectent les performances de bas niveau et la compatibilité."
#define CATEGORY_DIP_SWITCH_LABEL_FR "DIP switches"
#define CATEGORY_DIP_SWITCH_INFO_0_FR NULL
#define FCEUMM_REGION_LABEL_FR "Région"
#define FCEUMM_REGION_INFO_0_FR "Forcer le cœur à utiliser les timings de région NTSC, PAL ou Dendy."
#define OPTION_VAL_AUTO_FR NULL
#define OPTION_VAL_NTSC_FR NULL
#define OPTION_VAL_PAL_FR NULL
#define OPTION_VAL_DENDY_FR NULL
#define FCEUMM_GAME_GENIE_LABEL_FR "Module Game Genie (Redémarrage requis)"
#define FCEUMM_GAME_GENIE_INFO_0_FR "Activer l'émulation d'une cartouche de module d'extension Game Genie, permettant l'entrée de codes de cheat lors du lancement de jeux. Le fichier 'gamegenie.nes' doit être présent dans le dossier system de l'interface. Ne s'applique pas au contenu FDS ou arcade."
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_FR "Afficher les options système avancées"
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_FR "Afficher les options et les réglages avancés du système."
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_FR "Afficher les options audio avancées"
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_FR "Afficher les options et les réglages audio avancés."
#define FCEUMM_ASPECT_LABEL_FR "Rapport d'aspect"
#define FCEUMM_ASPECT_INFO_0_FR "Choisir le rapport d'aspect préféré."
#define OPTION_VAL_8_7_PAR_FR "Rapport d'aspect des pixels 8:7 (PAR)"
#define OPTION_VAL_4_3_FR NULL
#define OPTION_VAL_PP_FR "Pixels parfaits à l'échelle"
#define FCEUMM_OVERSCAN_LABEL_FR "Recadrer le surbalayage"
#define FCEUMM_OVERSCAN_INFO_0_FR "Supprime le surbalayage de 8 pixels de tous les côtés de l'écran."
#define FCEUMM_OVERSCAN_H_LABEL_FR "Recadrer le surbalayage horizontal"
#define FCEUMM_OVERSCAN_H_INFO_0_FR "Supprime 8 pixels des côtés gauche et droit de l'écran pour simuler le surbalayage vu sur les téléviseurs CRT standard."
#define FCEUMM_OVERSCAN_V_LABEL_FR "Recadrer le surbalayage vertical"
#define FCEUMM_OVERSCAN_V_INFO_0_FR "Supprime 8 pixels du haut et du bas de l'écran pour simuler le surbalayage vu sur les téléviseurs CRT standard."
#define FCEUMM_PALETTE_LABEL_FR "Palette de couleurs"
#define FCEUMM_PALETTE_INFO_0_FR "Choisir parmi les palettes pré-générées, une palette personnalisée de 64x3 à partir d'un fichier ou un format brut (nécessite d'utiliser un shader nes-decoder)."
#define OPTION_VAL_DEFAULT_FR "Par défaut"
#define OPTION_VAL_ASQREALC_FR "Réelle par AspiringSquire"
#define OPTION_VAL_NINTENDO_VC_FR "Console virtuelle Nintendo"
#define OPTION_VAL_RGB_FR "PPU RVB Nintendo"
#define OPTION_VAL_YUV_V3_FR "YUV-V3 de FBX"
#define OPTION_VAL_UNSATURATED_FINAL_FR "Finale non saturée de FBX"
#define OPTION_VAL_SONY_CXA2025AS_US_FR NULL
#define OPTION_VAL_BMF_FINAL2_FR "Finale 2 de BMF"
#define OPTION_VAL_BMF_FINAL3_FR "Finale 3 de BMF"
#define OPTION_VAL_SMOOTH_FBX_FR "Lisse de FBX"
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_FR "Composite directe de FBX"
#define OPTION_VAL_PVM_STYLE_D93_FBX_FR "Style PVM D93 de FBX"
#define OPTION_VAL_NTSC_HARDWARE_FBX_FR "Matériel NTSC de FBX"
#define OPTION_VAL_NES_CLASSIC_FBX_FS_FR NULL
#define OPTION_VAL_NESCAP_FR "NESCAP de RGBSource"
#define OPTION_VAL_WAVEBEAM_FR "Wavebeam de nakedarthur"
#define OPTION_VAL_DIGITAL_PRIME_FBX_FR NULL
#define OPTION_VAL_MAGNUM_FBX_FR NULL
#define OPTION_VAL_SMOOTH_V2_FBX_FR NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FR NULL
#define OPTION_VAL_RAW_FR "Données brutes"
#define OPTION_VAL_CUSTOM_FR "Personnalisée"
#define FCEUMM_NTSC_FILTER_LABEL_FR "Filtre NTSC"
#define FCEUMM_NTSC_FILTER_INFO_0_FR NULL
#define OPTION_VAL_COMPOSITE_FR NULL
#define OPTION_VAL_SVIDEO_FR NULL
#define OPTION_VAL_RGB_O9_FR "RVB"
#define OPTION_VAL_MONOCHROME_FR NULL
#define FCEUMM_SNDQUALITY_LABEL_FR "Qualité audio"
#define FCEUMM_SNDQUALITY_INFO_0_FR "Activer la meilleure qualité audio. Augmente les exigences de performances."
#define OPTION_VAL_LOW_FR "Basse"
#define OPTION_VAL_HIGH_FR "Élevée"
#define OPTION_VAL_VERY_HIGH_FR "Très élevée"
#define FCEUMM_SNDLOWPASS_LABEL_FR NULL
#define FCEUMM_SNDLOWPASS_INFO_0_FR NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_FR NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_FR NULL
#define OPTION_VAL_01_MS_DELAY_FR NULL
#define OPTION_VAL_02_MS_DELAY_FR NULL
#define OPTION_VAL_03_MS_DELAY_FR NULL
#define OPTION_VAL_04_MS_DELAY_FR NULL
#define OPTION_VAL_05_MS_DELAY_FR NULL
#define OPTION_VAL_06_MS_DELAY_FR NULL
#define OPTION_VAL_07_MS_DELAY_FR NULL
#define OPTION_VAL_08_MS_DELAY_FR NULL
#define OPTION_VAL_09_MS_DELAY_FR NULL
#define OPTION_VAL_10_MS_DELAY_FR NULL
#define OPTION_VAL_11_MS_DELAY_FR NULL
#define OPTION_VAL_12_MS_DELAY_FR NULL
#define OPTION_VAL_13_MS_DELAY_FR NULL
#define OPTION_VAL_14_MS_DELAY_FR NULL
#define OPTION_VAL_15_MS_DELAY_FR NULL
#define OPTION_VAL_16_MS_DELAY_FR NULL
#define OPTION_VAL_17_MS_DELAY_FR NULL
#define OPTION_VAL_18_MS_DELAY_FR NULL
#define OPTION_VAL_19_MS_DELAY_FR NULL
#define OPTION_VAL_20_MS_DELAY_FR NULL
#define OPTION_VAL_21_MS_DELAY_FR NULL
#define OPTION_VAL_22_MS_DELAY_FR NULL
#define OPTION_VAL_23_MS_DELAY_FR NULL
#define OPTION_VAL_24_MS_DELAY_FR NULL
#define OPTION_VAL_25_MS_DELAY_FR NULL
#define OPTION_VAL_26_MS_DELAY_FR NULL
#define OPTION_VAL_27_MS_DELAY_FR NULL
#define OPTION_VAL_28_MS_DELAY_FR NULL
#define OPTION_VAL_29_MS_DELAY_FR NULL
#define OPTION_VAL_30_MS_DELAY_FR NULL
#define OPTION_VAL_31_MS_DELAY_FR NULL
#define OPTION_VAL_32_MS_DELAY_FR NULL
#define FCEUMM_SWAPDUTY_LABEL_FR "Permuter les cycles de service audio"
#define FCEUMM_SWAPDUTY_LABEL_CAT_FR "Permuter les cycles de service"
#define FCEUMM_SWAPDUTY_INFO_0_FR "Simule le son des famiclones qui ont les bits des cycles de service des canaux d'ondes d'impulsion inversés."
#define FCEUMM_SNDVOLUME_LABEL_FR "Volume principal"
#define FCEUMM_SNDVOLUME_INFO_0_FR "Changer le niveau du volume principal."
#define OPTION_VAL_0_FR NULL
#define OPTION_VAL_1_FR NULL
#define OPTION_VAL_2_FR NULL
#define OPTION_VAL_3_FR NULL
#define OPTION_VAL_4_FR NULL
#define OPTION_VAL_5_FR NULL
#define OPTION_VAL_6_FR NULL
#define OPTION_VAL_7_FR NULL
#define OPTION_VAL_8_FR NULL
#define OPTION_VAL_9_FR NULL
#define OPTION_VAL_10_FR NULL
#define FCEUMM_APU_1_LABEL_FR "Canal audio 1 (carré 1)"
#define FCEUMM_APU_1_LABEL_CAT_FR "Canal 1 (carré 1)"
#define FCEUMM_APU_1_INFO_0_FR "Active ou désactive la sortie audio du générateur d'ondes d'impulsion 1."
#define FCEUMM_APU_2_LABEL_FR "Canal audio 2 (carré 2)"
#define FCEUMM_APU_2_LABEL_CAT_FR "Canal 2 (carré 2)"
#define FCEUMM_APU_2_INFO_0_FR "Active ou désactive la sortie audio du générateur d'ondes d'impulsion 2."
#define FCEUMM_APU_3_LABEL_FR "Canal audio 3 (triangle)"
#define FCEUMM_APU_3_LABEL_CAT_FR "Canal 3 (triangle)"
#define FCEUMM_APU_3_INFO_0_FR "Active ou désactive la sortie audio du générateur d'ondes triangulaires."
#define FCEUMM_APU_4_LABEL_FR "Canal audio 4 (bruit)"
#define FCEUMM_APU_4_LABEL_CAT_FR "Canal 4 (bruit)"
#define FCEUMM_APU_4_INFO_0_FR "Active ou désactive la sortie audio du générateur de bruit."
#define FCEUMM_APU_5_LABEL_FR "Canal audio 5 (PCM)"
#define FCEUMM_APU_5_LABEL_CAT_FR "Canal 5 (PCM)"
#define FCEUMM_APU_5_INFO_0_FR "Active ou désactive la sortie audio du canal de modulation delta."
#define FCEUMM_TURBO_ENABLE_LABEL_FR "Activer le turbo"
#define FCEUMM_TURBO_ENABLE_INFO_0_FR "Active ou désactive les touches en mode turbo."
#define OPTION_VAL_NONE_FR "Aucun"
#define OPTION_VAL_PLAYER_1_FR "Joueur 1"
#define OPTION_VAL_PLAYER_2_FR "Joueur 2"
#define OPTION_VAL_BOTH_FR "Les deux"
#define FCEUMM_TURBO_DELAY_LABEL_FR "Délai du turbo (en images)"
#define FCEUMM_TURBO_DELAY_INFO_0_FR "Vitesse de répétition des touches turbo en images."
#define FCEUMM_ZAPPER_MODE_LABEL_FR "Mode Zapper"
#define FCEUMM_ZAPPER_MODE_INFO_0_FR NULL
#define OPTION_VAL_CLIGHTGUN_FR NULL
#define OPTION_VAL_STLIGHTGUN_FR NULL
#define OPTION_VAL_TOUCHSCREEN_FR "Écran tactile"
#define OPTION_VAL_MOUSE_FR "Souris"
#define FCEUMM_SHOW_CROSSHAIR_LABEL_FR "Afficher le viseur du Zapper"
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_FR NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_FR "Tolérance du Zapper"
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_FR "Définit combien de pixels autour de la zone ciblée sont sur la cible."
#define FCEUMM_ZAPPER_TRIGGER_LABEL_FR NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_FR NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_FR NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_FR NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_FR "Autoriser les directions opposées"
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_FR "Autorise les combinaisons de touches haut+bas ou gauche+droite, qui peuvent créer différents effets dans certains jeux."
#define FCEUMM_NOSPRITELIMIT_LABEL_FR "Aucune limite de sprites"
#define FCEUMM_NOSPRITELIMIT_INFO_0_FR "Supprime la limitation matérielle de 8 par ligne de balayage. Cela réduit le scintillement des sprites mais peut causer des bugs dans certains jeux car certains utilisent ceci pour des effets."
#define FCEUMM_OVERCLOCKING_LABEL_FR NULL
#define FCEUMM_OVERCLOCKING_INFO_0_FR "Active ou désactive l'overclocking, ce qui peut réduire les ralentissements dans certains jeux. La méthode de post-rendu est plus compatible avec chaque jeu, celle de rafraîchissement vertical est plus efficace pour des jeux comme Contra Force."
#define OPTION_VAL_2X_POSTRENDER_FR "Post-rendu x2"
#define OPTION_VAL_2X_VBLANK_FR "Rafraîchissement vertical x2"
#define FCEUMM_RAMSTATE_LABEL_FR "Remplir la RAM lors de la mise sous tension (Redémarrage requis)"
#define FCEUMM_RAMSTATE_INFO_0_FR "Les valeurs dans la RAM lors de la mise sous tension. Certains jeux dépendent des valeurs initiales dans la RAM pour la génération de nombres aléatoires par exemple."
#define OPTION_VAL_FILL_FF_FR NULL
#define OPTION_VAL_FILL_00_FR NULL
#define OPTION_VAL_RANDOM_FR "Aléatoire"

struct retro_core_option_v2_category option_cats_fr[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_FR,
      CATEGORY_SYSTEM_INFO_0_FR
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_FR,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_FR
#else
      CATEGORY_VIDEO_INFO_1_FR
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_FR,
      CATEGORY_AUDIO_INFO_0_FR
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_FR,
      CATEGORY_INPUT_INFO_0_FR
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_FR,
      CATEGORY_HACKS_INFO_0_FR
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_FR,
      CATEGORY_DIP_SWITCH_INFO_0_FR
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_fr[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_FR,
      NULL,
      FCEUMM_REGION_INFO_0_FR,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_FR },
         { "NTSC",  OPTION_VAL_NTSC_FR },
         { "PAL",   OPTION_VAL_PAL_FR },
         { "Dendy", OPTION_VAL_DENDY_FR },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_FR,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_FR,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_FR,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_FR,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_FR,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_FR,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_FR,
      NULL,
      FCEUMM_ASPECT_INFO_0_FR,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_FR },
         { "4:3",     OPTION_VAL_4_3_FR },
         { "PP",      OPTION_VAL_PP_FR },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_FR,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_FR,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_FR,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_FR,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_FR,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_FR,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_FR,
      NULL,
      FCEUMM_PALETTE_INFO_0_FR,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_FR },
         { "asqrealc",             OPTION_VAL_ASQREALC_FR },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_FR },
         { "rgb",                  OPTION_VAL_RGB_FR },
         { "yuv-v3",               OPTION_VAL_YUV_V3_FR },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_FR },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_FR },
         { "pal",                  OPTION_VAL_PAL_FR },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_FR },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_FR },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_FR },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_FR },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_FR },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_FR },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_FR },
         { "nescap",               OPTION_VAL_NESCAP_FR },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_FR },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_FR },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_FR },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_FR },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_FR },
         { "raw",                  OPTION_VAL_RAW_FR },
         { "custom",               OPTION_VAL_CUSTOM_FR },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_FR,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_FR,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_FR },
         { "svideo",     OPTION_VAL_SVIDEO_FR },
         { "rgb",        OPTION_VAL_RGB_O9_FR },
         { "monochrome", OPTION_VAL_MONOCHROME_FR },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_FR,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_FR,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_FR },
         { "High",      OPTION_VAL_HIGH_FR },
         { "Very High", OPTION_VAL_VERY_HIGH_FR },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_FR,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_FR,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_FR,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_FR,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_FR },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_FR },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_FR },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_FR },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_FR },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_FR },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_FR },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_FR },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_FR },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_FR },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_FR },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_FR },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_FR },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_FR },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_FR },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_FR },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_FR },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_FR },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_FR },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_FR },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_FR },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_FR },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_FR },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_FR },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_FR },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_FR },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_FR },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_FR },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_FR },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_FR },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_FR },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_FR },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_FR,
      FCEUMM_SWAPDUTY_LABEL_CAT_FR,
      FCEUMM_SWAPDUTY_INFO_0_FR,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_FR,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_FR,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_FR },
         { "1",  OPTION_VAL_1_FR },
         { "2",  OPTION_VAL_2_FR },
         { "3",  OPTION_VAL_3_FR },
         { "4",  OPTION_VAL_4_FR },
         { "5",  OPTION_VAL_5_FR },
         { "6",  OPTION_VAL_6_FR },
         { "7",  OPTION_VAL_7_FR },
         { "8",  OPTION_VAL_8_FR },
         { "9",  OPTION_VAL_9_FR },
         { "10", OPTION_VAL_10_FR },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_FR,
      FCEUMM_APU_1_LABEL_CAT_FR,
      FCEUMM_APU_1_INFO_0_FR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_FR,
      FCEUMM_APU_2_LABEL_CAT_FR,
      FCEUMM_APU_2_INFO_0_FR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_FR,
      FCEUMM_APU_3_LABEL_CAT_FR,
      FCEUMM_APU_3_INFO_0_FR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_FR,
      FCEUMM_APU_4_LABEL_CAT_FR,
      FCEUMM_APU_4_INFO_0_FR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_FR,
      FCEUMM_APU_5_LABEL_CAT_FR,
      FCEUMM_APU_5_INFO_0_FR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_FR,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_FR,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_FR },
         { "Player 1", OPTION_VAL_PLAYER_1_FR },
         { "Player 2", OPTION_VAL_PLAYER_2_FR },
         { "Both",     OPTION_VAL_BOTH_FR },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_FR,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_FR,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_FR,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_FR,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_FR },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_FR },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_FR },
         { "mouse",       OPTION_VAL_MOUSE_FR },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_FR,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_FR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_FR,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_FR,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_FR,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_FR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_FR,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_FR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_FR,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_FR,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_FR,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_FR,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_FR,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_FR,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_FR },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_FR },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_FR,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_FR,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_FR },
         { "fill $00", OPTION_VAL_FILL_00_FR },
         { "random",   OPTION_VAL_RANDOM_FR },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_fr = {
   option_cats_fr,
   option_defs_fr
};

/* RETRO_LANGUAGE_GL */

#define CATEGORY_SYSTEM_LABEL_GL NULL
#define CATEGORY_SYSTEM_INFO_0_GL NULL
#define CATEGORY_VIDEO_LABEL_GL "Vídeo"
#define CATEGORY_VIDEO_INFO_0_GL NULL
#define CATEGORY_VIDEO_INFO_1_GL NULL
#define CATEGORY_AUDIO_LABEL_GL "Son"
#define CATEGORY_AUDIO_INFO_0_GL NULL
#define CATEGORY_INPUT_LABEL_GL "Entrada"
#define CATEGORY_INPUT_INFO_0_GL NULL
#define CATEGORY_HACKS_LABEL_GL "Hacks de emulación"
#define CATEGORY_HACKS_INFO_0_GL NULL
#define CATEGORY_DIP_SWITCH_LABEL_GL NULL
#define CATEGORY_DIP_SWITCH_INFO_0_GL NULL
#define FCEUMM_REGION_LABEL_GL NULL
#define FCEUMM_REGION_INFO_0_GL NULL
#define OPTION_VAL_AUTO_GL NULL
#define OPTION_VAL_NTSC_GL NULL
#define OPTION_VAL_PAL_GL NULL
#define OPTION_VAL_DENDY_GL NULL
#define FCEUMM_GAME_GENIE_LABEL_GL NULL
#define FCEUMM_GAME_GENIE_INFO_0_GL NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_GL NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_GL NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_GL NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_GL NULL
#define FCEUMM_ASPECT_LABEL_GL NULL
#define FCEUMM_ASPECT_INFO_0_GL NULL
#define OPTION_VAL_8_7_PAR_GL NULL
#define OPTION_VAL_4_3_GL NULL
#define OPTION_VAL_PP_GL NULL
#define FCEUMM_OVERSCAN_LABEL_GL NULL
#define FCEUMM_OVERSCAN_INFO_0_GL NULL
#define FCEUMM_OVERSCAN_H_LABEL_GL NULL
#define FCEUMM_OVERSCAN_H_INFO_0_GL NULL
#define FCEUMM_OVERSCAN_V_LABEL_GL NULL
#define FCEUMM_OVERSCAN_V_INFO_0_GL NULL
#define FCEUMM_PALETTE_LABEL_GL NULL
#define FCEUMM_PALETTE_INFO_0_GL NULL
#define OPTION_VAL_DEFAULT_GL NULL
#define OPTION_VAL_ASQREALC_GL NULL
#define OPTION_VAL_NINTENDO_VC_GL NULL
#define OPTION_VAL_RGB_GL NULL
#define OPTION_VAL_YUV_V3_GL NULL
#define OPTION_VAL_UNSATURATED_FINAL_GL NULL
#define OPTION_VAL_SONY_CXA2025AS_US_GL NULL
#define OPTION_VAL_BMF_FINAL2_GL NULL
#define OPTION_VAL_BMF_FINAL3_GL NULL
#define OPTION_VAL_SMOOTH_FBX_GL NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_GL NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_GL NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_GL NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_GL NULL
#define OPTION_VAL_NESCAP_GL NULL
#define OPTION_VAL_WAVEBEAM_GL NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_GL NULL
#define OPTION_VAL_MAGNUM_FBX_GL NULL
#define OPTION_VAL_SMOOTH_V2_FBX_GL NULL
#define OPTION_VAL_NES_CLASSIC_FBX_GL NULL
#define OPTION_VAL_RAW_GL NULL
#define OPTION_VAL_CUSTOM_GL NULL
#define FCEUMM_NTSC_FILTER_LABEL_GL NULL
#define FCEUMM_NTSC_FILTER_INFO_0_GL NULL
#define OPTION_VAL_COMPOSITE_GL NULL
#define OPTION_VAL_SVIDEO_GL NULL
#define OPTION_VAL_RGB_O9_GL NULL
#define OPTION_VAL_MONOCHROME_GL NULL
#define FCEUMM_SNDQUALITY_LABEL_GL NULL
#define FCEUMM_SNDQUALITY_INFO_0_GL NULL
#define OPTION_VAL_LOW_GL NULL
#define OPTION_VAL_HIGH_GL NULL
#define OPTION_VAL_VERY_HIGH_GL NULL
#define FCEUMM_SNDLOWPASS_LABEL_GL NULL
#define FCEUMM_SNDLOWPASS_INFO_0_GL NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_GL NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_GL NULL
#define OPTION_VAL_01_MS_DELAY_GL NULL
#define OPTION_VAL_02_MS_DELAY_GL NULL
#define OPTION_VAL_03_MS_DELAY_GL NULL
#define OPTION_VAL_04_MS_DELAY_GL NULL
#define OPTION_VAL_05_MS_DELAY_GL NULL
#define OPTION_VAL_06_MS_DELAY_GL NULL
#define OPTION_VAL_07_MS_DELAY_GL NULL
#define OPTION_VAL_08_MS_DELAY_GL NULL
#define OPTION_VAL_09_MS_DELAY_GL NULL
#define OPTION_VAL_10_MS_DELAY_GL NULL
#define OPTION_VAL_11_MS_DELAY_GL NULL
#define OPTION_VAL_12_MS_DELAY_GL NULL
#define OPTION_VAL_13_MS_DELAY_GL NULL
#define OPTION_VAL_14_MS_DELAY_GL NULL
#define OPTION_VAL_15_MS_DELAY_GL NULL
#define OPTION_VAL_16_MS_DELAY_GL NULL
#define OPTION_VAL_17_MS_DELAY_GL NULL
#define OPTION_VAL_18_MS_DELAY_GL NULL
#define OPTION_VAL_19_MS_DELAY_GL NULL
#define OPTION_VAL_20_MS_DELAY_GL NULL
#define OPTION_VAL_21_MS_DELAY_GL NULL
#define OPTION_VAL_22_MS_DELAY_GL NULL
#define OPTION_VAL_23_MS_DELAY_GL NULL
#define OPTION_VAL_24_MS_DELAY_GL NULL
#define OPTION_VAL_25_MS_DELAY_GL NULL
#define OPTION_VAL_26_MS_DELAY_GL NULL
#define OPTION_VAL_27_MS_DELAY_GL NULL
#define OPTION_VAL_28_MS_DELAY_GL NULL
#define OPTION_VAL_29_MS_DELAY_GL NULL
#define OPTION_VAL_30_MS_DELAY_GL NULL
#define OPTION_VAL_31_MS_DELAY_GL NULL
#define OPTION_VAL_32_MS_DELAY_GL NULL
#define FCEUMM_SWAPDUTY_LABEL_GL NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_GL NULL
#define FCEUMM_SWAPDUTY_INFO_0_GL NULL
#define FCEUMM_SNDVOLUME_LABEL_GL NULL
#define FCEUMM_SNDVOLUME_INFO_0_GL NULL
#define OPTION_VAL_0_GL NULL
#define OPTION_VAL_1_GL NULL
#define OPTION_VAL_2_GL NULL
#define OPTION_VAL_3_GL NULL
#define OPTION_VAL_4_GL NULL
#define OPTION_VAL_5_GL NULL
#define OPTION_VAL_6_GL NULL
#define OPTION_VAL_7_GL NULL
#define OPTION_VAL_8_GL NULL
#define OPTION_VAL_9_GL NULL
#define OPTION_VAL_10_GL NULL
#define FCEUMM_APU_1_LABEL_GL NULL
#define FCEUMM_APU_1_LABEL_CAT_GL NULL
#define FCEUMM_APU_1_INFO_0_GL NULL
#define FCEUMM_APU_2_LABEL_GL NULL
#define FCEUMM_APU_2_LABEL_CAT_GL NULL
#define FCEUMM_APU_2_INFO_0_GL NULL
#define FCEUMM_APU_3_LABEL_GL NULL
#define FCEUMM_APU_3_LABEL_CAT_GL NULL
#define FCEUMM_APU_3_INFO_0_GL NULL
#define FCEUMM_APU_4_LABEL_GL NULL
#define FCEUMM_APU_4_LABEL_CAT_GL NULL
#define FCEUMM_APU_4_INFO_0_GL NULL
#define FCEUMM_APU_5_LABEL_GL NULL
#define FCEUMM_APU_5_LABEL_CAT_GL NULL
#define FCEUMM_APU_5_INFO_0_GL NULL
#define FCEUMM_TURBO_ENABLE_LABEL_GL NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_GL NULL
#define OPTION_VAL_NONE_GL "Ningún"
#define OPTION_VAL_PLAYER_1_GL NULL
#define OPTION_VAL_PLAYER_2_GL NULL
#define OPTION_VAL_BOTH_GL NULL
#define FCEUMM_TURBO_DELAY_LABEL_GL NULL
#define FCEUMM_TURBO_DELAY_INFO_0_GL NULL
#define FCEUMM_ZAPPER_MODE_LABEL_GL NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_GL NULL
#define OPTION_VAL_CLIGHTGUN_GL NULL
#define OPTION_VAL_STLIGHTGUN_GL NULL
#define OPTION_VAL_TOUCHSCREEN_GL NULL
#define OPTION_VAL_MOUSE_GL NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_GL NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_GL NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_GL NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_GL NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_GL NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_GL NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_GL NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_GL NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_GL NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_GL NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_GL NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_GL NULL
#define FCEUMM_OVERCLOCKING_LABEL_GL NULL
#define FCEUMM_OVERCLOCKING_INFO_0_GL NULL
#define OPTION_VAL_2X_POSTRENDER_GL NULL
#define OPTION_VAL_2X_VBLANK_GL NULL
#define FCEUMM_RAMSTATE_LABEL_GL NULL
#define FCEUMM_RAMSTATE_INFO_0_GL NULL
#define OPTION_VAL_FILL_FF_GL NULL
#define OPTION_VAL_FILL_00_GL NULL
#define OPTION_VAL_RANDOM_GL NULL

struct retro_core_option_v2_category option_cats_gl[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_GL,
      CATEGORY_SYSTEM_INFO_0_GL
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_GL,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_GL
#else
      CATEGORY_VIDEO_INFO_1_GL
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_GL,
      CATEGORY_AUDIO_INFO_0_GL
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_GL,
      CATEGORY_INPUT_INFO_0_GL
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_GL,
      CATEGORY_HACKS_INFO_0_GL
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_GL,
      CATEGORY_DIP_SWITCH_INFO_0_GL
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_gl[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_GL,
      NULL,
      FCEUMM_REGION_INFO_0_GL,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_GL },
         { "NTSC",  OPTION_VAL_NTSC_GL },
         { "PAL",   OPTION_VAL_PAL_GL },
         { "Dendy", OPTION_VAL_DENDY_GL },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_GL,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_GL,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_GL,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_GL,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_GL,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_GL,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_GL,
      NULL,
      FCEUMM_ASPECT_INFO_0_GL,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_GL },
         { "4:3",     OPTION_VAL_4_3_GL },
         { "PP",      OPTION_VAL_PP_GL },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_GL,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_GL,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_GL,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_GL,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_GL,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_GL,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_GL,
      NULL,
      FCEUMM_PALETTE_INFO_0_GL,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_GL },
         { "asqrealc",             OPTION_VAL_ASQREALC_GL },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_GL },
         { "rgb",                  OPTION_VAL_RGB_GL },
         { "yuv-v3",               OPTION_VAL_YUV_V3_GL },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_GL },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_GL },
         { "pal",                  OPTION_VAL_PAL_GL },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_GL },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_GL },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_GL },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_GL },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_GL },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_GL },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_GL },
         { "nescap",               OPTION_VAL_NESCAP_GL },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_GL },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_GL },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_GL },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_GL },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_GL },
         { "raw",                  OPTION_VAL_RAW_GL },
         { "custom",               OPTION_VAL_CUSTOM_GL },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_GL,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_GL,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_GL },
         { "svideo",     OPTION_VAL_SVIDEO_GL },
         { "rgb",        OPTION_VAL_RGB_O9_GL },
         { "monochrome", OPTION_VAL_MONOCHROME_GL },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_GL,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_GL,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_GL },
         { "High",      OPTION_VAL_HIGH_GL },
         { "Very High", OPTION_VAL_VERY_HIGH_GL },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_GL,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_GL,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_GL,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_GL,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_GL },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_GL },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_GL },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_GL },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_GL },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_GL },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_GL },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_GL },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_GL },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_GL },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_GL },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_GL },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_GL },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_GL },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_GL },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_GL },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_GL },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_GL },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_GL },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_GL },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_GL },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_GL },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_GL },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_GL },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_GL },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_GL },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_GL },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_GL },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_GL },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_GL },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_GL },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_GL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_GL,
      FCEUMM_SWAPDUTY_LABEL_CAT_GL,
      FCEUMM_SWAPDUTY_INFO_0_GL,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_GL,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_GL,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_GL },
         { "1",  OPTION_VAL_1_GL },
         { "2",  OPTION_VAL_2_GL },
         { "3",  OPTION_VAL_3_GL },
         { "4",  OPTION_VAL_4_GL },
         { "5",  OPTION_VAL_5_GL },
         { "6",  OPTION_VAL_6_GL },
         { "7",  OPTION_VAL_7_GL },
         { "8",  OPTION_VAL_8_GL },
         { "9",  OPTION_VAL_9_GL },
         { "10", OPTION_VAL_10_GL },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_GL,
      FCEUMM_APU_1_LABEL_CAT_GL,
      FCEUMM_APU_1_INFO_0_GL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_GL,
      FCEUMM_APU_2_LABEL_CAT_GL,
      FCEUMM_APU_2_INFO_0_GL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_GL,
      FCEUMM_APU_3_LABEL_CAT_GL,
      FCEUMM_APU_3_INFO_0_GL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_GL,
      FCEUMM_APU_4_LABEL_CAT_GL,
      FCEUMM_APU_4_INFO_0_GL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_GL,
      FCEUMM_APU_5_LABEL_CAT_GL,
      FCEUMM_APU_5_INFO_0_GL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_GL,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_GL,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_GL },
         { "Player 1", OPTION_VAL_PLAYER_1_GL },
         { "Player 2", OPTION_VAL_PLAYER_2_GL },
         { "Both",     OPTION_VAL_BOTH_GL },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_GL,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_GL,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_GL,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_GL,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_GL },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_GL },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_GL },
         { "mouse",       OPTION_VAL_MOUSE_GL },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_GL,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_GL,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_GL,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_GL,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_GL,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_GL,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_GL,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_GL,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_GL,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_GL,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_GL,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_GL,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_GL,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_GL,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_GL },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_GL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_GL,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_GL,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_GL },
         { "fill $00", OPTION_VAL_FILL_00_GL },
         { "random",   OPTION_VAL_RANDOM_GL },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_gl = {
   option_cats_gl,
   option_defs_gl
};

/* RETRO_LANGUAGE_HE */

#define CATEGORY_SYSTEM_LABEL_HE NULL
#define CATEGORY_SYSTEM_INFO_0_HE NULL
#define CATEGORY_VIDEO_LABEL_HE "וידאו"
#define CATEGORY_VIDEO_INFO_0_HE NULL
#define CATEGORY_VIDEO_INFO_1_HE NULL
#define CATEGORY_AUDIO_LABEL_HE "שמע"
#define CATEGORY_AUDIO_INFO_0_HE NULL
#define CATEGORY_INPUT_LABEL_HE "קלט"
#define CATEGORY_INPUT_INFO_0_HE NULL
#define CATEGORY_HACKS_LABEL_HE NULL
#define CATEGORY_HACKS_INFO_0_HE NULL
#define CATEGORY_DIP_SWITCH_LABEL_HE NULL
#define CATEGORY_DIP_SWITCH_INFO_0_HE NULL
#define FCEUMM_REGION_LABEL_HE NULL
#define FCEUMM_REGION_INFO_0_HE NULL
#define OPTION_VAL_AUTO_HE NULL
#define OPTION_VAL_NTSC_HE NULL
#define OPTION_VAL_PAL_HE NULL
#define OPTION_VAL_DENDY_HE NULL
#define FCEUMM_GAME_GENIE_LABEL_HE NULL
#define FCEUMM_GAME_GENIE_INFO_0_HE NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_HE NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_HE NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_HE NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_HE NULL
#define FCEUMM_ASPECT_LABEL_HE "יחס גובה-רוחב"
#define FCEUMM_ASPECT_INFO_0_HE NULL
#define OPTION_VAL_8_7_PAR_HE NULL
#define OPTION_VAL_4_3_HE NULL
#define OPTION_VAL_PP_HE NULL
#define FCEUMM_OVERSCAN_LABEL_HE NULL
#define FCEUMM_OVERSCAN_INFO_0_HE NULL
#define FCEUMM_OVERSCAN_H_LABEL_HE NULL
#define FCEUMM_OVERSCAN_H_INFO_0_HE NULL
#define FCEUMM_OVERSCAN_V_LABEL_HE NULL
#define FCEUMM_OVERSCAN_V_INFO_0_HE NULL
#define FCEUMM_PALETTE_LABEL_HE NULL
#define FCEUMM_PALETTE_INFO_0_HE NULL
#define OPTION_VAL_DEFAULT_HE NULL
#define OPTION_VAL_ASQREALC_HE NULL
#define OPTION_VAL_NINTENDO_VC_HE NULL
#define OPTION_VAL_RGB_HE NULL
#define OPTION_VAL_YUV_V3_HE NULL
#define OPTION_VAL_UNSATURATED_FINAL_HE NULL
#define OPTION_VAL_SONY_CXA2025AS_US_HE NULL
#define OPTION_VAL_BMF_FINAL2_HE NULL
#define OPTION_VAL_BMF_FINAL3_HE NULL
#define OPTION_VAL_SMOOTH_FBX_HE NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_HE NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_HE NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_HE NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_HE NULL
#define OPTION_VAL_NESCAP_HE NULL
#define OPTION_VAL_WAVEBEAM_HE NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_HE NULL
#define OPTION_VAL_MAGNUM_FBX_HE NULL
#define OPTION_VAL_SMOOTH_V2_FBX_HE NULL
#define OPTION_VAL_NES_CLASSIC_FBX_HE NULL
#define OPTION_VAL_RAW_HE NULL
#define OPTION_VAL_CUSTOM_HE NULL
#define FCEUMM_NTSC_FILTER_LABEL_HE NULL
#define FCEUMM_NTSC_FILTER_INFO_0_HE NULL
#define OPTION_VAL_COMPOSITE_HE NULL
#define OPTION_VAL_SVIDEO_HE NULL
#define OPTION_VAL_RGB_O9_HE NULL
#define OPTION_VAL_MONOCHROME_HE NULL
#define FCEUMM_SNDQUALITY_LABEL_HE NULL
#define FCEUMM_SNDQUALITY_INFO_0_HE NULL
#define OPTION_VAL_LOW_HE NULL
#define OPTION_VAL_HIGH_HE NULL
#define OPTION_VAL_VERY_HIGH_HE NULL
#define FCEUMM_SNDLOWPASS_LABEL_HE NULL
#define FCEUMM_SNDLOWPASS_INFO_0_HE NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_HE NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_HE NULL
#define OPTION_VAL_01_MS_DELAY_HE NULL
#define OPTION_VAL_02_MS_DELAY_HE NULL
#define OPTION_VAL_03_MS_DELAY_HE NULL
#define OPTION_VAL_04_MS_DELAY_HE NULL
#define OPTION_VAL_05_MS_DELAY_HE NULL
#define OPTION_VAL_06_MS_DELAY_HE NULL
#define OPTION_VAL_07_MS_DELAY_HE NULL
#define OPTION_VAL_08_MS_DELAY_HE NULL
#define OPTION_VAL_09_MS_DELAY_HE NULL
#define OPTION_VAL_10_MS_DELAY_HE NULL
#define OPTION_VAL_11_MS_DELAY_HE NULL
#define OPTION_VAL_12_MS_DELAY_HE NULL
#define OPTION_VAL_13_MS_DELAY_HE NULL
#define OPTION_VAL_14_MS_DELAY_HE NULL
#define OPTION_VAL_15_MS_DELAY_HE NULL
#define OPTION_VAL_16_MS_DELAY_HE NULL
#define OPTION_VAL_17_MS_DELAY_HE NULL
#define OPTION_VAL_18_MS_DELAY_HE NULL
#define OPTION_VAL_19_MS_DELAY_HE NULL
#define OPTION_VAL_20_MS_DELAY_HE NULL
#define OPTION_VAL_21_MS_DELAY_HE NULL
#define OPTION_VAL_22_MS_DELAY_HE NULL
#define OPTION_VAL_23_MS_DELAY_HE NULL
#define OPTION_VAL_24_MS_DELAY_HE NULL
#define OPTION_VAL_25_MS_DELAY_HE NULL
#define OPTION_VAL_26_MS_DELAY_HE NULL
#define OPTION_VAL_27_MS_DELAY_HE NULL
#define OPTION_VAL_28_MS_DELAY_HE NULL
#define OPTION_VAL_29_MS_DELAY_HE NULL
#define OPTION_VAL_30_MS_DELAY_HE NULL
#define OPTION_VAL_31_MS_DELAY_HE NULL
#define OPTION_VAL_32_MS_DELAY_HE NULL
#define FCEUMM_SWAPDUTY_LABEL_HE NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_HE NULL
#define FCEUMM_SWAPDUTY_INFO_0_HE NULL
#define FCEUMM_SNDVOLUME_LABEL_HE NULL
#define FCEUMM_SNDVOLUME_INFO_0_HE NULL
#define OPTION_VAL_0_HE NULL
#define OPTION_VAL_1_HE NULL
#define OPTION_VAL_2_HE NULL
#define OPTION_VAL_3_HE NULL
#define OPTION_VAL_4_HE NULL
#define OPTION_VAL_5_HE NULL
#define OPTION_VAL_6_HE NULL
#define OPTION_VAL_7_HE NULL
#define OPTION_VAL_8_HE NULL
#define OPTION_VAL_9_HE NULL
#define OPTION_VAL_10_HE NULL
#define FCEUMM_APU_1_LABEL_HE NULL
#define FCEUMM_APU_1_LABEL_CAT_HE NULL
#define FCEUMM_APU_1_INFO_0_HE NULL
#define FCEUMM_APU_2_LABEL_HE NULL
#define FCEUMM_APU_2_LABEL_CAT_HE NULL
#define FCEUMM_APU_2_INFO_0_HE NULL
#define FCEUMM_APU_3_LABEL_HE NULL
#define FCEUMM_APU_3_LABEL_CAT_HE NULL
#define FCEUMM_APU_3_INFO_0_HE NULL
#define FCEUMM_APU_4_LABEL_HE NULL
#define FCEUMM_APU_4_LABEL_CAT_HE NULL
#define FCEUMM_APU_4_INFO_0_HE NULL
#define FCEUMM_APU_5_LABEL_HE NULL
#define FCEUMM_APU_5_LABEL_CAT_HE NULL
#define FCEUMM_APU_5_INFO_0_HE NULL
#define FCEUMM_TURBO_ENABLE_LABEL_HE NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_HE NULL
#define OPTION_VAL_NONE_HE NULL
#define OPTION_VAL_PLAYER_1_HE NULL
#define OPTION_VAL_PLAYER_2_HE NULL
#define OPTION_VAL_BOTH_HE NULL
#define FCEUMM_TURBO_DELAY_LABEL_HE NULL
#define FCEUMM_TURBO_DELAY_INFO_0_HE NULL
#define FCEUMM_ZAPPER_MODE_LABEL_HE NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_HE NULL
#define OPTION_VAL_CLIGHTGUN_HE NULL
#define OPTION_VAL_STLIGHTGUN_HE NULL
#define OPTION_VAL_TOUCHSCREEN_HE NULL
#define OPTION_VAL_MOUSE_HE NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_HE NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_HE NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_HE NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_HE NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_HE NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_HE NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_HE NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_HE NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_HE NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_HE NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_HE NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_HE NULL
#define FCEUMM_OVERCLOCKING_LABEL_HE NULL
#define FCEUMM_OVERCLOCKING_INFO_0_HE NULL
#define OPTION_VAL_2X_POSTRENDER_HE NULL
#define OPTION_VAL_2X_VBLANK_HE NULL
#define FCEUMM_RAMSTATE_LABEL_HE NULL
#define FCEUMM_RAMSTATE_INFO_0_HE NULL
#define OPTION_VAL_FILL_FF_HE NULL
#define OPTION_VAL_FILL_00_HE NULL
#define OPTION_VAL_RANDOM_HE NULL

struct retro_core_option_v2_category option_cats_he[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_HE,
      CATEGORY_SYSTEM_INFO_0_HE
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_HE,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_HE
#else
      CATEGORY_VIDEO_INFO_1_HE
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_HE,
      CATEGORY_AUDIO_INFO_0_HE
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_HE,
      CATEGORY_INPUT_INFO_0_HE
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_HE,
      CATEGORY_HACKS_INFO_0_HE
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_HE,
      CATEGORY_DIP_SWITCH_INFO_0_HE
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_he[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_HE,
      NULL,
      FCEUMM_REGION_INFO_0_HE,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_HE },
         { "NTSC",  OPTION_VAL_NTSC_HE },
         { "PAL",   OPTION_VAL_PAL_HE },
         { "Dendy", OPTION_VAL_DENDY_HE },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_HE,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_HE,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_HE,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_HE,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_HE,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_HE,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_HE,
      NULL,
      FCEUMM_ASPECT_INFO_0_HE,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_HE },
         { "4:3",     OPTION_VAL_4_3_HE },
         { "PP",      OPTION_VAL_PP_HE },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_HE,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_HE,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_HE,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_HE,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_HE,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_HE,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_HE,
      NULL,
      FCEUMM_PALETTE_INFO_0_HE,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_HE },
         { "asqrealc",             OPTION_VAL_ASQREALC_HE },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_HE },
         { "rgb",                  OPTION_VAL_RGB_HE },
         { "yuv-v3",               OPTION_VAL_YUV_V3_HE },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_HE },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_HE },
         { "pal",                  OPTION_VAL_PAL_HE },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_HE },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_HE },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_HE },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_HE },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_HE },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_HE },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_HE },
         { "nescap",               OPTION_VAL_NESCAP_HE },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_HE },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_HE },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_HE },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_HE },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_HE },
         { "raw",                  OPTION_VAL_RAW_HE },
         { "custom",               OPTION_VAL_CUSTOM_HE },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_HE,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_HE,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_HE },
         { "svideo",     OPTION_VAL_SVIDEO_HE },
         { "rgb",        OPTION_VAL_RGB_O9_HE },
         { "monochrome", OPTION_VAL_MONOCHROME_HE },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_HE,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_HE,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_HE },
         { "High",      OPTION_VAL_HIGH_HE },
         { "Very High", OPTION_VAL_VERY_HIGH_HE },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_HE,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_HE,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_HE,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_HE,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_HE },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_HE },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_HE },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_HE },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_HE },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_HE },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_HE },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_HE },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_HE },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_HE },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_HE },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_HE },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_HE },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_HE },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_HE },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_HE },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_HE },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_HE },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_HE },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_HE },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_HE },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_HE },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_HE },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_HE },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_HE },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_HE },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_HE },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_HE },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_HE },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_HE },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_HE },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_HE },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_HE,
      FCEUMM_SWAPDUTY_LABEL_CAT_HE,
      FCEUMM_SWAPDUTY_INFO_0_HE,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_HE,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_HE,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_HE },
         { "1",  OPTION_VAL_1_HE },
         { "2",  OPTION_VAL_2_HE },
         { "3",  OPTION_VAL_3_HE },
         { "4",  OPTION_VAL_4_HE },
         { "5",  OPTION_VAL_5_HE },
         { "6",  OPTION_VAL_6_HE },
         { "7",  OPTION_VAL_7_HE },
         { "8",  OPTION_VAL_8_HE },
         { "9",  OPTION_VAL_9_HE },
         { "10", OPTION_VAL_10_HE },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_HE,
      FCEUMM_APU_1_LABEL_CAT_HE,
      FCEUMM_APU_1_INFO_0_HE,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_HE,
      FCEUMM_APU_2_LABEL_CAT_HE,
      FCEUMM_APU_2_INFO_0_HE,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_HE,
      FCEUMM_APU_3_LABEL_CAT_HE,
      FCEUMM_APU_3_INFO_0_HE,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_HE,
      FCEUMM_APU_4_LABEL_CAT_HE,
      FCEUMM_APU_4_INFO_0_HE,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_HE,
      FCEUMM_APU_5_LABEL_CAT_HE,
      FCEUMM_APU_5_INFO_0_HE,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_HE,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_HE,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_HE },
         { "Player 1", OPTION_VAL_PLAYER_1_HE },
         { "Player 2", OPTION_VAL_PLAYER_2_HE },
         { "Both",     OPTION_VAL_BOTH_HE },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_HE,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_HE,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_HE,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_HE,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_HE },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_HE },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_HE },
         { "mouse",       OPTION_VAL_MOUSE_HE },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_HE,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_HE,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_HE,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_HE,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_HE,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_HE,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_HE,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_HE,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_HE,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_HE,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_HE,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_HE,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_HE,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_HE,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_HE },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_HE },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_HE,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_HE,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_HE },
         { "fill $00", OPTION_VAL_FILL_00_HE },
         { "random",   OPTION_VAL_RANDOM_HE },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_he = {
   option_cats_he,
   option_defs_he
};

/* RETRO_LANGUAGE_HR */

#define CATEGORY_SYSTEM_LABEL_HR NULL
#define CATEGORY_SYSTEM_INFO_0_HR NULL
#define CATEGORY_VIDEO_LABEL_HR NULL
#define CATEGORY_VIDEO_INFO_0_HR NULL
#define CATEGORY_VIDEO_INFO_1_HR NULL
#define CATEGORY_AUDIO_LABEL_HR NULL
#define CATEGORY_AUDIO_INFO_0_HR NULL
#define CATEGORY_INPUT_LABEL_HR NULL
#define CATEGORY_INPUT_INFO_0_HR NULL
#define CATEGORY_HACKS_LABEL_HR NULL
#define CATEGORY_HACKS_INFO_0_HR NULL
#define CATEGORY_DIP_SWITCH_LABEL_HR NULL
#define CATEGORY_DIP_SWITCH_INFO_0_HR NULL
#define FCEUMM_REGION_LABEL_HR "Regija"
#define FCEUMM_REGION_INFO_0_HR NULL
#define OPTION_VAL_AUTO_HR NULL
#define OPTION_VAL_NTSC_HR NULL
#define OPTION_VAL_PAL_HR NULL
#define OPTION_VAL_DENDY_HR NULL
#define FCEUMM_GAME_GENIE_LABEL_HR NULL
#define FCEUMM_GAME_GENIE_INFO_0_HR NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_HR NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_HR NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_HR NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_HR NULL
#define FCEUMM_ASPECT_LABEL_HR "Omjer slike"
#define FCEUMM_ASPECT_INFO_0_HR NULL
#define OPTION_VAL_8_7_PAR_HR NULL
#define OPTION_VAL_4_3_HR NULL
#define OPTION_VAL_PP_HR NULL
#define FCEUMM_OVERSCAN_LABEL_HR NULL
#define FCEUMM_OVERSCAN_INFO_0_HR NULL
#define FCEUMM_OVERSCAN_H_LABEL_HR NULL
#define FCEUMM_OVERSCAN_H_INFO_0_HR NULL
#define FCEUMM_OVERSCAN_V_LABEL_HR NULL
#define FCEUMM_OVERSCAN_V_INFO_0_HR NULL
#define FCEUMM_PALETTE_LABEL_HR NULL
#define FCEUMM_PALETTE_INFO_0_HR NULL
#define OPTION_VAL_DEFAULT_HR NULL
#define OPTION_VAL_ASQREALC_HR NULL
#define OPTION_VAL_NINTENDO_VC_HR NULL
#define OPTION_VAL_RGB_HR NULL
#define OPTION_VAL_YUV_V3_HR NULL
#define OPTION_VAL_UNSATURATED_FINAL_HR NULL
#define OPTION_VAL_SONY_CXA2025AS_US_HR NULL
#define OPTION_VAL_BMF_FINAL2_HR NULL
#define OPTION_VAL_BMF_FINAL3_HR NULL
#define OPTION_VAL_SMOOTH_FBX_HR NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_HR NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_HR NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_HR NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_HR NULL
#define OPTION_VAL_NESCAP_HR NULL
#define OPTION_VAL_WAVEBEAM_HR NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_HR NULL
#define OPTION_VAL_MAGNUM_FBX_HR NULL
#define OPTION_VAL_SMOOTH_V2_FBX_HR NULL
#define OPTION_VAL_NES_CLASSIC_FBX_HR NULL
#define OPTION_VAL_RAW_HR NULL
#define OPTION_VAL_CUSTOM_HR NULL
#define FCEUMM_NTSC_FILTER_LABEL_HR NULL
#define FCEUMM_NTSC_FILTER_INFO_0_HR NULL
#define OPTION_VAL_COMPOSITE_HR NULL
#define OPTION_VAL_SVIDEO_HR NULL
#define OPTION_VAL_RGB_O9_HR NULL
#define OPTION_VAL_MONOCHROME_HR NULL
#define FCEUMM_SNDQUALITY_LABEL_HR NULL
#define FCEUMM_SNDQUALITY_INFO_0_HR NULL
#define OPTION_VAL_LOW_HR NULL
#define OPTION_VAL_HIGH_HR NULL
#define OPTION_VAL_VERY_HIGH_HR NULL
#define FCEUMM_SNDLOWPASS_LABEL_HR NULL
#define FCEUMM_SNDLOWPASS_INFO_0_HR NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_HR NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_HR NULL
#define OPTION_VAL_01_MS_DELAY_HR NULL
#define OPTION_VAL_02_MS_DELAY_HR NULL
#define OPTION_VAL_03_MS_DELAY_HR NULL
#define OPTION_VAL_04_MS_DELAY_HR NULL
#define OPTION_VAL_05_MS_DELAY_HR NULL
#define OPTION_VAL_06_MS_DELAY_HR NULL
#define OPTION_VAL_07_MS_DELAY_HR NULL
#define OPTION_VAL_08_MS_DELAY_HR NULL
#define OPTION_VAL_09_MS_DELAY_HR NULL
#define OPTION_VAL_10_MS_DELAY_HR NULL
#define OPTION_VAL_11_MS_DELAY_HR NULL
#define OPTION_VAL_12_MS_DELAY_HR NULL
#define OPTION_VAL_13_MS_DELAY_HR NULL
#define OPTION_VAL_14_MS_DELAY_HR NULL
#define OPTION_VAL_15_MS_DELAY_HR NULL
#define OPTION_VAL_16_MS_DELAY_HR NULL
#define OPTION_VAL_17_MS_DELAY_HR NULL
#define OPTION_VAL_18_MS_DELAY_HR NULL
#define OPTION_VAL_19_MS_DELAY_HR NULL
#define OPTION_VAL_20_MS_DELAY_HR NULL
#define OPTION_VAL_21_MS_DELAY_HR NULL
#define OPTION_VAL_22_MS_DELAY_HR NULL
#define OPTION_VAL_23_MS_DELAY_HR NULL
#define OPTION_VAL_24_MS_DELAY_HR NULL
#define OPTION_VAL_25_MS_DELAY_HR NULL
#define OPTION_VAL_26_MS_DELAY_HR NULL
#define OPTION_VAL_27_MS_DELAY_HR NULL
#define OPTION_VAL_28_MS_DELAY_HR NULL
#define OPTION_VAL_29_MS_DELAY_HR NULL
#define OPTION_VAL_30_MS_DELAY_HR NULL
#define OPTION_VAL_31_MS_DELAY_HR NULL
#define OPTION_VAL_32_MS_DELAY_HR NULL
#define FCEUMM_SWAPDUTY_LABEL_HR NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_HR NULL
#define FCEUMM_SWAPDUTY_INFO_0_HR NULL
#define FCEUMM_SNDVOLUME_LABEL_HR NULL
#define FCEUMM_SNDVOLUME_INFO_0_HR NULL
#define OPTION_VAL_0_HR NULL
#define OPTION_VAL_1_HR NULL
#define OPTION_VAL_2_HR NULL
#define OPTION_VAL_3_HR NULL
#define OPTION_VAL_4_HR NULL
#define OPTION_VAL_5_HR NULL
#define OPTION_VAL_6_HR NULL
#define OPTION_VAL_7_HR NULL
#define OPTION_VAL_8_HR NULL
#define OPTION_VAL_9_HR NULL
#define OPTION_VAL_10_HR NULL
#define FCEUMM_APU_1_LABEL_HR NULL
#define FCEUMM_APU_1_LABEL_CAT_HR NULL
#define FCEUMM_APU_1_INFO_0_HR NULL
#define FCEUMM_APU_2_LABEL_HR NULL
#define FCEUMM_APU_2_LABEL_CAT_HR NULL
#define FCEUMM_APU_2_INFO_0_HR NULL
#define FCEUMM_APU_3_LABEL_HR NULL
#define FCEUMM_APU_3_LABEL_CAT_HR NULL
#define FCEUMM_APU_3_INFO_0_HR NULL
#define FCEUMM_APU_4_LABEL_HR NULL
#define FCEUMM_APU_4_LABEL_CAT_HR NULL
#define FCEUMM_APU_4_INFO_0_HR NULL
#define FCEUMM_APU_5_LABEL_HR NULL
#define FCEUMM_APU_5_LABEL_CAT_HR NULL
#define FCEUMM_APU_5_INFO_0_HR NULL
#define FCEUMM_TURBO_ENABLE_LABEL_HR NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_HR NULL
#define OPTION_VAL_NONE_HR NULL
#define OPTION_VAL_PLAYER_1_HR NULL
#define OPTION_VAL_PLAYER_2_HR NULL
#define OPTION_VAL_BOTH_HR NULL
#define FCEUMM_TURBO_DELAY_LABEL_HR NULL
#define FCEUMM_TURBO_DELAY_INFO_0_HR NULL
#define FCEUMM_ZAPPER_MODE_LABEL_HR NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_HR NULL
#define OPTION_VAL_CLIGHTGUN_HR NULL
#define OPTION_VAL_STLIGHTGUN_HR NULL
#define OPTION_VAL_TOUCHSCREEN_HR NULL
#define OPTION_VAL_MOUSE_HR NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_HR NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_HR NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_HR NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_HR NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_HR NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_HR NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_HR NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_HR NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_HR NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_HR NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_HR NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_HR NULL
#define FCEUMM_OVERCLOCKING_LABEL_HR NULL
#define FCEUMM_OVERCLOCKING_INFO_0_HR NULL
#define OPTION_VAL_2X_POSTRENDER_HR NULL
#define OPTION_VAL_2X_VBLANK_HR NULL
#define FCEUMM_RAMSTATE_LABEL_HR NULL
#define FCEUMM_RAMSTATE_INFO_0_HR NULL
#define OPTION_VAL_FILL_FF_HR NULL
#define OPTION_VAL_FILL_00_HR NULL
#define OPTION_VAL_RANDOM_HR NULL

struct retro_core_option_v2_category option_cats_hr[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_HR,
      CATEGORY_SYSTEM_INFO_0_HR
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_HR,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_HR
#else
      CATEGORY_VIDEO_INFO_1_HR
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_HR,
      CATEGORY_AUDIO_INFO_0_HR
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_HR,
      CATEGORY_INPUT_INFO_0_HR
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_HR,
      CATEGORY_HACKS_INFO_0_HR
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_HR,
      CATEGORY_DIP_SWITCH_INFO_0_HR
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_hr[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_HR,
      NULL,
      FCEUMM_REGION_INFO_0_HR,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_HR },
         { "NTSC",  OPTION_VAL_NTSC_HR },
         { "PAL",   OPTION_VAL_PAL_HR },
         { "Dendy", OPTION_VAL_DENDY_HR },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_HR,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_HR,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_HR,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_HR,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_HR,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_HR,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_HR,
      NULL,
      FCEUMM_ASPECT_INFO_0_HR,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_HR },
         { "4:3",     OPTION_VAL_4_3_HR },
         { "PP",      OPTION_VAL_PP_HR },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_HR,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_HR,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_HR,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_HR,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_HR,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_HR,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_HR,
      NULL,
      FCEUMM_PALETTE_INFO_0_HR,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_HR },
         { "asqrealc",             OPTION_VAL_ASQREALC_HR },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_HR },
         { "rgb",                  OPTION_VAL_RGB_HR },
         { "yuv-v3",               OPTION_VAL_YUV_V3_HR },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_HR },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_HR },
         { "pal",                  OPTION_VAL_PAL_HR },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_HR },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_HR },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_HR },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_HR },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_HR },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_HR },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_HR },
         { "nescap",               OPTION_VAL_NESCAP_HR },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_HR },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_HR },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_HR },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_HR },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_HR },
         { "raw",                  OPTION_VAL_RAW_HR },
         { "custom",               OPTION_VAL_CUSTOM_HR },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_HR,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_HR,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_HR },
         { "svideo",     OPTION_VAL_SVIDEO_HR },
         { "rgb",        OPTION_VAL_RGB_O9_HR },
         { "monochrome", OPTION_VAL_MONOCHROME_HR },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_HR,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_HR,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_HR },
         { "High",      OPTION_VAL_HIGH_HR },
         { "Very High", OPTION_VAL_VERY_HIGH_HR },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_HR,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_HR,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_HR,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_HR,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_HR },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_HR },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_HR },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_HR },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_HR },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_HR },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_HR },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_HR },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_HR },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_HR },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_HR },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_HR },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_HR },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_HR },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_HR },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_HR },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_HR },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_HR },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_HR },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_HR },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_HR },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_HR },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_HR },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_HR },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_HR },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_HR },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_HR },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_HR },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_HR },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_HR },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_HR },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_HR },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_HR,
      FCEUMM_SWAPDUTY_LABEL_CAT_HR,
      FCEUMM_SWAPDUTY_INFO_0_HR,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_HR,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_HR,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_HR },
         { "1",  OPTION_VAL_1_HR },
         { "2",  OPTION_VAL_2_HR },
         { "3",  OPTION_VAL_3_HR },
         { "4",  OPTION_VAL_4_HR },
         { "5",  OPTION_VAL_5_HR },
         { "6",  OPTION_VAL_6_HR },
         { "7",  OPTION_VAL_7_HR },
         { "8",  OPTION_VAL_8_HR },
         { "9",  OPTION_VAL_9_HR },
         { "10", OPTION_VAL_10_HR },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_HR,
      FCEUMM_APU_1_LABEL_CAT_HR,
      FCEUMM_APU_1_INFO_0_HR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_HR,
      FCEUMM_APU_2_LABEL_CAT_HR,
      FCEUMM_APU_2_INFO_0_HR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_HR,
      FCEUMM_APU_3_LABEL_CAT_HR,
      FCEUMM_APU_3_INFO_0_HR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_HR,
      FCEUMM_APU_4_LABEL_CAT_HR,
      FCEUMM_APU_4_INFO_0_HR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_HR,
      FCEUMM_APU_5_LABEL_CAT_HR,
      FCEUMM_APU_5_INFO_0_HR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_HR,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_HR,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_HR },
         { "Player 1", OPTION_VAL_PLAYER_1_HR },
         { "Player 2", OPTION_VAL_PLAYER_2_HR },
         { "Both",     OPTION_VAL_BOTH_HR },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_HR,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_HR,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_HR,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_HR,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_HR },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_HR },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_HR },
         { "mouse",       OPTION_VAL_MOUSE_HR },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_HR,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_HR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_HR,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_HR,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_HR,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_HR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_HR,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_HR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_HR,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_HR,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_HR,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_HR,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_HR,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_HR,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_HR },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_HR },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_HR,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_HR,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_HR },
         { "fill $00", OPTION_VAL_FILL_00_HR },
         { "random",   OPTION_VAL_RANDOM_HR },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_hr = {
   option_cats_hr,
   option_defs_hr
};

/* RETRO_LANGUAGE_HU */

#define CATEGORY_SYSTEM_LABEL_HU "Rendszer"
#define CATEGORY_SYSTEM_INFO_0_HU NULL
#define CATEGORY_VIDEO_LABEL_HU "Videó"
#define CATEGORY_VIDEO_INFO_0_HU NULL
#define CATEGORY_VIDEO_INFO_1_HU NULL
#define CATEGORY_AUDIO_LABEL_HU "Hang"
#define CATEGORY_AUDIO_INFO_0_HU NULL
#define CATEGORY_INPUT_LABEL_HU "Bevitel"
#define CATEGORY_INPUT_INFO_0_HU NULL
#define CATEGORY_HACKS_LABEL_HU "Emulációs trükkök"
#define CATEGORY_HACKS_INFO_0_HU "A processzor túlhajtás és az emuláció pontosságának beállításai, amelyek befolyásolják az alacsonyszintű teljesítményt és a kompatibilitást."
#define CATEGORY_DIP_SWITCH_LABEL_HU "DIP kapcsolók"
#define CATEGORY_DIP_SWITCH_INFO_0_HU NULL
#define FCEUMM_REGION_LABEL_HU "Terület"
#define FCEUMM_REGION_INFO_0_HU "A mag mindenképp az NTSC, PAL, vagy Dendy régiós időzítéseket használja."
#define OPTION_VAL_AUTO_HU "Automatikus"
#define OPTION_VAL_NTSC_HU NULL
#define OPTION_VAL_PAL_HU NULL
#define OPTION_VAL_DENDY_HU NULL
#define FCEUMM_GAME_GENIE_LABEL_HU "Game Genie add-on (újraindítás szükséges)"
#define FCEUMM_GAME_GENIE_INFO_0_HU "A Game Genie add-on cartridge emulálása, amivel csalókódok adhatók be játék indításakor. A Game Genie ROM-ja (gamegenie.nes) meg kell legyen a frontend rendszerkönyvtárában. FDS vagy arcade tartalmakhoz hatástalan."
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_HU "Haladó rendszerbeállítások"
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_HU "A rendszer haladó beállításai és hangolása."
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_HU "Haladó hangbeállítások"
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_HU "A hang haladó beállításai és hangolása."
#define FCEUMM_ASPECT_LABEL_HU "Képarány"
#define FCEUMM_ASPECT_INFO_0_HU "A kívánt képarány."
#define OPTION_VAL_8_7_PAR_HU NULL
#define OPTION_VAL_4_3_HU NULL
#define OPTION_VAL_PP_HU NULL
#define FCEUMM_OVERSCAN_LABEL_HU "Overscan levágása"
#define FCEUMM_OVERSCAN_INFO_0_HU "A képernyő minden oldaláról levág 8 pixelnyi overscan területet."
#define FCEUMM_OVERSCAN_H_LABEL_HU "Vízszintes overscan levágása"
#define FCEUMM_OVERSCAN_H_INFO_0_HU "A képernyő bal és jobb oldaláról levág 8-8 pixelt, a CRT televíziókon megszokott overscan szimulálásaként."
#define FCEUMM_OVERSCAN_V_LABEL_HU "Függőleges overscan levágása"
#define FCEUMM_OVERSCAN_V_INFO_0_HU "A képernyő alsó és felső széléről levág 8-8 pixelt, a CRT televíziókon megszokott overscan szimulálásaként."
#define FCEUMM_PALETTE_LABEL_HU "Színpaletta"
#define FCEUMM_PALETTE_INFO_0_HU "Előre generált paletták, egyedi 64x3-as paletta fájlból, vagy nyers formátum kiválasztása (nes-decoder shader szükséges hozzá)."
#define OPTION_VAL_DEFAULT_HU "Alapértelmezett"
#define OPTION_VAL_ASQREALC_HU NULL
#define OPTION_VAL_NINTENDO_VC_HU NULL
#define OPTION_VAL_RGB_HU NULL
#define OPTION_VAL_YUV_V3_HU NULL
#define OPTION_VAL_UNSATURATED_FINAL_HU NULL
#define OPTION_VAL_SONY_CXA2025AS_US_HU NULL
#define OPTION_VAL_BMF_FINAL2_HU NULL
#define OPTION_VAL_BMF_FINAL3_HU NULL
#define OPTION_VAL_SMOOTH_FBX_HU NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_HU NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_HU NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_HU NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_HU NULL
#define OPTION_VAL_NESCAP_HU NULL
#define OPTION_VAL_WAVEBEAM_HU NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_HU NULL
#define OPTION_VAL_MAGNUM_FBX_HU NULL
#define OPTION_VAL_SMOOTH_V2_FBX_HU NULL
#define OPTION_VAL_NES_CLASSIC_FBX_HU NULL
#define OPTION_VAL_RAW_HU "Nyers"
#define OPTION_VAL_CUSTOM_HU "Egyedi"
#define FCEUMM_NTSC_FILTER_LABEL_HU "NTSC szűrő"
#define FCEUMM_NTSC_FILTER_INFO_0_HU NULL
#define OPTION_VAL_COMPOSITE_HU "Kompozit"
#define OPTION_VAL_SVIDEO_HU NULL
#define OPTION_VAL_RGB_O9_HU NULL
#define OPTION_VAL_MONOCHROME_HU "Egyszínű"
#define FCEUMM_SNDQUALITY_LABEL_HU "Hangminőség"
#define FCEUMM_SNDQUALITY_INFO_0_HU "Jobb hangminőség engedélyezése. Növeli a teljesítményigényt."
#define OPTION_VAL_LOW_HU "Alacsony"
#define OPTION_VAL_HIGH_HU "Magas"
#define OPTION_VAL_VERY_HIGH_HU "Nagyon magas"
#define FCEUMM_SNDLOWPASS_LABEL_HU NULL
#define FCEUMM_SNDLOWPASS_INFO_0_HU NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_HU NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_HU NULL
#define OPTION_VAL_01_MS_DELAY_HU NULL
#define OPTION_VAL_02_MS_DELAY_HU NULL
#define OPTION_VAL_03_MS_DELAY_HU NULL
#define OPTION_VAL_04_MS_DELAY_HU NULL
#define OPTION_VAL_05_MS_DELAY_HU NULL
#define OPTION_VAL_06_MS_DELAY_HU NULL
#define OPTION_VAL_07_MS_DELAY_HU NULL
#define OPTION_VAL_08_MS_DELAY_HU NULL
#define OPTION_VAL_09_MS_DELAY_HU NULL
#define OPTION_VAL_10_MS_DELAY_HU NULL
#define OPTION_VAL_11_MS_DELAY_HU NULL
#define OPTION_VAL_12_MS_DELAY_HU NULL
#define OPTION_VAL_13_MS_DELAY_HU NULL
#define OPTION_VAL_14_MS_DELAY_HU NULL
#define OPTION_VAL_15_MS_DELAY_HU NULL
#define OPTION_VAL_16_MS_DELAY_HU NULL
#define OPTION_VAL_17_MS_DELAY_HU NULL
#define OPTION_VAL_18_MS_DELAY_HU NULL
#define OPTION_VAL_19_MS_DELAY_HU NULL
#define OPTION_VAL_20_MS_DELAY_HU NULL
#define OPTION_VAL_21_MS_DELAY_HU NULL
#define OPTION_VAL_22_MS_DELAY_HU NULL
#define OPTION_VAL_23_MS_DELAY_HU NULL
#define OPTION_VAL_24_MS_DELAY_HU NULL
#define OPTION_VAL_25_MS_DELAY_HU NULL
#define OPTION_VAL_26_MS_DELAY_HU NULL
#define OPTION_VAL_27_MS_DELAY_HU NULL
#define OPTION_VAL_28_MS_DELAY_HU NULL
#define OPTION_VAL_29_MS_DELAY_HU NULL
#define OPTION_VAL_30_MS_DELAY_HU NULL
#define OPTION_VAL_31_MS_DELAY_HU NULL
#define OPTION_VAL_32_MS_DELAY_HU NULL
#define FCEUMM_SWAPDUTY_LABEL_HU "Hang kitöltési tényezők felcserélése"
#define FCEUMM_SWAPDUTY_LABEL_CAT_HU "Kitöltési tényezők felcserélése"
#define FCEUMM_SWAPDUTY_INFO_0_HU "A famiclone-ok hangzásának szimulálása, ahol a pulzus hullámforma csatornáknak a kitöltési bitjei fordítva álltak."
#define FCEUMM_SNDVOLUME_LABEL_HU "Fő hangerő"
#define FCEUMM_SNDVOLUME_INFO_0_HU "A fő hangerő szintje."
#define OPTION_VAL_0_HU NULL
#define OPTION_VAL_1_HU NULL
#define OPTION_VAL_2_HU NULL
#define OPTION_VAL_3_HU NULL
#define OPTION_VAL_4_HU NULL
#define OPTION_VAL_5_HU NULL
#define OPTION_VAL_6_HU NULL
#define OPTION_VAL_7_HU NULL
#define OPTION_VAL_8_HU NULL
#define OPTION_VAL_9_HU NULL
#define OPTION_VAL_10_HU NULL
#define FCEUMM_APU_1_LABEL_HU "1. hangcsatorna (1. négyszögjel)"
#define FCEUMM_APU_1_LABEL_CAT_HU "1. csatorna (1. négyszögjel)"
#define FCEUMM_APU_1_INFO_0_HU "A pulzus hullámforma generátor 1. hangkimenetének engedélyezése."
#define FCEUMM_APU_2_LABEL_HU "2. hangcsatorna (2. négyszögjel)"
#define FCEUMM_APU_2_LABEL_CAT_HU "2. csatorna (2. négyszögjel)"
#define FCEUMM_APU_2_INFO_0_HU "A pulzus hullámforma generátor 2. hangkimenetének engedélyezése."
#define FCEUMM_APU_3_LABEL_HU "3. hangcsatorna (háromszögjel)"
#define FCEUMM_APU_3_LABEL_CAT_HU "3. csatorna (háromszögjel)"
#define FCEUMM_APU_3_INFO_0_HU "A háromszög hullámforma generátor hangkimenetének engedélyezése."
#define FCEUMM_APU_4_LABEL_HU "4. hangcsatorna (zaj)"
#define FCEUMM_APU_4_LABEL_CAT_HU "4. csatorna (zaj)"
#define FCEUMM_APU_4_INFO_0_HU "A zajgenerátor hangkimenetének engedélyezése."
#define FCEUMM_APU_5_LABEL_HU "5. hangcsatorna (PCM)"
#define FCEUMM_APU_5_LABEL_CAT_HU "5. csatorna (PCM)"
#define FCEUMM_APU_5_INFO_0_HU "A delta modulációs csatorna hangkimenetének engedélyezése."
#define FCEUMM_TURBO_ENABLE_LABEL_HU "Turbó engedélyezése"
#define FCEUMM_TURBO_ENABLE_INFO_0_HU "A turbó gombok engedélyezése."
#define OPTION_VAL_NONE_HU "Nincs"
#define OPTION_VAL_PLAYER_1_HU "1. játékos"
#define OPTION_VAL_PLAYER_2_HU "2. játékos"
#define OPTION_VAL_BOTH_HU "Mindkettő"
#define FCEUMM_TURBO_DELAY_LABEL_HU "Turbó késleltetése (képkockákban)"
#define FCEUMM_TURBO_DELAY_INFO_0_HU "A turbó gombok ismétlési gyorsasága képkockában."
#define FCEUMM_ZAPPER_MODE_LABEL_HU "Zapper mód"
#define FCEUMM_ZAPPER_MODE_INFO_0_HU NULL
#define OPTION_VAL_CLIGHTGUN_HU NULL
#define OPTION_VAL_STLIGHTGUN_HU NULL
#define OPTION_VAL_TOUCHSCREEN_HU "Érintőképernyő"
#define OPTION_VAL_MOUSE_HU "Egér"
#define FCEUMM_SHOW_CROSSHAIR_LABEL_HU "A zapper célkeresztje jelenjen meg"
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_HU NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_HU "Zapper tűrése"
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_HU "Hány pixel eltérés legyen megengedett a célterülettől."
#define FCEUMM_ZAPPER_TRIGGER_LABEL_HU NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_HU NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_HU NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_HU NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_HU "Ellentétes irányok engedélyezése"
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_HU "A fel+le vagy jobbra+balra gombok egyidejű megnyomásának engedélyezése, amely néhány játékban különféle effekteket idéz elő."
#define FCEUMM_NOSPRITELIMIT_LABEL_HU "Nincs sprite korlátozás"
#define FCEUMM_NOSPRITELIMIT_INFO_0_HU "Megszünteti a hardver soronkénti 8 sprite maximum korlátozását. Csökkenti a villódzást, de grafikai hibákat okozhat, mivel néhány játék a hardverkorlátozást speciális effektekhez használja fel."
#define FCEUMM_OVERCLOCKING_LABEL_HU "Túlhajtás"
#define FCEUMM_OVERCLOCKING_INFO_0_HU "A túlhajtás engedélyezése, amely csökkentheti a lassulást bizonyos játékokban. A postrender mód inkább kompatibilis a játékokkal, a Vblank hatásosabb olyan játékoknál, mint a Contra Force."
#define OPTION_VAL_2X_POSTRENDER_HU NULL
#define OPTION_VAL_2X_VBLANK_HU NULL
#define FCEUMM_RAMSTATE_LABEL_HU "RAM feltöltése bekapcsoláskor (újraindítás szükséges)"
#define FCEUMM_RAMSTATE_INFO_0_HU "A RAM értékei bekapcsoláskor. Néhány játék hagyatkozik erre például véletlenszám generáláshoz."
#define OPTION_VAL_FILL_FF_HU NULL
#define OPTION_VAL_FILL_00_HU NULL
#define OPTION_VAL_RANDOM_HU "Véletlenszerű"

struct retro_core_option_v2_category option_cats_hu[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_HU,
      CATEGORY_SYSTEM_INFO_0_HU
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_HU,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_HU
#else
      CATEGORY_VIDEO_INFO_1_HU
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_HU,
      CATEGORY_AUDIO_INFO_0_HU
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_HU,
      CATEGORY_INPUT_INFO_0_HU
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_HU,
      CATEGORY_HACKS_INFO_0_HU
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_HU,
      CATEGORY_DIP_SWITCH_INFO_0_HU
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_hu[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_HU,
      NULL,
      FCEUMM_REGION_INFO_0_HU,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_HU },
         { "NTSC",  OPTION_VAL_NTSC_HU },
         { "PAL",   OPTION_VAL_PAL_HU },
         { "Dendy", OPTION_VAL_DENDY_HU },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_HU,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_HU,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_HU,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_HU,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_HU,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_HU,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_HU,
      NULL,
      FCEUMM_ASPECT_INFO_0_HU,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_HU },
         { "4:3",     OPTION_VAL_4_3_HU },
         { "PP",      OPTION_VAL_PP_HU },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_HU,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_HU,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_HU,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_HU,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_HU,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_HU,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_HU,
      NULL,
      FCEUMM_PALETTE_INFO_0_HU,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_HU },
         { "asqrealc",             OPTION_VAL_ASQREALC_HU },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_HU },
         { "rgb",                  OPTION_VAL_RGB_HU },
         { "yuv-v3",               OPTION_VAL_YUV_V3_HU },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_HU },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_HU },
         { "pal",                  OPTION_VAL_PAL_HU },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_HU },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_HU },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_HU },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_HU },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_HU },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_HU },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_HU },
         { "nescap",               OPTION_VAL_NESCAP_HU },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_HU },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_HU },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_HU },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_HU },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_HU },
         { "raw",                  OPTION_VAL_RAW_HU },
         { "custom",               OPTION_VAL_CUSTOM_HU },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_HU,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_HU,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_HU },
         { "svideo",     OPTION_VAL_SVIDEO_HU },
         { "rgb",        OPTION_VAL_RGB_O9_HU },
         { "monochrome", OPTION_VAL_MONOCHROME_HU },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_HU,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_HU,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_HU },
         { "High",      OPTION_VAL_HIGH_HU },
         { "Very High", OPTION_VAL_VERY_HIGH_HU },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_HU,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_HU,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_HU,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_HU,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_HU },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_HU },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_HU },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_HU },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_HU },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_HU },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_HU },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_HU },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_HU },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_HU },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_HU },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_HU },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_HU },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_HU },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_HU },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_HU },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_HU },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_HU },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_HU },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_HU },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_HU },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_HU },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_HU },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_HU },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_HU },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_HU },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_HU },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_HU },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_HU },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_HU },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_HU },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_HU },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_HU,
      FCEUMM_SWAPDUTY_LABEL_CAT_HU,
      FCEUMM_SWAPDUTY_INFO_0_HU,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_HU,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_HU,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_HU },
         { "1",  OPTION_VAL_1_HU },
         { "2",  OPTION_VAL_2_HU },
         { "3",  OPTION_VAL_3_HU },
         { "4",  OPTION_VAL_4_HU },
         { "5",  OPTION_VAL_5_HU },
         { "6",  OPTION_VAL_6_HU },
         { "7",  OPTION_VAL_7_HU },
         { "8",  OPTION_VAL_8_HU },
         { "9",  OPTION_VAL_9_HU },
         { "10", OPTION_VAL_10_HU },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_HU,
      FCEUMM_APU_1_LABEL_CAT_HU,
      FCEUMM_APU_1_INFO_0_HU,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_HU,
      FCEUMM_APU_2_LABEL_CAT_HU,
      FCEUMM_APU_2_INFO_0_HU,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_HU,
      FCEUMM_APU_3_LABEL_CAT_HU,
      FCEUMM_APU_3_INFO_0_HU,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_HU,
      FCEUMM_APU_4_LABEL_CAT_HU,
      FCEUMM_APU_4_INFO_0_HU,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_HU,
      FCEUMM_APU_5_LABEL_CAT_HU,
      FCEUMM_APU_5_INFO_0_HU,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_HU,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_HU,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_HU },
         { "Player 1", OPTION_VAL_PLAYER_1_HU },
         { "Player 2", OPTION_VAL_PLAYER_2_HU },
         { "Both",     OPTION_VAL_BOTH_HU },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_HU,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_HU,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_HU,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_HU,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_HU },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_HU },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_HU },
         { "mouse",       OPTION_VAL_MOUSE_HU },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_HU,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_HU,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_HU,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_HU,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_HU,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_HU,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_HU,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_HU,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_HU,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_HU,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_HU,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_HU,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_HU,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_HU,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_HU },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_HU },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_HU,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_HU,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_HU },
         { "fill $00", OPTION_VAL_FILL_00_HU },
         { "random",   OPTION_VAL_RANDOM_HU },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_hu = {
   option_cats_hu,
   option_defs_hu
};

/* RETRO_LANGUAGE_ID */

#define CATEGORY_SYSTEM_LABEL_ID "Sistem"
#define CATEGORY_SYSTEM_INFO_0_ID NULL
#define CATEGORY_VIDEO_LABEL_ID NULL
#define CATEGORY_VIDEO_INFO_0_ID NULL
#define CATEGORY_VIDEO_INFO_1_ID NULL
#define CATEGORY_AUDIO_LABEL_ID "Suara"
#define CATEGORY_AUDIO_INFO_0_ID NULL
#define CATEGORY_INPUT_LABEL_ID "Masukan"
#define CATEGORY_INPUT_INFO_0_ID NULL
#define CATEGORY_HACKS_LABEL_ID NULL
#define CATEGORY_HACKS_INFO_0_ID NULL
#define CATEGORY_DIP_SWITCH_LABEL_ID NULL
#define CATEGORY_DIP_SWITCH_INFO_0_ID NULL
#define FCEUMM_REGION_LABEL_ID "Daerah"
#define FCEUMM_REGION_INFO_0_ID NULL
#define OPTION_VAL_AUTO_ID "Otomatis"
#define OPTION_VAL_NTSC_ID NULL
#define OPTION_VAL_PAL_ID NULL
#define OPTION_VAL_DENDY_ID NULL
#define FCEUMM_GAME_GENIE_LABEL_ID NULL
#define FCEUMM_GAME_GENIE_INFO_0_ID NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_ID NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_ID NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_ID NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_ID NULL
#define FCEUMM_ASPECT_LABEL_ID "Rasio Aspek"
#define FCEUMM_ASPECT_INFO_0_ID NULL
#define OPTION_VAL_8_7_PAR_ID NULL
#define OPTION_VAL_4_3_ID NULL
#define OPTION_VAL_PP_ID NULL
#define FCEUMM_OVERSCAN_LABEL_ID NULL
#define FCEUMM_OVERSCAN_INFO_0_ID NULL
#define FCEUMM_OVERSCAN_H_LABEL_ID NULL
#define FCEUMM_OVERSCAN_H_INFO_0_ID NULL
#define FCEUMM_OVERSCAN_V_LABEL_ID NULL
#define FCEUMM_OVERSCAN_V_INFO_0_ID NULL
#define FCEUMM_PALETTE_LABEL_ID NULL
#define FCEUMM_PALETTE_INFO_0_ID NULL
#define OPTION_VAL_DEFAULT_ID NULL
#define OPTION_VAL_ASQREALC_ID NULL
#define OPTION_VAL_NINTENDO_VC_ID NULL
#define OPTION_VAL_RGB_ID NULL
#define OPTION_VAL_YUV_V3_ID NULL
#define OPTION_VAL_UNSATURATED_FINAL_ID NULL
#define OPTION_VAL_SONY_CXA2025AS_US_ID NULL
#define OPTION_VAL_BMF_FINAL2_ID NULL
#define OPTION_VAL_BMF_FINAL3_ID NULL
#define OPTION_VAL_SMOOTH_FBX_ID NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_ID NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_ID NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_ID NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_ID NULL
#define OPTION_VAL_NESCAP_ID NULL
#define OPTION_VAL_WAVEBEAM_ID NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_ID NULL
#define OPTION_VAL_MAGNUM_FBX_ID NULL
#define OPTION_VAL_SMOOTH_V2_FBX_ID NULL
#define OPTION_VAL_NES_CLASSIC_FBX_ID NULL
#define OPTION_VAL_RAW_ID NULL
#define OPTION_VAL_CUSTOM_ID NULL
#define FCEUMM_NTSC_FILTER_LABEL_ID NULL
#define FCEUMM_NTSC_FILTER_INFO_0_ID NULL
#define OPTION_VAL_COMPOSITE_ID NULL
#define OPTION_VAL_SVIDEO_ID NULL
#define OPTION_VAL_RGB_O9_ID NULL
#define OPTION_VAL_MONOCHROME_ID NULL
#define FCEUMM_SNDQUALITY_LABEL_ID NULL
#define FCEUMM_SNDQUALITY_INFO_0_ID NULL
#define OPTION_VAL_LOW_ID NULL
#define OPTION_VAL_HIGH_ID NULL
#define OPTION_VAL_VERY_HIGH_ID NULL
#define FCEUMM_SNDLOWPASS_LABEL_ID NULL
#define FCEUMM_SNDLOWPASS_INFO_0_ID NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_ID NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_ID NULL
#define OPTION_VAL_01_MS_DELAY_ID NULL
#define OPTION_VAL_02_MS_DELAY_ID NULL
#define OPTION_VAL_03_MS_DELAY_ID NULL
#define OPTION_VAL_04_MS_DELAY_ID NULL
#define OPTION_VAL_05_MS_DELAY_ID NULL
#define OPTION_VAL_06_MS_DELAY_ID NULL
#define OPTION_VAL_07_MS_DELAY_ID NULL
#define OPTION_VAL_08_MS_DELAY_ID NULL
#define OPTION_VAL_09_MS_DELAY_ID NULL
#define OPTION_VAL_10_MS_DELAY_ID NULL
#define OPTION_VAL_11_MS_DELAY_ID NULL
#define OPTION_VAL_12_MS_DELAY_ID NULL
#define OPTION_VAL_13_MS_DELAY_ID NULL
#define OPTION_VAL_14_MS_DELAY_ID NULL
#define OPTION_VAL_15_MS_DELAY_ID NULL
#define OPTION_VAL_16_MS_DELAY_ID NULL
#define OPTION_VAL_17_MS_DELAY_ID NULL
#define OPTION_VAL_18_MS_DELAY_ID NULL
#define OPTION_VAL_19_MS_DELAY_ID NULL
#define OPTION_VAL_20_MS_DELAY_ID NULL
#define OPTION_VAL_21_MS_DELAY_ID NULL
#define OPTION_VAL_22_MS_DELAY_ID NULL
#define OPTION_VAL_23_MS_DELAY_ID NULL
#define OPTION_VAL_24_MS_DELAY_ID NULL
#define OPTION_VAL_25_MS_DELAY_ID NULL
#define OPTION_VAL_26_MS_DELAY_ID NULL
#define OPTION_VAL_27_MS_DELAY_ID NULL
#define OPTION_VAL_28_MS_DELAY_ID NULL
#define OPTION_VAL_29_MS_DELAY_ID NULL
#define OPTION_VAL_30_MS_DELAY_ID NULL
#define OPTION_VAL_31_MS_DELAY_ID NULL
#define OPTION_VAL_32_MS_DELAY_ID NULL
#define FCEUMM_SWAPDUTY_LABEL_ID NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_ID NULL
#define FCEUMM_SWAPDUTY_INFO_0_ID "Menyimulasi suara dari famiclone dengan saluran gelombang denyut yang bit kapan daurnya terbalik."
#define FCEUMM_SNDVOLUME_LABEL_ID NULL
#define FCEUMM_SNDVOLUME_INFO_0_ID NULL
#define OPTION_VAL_0_ID NULL
#define OPTION_VAL_1_ID NULL
#define OPTION_VAL_2_ID NULL
#define OPTION_VAL_3_ID NULL
#define OPTION_VAL_4_ID NULL
#define OPTION_VAL_5_ID NULL
#define OPTION_VAL_6_ID NULL
#define OPTION_VAL_7_ID NULL
#define OPTION_VAL_8_ID NULL
#define OPTION_VAL_9_ID NULL
#define OPTION_VAL_10_ID NULL
#define FCEUMM_APU_1_LABEL_ID NULL
#define FCEUMM_APU_1_LABEL_CAT_ID NULL
#define FCEUMM_APU_1_INFO_0_ID NULL
#define FCEUMM_APU_2_LABEL_ID NULL
#define FCEUMM_APU_2_LABEL_CAT_ID NULL
#define FCEUMM_APU_2_INFO_0_ID NULL
#define FCEUMM_APU_3_LABEL_ID NULL
#define FCEUMM_APU_3_LABEL_CAT_ID NULL
#define FCEUMM_APU_3_INFO_0_ID NULL
#define FCEUMM_APU_4_LABEL_ID NULL
#define FCEUMM_APU_4_LABEL_CAT_ID NULL
#define FCEUMM_APU_4_INFO_0_ID NULL
#define FCEUMM_APU_5_LABEL_ID NULL
#define FCEUMM_APU_5_LABEL_CAT_ID NULL
#define FCEUMM_APU_5_INFO_0_ID NULL
#define FCEUMM_TURBO_ENABLE_LABEL_ID NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_ID NULL
#define OPTION_VAL_NONE_ID "Tak ada"
#define OPTION_VAL_PLAYER_1_ID NULL
#define OPTION_VAL_PLAYER_2_ID NULL
#define OPTION_VAL_BOTH_ID NULL
#define FCEUMM_TURBO_DELAY_LABEL_ID NULL
#define FCEUMM_TURBO_DELAY_INFO_0_ID NULL
#define FCEUMM_ZAPPER_MODE_LABEL_ID NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_ID NULL
#define OPTION_VAL_CLIGHTGUN_ID NULL
#define OPTION_VAL_STLIGHTGUN_ID NULL
#define OPTION_VAL_TOUCHSCREEN_ID NULL
#define OPTION_VAL_MOUSE_ID NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_ID NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_ID NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_ID NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_ID NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_ID NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_ID NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_ID NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_ID NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_ID NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_ID NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_ID NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_ID NULL
#define FCEUMM_OVERCLOCKING_LABEL_ID NULL
#define FCEUMM_OVERCLOCKING_INFO_0_ID NULL
#define OPTION_VAL_2X_POSTRENDER_ID NULL
#define OPTION_VAL_2X_VBLANK_ID NULL
#define FCEUMM_RAMSTATE_LABEL_ID NULL
#define FCEUMM_RAMSTATE_INFO_0_ID NULL
#define OPTION_VAL_FILL_FF_ID NULL
#define OPTION_VAL_FILL_00_ID NULL
#define OPTION_VAL_RANDOM_ID NULL

struct retro_core_option_v2_category option_cats_id[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_ID,
      CATEGORY_SYSTEM_INFO_0_ID
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_ID,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_ID
#else
      CATEGORY_VIDEO_INFO_1_ID
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_ID,
      CATEGORY_AUDIO_INFO_0_ID
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_ID,
      CATEGORY_INPUT_INFO_0_ID
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_ID,
      CATEGORY_HACKS_INFO_0_ID
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_ID,
      CATEGORY_DIP_SWITCH_INFO_0_ID
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_id[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_ID,
      NULL,
      FCEUMM_REGION_INFO_0_ID,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_ID },
         { "NTSC",  OPTION_VAL_NTSC_ID },
         { "PAL",   OPTION_VAL_PAL_ID },
         { "Dendy", OPTION_VAL_DENDY_ID },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_ID,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_ID,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_ID,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_ID,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_ID,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_ID,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_ID,
      NULL,
      FCEUMM_ASPECT_INFO_0_ID,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_ID },
         { "4:3",     OPTION_VAL_4_3_ID },
         { "PP",      OPTION_VAL_PP_ID },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_ID,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_ID,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_ID,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_ID,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_ID,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_ID,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_ID,
      NULL,
      FCEUMM_PALETTE_INFO_0_ID,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_ID },
         { "asqrealc",             OPTION_VAL_ASQREALC_ID },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_ID },
         { "rgb",                  OPTION_VAL_RGB_ID },
         { "yuv-v3",               OPTION_VAL_YUV_V3_ID },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_ID },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_ID },
         { "pal",                  OPTION_VAL_PAL_ID },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_ID },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_ID },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_ID },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_ID },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_ID },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_ID },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_ID },
         { "nescap",               OPTION_VAL_NESCAP_ID },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_ID },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_ID },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_ID },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_ID },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_ID },
         { "raw",                  OPTION_VAL_RAW_ID },
         { "custom",               OPTION_VAL_CUSTOM_ID },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_ID,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_ID,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_ID },
         { "svideo",     OPTION_VAL_SVIDEO_ID },
         { "rgb",        OPTION_VAL_RGB_O9_ID },
         { "monochrome", OPTION_VAL_MONOCHROME_ID },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_ID,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_ID,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_ID },
         { "High",      OPTION_VAL_HIGH_ID },
         { "Very High", OPTION_VAL_VERY_HIGH_ID },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_ID,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_ID,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_ID,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_ID,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_ID },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_ID },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_ID },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_ID },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_ID },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_ID },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_ID },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_ID },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_ID },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_ID },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_ID },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_ID },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_ID },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_ID },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_ID },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_ID },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_ID },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_ID },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_ID },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_ID },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_ID },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_ID },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_ID },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_ID },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_ID },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_ID },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_ID },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_ID },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_ID },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_ID },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_ID },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_ID },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_ID,
      FCEUMM_SWAPDUTY_LABEL_CAT_ID,
      FCEUMM_SWAPDUTY_INFO_0_ID,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_ID,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_ID,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_ID },
         { "1",  OPTION_VAL_1_ID },
         { "2",  OPTION_VAL_2_ID },
         { "3",  OPTION_VAL_3_ID },
         { "4",  OPTION_VAL_4_ID },
         { "5",  OPTION_VAL_5_ID },
         { "6",  OPTION_VAL_6_ID },
         { "7",  OPTION_VAL_7_ID },
         { "8",  OPTION_VAL_8_ID },
         { "9",  OPTION_VAL_9_ID },
         { "10", OPTION_VAL_10_ID },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_ID,
      FCEUMM_APU_1_LABEL_CAT_ID,
      FCEUMM_APU_1_INFO_0_ID,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_ID,
      FCEUMM_APU_2_LABEL_CAT_ID,
      FCEUMM_APU_2_INFO_0_ID,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_ID,
      FCEUMM_APU_3_LABEL_CAT_ID,
      FCEUMM_APU_3_INFO_0_ID,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_ID,
      FCEUMM_APU_4_LABEL_CAT_ID,
      FCEUMM_APU_4_INFO_0_ID,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_ID,
      FCEUMM_APU_5_LABEL_CAT_ID,
      FCEUMM_APU_5_INFO_0_ID,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_ID,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_ID,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_ID },
         { "Player 1", OPTION_VAL_PLAYER_1_ID },
         { "Player 2", OPTION_VAL_PLAYER_2_ID },
         { "Both",     OPTION_VAL_BOTH_ID },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_ID,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_ID,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_ID,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_ID,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_ID },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_ID },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_ID },
         { "mouse",       OPTION_VAL_MOUSE_ID },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_ID,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_ID,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_ID,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_ID,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_ID,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_ID,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_ID,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_ID,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_ID,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_ID,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_ID,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_ID,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_ID,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_ID,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_ID },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_ID },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_ID,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_ID,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_ID },
         { "fill $00", OPTION_VAL_FILL_00_ID },
         { "random",   OPTION_VAL_RANDOM_ID },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_id = {
   option_cats_id,
   option_defs_id
};

/* RETRO_LANGUAGE_IT */

#define CATEGORY_SYSTEM_LABEL_IT "Sistema"
#define CATEGORY_SYSTEM_INFO_0_IT NULL
#define CATEGORY_VIDEO_LABEL_IT NULL
#define CATEGORY_VIDEO_INFO_0_IT NULL
#define CATEGORY_VIDEO_INFO_1_IT NULL
#define CATEGORY_AUDIO_LABEL_IT NULL
#define CATEGORY_AUDIO_INFO_0_IT NULL
#define CATEGORY_INPUT_LABEL_IT NULL
#define CATEGORY_INPUT_INFO_0_IT NULL
#define CATEGORY_HACKS_LABEL_IT "Hack di Emulazione"
#define CATEGORY_HACKS_INFO_0_IT "Cambia le impostazioni di overclocking e di precisione di emulazione del processore che influenzano prestazioni e compatibilità di basso livello."
#define CATEGORY_DIP_SWITCH_LABEL_IT "Interruttori DIP"
#define CATEGORY_DIP_SWITCH_INFO_0_IT NULL
#define FCEUMM_REGION_LABEL_IT "Regione"
#define FCEUMM_REGION_INFO_0_IT "Forza il core a utilizzare i tempi NTSC, PAL o Dendy."
#define OPTION_VAL_AUTO_IT NULL
#define OPTION_VAL_NTSC_IT NULL
#define OPTION_VAL_PAL_IT NULL
#define OPTION_VAL_DENDY_IT NULL
#define FCEUMM_GAME_GENIE_LABEL_IT "Game Genie Add-On (Riavvio Richiesto)"
#define FCEUMM_GAME_GENIE_INFO_0_IT "Abilita l'emulazione di una cartuccia add-on di Game Genie, consentendo di inserire i codici truffa quando si avviano i giochi. Il file di Game Genie ROM 'gamegenie.nes' deve essere presente nella directory di sistema del frontend. Non si applica al contenuto di FDS o arcade."
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_IT "Mostra Opzioni Avanzate Di Sistema"
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_IT "Mostra opzioni di sistema avanzate e modifiche."
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_IT "Mostra Opzioni Audio Avanzate"
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_IT "Mostra opzioni e le regole audio avanzate."
#define FCEUMM_ASPECT_LABEL_IT "Rapporto Dimensioni"
#define FCEUMM_ASPECT_INFO_0_IT "Scegli le proporzioni preferite."
#define OPTION_VAL_8_7_PAR_IT NULL
#define OPTION_VAL_4_3_IT NULL
#define OPTION_VAL_PP_IT "Pixel Perfetto"
#define FCEUMM_OVERSCAN_LABEL_IT "Ritaglia Sovrascrittura"
#define FCEUMM_OVERSCAN_INFO_0_IT "Rimuove 8 pixel overscan da tutti i lati dello schermo."
#define FCEUMM_OVERSCAN_H_LABEL_IT "Ritaglia Overscan Orizzontale"
#define FCEUMM_OVERSCAN_H_INFO_0_IT "Rimuove 8 pixel dai lati sinistro e destro dello schermo per simulare la sovrascrittura vista sui televisori CRT standard."
#define FCEUMM_OVERSCAN_V_LABEL_IT "Ritaglia Overscan Verticale"
#define FCEUMM_OVERSCAN_V_INFO_0_IT "Rimuove 8 pixel dall'alto e dal basso dello schermo per simulare overscan visti sui televisori CRT standard."
#define FCEUMM_PALETTE_LABEL_IT "Tavolozza Colore"
#define FCEUMM_PALETTE_INFO_0_IT "Scegli tra tavolozze pre-generate, una tavolozza 64x3 personalizzata dal file o dal formato grezzo (deve usare uno shader nes-decoder)."
#define OPTION_VAL_DEFAULT_IT "Predefinito"
#define OPTION_VAL_ASQREALC_IT "AspiringSquire Reale"
#define OPTION_VAL_NINTENDO_VC_IT "Console Virtuale Nintendo"
#define OPTION_VAL_RGB_IT NULL
#define OPTION_VAL_YUV_V3_IT "YUV-V3 Di Fbx"
#define OPTION_VAL_UNSATURATED_FINAL_IT "Finale Insaturo Di Fbx"
#define OPTION_VAL_SONY_CXA2025AS_US_IT NULL
#define OPTION_VAL_BMF_FINAL2_IT "Finale 2 Del Bfm"
#define OPTION_VAL_BMF_FINAL3_IT "Finale 3 Del Bfm"
#define OPTION_VAL_SMOOTH_FBX_IT "Liscio Fbx"
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_IT "Composito Diretto Fbx"
#define OPTION_VAL_PVM_STYLE_D93_FBX_IT "Stile Pvm D93 Di Fbx"
#define OPTION_VAL_NTSC_HARDWARE_FBX_IT "Hardware NTSC Di FBX"
#define OPTION_VAL_NES_CLASSIC_FBX_FS_IT NULL
#define OPTION_VAL_NESCAP_IT NULL
#define OPTION_VAL_WAVEBEAM_IT "wavebeam di nakedarthur"
#define OPTION_VAL_DIGITAL_PRIME_FBX_IT NULL
#define OPTION_VAL_MAGNUM_FBX_IT NULL
#define OPTION_VAL_SMOOTH_V2_FBX_IT NULL
#define OPTION_VAL_NES_CLASSIC_FBX_IT NULL
#define OPTION_VAL_RAW_IT "Grezzo"
#define OPTION_VAL_CUSTOM_IT "Personalizzato"
#define FCEUMM_NTSC_FILTER_LABEL_IT "Filtro NTSC"
#define FCEUMM_NTSC_FILTER_INFO_0_IT NULL
#define OPTION_VAL_COMPOSITE_IT "Composito"
#define OPTION_VAL_SVIDEO_IT NULL
#define OPTION_VAL_RGB_O9_IT NULL
#define OPTION_VAL_MONOCHROME_IT "Monocromatico"
#define FCEUMM_SNDQUALITY_LABEL_IT "Qualità Suono"
#define FCEUMM_SNDQUALITY_INFO_0_IT "Abilita suono di qualità superiore. Aumenta i requisiti di prestazioni."
#define OPTION_VAL_LOW_IT "Basso"
#define OPTION_VAL_HIGH_IT "Alta"
#define OPTION_VAL_VERY_HIGH_IT "Molto Alta"
#define FCEUMM_SNDLOWPASS_LABEL_IT NULL
#define FCEUMM_SNDLOWPASS_INFO_0_IT NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_IT NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_IT NULL
#define OPTION_VAL_01_MS_DELAY_IT NULL
#define OPTION_VAL_02_MS_DELAY_IT NULL
#define OPTION_VAL_03_MS_DELAY_IT NULL
#define OPTION_VAL_04_MS_DELAY_IT NULL
#define OPTION_VAL_05_MS_DELAY_IT NULL
#define OPTION_VAL_06_MS_DELAY_IT NULL
#define OPTION_VAL_07_MS_DELAY_IT NULL
#define OPTION_VAL_08_MS_DELAY_IT NULL
#define OPTION_VAL_09_MS_DELAY_IT NULL
#define OPTION_VAL_10_MS_DELAY_IT NULL
#define OPTION_VAL_11_MS_DELAY_IT NULL
#define OPTION_VAL_12_MS_DELAY_IT NULL
#define OPTION_VAL_13_MS_DELAY_IT NULL
#define OPTION_VAL_14_MS_DELAY_IT NULL
#define OPTION_VAL_15_MS_DELAY_IT NULL
#define OPTION_VAL_16_MS_DELAY_IT NULL
#define OPTION_VAL_17_MS_DELAY_IT NULL
#define OPTION_VAL_18_MS_DELAY_IT NULL
#define OPTION_VAL_19_MS_DELAY_IT NULL
#define OPTION_VAL_20_MS_DELAY_IT NULL
#define OPTION_VAL_21_MS_DELAY_IT NULL
#define OPTION_VAL_22_MS_DELAY_IT NULL
#define OPTION_VAL_23_MS_DELAY_IT NULL
#define OPTION_VAL_24_MS_DELAY_IT NULL
#define OPTION_VAL_25_MS_DELAY_IT NULL
#define OPTION_VAL_26_MS_DELAY_IT NULL
#define OPTION_VAL_27_MS_DELAY_IT NULL
#define OPTION_VAL_28_MS_DELAY_IT NULL
#define OPTION_VAL_29_MS_DELAY_IT NULL
#define OPTION_VAL_30_MS_DELAY_IT NULL
#define OPTION_VAL_31_MS_DELAY_IT NULL
#define OPTION_VAL_32_MS_DELAY_IT NULL
#define FCEUMM_SWAPDUTY_LABEL_IT "Scambia Cicli Di Funzione Audio"
#define FCEUMM_SWAPDUTY_LABEL_CAT_IT "Scambia Cicli Di Funzione"
#define FCEUMM_SWAPDUTY_INFO_0_IT "Simula il suono di famicloni che hanno i bit del ciclo di funzionamento dei canali di onda di impulso invertito."
#define FCEUMM_SNDVOLUME_LABEL_IT "Volume Principale"
#define FCEUMM_SNDVOLUME_INFO_0_IT "Cambia il livello del volume principale."
#define OPTION_VAL_0_IT NULL
#define OPTION_VAL_1_IT NULL
#define OPTION_VAL_2_IT NULL
#define OPTION_VAL_3_IT NULL
#define OPTION_VAL_4_IT NULL
#define OPTION_VAL_5_IT NULL
#define OPTION_VAL_6_IT NULL
#define OPTION_VAL_7_IT NULL
#define OPTION_VAL_8_IT NULL
#define OPTION_VAL_9_IT NULL
#define OPTION_VAL_10_IT NULL
#define FCEUMM_APU_1_LABEL_IT "Canale Audio 1 (Quadro 1)"
#define FCEUMM_APU_1_LABEL_CAT_IT "Canale 1 (Quadro 1)"
#define FCEUMM_APU_1_INFO_0_IT "Abilita o disabilita l'uscita audio del generatore di onde di impulso 1."
#define FCEUMM_APU_2_LABEL_IT "Canale Audio 2 (Quadro 2)"
#define FCEUMM_APU_2_LABEL_CAT_IT "Canale 2 (Quadro 2)"
#define FCEUMM_APU_2_INFO_0_IT "Abilita o disabilita l'uscita audio del generatore di onde di impulso 2."
#define FCEUMM_APU_3_LABEL_IT "Canale Audio 3 (Triangolo)"
#define FCEUMM_APU_3_LABEL_CAT_IT "Canale 3 (Triangolo)"
#define FCEUMM_APU_3_INFO_0_IT "Abilita o disabilita l'uscita audio del generatore di onde a triangolo."
#define FCEUMM_APU_4_LABEL_IT "Canale Audio 4 (Rumore)"
#define FCEUMM_APU_4_LABEL_CAT_IT "Canale 4 (Rumore)"
#define FCEUMM_APU_4_INFO_0_IT "Abilita o disabilita l'uscita audio del generatore di rumore."
#define FCEUMM_APU_5_LABEL_IT "Canale Audio 5 (PCM)"
#define FCEUMM_APU_5_LABEL_CAT_IT "Canale 5 (PCM)"
#define FCEUMM_APU_5_INFO_0_IT "Abilita o disabilita l'uscita audio del canale di modulazione delta."
#define FCEUMM_TURBO_ENABLE_LABEL_IT "Attiva Turbo"
#define FCEUMM_TURBO_ENABLE_INFO_0_IT "Abilita o disabilita i pulsanti turbo."
#define OPTION_VAL_NONE_IT "Nessuno"
#define OPTION_VAL_PLAYER_1_IT "Giocatore 1"
#define OPTION_VAL_PLAYER_2_IT "Giocatore 2"
#define OPTION_VAL_BOTH_IT "Entrambi"
#define FCEUMM_TURBO_DELAY_LABEL_IT "Ritardo Turbo (in fotogrammi)"
#define FCEUMM_TURBO_DELAY_INFO_0_IT "Ripetere la velocità dei pulsanti turbo nei fotogrammi."
#define FCEUMM_ZAPPER_MODE_LABEL_IT "Modalità Zapper"
#define FCEUMM_ZAPPER_MODE_INFO_0_IT NULL
#define OPTION_VAL_CLIGHTGUN_IT NULL
#define OPTION_VAL_STLIGHTGUN_IT NULL
#define OPTION_VAL_TOUCHSCREEN_IT "Schermo tattile"
#define OPTION_VAL_MOUSE_IT NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_IT "Mostra Mirino Zapper"
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_IT NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_IT "Tolleranza Zapper"
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_IT "Imposta quanti pixel dall'area di destinazione sono sul bersaglio."
#define FCEUMM_ZAPPER_TRIGGER_LABEL_IT NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_IT NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_IT NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_IT NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_IT "Permetti Direzioni Opposte"
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_IT "Consente combinazioni simultanee di pulsanti UP+DOWN o LEFT+DIRITTO, che possono creare effetti diversi in alcuni giochi."
#define FCEUMM_NOSPRITELIMIT_LABEL_IT "Nessun Limite Sprite"
#define FCEUMM_NOSPRITELIMIT_INFO_0_IT "Rimuove il limite hardware di 8-per-scanline. Questo riduce lo sprite tremolante ma può causare alcuni giochi di glitch poiché alcuni usano questo per gli effetti."
#define FCEUMM_OVERCLOCKING_LABEL_IT NULL
#define FCEUMM_OVERCLOCKING_INFO_0_IT "Abilita o disabilita l'overclocking, che può ridurre i rallentamenti in alcuni giochi. Il metodo Postrender è più compatibile con ogni gioco, Vblank è più efficace per i giochi come Contra Force."
#define OPTION_VAL_2X_POSTRENDER_IT NULL
#define OPTION_VAL_2X_VBLANK_IT NULL
#define FCEUMM_RAMSTATE_LABEL_IT "Riempimento Accensione Ram (Riavvio Richiesto)"
#define FCEUMM_RAMSTATE_INFO_0_IT "Valori RAM sull'alimentazione. Alcuni giochi si basano su valori RAM iniziali per la generazione di numeri casuali come esempio."
#define OPTION_VAL_FILL_FF_IT NULL
#define OPTION_VAL_FILL_00_IT NULL
#define OPTION_VAL_RANDOM_IT "Casuale"

struct retro_core_option_v2_category option_cats_it[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_IT,
      CATEGORY_SYSTEM_INFO_0_IT
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_IT,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_IT
#else
      CATEGORY_VIDEO_INFO_1_IT
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_IT,
      CATEGORY_AUDIO_INFO_0_IT
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_IT,
      CATEGORY_INPUT_INFO_0_IT
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_IT,
      CATEGORY_HACKS_INFO_0_IT
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_IT,
      CATEGORY_DIP_SWITCH_INFO_0_IT
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_it[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_IT,
      NULL,
      FCEUMM_REGION_INFO_0_IT,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_IT },
         { "NTSC",  OPTION_VAL_NTSC_IT },
         { "PAL",   OPTION_VAL_PAL_IT },
         { "Dendy", OPTION_VAL_DENDY_IT },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_IT,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_IT,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_IT,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_IT,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_IT,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_IT,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_IT,
      NULL,
      FCEUMM_ASPECT_INFO_0_IT,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_IT },
         { "4:3",     OPTION_VAL_4_3_IT },
         { "PP",      OPTION_VAL_PP_IT },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_IT,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_IT,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_IT,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_IT,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_IT,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_IT,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_IT,
      NULL,
      FCEUMM_PALETTE_INFO_0_IT,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_IT },
         { "asqrealc",             OPTION_VAL_ASQREALC_IT },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_IT },
         { "rgb",                  OPTION_VAL_RGB_IT },
         { "yuv-v3",               OPTION_VAL_YUV_V3_IT },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_IT },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_IT },
         { "pal",                  OPTION_VAL_PAL_IT },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_IT },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_IT },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_IT },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_IT },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_IT },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_IT },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_IT },
         { "nescap",               OPTION_VAL_NESCAP_IT },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_IT },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_IT },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_IT },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_IT },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_IT },
         { "raw",                  OPTION_VAL_RAW_IT },
         { "custom",               OPTION_VAL_CUSTOM_IT },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_IT,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_IT,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_IT },
         { "svideo",     OPTION_VAL_SVIDEO_IT },
         { "rgb",        OPTION_VAL_RGB_O9_IT },
         { "monochrome", OPTION_VAL_MONOCHROME_IT },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_IT,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_IT,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_IT },
         { "High",      OPTION_VAL_HIGH_IT },
         { "Very High", OPTION_VAL_VERY_HIGH_IT },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_IT,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_IT,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_IT,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_IT,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_IT },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_IT },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_IT },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_IT },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_IT },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_IT },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_IT },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_IT },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_IT },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_IT },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_IT },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_IT },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_IT },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_IT },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_IT },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_IT },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_IT },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_IT },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_IT },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_IT },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_IT },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_IT },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_IT },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_IT },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_IT },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_IT },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_IT },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_IT },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_IT },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_IT },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_IT },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_IT },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_IT,
      FCEUMM_SWAPDUTY_LABEL_CAT_IT,
      FCEUMM_SWAPDUTY_INFO_0_IT,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_IT,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_IT,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_IT },
         { "1",  OPTION_VAL_1_IT },
         { "2",  OPTION_VAL_2_IT },
         { "3",  OPTION_VAL_3_IT },
         { "4",  OPTION_VAL_4_IT },
         { "5",  OPTION_VAL_5_IT },
         { "6",  OPTION_VAL_6_IT },
         { "7",  OPTION_VAL_7_IT },
         { "8",  OPTION_VAL_8_IT },
         { "9",  OPTION_VAL_9_IT },
         { "10", OPTION_VAL_10_IT },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_IT,
      FCEUMM_APU_1_LABEL_CAT_IT,
      FCEUMM_APU_1_INFO_0_IT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_IT,
      FCEUMM_APU_2_LABEL_CAT_IT,
      FCEUMM_APU_2_INFO_0_IT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_IT,
      FCEUMM_APU_3_LABEL_CAT_IT,
      FCEUMM_APU_3_INFO_0_IT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_IT,
      FCEUMM_APU_4_LABEL_CAT_IT,
      FCEUMM_APU_4_INFO_0_IT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_IT,
      FCEUMM_APU_5_LABEL_CAT_IT,
      FCEUMM_APU_5_INFO_0_IT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_IT,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_IT,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_IT },
         { "Player 1", OPTION_VAL_PLAYER_1_IT },
         { "Player 2", OPTION_VAL_PLAYER_2_IT },
         { "Both",     OPTION_VAL_BOTH_IT },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_IT,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_IT,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_IT,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_IT,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_IT },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_IT },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_IT },
         { "mouse",       OPTION_VAL_MOUSE_IT },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_IT,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_IT,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_IT,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_IT,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_IT,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_IT,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_IT,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_IT,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_IT,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_IT,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_IT,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_IT,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_IT,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_IT,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_IT },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_IT },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_IT,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_IT,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_IT },
         { "fill $00", OPTION_VAL_FILL_00_IT },
         { "random",   OPTION_VAL_RANDOM_IT },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_it = {
   option_cats_it,
   option_defs_it
};

/* RETRO_LANGUAGE_JA */

#define CATEGORY_SYSTEM_LABEL_JA "システム"
#define CATEGORY_SYSTEM_INFO_0_JA NULL
#define CATEGORY_VIDEO_LABEL_JA "ビデオのドライバ"
#define CATEGORY_VIDEO_INFO_0_JA NULL
#define CATEGORY_VIDEO_INFO_1_JA NULL
#define CATEGORY_AUDIO_LABEL_JA "オーディオのドライバ"
#define CATEGORY_AUDIO_INFO_0_JA NULL
#define CATEGORY_INPUT_LABEL_JA "入力"
#define CATEGORY_INPUT_INFO_0_JA NULL
#define CATEGORY_HACKS_LABEL_JA "エミュレーションハック"
#define CATEGORY_HACKS_INFO_0_JA NULL
#define CATEGORY_DIP_SWITCH_LABEL_JA NULL
#define CATEGORY_DIP_SWITCH_INFO_0_JA NULL
#define FCEUMM_REGION_LABEL_JA "地域"
#define FCEUMM_REGION_INFO_0_JA NULL
#define OPTION_VAL_AUTO_JA "自動"
#define OPTION_VAL_NTSC_JA NULL
#define OPTION_VAL_PAL_JA NULL
#define OPTION_VAL_DENDY_JA NULL
#define FCEUMM_GAME_GENIE_LABEL_JA NULL
#define FCEUMM_GAME_GENIE_INFO_0_JA NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_JA NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_JA NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_JA NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_JA NULL
#define FCEUMM_ASPECT_LABEL_JA "アスペクト比"
#define FCEUMM_ASPECT_INFO_0_JA NULL
#define OPTION_VAL_8_7_PAR_JA NULL
#define OPTION_VAL_4_3_JA NULL
#define OPTION_VAL_PP_JA NULL
#define FCEUMM_OVERSCAN_LABEL_JA NULL
#define FCEUMM_OVERSCAN_INFO_0_JA NULL
#define FCEUMM_OVERSCAN_H_LABEL_JA NULL
#define FCEUMM_OVERSCAN_H_INFO_0_JA NULL
#define FCEUMM_OVERSCAN_V_LABEL_JA NULL
#define FCEUMM_OVERSCAN_V_INFO_0_JA NULL
#define FCEUMM_PALETTE_LABEL_JA "カラーパレット"
#define FCEUMM_PALETTE_INFO_0_JA NULL
#define OPTION_VAL_DEFAULT_JA "デフォルト"
#define OPTION_VAL_ASQREALC_JA NULL
#define OPTION_VAL_NINTENDO_VC_JA NULL
#define OPTION_VAL_RGB_JA NULL
#define OPTION_VAL_YUV_V3_JA NULL
#define OPTION_VAL_UNSATURATED_FINAL_JA NULL
#define OPTION_VAL_SONY_CXA2025AS_US_JA NULL
#define OPTION_VAL_BMF_FINAL2_JA NULL
#define OPTION_VAL_BMF_FINAL3_JA NULL
#define OPTION_VAL_SMOOTH_FBX_JA NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_JA NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_JA NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_JA NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_JA NULL
#define OPTION_VAL_NESCAP_JA NULL
#define OPTION_VAL_WAVEBEAM_JA NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_JA NULL
#define OPTION_VAL_MAGNUM_FBX_JA NULL
#define OPTION_VAL_SMOOTH_V2_FBX_JA NULL
#define OPTION_VAL_NES_CLASSIC_FBX_JA NULL
#define OPTION_VAL_RAW_JA NULL
#define OPTION_VAL_CUSTOM_JA "カスタム"
#define FCEUMM_NTSC_FILTER_LABEL_JA NULL
#define FCEUMM_NTSC_FILTER_INFO_0_JA NULL
#define OPTION_VAL_COMPOSITE_JA "コンポジット"
#define OPTION_VAL_SVIDEO_JA NULL
#define OPTION_VAL_RGB_O9_JA NULL
#define OPTION_VAL_MONOCHROME_JA "モノクローム"
#define FCEUMM_SNDQUALITY_LABEL_JA NULL
#define FCEUMM_SNDQUALITY_INFO_0_JA NULL
#define OPTION_VAL_LOW_JA "低"
#define OPTION_VAL_HIGH_JA "高"
#define OPTION_VAL_VERY_HIGH_JA NULL
#define FCEUMM_SNDLOWPASS_LABEL_JA NULL
#define FCEUMM_SNDLOWPASS_INFO_0_JA NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_JA NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_JA NULL
#define OPTION_VAL_01_MS_DELAY_JA NULL
#define OPTION_VAL_02_MS_DELAY_JA NULL
#define OPTION_VAL_03_MS_DELAY_JA NULL
#define OPTION_VAL_04_MS_DELAY_JA NULL
#define OPTION_VAL_05_MS_DELAY_JA NULL
#define OPTION_VAL_06_MS_DELAY_JA NULL
#define OPTION_VAL_07_MS_DELAY_JA NULL
#define OPTION_VAL_08_MS_DELAY_JA NULL
#define OPTION_VAL_09_MS_DELAY_JA NULL
#define OPTION_VAL_10_MS_DELAY_JA NULL
#define OPTION_VAL_11_MS_DELAY_JA NULL
#define OPTION_VAL_12_MS_DELAY_JA NULL
#define OPTION_VAL_13_MS_DELAY_JA NULL
#define OPTION_VAL_14_MS_DELAY_JA NULL
#define OPTION_VAL_15_MS_DELAY_JA NULL
#define OPTION_VAL_16_MS_DELAY_JA NULL
#define OPTION_VAL_17_MS_DELAY_JA NULL
#define OPTION_VAL_18_MS_DELAY_JA NULL
#define OPTION_VAL_19_MS_DELAY_JA NULL
#define OPTION_VAL_20_MS_DELAY_JA NULL
#define OPTION_VAL_21_MS_DELAY_JA NULL
#define OPTION_VAL_22_MS_DELAY_JA NULL
#define OPTION_VAL_23_MS_DELAY_JA NULL
#define OPTION_VAL_24_MS_DELAY_JA NULL
#define OPTION_VAL_25_MS_DELAY_JA NULL
#define OPTION_VAL_26_MS_DELAY_JA NULL
#define OPTION_VAL_27_MS_DELAY_JA NULL
#define OPTION_VAL_28_MS_DELAY_JA NULL
#define OPTION_VAL_29_MS_DELAY_JA NULL
#define OPTION_VAL_30_MS_DELAY_JA NULL
#define OPTION_VAL_31_MS_DELAY_JA NULL
#define OPTION_VAL_32_MS_DELAY_JA NULL
#define FCEUMM_SWAPDUTY_LABEL_JA NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_JA NULL
#define FCEUMM_SWAPDUTY_INFO_0_JA NULL
#define FCEUMM_SNDVOLUME_LABEL_JA NULL
#define FCEUMM_SNDVOLUME_INFO_0_JA NULL
#define OPTION_VAL_0_JA NULL
#define OPTION_VAL_1_JA NULL
#define OPTION_VAL_2_JA NULL
#define OPTION_VAL_3_JA NULL
#define OPTION_VAL_4_JA NULL
#define OPTION_VAL_5_JA NULL
#define OPTION_VAL_6_JA NULL
#define OPTION_VAL_7_JA NULL
#define OPTION_VAL_8_JA NULL
#define OPTION_VAL_9_JA NULL
#define OPTION_VAL_10_JA NULL
#define FCEUMM_APU_1_LABEL_JA NULL
#define FCEUMM_APU_1_LABEL_CAT_JA NULL
#define FCEUMM_APU_1_INFO_0_JA NULL
#define FCEUMM_APU_2_LABEL_JA NULL
#define FCEUMM_APU_2_LABEL_CAT_JA NULL
#define FCEUMM_APU_2_INFO_0_JA NULL
#define FCEUMM_APU_3_LABEL_JA NULL
#define FCEUMM_APU_3_LABEL_CAT_JA NULL
#define FCEUMM_APU_3_INFO_0_JA NULL
#define FCEUMM_APU_4_LABEL_JA NULL
#define FCEUMM_APU_4_LABEL_CAT_JA NULL
#define FCEUMM_APU_4_INFO_0_JA NULL
#define FCEUMM_APU_5_LABEL_JA NULL
#define FCEUMM_APU_5_LABEL_CAT_JA NULL
#define FCEUMM_APU_5_INFO_0_JA NULL
#define FCEUMM_TURBO_ENABLE_LABEL_JA NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_JA NULL
#define OPTION_VAL_NONE_JA "なし"
#define OPTION_VAL_PLAYER_1_JA NULL
#define OPTION_VAL_PLAYER_2_JA NULL
#define OPTION_VAL_BOTH_JA NULL
#define FCEUMM_TURBO_DELAY_LABEL_JA NULL
#define FCEUMM_TURBO_DELAY_INFO_0_JA NULL
#define FCEUMM_ZAPPER_MODE_LABEL_JA NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_JA NULL
#define OPTION_VAL_CLIGHTGUN_JA NULL
#define OPTION_VAL_STLIGHTGUN_JA NULL
#define OPTION_VAL_TOUCHSCREEN_JA NULL
#define OPTION_VAL_MOUSE_JA NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_JA NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_JA NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_JA NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_JA NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_JA NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_JA NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_JA NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_JA NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_JA NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_JA NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_JA NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_JA NULL
#define FCEUMM_OVERCLOCKING_LABEL_JA NULL
#define FCEUMM_OVERCLOCKING_INFO_0_JA NULL
#define OPTION_VAL_2X_POSTRENDER_JA NULL
#define OPTION_VAL_2X_VBLANK_JA NULL
#define FCEUMM_RAMSTATE_LABEL_JA NULL
#define FCEUMM_RAMSTATE_INFO_0_JA NULL
#define OPTION_VAL_FILL_FF_JA NULL
#define OPTION_VAL_FILL_00_JA NULL
#define OPTION_VAL_RANDOM_JA NULL

struct retro_core_option_v2_category option_cats_ja[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_JA,
      CATEGORY_SYSTEM_INFO_0_JA
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_JA,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_JA
#else
      CATEGORY_VIDEO_INFO_1_JA
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_JA,
      CATEGORY_AUDIO_INFO_0_JA
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_JA,
      CATEGORY_INPUT_INFO_0_JA
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_JA,
      CATEGORY_HACKS_INFO_0_JA
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_JA,
      CATEGORY_DIP_SWITCH_INFO_0_JA
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_ja[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_JA,
      NULL,
      FCEUMM_REGION_INFO_0_JA,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_JA },
         { "NTSC",  OPTION_VAL_NTSC_JA },
         { "PAL",   OPTION_VAL_PAL_JA },
         { "Dendy", OPTION_VAL_DENDY_JA },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_JA,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_JA,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_JA,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_JA,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_JA,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_JA,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_JA,
      NULL,
      FCEUMM_ASPECT_INFO_0_JA,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_JA },
         { "4:3",     OPTION_VAL_4_3_JA },
         { "PP",      OPTION_VAL_PP_JA },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_JA,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_JA,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_JA,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_JA,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_JA,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_JA,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_JA,
      NULL,
      FCEUMM_PALETTE_INFO_0_JA,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_JA },
         { "asqrealc",             OPTION_VAL_ASQREALC_JA },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_JA },
         { "rgb",                  OPTION_VAL_RGB_JA },
         { "yuv-v3",               OPTION_VAL_YUV_V3_JA },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_JA },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_JA },
         { "pal",                  OPTION_VAL_PAL_JA },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_JA },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_JA },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_JA },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_JA },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_JA },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_JA },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_JA },
         { "nescap",               OPTION_VAL_NESCAP_JA },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_JA },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_JA },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_JA },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_JA },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_JA },
         { "raw",                  OPTION_VAL_RAW_JA },
         { "custom",               OPTION_VAL_CUSTOM_JA },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_JA,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_JA,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_JA },
         { "svideo",     OPTION_VAL_SVIDEO_JA },
         { "rgb",        OPTION_VAL_RGB_O9_JA },
         { "monochrome", OPTION_VAL_MONOCHROME_JA },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_JA,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_JA,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_JA },
         { "High",      OPTION_VAL_HIGH_JA },
         { "Very High", OPTION_VAL_VERY_HIGH_JA },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_JA,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_JA,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_JA,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_JA,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_JA },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_JA },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_JA },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_JA },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_JA },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_JA },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_JA },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_JA },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_JA },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_JA },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_JA },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_JA },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_JA },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_JA },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_JA },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_JA },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_JA },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_JA },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_JA },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_JA },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_JA },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_JA },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_JA },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_JA },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_JA },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_JA },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_JA },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_JA },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_JA },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_JA },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_JA },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_JA },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_JA,
      FCEUMM_SWAPDUTY_LABEL_CAT_JA,
      FCEUMM_SWAPDUTY_INFO_0_JA,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_JA,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_JA,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_JA },
         { "1",  OPTION_VAL_1_JA },
         { "2",  OPTION_VAL_2_JA },
         { "3",  OPTION_VAL_3_JA },
         { "4",  OPTION_VAL_4_JA },
         { "5",  OPTION_VAL_5_JA },
         { "6",  OPTION_VAL_6_JA },
         { "7",  OPTION_VAL_7_JA },
         { "8",  OPTION_VAL_8_JA },
         { "9",  OPTION_VAL_9_JA },
         { "10", OPTION_VAL_10_JA },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_JA,
      FCEUMM_APU_1_LABEL_CAT_JA,
      FCEUMM_APU_1_INFO_0_JA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_JA,
      FCEUMM_APU_2_LABEL_CAT_JA,
      FCEUMM_APU_2_INFO_0_JA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_JA,
      FCEUMM_APU_3_LABEL_CAT_JA,
      FCEUMM_APU_3_INFO_0_JA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_JA,
      FCEUMM_APU_4_LABEL_CAT_JA,
      FCEUMM_APU_4_INFO_0_JA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_JA,
      FCEUMM_APU_5_LABEL_CAT_JA,
      FCEUMM_APU_5_INFO_0_JA,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_JA,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_JA,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_JA },
         { "Player 1", OPTION_VAL_PLAYER_1_JA },
         { "Player 2", OPTION_VAL_PLAYER_2_JA },
         { "Both",     OPTION_VAL_BOTH_JA },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_JA,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_JA,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_JA,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_JA,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_JA },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_JA },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_JA },
         { "mouse",       OPTION_VAL_MOUSE_JA },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_JA,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_JA,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_JA,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_JA,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_JA,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_JA,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_JA,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_JA,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_JA,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_JA,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_JA,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_JA,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_JA,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_JA,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_JA },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_JA },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_JA,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_JA,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_JA },
         { "fill $00", OPTION_VAL_FILL_00_JA },
         { "random",   OPTION_VAL_RANDOM_JA },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_ja = {
   option_cats_ja,
   option_defs_ja
};

/* RETRO_LANGUAGE_KO */

#define CATEGORY_SYSTEM_LABEL_KO "시스템"
#define CATEGORY_SYSTEM_INFO_0_KO NULL
#define CATEGORY_VIDEO_LABEL_KO "비디오"
#define CATEGORY_VIDEO_INFO_0_KO NULL
#define CATEGORY_VIDEO_INFO_1_KO NULL
#define CATEGORY_AUDIO_LABEL_KO "오디오"
#define CATEGORY_AUDIO_INFO_0_KO NULL
#define CATEGORY_INPUT_LABEL_KO "입력"
#define CATEGORY_INPUT_INFO_0_KO NULL
#define CATEGORY_HACKS_LABEL_KO "에뮬레이션 핵"
#define CATEGORY_HACKS_INFO_0_KO "성능과 호환성에 영향을 끼치는 프로세서 오버클러킹 및 에뮬레이션 정확도 설정 등을 변경합니다."
#define CATEGORY_DIP_SWITCH_LABEL_KO "DIP 스위치"
#define CATEGORY_DIP_SWITCH_INFO_0_KO NULL
#define FCEUMM_REGION_LABEL_KO "지역"
#define FCEUMM_REGION_INFO_0_KO "코어가 강제로 NTSC, PAL, Dendy의 지역별 타이밍을 사용하도록 합니다."
#define OPTION_VAL_AUTO_KO "자동"
#define OPTION_VAL_NTSC_KO NULL
#define OPTION_VAL_PAL_KO NULL
#define OPTION_VAL_DENDY_KO NULL
#define FCEUMM_GAME_GENIE_LABEL_KO "Game Genie 애드온 (재시작 필요)"
#define FCEUMM_GAME_GENIE_INFO_0_KO "Game Genie 애드온 카트리지를 에뮬레이트하여 게임을 실행할 때 치트 코드를 입력할 수 있게 합니다. Game Genie ROM 파일 'gamegenie.nes'가 프론트엔드의 시스템 디렉토리에 존재해야 합니다. FDS 또는 아케이드 컨텐츠에는 적용되지 않습니다."
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_KO "고급 시스템 옵션 표시"
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_KO "고급 시스템 옵션을 표시합니다."
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_KO "고급 사운드 옵션 표시"
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_KO "고급 사운드 옵션을 표시합니다."
#define FCEUMM_ASPECT_LABEL_KO "화면비"
#define FCEUMM_ASPECT_INFO_0_KO "선호하는 화면비를 선택합니다."
#define OPTION_VAL_8_7_PAR_KO NULL
#define OPTION_VAL_4_3_KO NULL
#define OPTION_VAL_PP_KO "픽셀 퍼펙트"
#define FCEUMM_OVERSCAN_LABEL_KO "오버스캔 잘라내기"
#define FCEUMM_OVERSCAN_INFO_0_KO "모든 방향에서 8 픽셀의 오버스캔 영역을 제거합니다."
#define FCEUMM_OVERSCAN_H_LABEL_KO "가로 오버스캔 잘라내기"
#define FCEUMM_OVERSCAN_H_INFO_0_KO "왼쪽과 오른쪽 끝에서 8 픽셀씩을 제거해 일반적인 CRT 텔레비전의 오버스캔을 흉내냅니다."
#define FCEUMM_OVERSCAN_V_LABEL_KO "세로 오버스캔 잘라내기"
#define FCEUMM_OVERSCAN_V_INFO_0_KO "위쪽과 아래쪽 끝에서 8 픽셀씩을 제거해 일반적인 CRT 텔레비전의 오버스캔을 흉내냅니다."
#define FCEUMM_PALETTE_LABEL_KO "색상 팔레트"
#define FCEUMM_PALETTE_INFO_0_KO "미리 정의된 팔레트, 또는 파일이나 원본 형식에서 불러올 수 있는 사용자 정의 64x3 팔레트(nes-decoder 셰이더 필요)를 선택할 수 있습니다."
#define OPTION_VAL_DEFAULT_KO "기본"
#define OPTION_VAL_ASQREALC_KO NULL
#define OPTION_VAL_NINTENDO_VC_KO NULL
#define OPTION_VAL_RGB_KO NULL
#define OPTION_VAL_YUV_V3_KO NULL
#define OPTION_VAL_UNSATURATED_FINAL_KO NULL
#define OPTION_VAL_SONY_CXA2025AS_US_KO NULL
#define OPTION_VAL_BMF_FINAL2_KO NULL
#define OPTION_VAL_BMF_FINAL3_KO NULL
#define OPTION_VAL_SMOOTH_FBX_KO NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_KO NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_KO NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_KO NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_KO NULL
#define OPTION_VAL_NESCAP_KO NULL
#define OPTION_VAL_WAVEBEAM_KO NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_KO NULL
#define OPTION_VAL_MAGNUM_FBX_KO NULL
#define OPTION_VAL_SMOOTH_V2_FBX_KO NULL
#define OPTION_VAL_NES_CLASSIC_FBX_KO NULL
#define OPTION_VAL_RAW_KO "원본 형식"
#define OPTION_VAL_CUSTOM_KO "사용자 정의"
#define FCEUMM_NTSC_FILTER_LABEL_KO "NTSC 필터"
#define FCEUMM_NTSC_FILTER_INFO_0_KO NULL
#define OPTION_VAL_COMPOSITE_KO "컴포지트"
#define OPTION_VAL_SVIDEO_KO "S-비디오"
#define OPTION_VAL_RGB_O9_KO NULL
#define OPTION_VAL_MONOCHROME_KO "흑백"
#define FCEUMM_SNDQUALITY_LABEL_KO "사운드 품질"
#define FCEUMM_SNDQUALITY_INFO_0_KO "고품질 사운드를 사용합니다. 성능 요구 사항이 높아집니다."
#define OPTION_VAL_LOW_KO "낮음"
#define OPTION_VAL_HIGH_KO "높음"
#define OPTION_VAL_VERY_HIGH_KO "매우 높음"
#define FCEUMM_SNDLOWPASS_LABEL_KO NULL
#define FCEUMM_SNDLOWPASS_INFO_0_KO NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_KO NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_KO NULL
#define OPTION_VAL_01_MS_DELAY_KO NULL
#define OPTION_VAL_02_MS_DELAY_KO NULL
#define OPTION_VAL_03_MS_DELAY_KO NULL
#define OPTION_VAL_04_MS_DELAY_KO NULL
#define OPTION_VAL_05_MS_DELAY_KO NULL
#define OPTION_VAL_06_MS_DELAY_KO NULL
#define OPTION_VAL_07_MS_DELAY_KO NULL
#define OPTION_VAL_08_MS_DELAY_KO NULL
#define OPTION_VAL_09_MS_DELAY_KO NULL
#define OPTION_VAL_10_MS_DELAY_KO NULL
#define OPTION_VAL_11_MS_DELAY_KO NULL
#define OPTION_VAL_12_MS_DELAY_KO NULL
#define OPTION_VAL_13_MS_DELAY_KO NULL
#define OPTION_VAL_14_MS_DELAY_KO NULL
#define OPTION_VAL_15_MS_DELAY_KO NULL
#define OPTION_VAL_16_MS_DELAY_KO NULL
#define OPTION_VAL_17_MS_DELAY_KO NULL
#define OPTION_VAL_18_MS_DELAY_KO NULL
#define OPTION_VAL_19_MS_DELAY_KO NULL
#define OPTION_VAL_20_MS_DELAY_KO NULL
#define OPTION_VAL_21_MS_DELAY_KO NULL
#define OPTION_VAL_22_MS_DELAY_KO NULL
#define OPTION_VAL_23_MS_DELAY_KO NULL
#define OPTION_VAL_24_MS_DELAY_KO NULL
#define OPTION_VAL_25_MS_DELAY_KO NULL
#define OPTION_VAL_26_MS_DELAY_KO NULL
#define OPTION_VAL_27_MS_DELAY_KO NULL
#define OPTION_VAL_28_MS_DELAY_KO NULL
#define OPTION_VAL_29_MS_DELAY_KO NULL
#define OPTION_VAL_30_MS_DELAY_KO NULL
#define OPTION_VAL_31_MS_DELAY_KO NULL
#define OPTION_VAL_32_MS_DELAY_KO NULL
#define FCEUMM_SWAPDUTY_LABEL_KO "오디오 듀티 사이클 반전"
#define FCEUMM_SWAPDUTY_LABEL_CAT_KO "듀티 사이클 반전"
#define FCEUMM_SWAPDUTY_INFO_0_KO "구형파 채널의 듀티 사이클 비트가 반전된 일부 패미클론의 소리를 흉내냅니다."
#define FCEUMM_SNDVOLUME_LABEL_KO "주 볼륨"
#define FCEUMM_SNDVOLUME_INFO_0_KO "주 볼륨 수준을 변경합니다."
#define OPTION_VAL_0_KO NULL
#define OPTION_VAL_1_KO NULL
#define OPTION_VAL_2_KO NULL
#define OPTION_VAL_3_KO NULL
#define OPTION_VAL_4_KO NULL
#define OPTION_VAL_5_KO NULL
#define OPTION_VAL_6_KO NULL
#define OPTION_VAL_7_KO NULL
#define OPTION_VAL_8_KO NULL
#define OPTION_VAL_9_KO NULL
#define OPTION_VAL_10_KO NULL
#define FCEUMM_APU_1_LABEL_KO "오디오 채널 1 (구형파 1)"
#define FCEUMM_APU_1_LABEL_CAT_KO "채널 1 (구형파 1)"
#define FCEUMM_APU_1_INFO_0_KO "1번 구형파 생성기의 오디오 출력을 활성화합니다."
#define FCEUMM_APU_2_LABEL_KO "오디오 채널 2 (구형파 2)"
#define FCEUMM_APU_2_LABEL_CAT_KO "채널 2 (구형파 2)"
#define FCEUMM_APU_2_INFO_0_KO "2번 구형파 생성기의 오디오 출력을 활성화합니다."
#define FCEUMM_APU_3_LABEL_KO "오디오 채널 3 (삼각파)"
#define FCEUMM_APU_3_LABEL_CAT_KO "채널 3 (삼각파)"
#define FCEUMM_APU_3_INFO_0_KO "삼각파 생성기의 오디오 출력을 활성화합니다."
#define FCEUMM_APU_4_LABEL_KO "오디오 채널 4 (노이즈)"
#define FCEUMM_APU_4_LABEL_CAT_KO "채널 4 (노이즈)"
#define FCEUMM_APU_4_INFO_0_KO "노이즈 생성기의 오디오 출력을 활성화합니다."
#define FCEUMM_APU_5_LABEL_KO "오디오 채널 5 (PCM)"
#define FCEUMM_APU_5_LABEL_CAT_KO "채널 5 (PCM)"
#define FCEUMM_APU_5_INFO_0_KO "델타 변조 채널의 오디오 출력을 활성화합니다."
#define FCEUMM_TURBO_ENABLE_LABEL_KO "터보 사용"
#define FCEUMM_TURBO_ENABLE_INFO_0_KO "터보 버튼을 활성화합니다."
#define OPTION_VAL_NONE_KO "없음"
#define OPTION_VAL_PLAYER_1_KO "플레이어 1"
#define OPTION_VAL_PLAYER_2_KO "플레이어 2"
#define OPTION_VAL_BOTH_KO "둘 다"
#define FCEUMM_TURBO_DELAY_LABEL_KO "터보 간격 (프레임)"
#define FCEUMM_TURBO_DELAY_INFO_0_KO "터보 버튼을 반복할 속도(프레임)입니다."
#define FCEUMM_ZAPPER_MODE_LABEL_KO "Zapper 모드"
#define FCEUMM_ZAPPER_MODE_INFO_0_KO NULL
#define OPTION_VAL_CLIGHTGUN_KO NULL
#define OPTION_VAL_STLIGHTGUN_KO NULL
#define OPTION_VAL_TOUCHSCREEN_KO "터치스크린"
#define OPTION_VAL_MOUSE_KO "마우스"
#define FCEUMM_SHOW_CROSSHAIR_LABEL_KO "Zapper 십자선 표시"
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_KO NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_KO "Zapper 인식 범위"
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_KO "목표 위치에서 몇 픽셀까지를 목표로 인식할지 선택합니다."
#define FCEUMM_ZAPPER_TRIGGER_LABEL_KO NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_KO NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_KO NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_KO NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_KO "반대 방향 동시 입력 허용"
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_KO "일부 게임에서 다양한 효과를 유발하는 위+아래 또는 왼쪽+오른쪽 동시 입력을 허용합니다."
#define FCEUMM_NOSPRITELIMIT_LABEL_KO "스프라이트 제한 해제"
#define FCEUMM_NOSPRITELIMIT_INFO_0_KO "하드웨어의 스캔라인 당 8 스프라이트 제한을 제거합니다. 스프라이트 깜빡임을 없앨 수 있지만 이를 효과로 활용하는 일부 게임에서는 버그가 발생할 수 있습니다."
#define FCEUMM_OVERCLOCKING_LABEL_KO "오버클럭"
#define FCEUMM_OVERCLOCKING_INFO_0_KO "일부 게임에서 느려지는 현상을 줄일 수 있는 오버클럭을 활성화합니다. Postrender 방식은 더 많은 게임과 호환되지만 Vblank 방식은 Contra Force 등의 일부 게임에 더 효과적입니다."
#define OPTION_VAL_2X_POSTRENDER_KO NULL
#define OPTION_VAL_2X_VBLANK_KO NULL
#define FCEUMM_RAMSTATE_LABEL_KO "기동 시 RAM 채우기 (재시작 필요)"
#define FCEUMM_RAMSTATE_INFO_0_KO "전원이 들어올 때 RAM의 값입니다. 일부 게임은 초기 RAM 값을 난수 생성에 활용하기도 합니다."
#define OPTION_VAL_FILL_FF_KO NULL
#define OPTION_VAL_FILL_00_KO NULL
#define OPTION_VAL_RANDOM_KO "무작위"

struct retro_core_option_v2_category option_cats_ko[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_KO,
      CATEGORY_SYSTEM_INFO_0_KO
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_KO,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_KO
#else
      CATEGORY_VIDEO_INFO_1_KO
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_KO,
      CATEGORY_AUDIO_INFO_0_KO
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_KO,
      CATEGORY_INPUT_INFO_0_KO
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_KO,
      CATEGORY_HACKS_INFO_0_KO
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_KO,
      CATEGORY_DIP_SWITCH_INFO_0_KO
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_ko[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_KO,
      NULL,
      FCEUMM_REGION_INFO_0_KO,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_KO },
         { "NTSC",  OPTION_VAL_NTSC_KO },
         { "PAL",   OPTION_VAL_PAL_KO },
         { "Dendy", OPTION_VAL_DENDY_KO },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_KO,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_KO,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_KO,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_KO,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_KO,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_KO,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_KO,
      NULL,
      FCEUMM_ASPECT_INFO_0_KO,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_KO },
         { "4:3",     OPTION_VAL_4_3_KO },
         { "PP",      OPTION_VAL_PP_KO },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_KO,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_KO,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_KO,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_KO,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_KO,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_KO,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_KO,
      NULL,
      FCEUMM_PALETTE_INFO_0_KO,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_KO },
         { "asqrealc",             OPTION_VAL_ASQREALC_KO },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_KO },
         { "rgb",                  OPTION_VAL_RGB_KO },
         { "yuv-v3",               OPTION_VAL_YUV_V3_KO },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_KO },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_KO },
         { "pal",                  OPTION_VAL_PAL_KO },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_KO },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_KO },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_KO },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_KO },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_KO },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_KO },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_KO },
         { "nescap",               OPTION_VAL_NESCAP_KO },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_KO },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_KO },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_KO },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_KO },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_KO },
         { "raw",                  OPTION_VAL_RAW_KO },
         { "custom",               OPTION_VAL_CUSTOM_KO },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_KO,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_KO,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_KO },
         { "svideo",     OPTION_VAL_SVIDEO_KO },
         { "rgb",        OPTION_VAL_RGB_O9_KO },
         { "monochrome", OPTION_VAL_MONOCHROME_KO },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_KO,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_KO,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_KO },
         { "High",      OPTION_VAL_HIGH_KO },
         { "Very High", OPTION_VAL_VERY_HIGH_KO },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_KO,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_KO,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_KO,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_KO,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_KO },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_KO },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_KO },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_KO },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_KO },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_KO },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_KO },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_KO },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_KO },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_KO },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_KO },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_KO },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_KO },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_KO },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_KO },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_KO },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_KO },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_KO },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_KO },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_KO },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_KO },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_KO },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_KO },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_KO },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_KO },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_KO },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_KO },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_KO },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_KO },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_KO },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_KO },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_KO },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_KO,
      FCEUMM_SWAPDUTY_LABEL_CAT_KO,
      FCEUMM_SWAPDUTY_INFO_0_KO,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_KO,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_KO,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_KO },
         { "1",  OPTION_VAL_1_KO },
         { "2",  OPTION_VAL_2_KO },
         { "3",  OPTION_VAL_3_KO },
         { "4",  OPTION_VAL_4_KO },
         { "5",  OPTION_VAL_5_KO },
         { "6",  OPTION_VAL_6_KO },
         { "7",  OPTION_VAL_7_KO },
         { "8",  OPTION_VAL_8_KO },
         { "9",  OPTION_VAL_9_KO },
         { "10", OPTION_VAL_10_KO },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_KO,
      FCEUMM_APU_1_LABEL_CAT_KO,
      FCEUMM_APU_1_INFO_0_KO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_KO,
      FCEUMM_APU_2_LABEL_CAT_KO,
      FCEUMM_APU_2_INFO_0_KO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_KO,
      FCEUMM_APU_3_LABEL_CAT_KO,
      FCEUMM_APU_3_INFO_0_KO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_KO,
      FCEUMM_APU_4_LABEL_CAT_KO,
      FCEUMM_APU_4_INFO_0_KO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_KO,
      FCEUMM_APU_5_LABEL_CAT_KO,
      FCEUMM_APU_5_INFO_0_KO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_KO,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_KO,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_KO },
         { "Player 1", OPTION_VAL_PLAYER_1_KO },
         { "Player 2", OPTION_VAL_PLAYER_2_KO },
         { "Both",     OPTION_VAL_BOTH_KO },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_KO,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_KO,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_KO,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_KO,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_KO },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_KO },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_KO },
         { "mouse",       OPTION_VAL_MOUSE_KO },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_KO,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_KO,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_KO,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_KO,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_KO,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_KO,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_KO,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_KO,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_KO,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_KO,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_KO,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_KO,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_KO,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_KO,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_KO },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_KO },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_KO,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_KO,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_KO },
         { "fill $00", OPTION_VAL_FILL_00_KO },
         { "random",   OPTION_VAL_RANDOM_KO },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_ko = {
   option_cats_ko,
   option_defs_ko
};

/* RETRO_LANGUAGE_MT */

#define CATEGORY_SYSTEM_LABEL_MT NULL
#define CATEGORY_SYSTEM_INFO_0_MT NULL
#define CATEGORY_VIDEO_LABEL_MT NULL
#define CATEGORY_VIDEO_INFO_0_MT NULL
#define CATEGORY_VIDEO_INFO_1_MT NULL
#define CATEGORY_AUDIO_LABEL_MT NULL
#define CATEGORY_AUDIO_INFO_0_MT NULL
#define CATEGORY_INPUT_LABEL_MT NULL
#define CATEGORY_INPUT_INFO_0_MT NULL
#define CATEGORY_HACKS_LABEL_MT NULL
#define CATEGORY_HACKS_INFO_0_MT NULL
#define CATEGORY_DIP_SWITCH_LABEL_MT NULL
#define CATEGORY_DIP_SWITCH_INFO_0_MT NULL
#define FCEUMM_REGION_LABEL_MT NULL
#define FCEUMM_REGION_INFO_0_MT NULL
#define OPTION_VAL_AUTO_MT NULL
#define OPTION_VAL_NTSC_MT NULL
#define OPTION_VAL_PAL_MT NULL
#define OPTION_VAL_DENDY_MT NULL
#define FCEUMM_GAME_GENIE_LABEL_MT NULL
#define FCEUMM_GAME_GENIE_INFO_0_MT NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_MT NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_MT NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_MT NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_MT NULL
#define FCEUMM_ASPECT_LABEL_MT NULL
#define FCEUMM_ASPECT_INFO_0_MT NULL
#define OPTION_VAL_8_7_PAR_MT NULL
#define OPTION_VAL_4_3_MT NULL
#define OPTION_VAL_PP_MT NULL
#define FCEUMM_OVERSCAN_LABEL_MT NULL
#define FCEUMM_OVERSCAN_INFO_0_MT NULL
#define FCEUMM_OVERSCAN_H_LABEL_MT NULL
#define FCEUMM_OVERSCAN_H_INFO_0_MT NULL
#define FCEUMM_OVERSCAN_V_LABEL_MT NULL
#define FCEUMM_OVERSCAN_V_INFO_0_MT NULL
#define FCEUMM_PALETTE_LABEL_MT NULL
#define FCEUMM_PALETTE_INFO_0_MT NULL
#define OPTION_VAL_DEFAULT_MT NULL
#define OPTION_VAL_ASQREALC_MT NULL
#define OPTION_VAL_NINTENDO_VC_MT NULL
#define OPTION_VAL_RGB_MT NULL
#define OPTION_VAL_YUV_V3_MT NULL
#define OPTION_VAL_UNSATURATED_FINAL_MT NULL
#define OPTION_VAL_SONY_CXA2025AS_US_MT NULL
#define OPTION_VAL_BMF_FINAL2_MT NULL
#define OPTION_VAL_BMF_FINAL3_MT NULL
#define OPTION_VAL_SMOOTH_FBX_MT NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_MT NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_MT NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_MT NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_MT NULL
#define OPTION_VAL_NESCAP_MT NULL
#define OPTION_VAL_WAVEBEAM_MT NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_MT NULL
#define OPTION_VAL_MAGNUM_FBX_MT NULL
#define OPTION_VAL_SMOOTH_V2_FBX_MT NULL
#define OPTION_VAL_NES_CLASSIC_FBX_MT NULL
#define OPTION_VAL_RAW_MT NULL
#define OPTION_VAL_CUSTOM_MT NULL
#define FCEUMM_NTSC_FILTER_LABEL_MT NULL
#define FCEUMM_NTSC_FILTER_INFO_0_MT NULL
#define OPTION_VAL_COMPOSITE_MT NULL
#define OPTION_VAL_SVIDEO_MT NULL
#define OPTION_VAL_RGB_O9_MT NULL
#define OPTION_VAL_MONOCHROME_MT NULL
#define FCEUMM_SNDQUALITY_LABEL_MT NULL
#define FCEUMM_SNDQUALITY_INFO_0_MT NULL
#define OPTION_VAL_LOW_MT NULL
#define OPTION_VAL_HIGH_MT NULL
#define OPTION_VAL_VERY_HIGH_MT NULL
#define FCEUMM_SNDLOWPASS_LABEL_MT NULL
#define FCEUMM_SNDLOWPASS_INFO_0_MT NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_MT NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_MT NULL
#define OPTION_VAL_01_MS_DELAY_MT NULL
#define OPTION_VAL_02_MS_DELAY_MT NULL
#define OPTION_VAL_03_MS_DELAY_MT NULL
#define OPTION_VAL_04_MS_DELAY_MT NULL
#define OPTION_VAL_05_MS_DELAY_MT NULL
#define OPTION_VAL_06_MS_DELAY_MT NULL
#define OPTION_VAL_07_MS_DELAY_MT NULL
#define OPTION_VAL_08_MS_DELAY_MT NULL
#define OPTION_VAL_09_MS_DELAY_MT NULL
#define OPTION_VAL_10_MS_DELAY_MT NULL
#define OPTION_VAL_11_MS_DELAY_MT NULL
#define OPTION_VAL_12_MS_DELAY_MT NULL
#define OPTION_VAL_13_MS_DELAY_MT NULL
#define OPTION_VAL_14_MS_DELAY_MT NULL
#define OPTION_VAL_15_MS_DELAY_MT NULL
#define OPTION_VAL_16_MS_DELAY_MT NULL
#define OPTION_VAL_17_MS_DELAY_MT NULL
#define OPTION_VAL_18_MS_DELAY_MT NULL
#define OPTION_VAL_19_MS_DELAY_MT NULL
#define OPTION_VAL_20_MS_DELAY_MT NULL
#define OPTION_VAL_21_MS_DELAY_MT NULL
#define OPTION_VAL_22_MS_DELAY_MT NULL
#define OPTION_VAL_23_MS_DELAY_MT NULL
#define OPTION_VAL_24_MS_DELAY_MT NULL
#define OPTION_VAL_25_MS_DELAY_MT NULL
#define OPTION_VAL_26_MS_DELAY_MT NULL
#define OPTION_VAL_27_MS_DELAY_MT NULL
#define OPTION_VAL_28_MS_DELAY_MT NULL
#define OPTION_VAL_29_MS_DELAY_MT NULL
#define OPTION_VAL_30_MS_DELAY_MT NULL
#define OPTION_VAL_31_MS_DELAY_MT NULL
#define OPTION_VAL_32_MS_DELAY_MT NULL
#define FCEUMM_SWAPDUTY_LABEL_MT NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_MT NULL
#define FCEUMM_SWAPDUTY_INFO_0_MT NULL
#define FCEUMM_SNDVOLUME_LABEL_MT NULL
#define FCEUMM_SNDVOLUME_INFO_0_MT NULL
#define OPTION_VAL_0_MT NULL
#define OPTION_VAL_1_MT NULL
#define OPTION_VAL_2_MT NULL
#define OPTION_VAL_3_MT NULL
#define OPTION_VAL_4_MT NULL
#define OPTION_VAL_5_MT NULL
#define OPTION_VAL_6_MT NULL
#define OPTION_VAL_7_MT NULL
#define OPTION_VAL_8_MT NULL
#define OPTION_VAL_9_MT NULL
#define OPTION_VAL_10_MT NULL
#define FCEUMM_APU_1_LABEL_MT NULL
#define FCEUMM_APU_1_LABEL_CAT_MT NULL
#define FCEUMM_APU_1_INFO_0_MT NULL
#define FCEUMM_APU_2_LABEL_MT NULL
#define FCEUMM_APU_2_LABEL_CAT_MT NULL
#define FCEUMM_APU_2_INFO_0_MT NULL
#define FCEUMM_APU_3_LABEL_MT NULL
#define FCEUMM_APU_3_LABEL_CAT_MT NULL
#define FCEUMM_APU_3_INFO_0_MT NULL
#define FCEUMM_APU_4_LABEL_MT NULL
#define FCEUMM_APU_4_LABEL_CAT_MT NULL
#define FCEUMM_APU_4_INFO_0_MT NULL
#define FCEUMM_APU_5_LABEL_MT NULL
#define FCEUMM_APU_5_LABEL_CAT_MT NULL
#define FCEUMM_APU_5_INFO_0_MT NULL
#define FCEUMM_TURBO_ENABLE_LABEL_MT NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_MT NULL
#define OPTION_VAL_NONE_MT NULL
#define OPTION_VAL_PLAYER_1_MT NULL
#define OPTION_VAL_PLAYER_2_MT NULL
#define OPTION_VAL_BOTH_MT NULL
#define FCEUMM_TURBO_DELAY_LABEL_MT NULL
#define FCEUMM_TURBO_DELAY_INFO_0_MT NULL
#define FCEUMM_ZAPPER_MODE_LABEL_MT NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_MT NULL
#define OPTION_VAL_CLIGHTGUN_MT NULL
#define OPTION_VAL_STLIGHTGUN_MT NULL
#define OPTION_VAL_TOUCHSCREEN_MT NULL
#define OPTION_VAL_MOUSE_MT NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_MT NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_MT NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_MT NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_MT NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_MT NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_MT NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_MT NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_MT NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_MT NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_MT NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_MT NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_MT NULL
#define FCEUMM_OVERCLOCKING_LABEL_MT NULL
#define FCEUMM_OVERCLOCKING_INFO_0_MT NULL
#define OPTION_VAL_2X_POSTRENDER_MT NULL
#define OPTION_VAL_2X_VBLANK_MT NULL
#define FCEUMM_RAMSTATE_LABEL_MT NULL
#define FCEUMM_RAMSTATE_INFO_0_MT NULL
#define OPTION_VAL_FILL_FF_MT NULL
#define OPTION_VAL_FILL_00_MT NULL
#define OPTION_VAL_RANDOM_MT NULL

struct retro_core_option_v2_category option_cats_mt[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_MT,
      CATEGORY_SYSTEM_INFO_0_MT
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_MT,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_MT
#else
      CATEGORY_VIDEO_INFO_1_MT
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_MT,
      CATEGORY_AUDIO_INFO_0_MT
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_MT,
      CATEGORY_INPUT_INFO_0_MT
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_MT,
      CATEGORY_HACKS_INFO_0_MT
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_MT,
      CATEGORY_DIP_SWITCH_INFO_0_MT
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_mt[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_MT,
      NULL,
      FCEUMM_REGION_INFO_0_MT,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_MT },
         { "NTSC",  OPTION_VAL_NTSC_MT },
         { "PAL",   OPTION_VAL_PAL_MT },
         { "Dendy", OPTION_VAL_DENDY_MT },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_MT,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_MT,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_MT,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_MT,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_MT,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_MT,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_MT,
      NULL,
      FCEUMM_ASPECT_INFO_0_MT,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_MT },
         { "4:3",     OPTION_VAL_4_3_MT },
         { "PP",      OPTION_VAL_PP_MT },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_MT,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_MT,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_MT,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_MT,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_MT,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_MT,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_MT,
      NULL,
      FCEUMM_PALETTE_INFO_0_MT,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_MT },
         { "asqrealc",             OPTION_VAL_ASQREALC_MT },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_MT },
         { "rgb",                  OPTION_VAL_RGB_MT },
         { "yuv-v3",               OPTION_VAL_YUV_V3_MT },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_MT },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_MT },
         { "pal",                  OPTION_VAL_PAL_MT },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_MT },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_MT },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_MT },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_MT },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_MT },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_MT },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_MT },
         { "nescap",               OPTION_VAL_NESCAP_MT },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_MT },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_MT },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_MT },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_MT },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_MT },
         { "raw",                  OPTION_VAL_RAW_MT },
         { "custom",               OPTION_VAL_CUSTOM_MT },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_MT,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_MT,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_MT },
         { "svideo",     OPTION_VAL_SVIDEO_MT },
         { "rgb",        OPTION_VAL_RGB_O9_MT },
         { "monochrome", OPTION_VAL_MONOCHROME_MT },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_MT,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_MT,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_MT },
         { "High",      OPTION_VAL_HIGH_MT },
         { "Very High", OPTION_VAL_VERY_HIGH_MT },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_MT,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_MT,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_MT,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_MT,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_MT },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_MT },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_MT },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_MT },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_MT },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_MT },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_MT },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_MT },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_MT },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_MT },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_MT },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_MT },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_MT },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_MT },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_MT },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_MT },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_MT },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_MT },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_MT },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_MT },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_MT },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_MT },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_MT },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_MT },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_MT },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_MT },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_MT },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_MT },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_MT },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_MT },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_MT },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_MT },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_MT,
      FCEUMM_SWAPDUTY_LABEL_CAT_MT,
      FCEUMM_SWAPDUTY_INFO_0_MT,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_MT,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_MT,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_MT },
         { "1",  OPTION_VAL_1_MT },
         { "2",  OPTION_VAL_2_MT },
         { "3",  OPTION_VAL_3_MT },
         { "4",  OPTION_VAL_4_MT },
         { "5",  OPTION_VAL_5_MT },
         { "6",  OPTION_VAL_6_MT },
         { "7",  OPTION_VAL_7_MT },
         { "8",  OPTION_VAL_8_MT },
         { "9",  OPTION_VAL_9_MT },
         { "10", OPTION_VAL_10_MT },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_MT,
      FCEUMM_APU_1_LABEL_CAT_MT,
      FCEUMM_APU_1_INFO_0_MT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_MT,
      FCEUMM_APU_2_LABEL_CAT_MT,
      FCEUMM_APU_2_INFO_0_MT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_MT,
      FCEUMM_APU_3_LABEL_CAT_MT,
      FCEUMM_APU_3_INFO_0_MT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_MT,
      FCEUMM_APU_4_LABEL_CAT_MT,
      FCEUMM_APU_4_INFO_0_MT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_MT,
      FCEUMM_APU_5_LABEL_CAT_MT,
      FCEUMM_APU_5_INFO_0_MT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_MT,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_MT,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_MT },
         { "Player 1", OPTION_VAL_PLAYER_1_MT },
         { "Player 2", OPTION_VAL_PLAYER_2_MT },
         { "Both",     OPTION_VAL_BOTH_MT },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_MT,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_MT,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_MT,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_MT,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_MT },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_MT },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_MT },
         { "mouse",       OPTION_VAL_MOUSE_MT },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_MT,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_MT,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_MT,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_MT,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_MT,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_MT,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_MT,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_MT,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_MT,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_MT,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_MT,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_MT,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_MT,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_MT,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_MT },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_MT },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_MT,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_MT,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_MT },
         { "fill $00", OPTION_VAL_FILL_00_MT },
         { "random",   OPTION_VAL_RANDOM_MT },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_mt = {
   option_cats_mt,
   option_defs_mt
};

/* RETRO_LANGUAGE_NL */

#define CATEGORY_SYSTEM_LABEL_NL "Systeem"
#define CATEGORY_SYSTEM_INFO_0_NL NULL
#define CATEGORY_VIDEO_LABEL_NL NULL
#define CATEGORY_VIDEO_INFO_0_NL NULL
#define CATEGORY_VIDEO_INFO_1_NL NULL
#define CATEGORY_AUDIO_LABEL_NL "Geluid"
#define CATEGORY_AUDIO_INFO_0_NL NULL
#define CATEGORY_INPUT_LABEL_NL "Invoer"
#define CATEGORY_INPUT_INFO_0_NL NULL
#define CATEGORY_HACKS_LABEL_NL NULL
#define CATEGORY_HACKS_INFO_0_NL NULL
#define CATEGORY_DIP_SWITCH_LABEL_NL NULL
#define CATEGORY_DIP_SWITCH_INFO_0_NL NULL
#define FCEUMM_REGION_LABEL_NL "Regio"
#define FCEUMM_REGION_INFO_0_NL "Forceer de core om NTSC, PAL of Dendy regio-timingen te gebruiken."
#define OPTION_VAL_AUTO_NL "Automatisch"
#define OPTION_VAL_NTSC_NL NULL
#define OPTION_VAL_PAL_NL NULL
#define OPTION_VAL_DENDY_NL NULL
#define FCEUMM_GAME_GENIE_LABEL_NL NULL
#define FCEUMM_GAME_GENIE_INFO_0_NL NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_NL NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_NL NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_NL NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_NL NULL
#define FCEUMM_ASPECT_LABEL_NL "Beeldverhouding"
#define FCEUMM_ASPECT_INFO_0_NL NULL
#define OPTION_VAL_8_7_PAR_NL NULL
#define OPTION_VAL_4_3_NL NULL
#define OPTION_VAL_PP_NL NULL
#define FCEUMM_OVERSCAN_LABEL_NL NULL
#define FCEUMM_OVERSCAN_INFO_0_NL NULL
#define FCEUMM_OVERSCAN_H_LABEL_NL NULL
#define FCEUMM_OVERSCAN_H_INFO_0_NL NULL
#define FCEUMM_OVERSCAN_V_LABEL_NL NULL
#define FCEUMM_OVERSCAN_V_INFO_0_NL NULL
#define FCEUMM_PALETTE_LABEL_NL NULL
#define FCEUMM_PALETTE_INFO_0_NL NULL
#define OPTION_VAL_DEFAULT_NL NULL
#define OPTION_VAL_ASQREALC_NL NULL
#define OPTION_VAL_NINTENDO_VC_NL NULL
#define OPTION_VAL_RGB_NL NULL
#define OPTION_VAL_YUV_V3_NL NULL
#define OPTION_VAL_UNSATURATED_FINAL_NL NULL
#define OPTION_VAL_SONY_CXA2025AS_US_NL NULL
#define OPTION_VAL_BMF_FINAL2_NL NULL
#define OPTION_VAL_BMF_FINAL3_NL NULL
#define OPTION_VAL_SMOOTH_FBX_NL NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_NL NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_NL NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_NL NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_NL NULL
#define OPTION_VAL_NESCAP_NL NULL
#define OPTION_VAL_WAVEBEAM_NL NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_NL NULL
#define OPTION_VAL_MAGNUM_FBX_NL NULL
#define OPTION_VAL_SMOOTH_V2_FBX_NL NULL
#define OPTION_VAL_NES_CLASSIC_FBX_NL NULL
#define OPTION_VAL_RAW_NL NULL
#define OPTION_VAL_CUSTOM_NL "Aangepast"
#define FCEUMM_NTSC_FILTER_LABEL_NL NULL
#define FCEUMM_NTSC_FILTER_INFO_0_NL NULL
#define OPTION_VAL_COMPOSITE_NL NULL
#define OPTION_VAL_SVIDEO_NL NULL
#define OPTION_VAL_RGB_O9_NL NULL
#define OPTION_VAL_MONOCHROME_NL NULL
#define FCEUMM_SNDQUALITY_LABEL_NL NULL
#define FCEUMM_SNDQUALITY_INFO_0_NL NULL
#define OPTION_VAL_LOW_NL NULL
#define OPTION_VAL_HIGH_NL NULL
#define OPTION_VAL_VERY_HIGH_NL NULL
#define FCEUMM_SNDLOWPASS_LABEL_NL NULL
#define FCEUMM_SNDLOWPASS_INFO_0_NL NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_NL NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_NL NULL
#define OPTION_VAL_01_MS_DELAY_NL NULL
#define OPTION_VAL_02_MS_DELAY_NL NULL
#define OPTION_VAL_03_MS_DELAY_NL NULL
#define OPTION_VAL_04_MS_DELAY_NL NULL
#define OPTION_VAL_05_MS_DELAY_NL NULL
#define OPTION_VAL_06_MS_DELAY_NL NULL
#define OPTION_VAL_07_MS_DELAY_NL NULL
#define OPTION_VAL_08_MS_DELAY_NL NULL
#define OPTION_VAL_09_MS_DELAY_NL NULL
#define OPTION_VAL_10_MS_DELAY_NL NULL
#define OPTION_VAL_11_MS_DELAY_NL NULL
#define OPTION_VAL_12_MS_DELAY_NL NULL
#define OPTION_VAL_13_MS_DELAY_NL NULL
#define OPTION_VAL_14_MS_DELAY_NL NULL
#define OPTION_VAL_15_MS_DELAY_NL NULL
#define OPTION_VAL_16_MS_DELAY_NL NULL
#define OPTION_VAL_17_MS_DELAY_NL NULL
#define OPTION_VAL_18_MS_DELAY_NL NULL
#define OPTION_VAL_19_MS_DELAY_NL NULL
#define OPTION_VAL_20_MS_DELAY_NL NULL
#define OPTION_VAL_21_MS_DELAY_NL NULL
#define OPTION_VAL_22_MS_DELAY_NL NULL
#define OPTION_VAL_23_MS_DELAY_NL NULL
#define OPTION_VAL_24_MS_DELAY_NL NULL
#define OPTION_VAL_25_MS_DELAY_NL NULL
#define OPTION_VAL_26_MS_DELAY_NL NULL
#define OPTION_VAL_27_MS_DELAY_NL NULL
#define OPTION_VAL_28_MS_DELAY_NL NULL
#define OPTION_VAL_29_MS_DELAY_NL NULL
#define OPTION_VAL_30_MS_DELAY_NL NULL
#define OPTION_VAL_31_MS_DELAY_NL NULL
#define OPTION_VAL_32_MS_DELAY_NL NULL
#define FCEUMM_SWAPDUTY_LABEL_NL NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_NL NULL
#define FCEUMM_SWAPDUTY_INFO_0_NL NULL
#define FCEUMM_SNDVOLUME_LABEL_NL NULL
#define FCEUMM_SNDVOLUME_INFO_0_NL NULL
#define OPTION_VAL_0_NL NULL
#define OPTION_VAL_1_NL NULL
#define OPTION_VAL_2_NL NULL
#define OPTION_VAL_3_NL NULL
#define OPTION_VAL_4_NL NULL
#define OPTION_VAL_5_NL NULL
#define OPTION_VAL_6_NL NULL
#define OPTION_VAL_7_NL NULL
#define OPTION_VAL_8_NL NULL
#define OPTION_VAL_9_NL NULL
#define OPTION_VAL_10_NL NULL
#define FCEUMM_APU_1_LABEL_NL NULL
#define FCEUMM_APU_1_LABEL_CAT_NL NULL
#define FCEUMM_APU_1_INFO_0_NL NULL
#define FCEUMM_APU_2_LABEL_NL NULL
#define FCEUMM_APU_2_LABEL_CAT_NL NULL
#define FCEUMM_APU_2_INFO_0_NL NULL
#define FCEUMM_APU_3_LABEL_NL NULL
#define FCEUMM_APU_3_LABEL_CAT_NL NULL
#define FCEUMM_APU_3_INFO_0_NL NULL
#define FCEUMM_APU_4_LABEL_NL NULL
#define FCEUMM_APU_4_LABEL_CAT_NL NULL
#define FCEUMM_APU_4_INFO_0_NL NULL
#define FCEUMM_APU_5_LABEL_NL NULL
#define FCEUMM_APU_5_LABEL_CAT_NL NULL
#define FCEUMM_APU_5_INFO_0_NL NULL
#define FCEUMM_TURBO_ENABLE_LABEL_NL NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_NL NULL
#define OPTION_VAL_NONE_NL "Geen"
#define OPTION_VAL_PLAYER_1_NL NULL
#define OPTION_VAL_PLAYER_2_NL NULL
#define OPTION_VAL_BOTH_NL NULL
#define FCEUMM_TURBO_DELAY_LABEL_NL NULL
#define FCEUMM_TURBO_DELAY_INFO_0_NL NULL
#define FCEUMM_ZAPPER_MODE_LABEL_NL NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_NL NULL
#define OPTION_VAL_CLIGHTGUN_NL NULL
#define OPTION_VAL_STLIGHTGUN_NL NULL
#define OPTION_VAL_TOUCHSCREEN_NL NULL
#define OPTION_VAL_MOUSE_NL NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_NL NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_NL NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_NL NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_NL NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_NL NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_NL NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_NL NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_NL NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_NL NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_NL NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_NL NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_NL NULL
#define FCEUMM_OVERCLOCKING_LABEL_NL NULL
#define FCEUMM_OVERCLOCKING_INFO_0_NL NULL
#define OPTION_VAL_2X_POSTRENDER_NL NULL
#define OPTION_VAL_2X_VBLANK_NL NULL
#define FCEUMM_RAMSTATE_LABEL_NL NULL
#define FCEUMM_RAMSTATE_INFO_0_NL NULL
#define OPTION_VAL_FILL_FF_NL NULL
#define OPTION_VAL_FILL_00_NL NULL
#define OPTION_VAL_RANDOM_NL NULL

struct retro_core_option_v2_category option_cats_nl[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_NL,
      CATEGORY_SYSTEM_INFO_0_NL
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_NL,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_NL
#else
      CATEGORY_VIDEO_INFO_1_NL
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_NL,
      CATEGORY_AUDIO_INFO_0_NL
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_NL,
      CATEGORY_INPUT_INFO_0_NL
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_NL,
      CATEGORY_HACKS_INFO_0_NL
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_NL,
      CATEGORY_DIP_SWITCH_INFO_0_NL
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_nl[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_NL,
      NULL,
      FCEUMM_REGION_INFO_0_NL,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_NL },
         { "NTSC",  OPTION_VAL_NTSC_NL },
         { "PAL",   OPTION_VAL_PAL_NL },
         { "Dendy", OPTION_VAL_DENDY_NL },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_NL,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_NL,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_NL,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_NL,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_NL,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_NL,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_NL,
      NULL,
      FCEUMM_ASPECT_INFO_0_NL,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_NL },
         { "4:3",     OPTION_VAL_4_3_NL },
         { "PP",      OPTION_VAL_PP_NL },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_NL,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_NL,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_NL,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_NL,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_NL,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_NL,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_NL,
      NULL,
      FCEUMM_PALETTE_INFO_0_NL,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_NL },
         { "asqrealc",             OPTION_VAL_ASQREALC_NL },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_NL },
         { "rgb",                  OPTION_VAL_RGB_NL },
         { "yuv-v3",               OPTION_VAL_YUV_V3_NL },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_NL },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_NL },
         { "pal",                  OPTION_VAL_PAL_NL },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_NL },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_NL },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_NL },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_NL },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_NL },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_NL },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_NL },
         { "nescap",               OPTION_VAL_NESCAP_NL },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_NL },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_NL },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_NL },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_NL },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_NL },
         { "raw",                  OPTION_VAL_RAW_NL },
         { "custom",               OPTION_VAL_CUSTOM_NL },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_NL,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_NL,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_NL },
         { "svideo",     OPTION_VAL_SVIDEO_NL },
         { "rgb",        OPTION_VAL_RGB_O9_NL },
         { "monochrome", OPTION_VAL_MONOCHROME_NL },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_NL,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_NL,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_NL },
         { "High",      OPTION_VAL_HIGH_NL },
         { "Very High", OPTION_VAL_VERY_HIGH_NL },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_NL,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_NL,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_NL,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_NL,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_NL },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_NL },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_NL },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_NL },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_NL },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_NL },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_NL },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_NL },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_NL },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_NL },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_NL },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_NL },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_NL },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_NL },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_NL },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_NL },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_NL },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_NL },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_NL },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_NL },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_NL },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_NL },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_NL },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_NL },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_NL },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_NL },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_NL },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_NL },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_NL },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_NL },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_NL },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_NL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_NL,
      FCEUMM_SWAPDUTY_LABEL_CAT_NL,
      FCEUMM_SWAPDUTY_INFO_0_NL,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_NL,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_NL,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_NL },
         { "1",  OPTION_VAL_1_NL },
         { "2",  OPTION_VAL_2_NL },
         { "3",  OPTION_VAL_3_NL },
         { "4",  OPTION_VAL_4_NL },
         { "5",  OPTION_VAL_5_NL },
         { "6",  OPTION_VAL_6_NL },
         { "7",  OPTION_VAL_7_NL },
         { "8",  OPTION_VAL_8_NL },
         { "9",  OPTION_VAL_9_NL },
         { "10", OPTION_VAL_10_NL },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_NL,
      FCEUMM_APU_1_LABEL_CAT_NL,
      FCEUMM_APU_1_INFO_0_NL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_NL,
      FCEUMM_APU_2_LABEL_CAT_NL,
      FCEUMM_APU_2_INFO_0_NL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_NL,
      FCEUMM_APU_3_LABEL_CAT_NL,
      FCEUMM_APU_3_INFO_0_NL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_NL,
      FCEUMM_APU_4_LABEL_CAT_NL,
      FCEUMM_APU_4_INFO_0_NL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_NL,
      FCEUMM_APU_5_LABEL_CAT_NL,
      FCEUMM_APU_5_INFO_0_NL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_NL,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_NL,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_NL },
         { "Player 1", OPTION_VAL_PLAYER_1_NL },
         { "Player 2", OPTION_VAL_PLAYER_2_NL },
         { "Both",     OPTION_VAL_BOTH_NL },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_NL,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_NL,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_NL,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_NL,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_NL },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_NL },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_NL },
         { "mouse",       OPTION_VAL_MOUSE_NL },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_NL,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_NL,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_NL,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_NL,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_NL,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_NL,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_NL,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_NL,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_NL,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_NL,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_NL,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_NL,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_NL,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_NL,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_NL },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_NL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_NL,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_NL,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_NL },
         { "fill $00", OPTION_VAL_FILL_00_NL },
         { "random",   OPTION_VAL_RANDOM_NL },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_nl = {
   option_cats_nl,
   option_defs_nl
};

/* RETRO_LANGUAGE_NO */

#define CATEGORY_SYSTEM_LABEL_NO NULL
#define CATEGORY_SYSTEM_INFO_0_NO NULL
#define CATEGORY_VIDEO_LABEL_NO NULL
#define CATEGORY_VIDEO_INFO_0_NO NULL
#define CATEGORY_VIDEO_INFO_1_NO NULL
#define CATEGORY_AUDIO_LABEL_NO "Lyd"
#define CATEGORY_AUDIO_INFO_0_NO NULL
#define CATEGORY_INPUT_LABEL_NO "Inndata"
#define CATEGORY_INPUT_INFO_0_NO NULL
#define CATEGORY_HACKS_LABEL_NO NULL
#define CATEGORY_HACKS_INFO_0_NO NULL
#define CATEGORY_DIP_SWITCH_LABEL_NO NULL
#define CATEGORY_DIP_SWITCH_INFO_0_NO NULL
#define FCEUMM_REGION_LABEL_NO NULL
#define FCEUMM_REGION_INFO_0_NO NULL
#define OPTION_VAL_AUTO_NO NULL
#define OPTION_VAL_NTSC_NO NULL
#define OPTION_VAL_PAL_NO NULL
#define OPTION_VAL_DENDY_NO NULL
#define FCEUMM_GAME_GENIE_LABEL_NO NULL
#define FCEUMM_GAME_GENIE_INFO_0_NO NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_NO NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_NO NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_NO NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_NO NULL
#define FCEUMM_ASPECT_LABEL_NO "Størrelsesforhold"
#define FCEUMM_ASPECT_INFO_0_NO NULL
#define OPTION_VAL_8_7_PAR_NO NULL
#define OPTION_VAL_4_3_NO NULL
#define OPTION_VAL_PP_NO NULL
#define FCEUMM_OVERSCAN_LABEL_NO NULL
#define FCEUMM_OVERSCAN_INFO_0_NO NULL
#define FCEUMM_OVERSCAN_H_LABEL_NO NULL
#define FCEUMM_OVERSCAN_H_INFO_0_NO NULL
#define FCEUMM_OVERSCAN_V_LABEL_NO NULL
#define FCEUMM_OVERSCAN_V_INFO_0_NO NULL
#define FCEUMM_PALETTE_LABEL_NO NULL
#define FCEUMM_PALETTE_INFO_0_NO NULL
#define OPTION_VAL_DEFAULT_NO NULL
#define OPTION_VAL_ASQREALC_NO NULL
#define OPTION_VAL_NINTENDO_VC_NO NULL
#define OPTION_VAL_RGB_NO NULL
#define OPTION_VAL_YUV_V3_NO NULL
#define OPTION_VAL_UNSATURATED_FINAL_NO NULL
#define OPTION_VAL_SONY_CXA2025AS_US_NO NULL
#define OPTION_VAL_BMF_FINAL2_NO NULL
#define OPTION_VAL_BMF_FINAL3_NO NULL
#define OPTION_VAL_SMOOTH_FBX_NO NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_NO NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_NO NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_NO NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_NO NULL
#define OPTION_VAL_NESCAP_NO NULL
#define OPTION_VAL_WAVEBEAM_NO NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_NO NULL
#define OPTION_VAL_MAGNUM_FBX_NO NULL
#define OPTION_VAL_SMOOTH_V2_FBX_NO NULL
#define OPTION_VAL_NES_CLASSIC_FBX_NO NULL
#define OPTION_VAL_RAW_NO NULL
#define OPTION_VAL_CUSTOM_NO NULL
#define FCEUMM_NTSC_FILTER_LABEL_NO NULL
#define FCEUMM_NTSC_FILTER_INFO_0_NO NULL
#define OPTION_VAL_COMPOSITE_NO NULL
#define OPTION_VAL_SVIDEO_NO NULL
#define OPTION_VAL_RGB_O9_NO NULL
#define OPTION_VAL_MONOCHROME_NO NULL
#define FCEUMM_SNDQUALITY_LABEL_NO NULL
#define FCEUMM_SNDQUALITY_INFO_0_NO NULL
#define OPTION_VAL_LOW_NO NULL
#define OPTION_VAL_HIGH_NO NULL
#define OPTION_VAL_VERY_HIGH_NO NULL
#define FCEUMM_SNDLOWPASS_LABEL_NO NULL
#define FCEUMM_SNDLOWPASS_INFO_0_NO NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_NO NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_NO NULL
#define OPTION_VAL_01_MS_DELAY_NO NULL
#define OPTION_VAL_02_MS_DELAY_NO NULL
#define OPTION_VAL_03_MS_DELAY_NO NULL
#define OPTION_VAL_04_MS_DELAY_NO NULL
#define OPTION_VAL_05_MS_DELAY_NO NULL
#define OPTION_VAL_06_MS_DELAY_NO NULL
#define OPTION_VAL_07_MS_DELAY_NO NULL
#define OPTION_VAL_08_MS_DELAY_NO NULL
#define OPTION_VAL_09_MS_DELAY_NO NULL
#define OPTION_VAL_10_MS_DELAY_NO NULL
#define OPTION_VAL_11_MS_DELAY_NO NULL
#define OPTION_VAL_12_MS_DELAY_NO NULL
#define OPTION_VAL_13_MS_DELAY_NO NULL
#define OPTION_VAL_14_MS_DELAY_NO NULL
#define OPTION_VAL_15_MS_DELAY_NO NULL
#define OPTION_VAL_16_MS_DELAY_NO NULL
#define OPTION_VAL_17_MS_DELAY_NO NULL
#define OPTION_VAL_18_MS_DELAY_NO NULL
#define OPTION_VAL_19_MS_DELAY_NO NULL
#define OPTION_VAL_20_MS_DELAY_NO NULL
#define OPTION_VAL_21_MS_DELAY_NO NULL
#define OPTION_VAL_22_MS_DELAY_NO NULL
#define OPTION_VAL_23_MS_DELAY_NO NULL
#define OPTION_VAL_24_MS_DELAY_NO NULL
#define OPTION_VAL_25_MS_DELAY_NO NULL
#define OPTION_VAL_26_MS_DELAY_NO NULL
#define OPTION_VAL_27_MS_DELAY_NO NULL
#define OPTION_VAL_28_MS_DELAY_NO NULL
#define OPTION_VAL_29_MS_DELAY_NO NULL
#define OPTION_VAL_30_MS_DELAY_NO NULL
#define OPTION_VAL_31_MS_DELAY_NO NULL
#define OPTION_VAL_32_MS_DELAY_NO NULL
#define FCEUMM_SWAPDUTY_LABEL_NO NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_NO NULL
#define FCEUMM_SWAPDUTY_INFO_0_NO NULL
#define FCEUMM_SNDVOLUME_LABEL_NO NULL
#define FCEUMM_SNDVOLUME_INFO_0_NO NULL
#define OPTION_VAL_0_NO NULL
#define OPTION_VAL_1_NO NULL
#define OPTION_VAL_2_NO NULL
#define OPTION_VAL_3_NO NULL
#define OPTION_VAL_4_NO NULL
#define OPTION_VAL_5_NO NULL
#define OPTION_VAL_6_NO NULL
#define OPTION_VAL_7_NO NULL
#define OPTION_VAL_8_NO NULL
#define OPTION_VAL_9_NO NULL
#define OPTION_VAL_10_NO NULL
#define FCEUMM_APU_1_LABEL_NO NULL
#define FCEUMM_APU_1_LABEL_CAT_NO NULL
#define FCEUMM_APU_1_INFO_0_NO NULL
#define FCEUMM_APU_2_LABEL_NO NULL
#define FCEUMM_APU_2_LABEL_CAT_NO NULL
#define FCEUMM_APU_2_INFO_0_NO NULL
#define FCEUMM_APU_3_LABEL_NO NULL
#define FCEUMM_APU_3_LABEL_CAT_NO NULL
#define FCEUMM_APU_3_INFO_0_NO NULL
#define FCEUMM_APU_4_LABEL_NO NULL
#define FCEUMM_APU_4_LABEL_CAT_NO NULL
#define FCEUMM_APU_4_INFO_0_NO NULL
#define FCEUMM_APU_5_LABEL_NO NULL
#define FCEUMM_APU_5_LABEL_CAT_NO NULL
#define FCEUMM_APU_5_INFO_0_NO NULL
#define FCEUMM_TURBO_ENABLE_LABEL_NO NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_NO NULL
#define OPTION_VAL_NONE_NO "Ingen"
#define OPTION_VAL_PLAYER_1_NO NULL
#define OPTION_VAL_PLAYER_2_NO NULL
#define OPTION_VAL_BOTH_NO NULL
#define FCEUMM_TURBO_DELAY_LABEL_NO NULL
#define FCEUMM_TURBO_DELAY_INFO_0_NO NULL
#define FCEUMM_ZAPPER_MODE_LABEL_NO NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_NO NULL
#define OPTION_VAL_CLIGHTGUN_NO NULL
#define OPTION_VAL_STLIGHTGUN_NO NULL
#define OPTION_VAL_TOUCHSCREEN_NO NULL
#define OPTION_VAL_MOUSE_NO NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_NO NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_NO NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_NO NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_NO NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_NO NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_NO NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_NO NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_NO NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_NO NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_NO NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_NO NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_NO NULL
#define FCEUMM_OVERCLOCKING_LABEL_NO NULL
#define FCEUMM_OVERCLOCKING_INFO_0_NO NULL
#define OPTION_VAL_2X_POSTRENDER_NO NULL
#define OPTION_VAL_2X_VBLANK_NO NULL
#define FCEUMM_RAMSTATE_LABEL_NO NULL
#define FCEUMM_RAMSTATE_INFO_0_NO NULL
#define OPTION_VAL_FILL_FF_NO NULL
#define OPTION_VAL_FILL_00_NO NULL
#define OPTION_VAL_RANDOM_NO NULL

struct retro_core_option_v2_category option_cats_no[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_NO,
      CATEGORY_SYSTEM_INFO_0_NO
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_NO,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_NO
#else
      CATEGORY_VIDEO_INFO_1_NO
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_NO,
      CATEGORY_AUDIO_INFO_0_NO
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_NO,
      CATEGORY_INPUT_INFO_0_NO
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_NO,
      CATEGORY_HACKS_INFO_0_NO
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_NO,
      CATEGORY_DIP_SWITCH_INFO_0_NO
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_no[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_NO,
      NULL,
      FCEUMM_REGION_INFO_0_NO,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_NO },
         { "NTSC",  OPTION_VAL_NTSC_NO },
         { "PAL",   OPTION_VAL_PAL_NO },
         { "Dendy", OPTION_VAL_DENDY_NO },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_NO,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_NO,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_NO,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_NO,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_NO,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_NO,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_NO,
      NULL,
      FCEUMM_ASPECT_INFO_0_NO,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_NO },
         { "4:3",     OPTION_VAL_4_3_NO },
         { "PP",      OPTION_VAL_PP_NO },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_NO,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_NO,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_NO,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_NO,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_NO,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_NO,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_NO,
      NULL,
      FCEUMM_PALETTE_INFO_0_NO,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_NO },
         { "asqrealc",             OPTION_VAL_ASQREALC_NO },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_NO },
         { "rgb",                  OPTION_VAL_RGB_NO },
         { "yuv-v3",               OPTION_VAL_YUV_V3_NO },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_NO },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_NO },
         { "pal",                  OPTION_VAL_PAL_NO },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_NO },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_NO },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_NO },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_NO },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_NO },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_NO },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_NO },
         { "nescap",               OPTION_VAL_NESCAP_NO },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_NO },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_NO },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_NO },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_NO },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_NO },
         { "raw",                  OPTION_VAL_RAW_NO },
         { "custom",               OPTION_VAL_CUSTOM_NO },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_NO,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_NO,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_NO },
         { "svideo",     OPTION_VAL_SVIDEO_NO },
         { "rgb",        OPTION_VAL_RGB_O9_NO },
         { "monochrome", OPTION_VAL_MONOCHROME_NO },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_NO,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_NO,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_NO },
         { "High",      OPTION_VAL_HIGH_NO },
         { "Very High", OPTION_VAL_VERY_HIGH_NO },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_NO,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_NO,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_NO,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_NO,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_NO },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_NO },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_NO },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_NO },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_NO },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_NO },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_NO },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_NO },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_NO },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_NO },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_NO },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_NO },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_NO },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_NO },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_NO },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_NO },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_NO },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_NO },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_NO },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_NO },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_NO },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_NO },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_NO },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_NO },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_NO },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_NO },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_NO },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_NO },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_NO },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_NO },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_NO },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_NO },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_NO,
      FCEUMM_SWAPDUTY_LABEL_CAT_NO,
      FCEUMM_SWAPDUTY_INFO_0_NO,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_NO,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_NO,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_NO },
         { "1",  OPTION_VAL_1_NO },
         { "2",  OPTION_VAL_2_NO },
         { "3",  OPTION_VAL_3_NO },
         { "4",  OPTION_VAL_4_NO },
         { "5",  OPTION_VAL_5_NO },
         { "6",  OPTION_VAL_6_NO },
         { "7",  OPTION_VAL_7_NO },
         { "8",  OPTION_VAL_8_NO },
         { "9",  OPTION_VAL_9_NO },
         { "10", OPTION_VAL_10_NO },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_NO,
      FCEUMM_APU_1_LABEL_CAT_NO,
      FCEUMM_APU_1_INFO_0_NO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_NO,
      FCEUMM_APU_2_LABEL_CAT_NO,
      FCEUMM_APU_2_INFO_0_NO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_NO,
      FCEUMM_APU_3_LABEL_CAT_NO,
      FCEUMM_APU_3_INFO_0_NO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_NO,
      FCEUMM_APU_4_LABEL_CAT_NO,
      FCEUMM_APU_4_INFO_0_NO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_NO,
      FCEUMM_APU_5_LABEL_CAT_NO,
      FCEUMM_APU_5_INFO_0_NO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_NO,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_NO,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_NO },
         { "Player 1", OPTION_VAL_PLAYER_1_NO },
         { "Player 2", OPTION_VAL_PLAYER_2_NO },
         { "Both",     OPTION_VAL_BOTH_NO },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_NO,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_NO,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_NO,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_NO,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_NO },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_NO },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_NO },
         { "mouse",       OPTION_VAL_MOUSE_NO },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_NO,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_NO,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_NO,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_NO,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_NO,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_NO,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_NO,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_NO,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_NO,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_NO,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_NO,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_NO,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_NO,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_NO,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_NO },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_NO },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_NO,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_NO,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_NO },
         { "fill $00", OPTION_VAL_FILL_00_NO },
         { "random",   OPTION_VAL_RANDOM_NO },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_no = {
   option_cats_no,
   option_defs_no
};

/* RETRO_LANGUAGE_OC */

#define CATEGORY_SYSTEM_LABEL_OC NULL
#define CATEGORY_SYSTEM_INFO_0_OC NULL
#define CATEGORY_VIDEO_LABEL_OC "Vidèo"
#define CATEGORY_VIDEO_INFO_0_OC NULL
#define CATEGORY_VIDEO_INFO_1_OC NULL
#define CATEGORY_AUDIO_LABEL_OC NULL
#define CATEGORY_AUDIO_INFO_0_OC NULL
#define CATEGORY_INPUT_LABEL_OC NULL
#define CATEGORY_INPUT_INFO_0_OC NULL
#define CATEGORY_HACKS_LABEL_OC NULL
#define CATEGORY_HACKS_INFO_0_OC NULL
#define CATEGORY_DIP_SWITCH_LABEL_OC NULL
#define CATEGORY_DIP_SWITCH_INFO_0_OC NULL
#define FCEUMM_REGION_LABEL_OC NULL
#define FCEUMM_REGION_INFO_0_OC NULL
#define OPTION_VAL_AUTO_OC NULL
#define OPTION_VAL_NTSC_OC NULL
#define OPTION_VAL_PAL_OC NULL
#define OPTION_VAL_DENDY_OC NULL
#define FCEUMM_GAME_GENIE_LABEL_OC NULL
#define FCEUMM_GAME_GENIE_INFO_0_OC NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_OC NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_OC NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_OC NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_OC NULL
#define FCEUMM_ASPECT_LABEL_OC NULL
#define FCEUMM_ASPECT_INFO_0_OC NULL
#define OPTION_VAL_8_7_PAR_OC NULL
#define OPTION_VAL_4_3_OC NULL
#define OPTION_VAL_PP_OC NULL
#define FCEUMM_OVERSCAN_LABEL_OC NULL
#define FCEUMM_OVERSCAN_INFO_0_OC NULL
#define FCEUMM_OVERSCAN_H_LABEL_OC NULL
#define FCEUMM_OVERSCAN_H_INFO_0_OC NULL
#define FCEUMM_OVERSCAN_V_LABEL_OC NULL
#define FCEUMM_OVERSCAN_V_INFO_0_OC NULL
#define FCEUMM_PALETTE_LABEL_OC NULL
#define FCEUMM_PALETTE_INFO_0_OC NULL
#define OPTION_VAL_DEFAULT_OC NULL
#define OPTION_VAL_ASQREALC_OC NULL
#define OPTION_VAL_NINTENDO_VC_OC NULL
#define OPTION_VAL_RGB_OC NULL
#define OPTION_VAL_YUV_V3_OC NULL
#define OPTION_VAL_UNSATURATED_FINAL_OC NULL
#define OPTION_VAL_SONY_CXA2025AS_US_OC NULL
#define OPTION_VAL_BMF_FINAL2_OC NULL
#define OPTION_VAL_BMF_FINAL3_OC NULL
#define OPTION_VAL_SMOOTH_FBX_OC NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_OC NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_OC NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_OC NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_OC NULL
#define OPTION_VAL_NESCAP_OC NULL
#define OPTION_VAL_WAVEBEAM_OC NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_OC NULL
#define OPTION_VAL_MAGNUM_FBX_OC NULL
#define OPTION_VAL_SMOOTH_V2_FBX_OC NULL
#define OPTION_VAL_NES_CLASSIC_FBX_OC NULL
#define OPTION_VAL_RAW_OC NULL
#define OPTION_VAL_CUSTOM_OC NULL
#define FCEUMM_NTSC_FILTER_LABEL_OC NULL
#define FCEUMM_NTSC_FILTER_INFO_0_OC NULL
#define OPTION_VAL_COMPOSITE_OC NULL
#define OPTION_VAL_SVIDEO_OC NULL
#define OPTION_VAL_RGB_O9_OC NULL
#define OPTION_VAL_MONOCHROME_OC NULL
#define FCEUMM_SNDQUALITY_LABEL_OC NULL
#define FCEUMM_SNDQUALITY_INFO_0_OC NULL
#define OPTION_VAL_LOW_OC NULL
#define OPTION_VAL_HIGH_OC NULL
#define OPTION_VAL_VERY_HIGH_OC NULL
#define FCEUMM_SNDLOWPASS_LABEL_OC NULL
#define FCEUMM_SNDLOWPASS_INFO_0_OC NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_OC NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_OC NULL
#define OPTION_VAL_01_MS_DELAY_OC NULL
#define OPTION_VAL_02_MS_DELAY_OC NULL
#define OPTION_VAL_03_MS_DELAY_OC NULL
#define OPTION_VAL_04_MS_DELAY_OC NULL
#define OPTION_VAL_05_MS_DELAY_OC NULL
#define OPTION_VAL_06_MS_DELAY_OC NULL
#define OPTION_VAL_07_MS_DELAY_OC NULL
#define OPTION_VAL_08_MS_DELAY_OC NULL
#define OPTION_VAL_09_MS_DELAY_OC NULL
#define OPTION_VAL_10_MS_DELAY_OC NULL
#define OPTION_VAL_11_MS_DELAY_OC NULL
#define OPTION_VAL_12_MS_DELAY_OC NULL
#define OPTION_VAL_13_MS_DELAY_OC NULL
#define OPTION_VAL_14_MS_DELAY_OC NULL
#define OPTION_VAL_15_MS_DELAY_OC NULL
#define OPTION_VAL_16_MS_DELAY_OC NULL
#define OPTION_VAL_17_MS_DELAY_OC NULL
#define OPTION_VAL_18_MS_DELAY_OC NULL
#define OPTION_VAL_19_MS_DELAY_OC NULL
#define OPTION_VAL_20_MS_DELAY_OC NULL
#define OPTION_VAL_21_MS_DELAY_OC NULL
#define OPTION_VAL_22_MS_DELAY_OC NULL
#define OPTION_VAL_23_MS_DELAY_OC NULL
#define OPTION_VAL_24_MS_DELAY_OC NULL
#define OPTION_VAL_25_MS_DELAY_OC NULL
#define OPTION_VAL_26_MS_DELAY_OC NULL
#define OPTION_VAL_27_MS_DELAY_OC NULL
#define OPTION_VAL_28_MS_DELAY_OC NULL
#define OPTION_VAL_29_MS_DELAY_OC NULL
#define OPTION_VAL_30_MS_DELAY_OC NULL
#define OPTION_VAL_31_MS_DELAY_OC NULL
#define OPTION_VAL_32_MS_DELAY_OC NULL
#define FCEUMM_SWAPDUTY_LABEL_OC NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_OC NULL
#define FCEUMM_SWAPDUTY_INFO_0_OC NULL
#define FCEUMM_SNDVOLUME_LABEL_OC NULL
#define FCEUMM_SNDVOLUME_INFO_0_OC NULL
#define OPTION_VAL_0_OC NULL
#define OPTION_VAL_1_OC NULL
#define OPTION_VAL_2_OC NULL
#define OPTION_VAL_3_OC NULL
#define OPTION_VAL_4_OC NULL
#define OPTION_VAL_5_OC NULL
#define OPTION_VAL_6_OC NULL
#define OPTION_VAL_7_OC NULL
#define OPTION_VAL_8_OC NULL
#define OPTION_VAL_9_OC NULL
#define OPTION_VAL_10_OC NULL
#define FCEUMM_APU_1_LABEL_OC NULL
#define FCEUMM_APU_1_LABEL_CAT_OC NULL
#define FCEUMM_APU_1_INFO_0_OC NULL
#define FCEUMM_APU_2_LABEL_OC NULL
#define FCEUMM_APU_2_LABEL_CAT_OC NULL
#define FCEUMM_APU_2_INFO_0_OC NULL
#define FCEUMM_APU_3_LABEL_OC NULL
#define FCEUMM_APU_3_LABEL_CAT_OC NULL
#define FCEUMM_APU_3_INFO_0_OC NULL
#define FCEUMM_APU_4_LABEL_OC NULL
#define FCEUMM_APU_4_LABEL_CAT_OC NULL
#define FCEUMM_APU_4_INFO_0_OC NULL
#define FCEUMM_APU_5_LABEL_OC NULL
#define FCEUMM_APU_5_LABEL_CAT_OC NULL
#define FCEUMM_APU_5_INFO_0_OC NULL
#define FCEUMM_TURBO_ENABLE_LABEL_OC NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_OC NULL
#define OPTION_VAL_NONE_OC NULL
#define OPTION_VAL_PLAYER_1_OC NULL
#define OPTION_VAL_PLAYER_2_OC NULL
#define OPTION_VAL_BOTH_OC NULL
#define FCEUMM_TURBO_DELAY_LABEL_OC NULL
#define FCEUMM_TURBO_DELAY_INFO_0_OC NULL
#define FCEUMM_ZAPPER_MODE_LABEL_OC NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_OC NULL
#define OPTION_VAL_CLIGHTGUN_OC NULL
#define OPTION_VAL_STLIGHTGUN_OC NULL
#define OPTION_VAL_TOUCHSCREEN_OC NULL
#define OPTION_VAL_MOUSE_OC NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_OC NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_OC NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_OC NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_OC NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_OC NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_OC NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_OC NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_OC NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_OC NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_OC NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_OC NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_OC NULL
#define FCEUMM_OVERCLOCKING_LABEL_OC NULL
#define FCEUMM_OVERCLOCKING_INFO_0_OC NULL
#define OPTION_VAL_2X_POSTRENDER_OC NULL
#define OPTION_VAL_2X_VBLANK_OC NULL
#define FCEUMM_RAMSTATE_LABEL_OC NULL
#define FCEUMM_RAMSTATE_INFO_0_OC NULL
#define OPTION_VAL_FILL_FF_OC NULL
#define OPTION_VAL_FILL_00_OC NULL
#define OPTION_VAL_RANDOM_OC NULL

struct retro_core_option_v2_category option_cats_oc[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_OC,
      CATEGORY_SYSTEM_INFO_0_OC
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_OC,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_OC
#else
      CATEGORY_VIDEO_INFO_1_OC
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_OC,
      CATEGORY_AUDIO_INFO_0_OC
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_OC,
      CATEGORY_INPUT_INFO_0_OC
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_OC,
      CATEGORY_HACKS_INFO_0_OC
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_OC,
      CATEGORY_DIP_SWITCH_INFO_0_OC
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_oc[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_OC,
      NULL,
      FCEUMM_REGION_INFO_0_OC,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_OC },
         { "NTSC",  OPTION_VAL_NTSC_OC },
         { "PAL",   OPTION_VAL_PAL_OC },
         { "Dendy", OPTION_VAL_DENDY_OC },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_OC,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_OC,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_OC,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_OC,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_OC,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_OC,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_OC,
      NULL,
      FCEUMM_ASPECT_INFO_0_OC,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_OC },
         { "4:3",     OPTION_VAL_4_3_OC },
         { "PP",      OPTION_VAL_PP_OC },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_OC,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_OC,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_OC,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_OC,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_OC,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_OC,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_OC,
      NULL,
      FCEUMM_PALETTE_INFO_0_OC,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_OC },
         { "asqrealc",             OPTION_VAL_ASQREALC_OC },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_OC },
         { "rgb",                  OPTION_VAL_RGB_OC },
         { "yuv-v3",               OPTION_VAL_YUV_V3_OC },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_OC },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_OC },
         { "pal",                  OPTION_VAL_PAL_OC },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_OC },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_OC },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_OC },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_OC },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_OC },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_OC },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_OC },
         { "nescap",               OPTION_VAL_NESCAP_OC },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_OC },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_OC },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_OC },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_OC },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_OC },
         { "raw",                  OPTION_VAL_RAW_OC },
         { "custom",               OPTION_VAL_CUSTOM_OC },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_OC,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_OC,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_OC },
         { "svideo",     OPTION_VAL_SVIDEO_OC },
         { "rgb",        OPTION_VAL_RGB_O9_OC },
         { "monochrome", OPTION_VAL_MONOCHROME_OC },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_OC,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_OC,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_OC },
         { "High",      OPTION_VAL_HIGH_OC },
         { "Very High", OPTION_VAL_VERY_HIGH_OC },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_OC,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_OC,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_OC,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_OC,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_OC },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_OC },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_OC },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_OC },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_OC },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_OC },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_OC },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_OC },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_OC },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_OC },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_OC },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_OC },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_OC },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_OC },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_OC },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_OC },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_OC },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_OC },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_OC },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_OC },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_OC },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_OC },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_OC },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_OC },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_OC },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_OC },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_OC },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_OC },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_OC },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_OC },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_OC },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_OC },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_OC,
      FCEUMM_SWAPDUTY_LABEL_CAT_OC,
      FCEUMM_SWAPDUTY_INFO_0_OC,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_OC,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_OC,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_OC },
         { "1",  OPTION_VAL_1_OC },
         { "2",  OPTION_VAL_2_OC },
         { "3",  OPTION_VAL_3_OC },
         { "4",  OPTION_VAL_4_OC },
         { "5",  OPTION_VAL_5_OC },
         { "6",  OPTION_VAL_6_OC },
         { "7",  OPTION_VAL_7_OC },
         { "8",  OPTION_VAL_8_OC },
         { "9",  OPTION_VAL_9_OC },
         { "10", OPTION_VAL_10_OC },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_OC,
      FCEUMM_APU_1_LABEL_CAT_OC,
      FCEUMM_APU_1_INFO_0_OC,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_OC,
      FCEUMM_APU_2_LABEL_CAT_OC,
      FCEUMM_APU_2_INFO_0_OC,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_OC,
      FCEUMM_APU_3_LABEL_CAT_OC,
      FCEUMM_APU_3_INFO_0_OC,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_OC,
      FCEUMM_APU_4_LABEL_CAT_OC,
      FCEUMM_APU_4_INFO_0_OC,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_OC,
      FCEUMM_APU_5_LABEL_CAT_OC,
      FCEUMM_APU_5_INFO_0_OC,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_OC,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_OC,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_OC },
         { "Player 1", OPTION_VAL_PLAYER_1_OC },
         { "Player 2", OPTION_VAL_PLAYER_2_OC },
         { "Both",     OPTION_VAL_BOTH_OC },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_OC,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_OC,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_OC,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_OC,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_OC },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_OC },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_OC },
         { "mouse",       OPTION_VAL_MOUSE_OC },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_OC,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_OC,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_OC,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_OC,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_OC,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_OC,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_OC,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_OC,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_OC,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_OC,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_OC,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_OC,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_OC,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_OC,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_OC },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_OC },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_OC,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_OC,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_OC },
         { "fill $00", OPTION_VAL_FILL_00_OC },
         { "random",   OPTION_VAL_RANDOM_OC },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_oc = {
   option_cats_oc,
   option_defs_oc
};

/* RETRO_LANGUAGE_PL */

#define CATEGORY_SYSTEM_LABEL_PL NULL
#define CATEGORY_SYSTEM_INFO_0_PL NULL
#define CATEGORY_VIDEO_LABEL_PL "Wideo"
#define CATEGORY_VIDEO_INFO_0_PL NULL
#define CATEGORY_VIDEO_INFO_1_PL NULL
#define CATEGORY_AUDIO_LABEL_PL NULL
#define CATEGORY_AUDIO_INFO_0_PL NULL
#define CATEGORY_INPUT_LABEL_PL "Wejście"
#define CATEGORY_INPUT_INFO_0_PL NULL
#define CATEGORY_HACKS_LABEL_PL "Hacki emulacyjne"
#define CATEGORY_HACKS_INFO_0_PL "Zmiana ustawień podkręcania procesora i dokładności emulacji, mających wpływ na niskopoziomową wydajność i kompatybilność."
#define CATEGORY_DIP_SWITCH_LABEL_PL "Przełączniki DIP"
#define CATEGORY_DIP_SWITCH_INFO_0_PL NULL
#define FCEUMM_REGION_LABEL_PL NULL
#define FCEUMM_REGION_INFO_0_PL NULL
#define OPTION_VAL_AUTO_PL NULL
#define OPTION_VAL_NTSC_PL NULL
#define OPTION_VAL_PAL_PL NULL
#define OPTION_VAL_DENDY_PL NULL
#define FCEUMM_GAME_GENIE_LABEL_PL "Dodatek Game Genie (wymagany tekst)"
#define FCEUMM_GAME_GENIE_INFO_0_PL NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_PL "Pokaż zaawansowane opcje systemowe"
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_PL "Pokaż zaawansowane opcje systemowe i ulepszenia."
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_PL "Pokaż zaawansowane opcje dźwięku"
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_PL "Pokaż zaawansowane opcje dźwięku i ulepszenia."
#define FCEUMM_ASPECT_LABEL_PL "Współczynnik proporcji"
#define FCEUMM_ASPECT_INFO_0_PL "Wybierz preferowany współczynnik aspektów."
#define OPTION_VAL_8_7_PAR_PL NULL
#define OPTION_VAL_4_3_PL NULL
#define OPTION_VAL_PP_PL "Pikselowa perfekcja"
#define FCEUMM_OVERSCAN_LABEL_PL "Przytnij overscan"
#define FCEUMM_OVERSCAN_INFO_0_PL "Usuwa przeskanowanie 8 pikseli ze wszystkich stron ekranu."
#define FCEUMM_OVERSCAN_H_LABEL_PL "Przytnij poziomy overscan"
#define FCEUMM_OVERSCAN_H_INFO_0_PL NULL
#define FCEUMM_OVERSCAN_V_LABEL_PL "Przytnij pionowy overscan"
#define FCEUMM_OVERSCAN_V_INFO_0_PL "Usuwa 8 pikseli z góry i na dole ekranu, aby symulować przeskanowanie widoczne w standardowych telewizorach CRT."
#define FCEUMM_PALETTE_LABEL_PL "Paleta kolorów"
#define FCEUMM_PALETTE_INFO_0_PL NULL
#define OPTION_VAL_DEFAULT_PL "Domyślny"
#define OPTION_VAL_ASQREALC_PL NULL
#define OPTION_VAL_NINTENDO_VC_PL "Konsola wirtualna Nintendo"
#define OPTION_VAL_RGB_PL NULL
#define OPTION_VAL_YUV_V3_PL "FBX YUV-V3"
#define OPTION_VAL_UNSATURATED_FINAL_PL NULL
#define OPTION_VAL_SONY_CXA2025AS_US_PL "Sony CXA2025AS USA"
#define OPTION_VAL_BMF_FINAL2_PL NULL
#define OPTION_VAL_BMF_FINAL3_PL NULL
#define OPTION_VAL_SMOOTH_FBX_PL NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_PL NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_PL NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_PL NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_PL NULL
#define OPTION_VAL_NESCAP_PL NULL
#define OPTION_VAL_WAVEBEAM_PL NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_PL NULL
#define OPTION_VAL_MAGNUM_FBX_PL NULL
#define OPTION_VAL_SMOOTH_V2_FBX_PL NULL
#define OPTION_VAL_NES_CLASSIC_FBX_PL NULL
#define OPTION_VAL_RAW_PL NULL
#define OPTION_VAL_CUSTOM_PL "Własny"
#define FCEUMM_NTSC_FILTER_LABEL_PL "Filtr NTSC"
#define FCEUMM_NTSC_FILTER_INFO_0_PL NULL
#define OPTION_VAL_COMPOSITE_PL "Kompozytowy"
#define OPTION_VAL_SVIDEO_PL NULL
#define OPTION_VAL_RGB_O9_PL NULL
#define OPTION_VAL_MONOCHROME_PL "Monochromia"
#define FCEUMM_SNDQUALITY_LABEL_PL "Jakość dźwięku"
#define FCEUMM_SNDQUALITY_INFO_0_PL "Włącz wyższą jakość dźwięku. Zwiększa wymagania wydajności."
#define OPTION_VAL_LOW_PL "Niski"
#define OPTION_VAL_HIGH_PL "Wysoki"
#define OPTION_VAL_VERY_HIGH_PL "Bardzo wysoki"
#define FCEUMM_SNDLOWPASS_LABEL_PL NULL
#define FCEUMM_SNDLOWPASS_INFO_0_PL NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_PL NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_PL NULL
#define OPTION_VAL_01_MS_DELAY_PL NULL
#define OPTION_VAL_02_MS_DELAY_PL NULL
#define OPTION_VAL_03_MS_DELAY_PL NULL
#define OPTION_VAL_04_MS_DELAY_PL NULL
#define OPTION_VAL_05_MS_DELAY_PL NULL
#define OPTION_VAL_06_MS_DELAY_PL NULL
#define OPTION_VAL_07_MS_DELAY_PL NULL
#define OPTION_VAL_08_MS_DELAY_PL NULL
#define OPTION_VAL_09_MS_DELAY_PL NULL
#define OPTION_VAL_10_MS_DELAY_PL NULL
#define OPTION_VAL_11_MS_DELAY_PL NULL
#define OPTION_VAL_12_MS_DELAY_PL NULL
#define OPTION_VAL_13_MS_DELAY_PL NULL
#define OPTION_VAL_14_MS_DELAY_PL NULL
#define OPTION_VAL_15_MS_DELAY_PL NULL
#define OPTION_VAL_16_MS_DELAY_PL NULL
#define OPTION_VAL_17_MS_DELAY_PL NULL
#define OPTION_VAL_18_MS_DELAY_PL NULL
#define OPTION_VAL_19_MS_DELAY_PL NULL
#define OPTION_VAL_20_MS_DELAY_PL NULL
#define OPTION_VAL_21_MS_DELAY_PL NULL
#define OPTION_VAL_22_MS_DELAY_PL NULL
#define OPTION_VAL_23_MS_DELAY_PL NULL
#define OPTION_VAL_24_MS_DELAY_PL NULL
#define OPTION_VAL_25_MS_DELAY_PL NULL
#define OPTION_VAL_26_MS_DELAY_PL NULL
#define OPTION_VAL_27_MS_DELAY_PL NULL
#define OPTION_VAL_28_MS_DELAY_PL NULL
#define OPTION_VAL_29_MS_DELAY_PL NULL
#define OPTION_VAL_30_MS_DELAY_PL NULL
#define OPTION_VAL_31_MS_DELAY_PL NULL
#define OPTION_VAL_32_MS_DELAY_PL NULL
#define FCEUMM_SWAPDUTY_LABEL_PL "Zamień cykle pracy audio"
#define FCEUMM_SWAPDUTY_LABEL_CAT_PL NULL
#define FCEUMM_SWAPDUTY_INFO_0_PL NULL
#define FCEUMM_SNDVOLUME_LABEL_PL NULL
#define FCEUMM_SNDVOLUME_INFO_0_PL "Zmień główny poziom głośności."
#define OPTION_VAL_0_PL NULL
#define OPTION_VAL_1_PL NULL
#define OPTION_VAL_2_PL NULL
#define OPTION_VAL_3_PL "30 %"
#define OPTION_VAL_4_PL NULL
#define OPTION_VAL_5_PL NULL
#define OPTION_VAL_6_PL NULL
#define OPTION_VAL_7_PL NULL
#define OPTION_VAL_8_PL NULL
#define OPTION_VAL_9_PL NULL
#define OPTION_VAL_10_PL NULL
#define FCEUMM_APU_1_LABEL_PL "Kanał audio 1 (Square 1)"
#define FCEUMM_APU_1_LABEL_CAT_PL "Kanał 1 (Square 1)"
#define FCEUMM_APU_1_INFO_0_PL "Włącza lub wyłącza generator dźwięku fal impulsowych 1."
#define FCEUMM_APU_2_LABEL_PL "Kanał audio 2 (Square 2)"
#define FCEUMM_APU_2_LABEL_CAT_PL "Kanał 2 (Square 2)"
#define FCEUMM_APU_2_INFO_0_PL "Włącza lub wyłącza generator dźwięku fal impulsowych 2."
#define FCEUMM_APU_3_LABEL_PL "Kanał audio 3 (Triangle)"
#define FCEUMM_APU_3_LABEL_CAT_PL "Kanał 3 (Triangle)"
#define FCEUMM_APU_3_INFO_0_PL NULL
#define FCEUMM_APU_4_LABEL_PL NULL
#define FCEUMM_APU_4_LABEL_CAT_PL NULL
#define FCEUMM_APU_4_INFO_0_PL NULL
#define FCEUMM_APU_5_LABEL_PL NULL
#define FCEUMM_APU_5_LABEL_CAT_PL NULL
#define FCEUMM_APU_5_INFO_0_PL NULL
#define FCEUMM_TURBO_ENABLE_LABEL_PL "Włącz turbo"
#define FCEUMM_TURBO_ENABLE_INFO_0_PL NULL
#define OPTION_VAL_NONE_PL "Brak"
#define OPTION_VAL_PLAYER_1_PL "Gracz nr. 1"
#define OPTION_VAL_PLAYER_2_PL "Gracz nr. 2"
#define OPTION_VAL_BOTH_PL NULL
#define FCEUMM_TURBO_DELAY_LABEL_PL NULL
#define FCEUMM_TURBO_DELAY_INFO_0_PL NULL
#define FCEUMM_ZAPPER_MODE_LABEL_PL NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_PL NULL
#define OPTION_VAL_CLIGHTGUN_PL NULL
#define OPTION_VAL_STLIGHTGUN_PL NULL
#define OPTION_VAL_TOUCHSCREEN_PL "Ekran dotykowy"
#define OPTION_VAL_MOUSE_PL NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_PL NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_PL NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_PL NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_PL NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_PL NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_PL NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_PL NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_PL NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_PL "Zezwalaj na przeciwne kierunki"
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_PL NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_PL "Bez limitu duszków"
#define FCEUMM_NOSPRITELIMIT_INFO_0_PL NULL
#define FCEUMM_OVERCLOCKING_LABEL_PL NULL
#define FCEUMM_OVERCLOCKING_INFO_0_PL NULL
#define OPTION_VAL_2X_POSTRENDER_PL NULL
#define OPTION_VAL_2X_VBLANK_PL NULL
#define FCEUMM_RAMSTATE_LABEL_PL NULL
#define FCEUMM_RAMSTATE_INFO_0_PL NULL
#define OPTION_VAL_FILL_FF_PL NULL
#define OPTION_VAL_FILL_00_PL NULL
#define OPTION_VAL_RANDOM_PL NULL

struct retro_core_option_v2_category option_cats_pl[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_PL,
      CATEGORY_SYSTEM_INFO_0_PL
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_PL,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_PL
#else
      CATEGORY_VIDEO_INFO_1_PL
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_PL,
      CATEGORY_AUDIO_INFO_0_PL
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_PL,
      CATEGORY_INPUT_INFO_0_PL
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_PL,
      CATEGORY_HACKS_INFO_0_PL
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_PL,
      CATEGORY_DIP_SWITCH_INFO_0_PL
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_pl[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_PL,
      NULL,
      FCEUMM_REGION_INFO_0_PL,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_PL },
         { "NTSC",  OPTION_VAL_NTSC_PL },
         { "PAL",   OPTION_VAL_PAL_PL },
         { "Dendy", OPTION_VAL_DENDY_PL },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_PL,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_PL,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_PL,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_PL,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_PL,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_PL,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_PL,
      NULL,
      FCEUMM_ASPECT_INFO_0_PL,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_PL },
         { "4:3",     OPTION_VAL_4_3_PL },
         { "PP",      OPTION_VAL_PP_PL },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_PL,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_PL,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_PL,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_PL,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_PL,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_PL,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_PL,
      NULL,
      FCEUMM_PALETTE_INFO_0_PL,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_PL },
         { "asqrealc",             OPTION_VAL_ASQREALC_PL },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_PL },
         { "rgb",                  OPTION_VAL_RGB_PL },
         { "yuv-v3",               OPTION_VAL_YUV_V3_PL },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_PL },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_PL },
         { "pal",                  OPTION_VAL_PAL_PL },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_PL },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_PL },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_PL },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_PL },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_PL },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_PL },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_PL },
         { "nescap",               OPTION_VAL_NESCAP_PL },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_PL },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_PL },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_PL },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_PL },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_PL },
         { "raw",                  OPTION_VAL_RAW_PL },
         { "custom",               OPTION_VAL_CUSTOM_PL },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_PL,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_PL,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_PL },
         { "svideo",     OPTION_VAL_SVIDEO_PL },
         { "rgb",        OPTION_VAL_RGB_O9_PL },
         { "monochrome", OPTION_VAL_MONOCHROME_PL },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_PL,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_PL,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_PL },
         { "High",      OPTION_VAL_HIGH_PL },
         { "Very High", OPTION_VAL_VERY_HIGH_PL },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_PL,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_PL,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_PL,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_PL,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_PL },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_PL },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_PL },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_PL },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_PL },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_PL },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_PL },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_PL },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_PL },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_PL },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_PL },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_PL },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_PL },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_PL },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_PL },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_PL },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_PL },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_PL },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_PL },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_PL },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_PL },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_PL },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_PL },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_PL },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_PL },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_PL },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_PL },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_PL },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_PL },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_PL },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_PL },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_PL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_PL,
      FCEUMM_SWAPDUTY_LABEL_CAT_PL,
      FCEUMM_SWAPDUTY_INFO_0_PL,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_PL,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_PL,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_PL },
         { "1",  OPTION_VAL_1_PL },
         { "2",  OPTION_VAL_2_PL },
         { "3",  OPTION_VAL_3_PL },
         { "4",  OPTION_VAL_4_PL },
         { "5",  OPTION_VAL_5_PL },
         { "6",  OPTION_VAL_6_PL },
         { "7",  OPTION_VAL_7_PL },
         { "8",  OPTION_VAL_8_PL },
         { "9",  OPTION_VAL_9_PL },
         { "10", OPTION_VAL_10_PL },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_PL,
      FCEUMM_APU_1_LABEL_CAT_PL,
      FCEUMM_APU_1_INFO_0_PL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_PL,
      FCEUMM_APU_2_LABEL_CAT_PL,
      FCEUMM_APU_2_INFO_0_PL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_PL,
      FCEUMM_APU_3_LABEL_CAT_PL,
      FCEUMM_APU_3_INFO_0_PL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_PL,
      FCEUMM_APU_4_LABEL_CAT_PL,
      FCEUMM_APU_4_INFO_0_PL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_PL,
      FCEUMM_APU_5_LABEL_CAT_PL,
      FCEUMM_APU_5_INFO_0_PL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_PL,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_PL,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_PL },
         { "Player 1", OPTION_VAL_PLAYER_1_PL },
         { "Player 2", OPTION_VAL_PLAYER_2_PL },
         { "Both",     OPTION_VAL_BOTH_PL },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_PL,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_PL,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_PL,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_PL,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_PL },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_PL },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_PL },
         { "mouse",       OPTION_VAL_MOUSE_PL },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_PL,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_PL,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_PL,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_PL,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_PL,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_PL,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_PL,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_PL,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_PL,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_PL,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_PL,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_PL,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_PL,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_PL,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_PL },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_PL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_PL,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_PL,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_PL },
         { "fill $00", OPTION_VAL_FILL_00_PL },
         { "random",   OPTION_VAL_RANDOM_PL },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_pl = {
   option_cats_pl,
   option_defs_pl
};

/* RETRO_LANGUAGE_PT_BR */

#define CATEGORY_SYSTEM_LABEL_PT_BR "Sistema"
#define CATEGORY_SYSTEM_INFO_0_PT_BR NULL
#define CATEGORY_VIDEO_LABEL_PT_BR "Vídeo"
#define CATEGORY_VIDEO_INFO_0_PT_BR NULL
#define CATEGORY_VIDEO_INFO_1_PT_BR NULL
#define CATEGORY_AUDIO_LABEL_PT_BR "Áudio"
#define CATEGORY_AUDIO_INFO_0_PT_BR NULL
#define CATEGORY_INPUT_LABEL_PT_BR "Entrada"
#define CATEGORY_INPUT_INFO_0_PT_BR NULL
#define CATEGORY_HACKS_LABEL_PT_BR "Hacks de emulação"
#define CATEGORY_HACKS_INFO_0_PT_BR "Seleciona as configurações de precisão do overclock e da emulação do processador que afetam o baixo desempenho e a compatibilidade."
#define CATEGORY_DIP_SWITCH_LABEL_PT_BR "Chaves DIP"
#define CATEGORY_DIP_SWITCH_INFO_0_PT_BR NULL
#define FCEUMM_REGION_LABEL_PT_BR "Região"
#define FCEUMM_REGION_INFO_0_PT_BR "Impõem que o núcleo faça uso do sincronismo com base nas regiões NTSC, PAL ou Dendy."
#define OPTION_VAL_AUTO_PT_BR "Automático"
#define OPTION_VAL_NTSC_PT_BR NULL
#define OPTION_VAL_PAL_PT_BR NULL
#define OPTION_VAL_DENDY_PT_BR NULL
#define FCEUMM_GAME_GENIE_LABEL_PT_BR "Complemento do Game Genie (requer reinício)"
#define FCEUMM_GAME_GENIE_INFO_0_PT_BR "Ativa a emulação de um cartucho do Game Genie, permitindo a inserção de códigos de trapaça ao iniciar os jogos. O arquivo ROM 'gamegenie.nes' do Game Genie deve estar presente no diretório do sistema. Não se aplica ao conteúdo FDS ou arcade."
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_PT_BR "Mostra opções avançadas do sistema"
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_PT_BR "Mostra opções avançadas e ajustes do sistema."
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_PT_BR "Mostra opções avançadas do áudio"
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_PT_BR "Mostra opções avançadas e ajustes do áudio."
#define FCEUMM_ASPECT_LABEL_PT_BR "Proporção da tela"
#define FCEUMM_ASPECT_INFO_0_PT_BR "Escolha a proporção preferida da tela."
#define OPTION_VAL_8_7_PAR_PT_BR NULL
#define OPTION_VAL_4_3_PT_BR NULL
#define OPTION_VAL_PP_PT_BR "Pixel perfeito"
#define FCEUMM_OVERSCAN_LABEL_PT_BR "Cortar overscan"
#define FCEUMM_OVERSCAN_INFO_0_PT_BR "Remove 8 pixels excedentes de todos os lados da tela."
#define FCEUMM_OVERSCAN_H_LABEL_PT_BR "Corta o excedente horizontal"
#define FCEUMM_OVERSCAN_H_INFO_0_PT_BR "Remove 8 pixels excedentes do lado esquerdo e direito da tela para simular um transbordamento que ocorre nas telas CRT."
#define FCEUMM_OVERSCAN_V_LABEL_PT_BR "Corta o excedente vertical"
#define FCEUMM_OVERSCAN_V_INFO_0_PT_BR "Remove 8 pixels excedentes de cima e de baixo da tela para simular um transbordamento que ocorre nas telas CRT."
#define FCEUMM_PALETTE_LABEL_PT_BR "Paleta de cores"
#define FCEUMM_PALETTE_INFO_0_PT_BR "Escolha a partir de paletas geradas previamente, uma paleta personalizada com 64x3 a partir de um arquivo ou um formato bruto (precisa usar um shader do nes-decoder)."
#define OPTION_VAL_DEFAULT_PT_BR "Padrão"
#define OPTION_VAL_ASQREALC_PT_BR "Real do AspiringSquire"
#define OPTION_VAL_NINTENDO_VC_PT_BR "Console virtual da Nintendo"
#define OPTION_VAL_RGB_PT_BR "PPU RGB da Nintendo"
#define OPTION_VAL_YUV_V3_PT_BR "YUV-V3 do FBX"
#define OPTION_VAL_UNSATURATED_FINAL_PT_BR "Final Dessaturado do FBX"
#define OPTION_VAL_SONY_CXA2025AS_US_PT_BR "Sony CXA2025AS EUA"
#define OPTION_VAL_BMF_FINAL2_PT_BR "Final 2 do BMF"
#define OPTION_VAL_BMF_FINAL3_PT_BR "Final 3 do BMF"
#define OPTION_VAL_SMOOTH_FBX_PT_BR "Suavização do FBX"
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_PT_BR "Composto direto do FBX"
#define OPTION_VAL_PVM_STYLE_D93_FBX_PT_BR "Estilo PVM do FBX D93"
#define OPTION_VAL_NTSC_HARDWARE_FBX_PT_BR "Hardware NTSC do FBX"
#define OPTION_VAL_NES_CLASSIC_FBX_FS_PT_BR NULL
#define OPTION_VAL_NESCAP_PT_BR NULL
#define OPTION_VAL_WAVEBEAM_PT_BR "Wvebeam do nakedarthur"
#define OPTION_VAL_DIGITAL_PRIME_FBX_PT_BR NULL
#define OPTION_VAL_MAGNUM_FBX_PT_BR NULL
#define OPTION_VAL_SMOOTH_V2_FBX_PT_BR NULL
#define OPTION_VAL_NES_CLASSIC_FBX_PT_BR NULL
#define OPTION_VAL_RAW_PT_BR "Bruto"
#define OPTION_VAL_CUSTOM_PT_BR "Personalizado"
#define FCEUMM_NTSC_FILTER_LABEL_PT_BR "Filtro NTSC"
#define FCEUMM_NTSC_FILTER_INFO_0_PT_BR NULL
#define OPTION_VAL_COMPOSITE_PT_BR "Composto"
#define OPTION_VAL_SVIDEO_PT_BR NULL
#define OPTION_VAL_RGB_O9_PT_BR NULL
#define OPTION_VAL_MONOCHROME_PT_BR "Monocromático"
#define FCEUMM_SNDQUALITY_LABEL_PT_BR "Qualidade do áudio"
#define FCEUMM_SNDQUALITY_INFO_0_PT_BR "Ativa um áudio com maior qualidade. Demanda mais recursos."
#define OPTION_VAL_LOW_PT_BR "Baixo"
#define OPTION_VAL_HIGH_PT_BR "Alto"
#define OPTION_VAL_VERY_HIGH_PT_BR "Muito alto"
#define FCEUMM_SNDLOWPASS_LABEL_PT_BR NULL
#define FCEUMM_SNDLOWPASS_INFO_0_PT_BR NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_PT_BR NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_PT_BR NULL
#define OPTION_VAL_01_MS_DELAY_PT_BR NULL
#define OPTION_VAL_02_MS_DELAY_PT_BR NULL
#define OPTION_VAL_03_MS_DELAY_PT_BR NULL
#define OPTION_VAL_04_MS_DELAY_PT_BR NULL
#define OPTION_VAL_05_MS_DELAY_PT_BR NULL
#define OPTION_VAL_06_MS_DELAY_PT_BR NULL
#define OPTION_VAL_07_MS_DELAY_PT_BR NULL
#define OPTION_VAL_08_MS_DELAY_PT_BR NULL
#define OPTION_VAL_09_MS_DELAY_PT_BR NULL
#define OPTION_VAL_10_MS_DELAY_PT_BR NULL
#define OPTION_VAL_11_MS_DELAY_PT_BR NULL
#define OPTION_VAL_12_MS_DELAY_PT_BR NULL
#define OPTION_VAL_13_MS_DELAY_PT_BR NULL
#define OPTION_VAL_14_MS_DELAY_PT_BR NULL
#define OPTION_VAL_15_MS_DELAY_PT_BR NULL
#define OPTION_VAL_16_MS_DELAY_PT_BR NULL
#define OPTION_VAL_17_MS_DELAY_PT_BR NULL
#define OPTION_VAL_18_MS_DELAY_PT_BR NULL
#define OPTION_VAL_19_MS_DELAY_PT_BR NULL
#define OPTION_VAL_20_MS_DELAY_PT_BR NULL
#define OPTION_VAL_21_MS_DELAY_PT_BR NULL
#define OPTION_VAL_22_MS_DELAY_PT_BR NULL
#define OPTION_VAL_23_MS_DELAY_PT_BR NULL
#define OPTION_VAL_24_MS_DELAY_PT_BR NULL
#define OPTION_VAL_25_MS_DELAY_PT_BR NULL
#define OPTION_VAL_26_MS_DELAY_PT_BR NULL
#define OPTION_VAL_27_MS_DELAY_PT_BR NULL
#define OPTION_VAL_28_MS_DELAY_PT_BR NULL
#define OPTION_VAL_29_MS_DELAY_PT_BR NULL
#define OPTION_VAL_30_MS_DELAY_PT_BR NULL
#define OPTION_VAL_31_MS_DELAY_PT_BR NULL
#define OPTION_VAL_32_MS_DELAY_PT_BR NULL
#define FCEUMM_SWAPDUTY_LABEL_PT_BR "Alterna os ciclos de trabalho do áudio"
#define FCEUMM_SWAPDUTY_LABEL_CAT_PT_BR "Alterna os ciclos de trabalho"
#define FCEUMM_SWAPDUTY_INFO_0_PT_BR "Faz a simulação de áudio de clones do famicom que tem o pulso dos bits da onda dos canais invertido."
#define FCEUMM_SNDVOLUME_LABEL_PT_BR "Volume principal"
#define FCEUMM_SNDVOLUME_INFO_0_PT_BR "Altera o nível do volume principal."
#define OPTION_VAL_0_PT_BR NULL
#define OPTION_VAL_1_PT_BR NULL
#define OPTION_VAL_2_PT_BR NULL
#define OPTION_VAL_3_PT_BR NULL
#define OPTION_VAL_4_PT_BR NULL
#define OPTION_VAL_5_PT_BR NULL
#define OPTION_VAL_6_PT_BR NULL
#define OPTION_VAL_7_PT_BR NULL
#define OPTION_VAL_8_PT_BR NULL
#define OPTION_VAL_9_PT_BR NULL
#define OPTION_VAL_10_PT_BR NULL
#define FCEUMM_APU_1_LABEL_PT_BR "Canal de áudio 1 (Quadrado 1)"
#define FCEUMM_APU_1_LABEL_CAT_PT_BR "Canal 1 (Quadrado 1)"
#define FCEUMM_APU_1_INFO_0_PT_BR "Ativa ou desativa a saída de áudio 1 do gerador de pulso de ondas."
#define FCEUMM_APU_2_LABEL_PT_BR "Canal de áudio 2 (Quadrado 2)"
#define FCEUMM_APU_2_LABEL_CAT_PT_BR "Canal 2 (Quadrado 2)"
#define FCEUMM_APU_2_INFO_0_PT_BR "Ativa ou desativa a saída de áudio 2 do gerador de pulso de ondas."
#define FCEUMM_APU_3_LABEL_PT_BR "Canal de áudio 3 (Triângulo)"
#define FCEUMM_APU_3_LABEL_CAT_PT_BR "Canal 3 (Triângulo)"
#define FCEUMM_APU_3_INFO_0_PT_BR "Ativa ou desativa a saída de áudio do gerador de onda triangular."
#define FCEUMM_APU_4_LABEL_PT_BR "Canal de áudio 4 (Ruído)"
#define FCEUMM_APU_4_LABEL_CAT_PT_BR "Canal 4 (Ruído)"
#define FCEUMM_APU_4_INFO_0_PT_BR "Ativa ou desativa a saída de áudio do gerador de ruído."
#define FCEUMM_APU_5_LABEL_PT_BR "Canal de áudio 5 (PCM)"
#define FCEUMM_APU_5_LABEL_CAT_PT_BR "Canal 5 (PCM)"
#define FCEUMM_APU_5_INFO_0_PT_BR "Ativa ou desativa a saída de áudio do canal de modulação delta."
#define FCEUMM_TURBO_ENABLE_LABEL_PT_BR "Ativa o turbo"
#define FCEUMM_TURBO_ENABLE_INFO_0_PT_BR "Ativa ou desativa os botões turbo."
#define OPTION_VAL_NONE_PT_BR "Nenhum"
#define OPTION_VAL_PLAYER_1_PT_BR "Jogador 1"
#define OPTION_VAL_PLAYER_2_PT_BR "Jogador 2"
#define OPTION_VAL_BOTH_PT_BR "Ambos"
#define FCEUMM_TURBO_DELAY_LABEL_PT_BR "Atraso do turbo (em quadros)"
#define FCEUMM_TURBO_DELAY_INFO_0_PT_BR "A taxa de repetição dos botões turbo em quadros."
#define FCEUMM_ZAPPER_MODE_LABEL_PT_BR "Modo zapper"
#define FCEUMM_ZAPPER_MODE_INFO_0_PT_BR NULL
#define OPTION_VAL_CLIGHTGUN_PT_BR NULL
#define OPTION_VAL_STLIGHTGUN_PT_BR NULL
#define OPTION_VAL_TOUCHSCREEN_PT_BR "Tela sensível ao toque"
#define OPTION_VAL_MOUSE_PT_BR NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_PT_BR "Mostra a mira do zapper"
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_PT_BR NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_PT_BR "Tolerância do zapper"
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_PT_BR "Define quantos pixels na área de destino está no alvo."
#define FCEUMM_ZAPPER_TRIGGER_LABEL_PT_BR NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_PT_BR NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_PT_BR NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_PT_BR NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_PT_BR "Permite direções contrárias"
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_PT_BR "Permite que as posições CIMA+BAIXO ou ESQUERDA+DIREITA possam ser pressionadas simultaneamente."
#define FCEUMM_NOSPRITELIMIT_LABEL_PT_BR "Sem tempo limite do sprite"
#define FCEUMM_NOSPRITELIMIT_INFO_0_PT_BR "Remove o limite do hardware original de 8 sprites por linha de varredura. Reduz as tremulações dos sprites, mas alguns jogos podem apresentar defeitos gráficos, pois aproveitam este limite para exibir certos efeitos."
#define FCEUMM_OVERCLOCKING_LABEL_PT_BR NULL
#define FCEUMM_OVERCLOCKING_INFO_0_PT_BR "Habilita ou desabilita o overclock, o que pode reduzir lentidão em alguns jogos. O método de Pós-renderização é mais compatível com todos os jogos, Vblank é mais eficaz para jogos como Contra Force."
#define OPTION_VAL_2X_POSTRENDER_PT_BR "Pós-renderização 2x"
#define OPTION_VAL_2X_VBLANK_PT_BR "VBlank 2x"
#define FCEUMM_RAMSTATE_LABEL_PT_BR "Preenchimento da RAM na inicialização (requer reinício)"
#define FCEUMM_RAMSTATE_INFO_0_PT_BR "Valores da RAM ao ligar. Alguns jogos dependem de valores iniciais da RAM para a geração de números aleatórios."
#define OPTION_VAL_FILL_FF_PT_BR NULL
#define OPTION_VAL_FILL_00_PT_BR NULL
#define OPTION_VAL_RANDOM_PT_BR "Aleatório"

struct retro_core_option_v2_category option_cats_pt_br[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_PT_BR,
      CATEGORY_SYSTEM_INFO_0_PT_BR
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_PT_BR,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_PT_BR
#else
      CATEGORY_VIDEO_INFO_1_PT_BR
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_PT_BR,
      CATEGORY_AUDIO_INFO_0_PT_BR
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_PT_BR,
      CATEGORY_INPUT_INFO_0_PT_BR
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_PT_BR,
      CATEGORY_HACKS_INFO_0_PT_BR
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_PT_BR,
      CATEGORY_DIP_SWITCH_INFO_0_PT_BR
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_pt_br[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_PT_BR,
      NULL,
      FCEUMM_REGION_INFO_0_PT_BR,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_PT_BR },
         { "NTSC",  OPTION_VAL_NTSC_PT_BR },
         { "PAL",   OPTION_VAL_PAL_PT_BR },
         { "Dendy", OPTION_VAL_DENDY_PT_BR },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_PT_BR,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_PT_BR,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_PT_BR,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_PT_BR,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_PT_BR,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_PT_BR,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_PT_BR,
      NULL,
      FCEUMM_ASPECT_INFO_0_PT_BR,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_PT_BR },
         { "4:3",     OPTION_VAL_4_3_PT_BR },
         { "PP",      OPTION_VAL_PP_PT_BR },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_PT_BR,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_PT_BR,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_PT_BR,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_PT_BR,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_PT_BR,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_PT_BR,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_PT_BR,
      NULL,
      FCEUMM_PALETTE_INFO_0_PT_BR,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_PT_BR },
         { "asqrealc",             OPTION_VAL_ASQREALC_PT_BR },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_PT_BR },
         { "rgb",                  OPTION_VAL_RGB_PT_BR },
         { "yuv-v3",               OPTION_VAL_YUV_V3_PT_BR },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_PT_BR },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_PT_BR },
         { "pal",                  OPTION_VAL_PAL_PT_BR },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_PT_BR },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_PT_BR },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_PT_BR },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_PT_BR },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_PT_BR },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_PT_BR },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_PT_BR },
         { "nescap",               OPTION_VAL_NESCAP_PT_BR },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_PT_BR },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_PT_BR },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_PT_BR },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_PT_BR },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_PT_BR },
         { "raw",                  OPTION_VAL_RAW_PT_BR },
         { "custom",               OPTION_VAL_CUSTOM_PT_BR },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_PT_BR,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_PT_BR,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_PT_BR },
         { "svideo",     OPTION_VAL_SVIDEO_PT_BR },
         { "rgb",        OPTION_VAL_RGB_O9_PT_BR },
         { "monochrome", OPTION_VAL_MONOCHROME_PT_BR },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_PT_BR,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_PT_BR,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_PT_BR },
         { "High",      OPTION_VAL_HIGH_PT_BR },
         { "Very High", OPTION_VAL_VERY_HIGH_PT_BR },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_PT_BR,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_PT_BR,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_PT_BR,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_PT_BR,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_PT_BR },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_PT_BR },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_PT_BR },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_PT_BR },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_PT_BR },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_PT_BR },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_PT_BR },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_PT_BR },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_PT_BR },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_PT_BR },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_PT_BR },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_PT_BR },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_PT_BR },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_PT_BR },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_PT_BR },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_PT_BR },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_PT_BR },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_PT_BR },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_PT_BR },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_PT_BR },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_PT_BR },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_PT_BR },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_PT_BR },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_PT_BR },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_PT_BR },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_PT_BR },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_PT_BR },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_PT_BR },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_PT_BR },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_PT_BR },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_PT_BR },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_PT_BR },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_PT_BR,
      FCEUMM_SWAPDUTY_LABEL_CAT_PT_BR,
      FCEUMM_SWAPDUTY_INFO_0_PT_BR,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_PT_BR,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_PT_BR,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_PT_BR },
         { "1",  OPTION_VAL_1_PT_BR },
         { "2",  OPTION_VAL_2_PT_BR },
         { "3",  OPTION_VAL_3_PT_BR },
         { "4",  OPTION_VAL_4_PT_BR },
         { "5",  OPTION_VAL_5_PT_BR },
         { "6",  OPTION_VAL_6_PT_BR },
         { "7",  OPTION_VAL_7_PT_BR },
         { "8",  OPTION_VAL_8_PT_BR },
         { "9",  OPTION_VAL_9_PT_BR },
         { "10", OPTION_VAL_10_PT_BR },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_PT_BR,
      FCEUMM_APU_1_LABEL_CAT_PT_BR,
      FCEUMM_APU_1_INFO_0_PT_BR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_PT_BR,
      FCEUMM_APU_2_LABEL_CAT_PT_BR,
      FCEUMM_APU_2_INFO_0_PT_BR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_PT_BR,
      FCEUMM_APU_3_LABEL_CAT_PT_BR,
      FCEUMM_APU_3_INFO_0_PT_BR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_PT_BR,
      FCEUMM_APU_4_LABEL_CAT_PT_BR,
      FCEUMM_APU_4_INFO_0_PT_BR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_PT_BR,
      FCEUMM_APU_5_LABEL_CAT_PT_BR,
      FCEUMM_APU_5_INFO_0_PT_BR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_PT_BR,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_PT_BR,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_PT_BR },
         { "Player 1", OPTION_VAL_PLAYER_1_PT_BR },
         { "Player 2", OPTION_VAL_PLAYER_2_PT_BR },
         { "Both",     OPTION_VAL_BOTH_PT_BR },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_PT_BR,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_PT_BR,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_PT_BR,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_PT_BR,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_PT_BR },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_PT_BR },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_PT_BR },
         { "mouse",       OPTION_VAL_MOUSE_PT_BR },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_PT_BR,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_PT_BR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_PT_BR,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_PT_BR,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_PT_BR,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_PT_BR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_PT_BR,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_PT_BR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_PT_BR,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_PT_BR,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_PT_BR,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_PT_BR,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_PT_BR,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_PT_BR,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_PT_BR },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_PT_BR },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_PT_BR,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_PT_BR,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_PT_BR },
         { "fill $00", OPTION_VAL_FILL_00_PT_BR },
         { "random",   OPTION_VAL_RANDOM_PT_BR },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_pt_br = {
   option_cats_pt_br,
   option_defs_pt_br
};

/* RETRO_LANGUAGE_PT_PT */

#define CATEGORY_SYSTEM_LABEL_PT_PT NULL
#define CATEGORY_SYSTEM_INFO_0_PT_PT NULL
#define CATEGORY_VIDEO_LABEL_PT_PT "Vídeo"
#define CATEGORY_VIDEO_INFO_0_PT_PT NULL
#define CATEGORY_VIDEO_INFO_1_PT_PT NULL
#define CATEGORY_AUDIO_LABEL_PT_PT "Áudio"
#define CATEGORY_AUDIO_INFO_0_PT_PT NULL
#define CATEGORY_INPUT_LABEL_PT_PT "Entrada"
#define CATEGORY_INPUT_INFO_0_PT_PT NULL
#define CATEGORY_HACKS_LABEL_PT_PT NULL
#define CATEGORY_HACKS_INFO_0_PT_PT NULL
#define CATEGORY_DIP_SWITCH_LABEL_PT_PT NULL
#define CATEGORY_DIP_SWITCH_INFO_0_PT_PT NULL
#define FCEUMM_REGION_LABEL_PT_PT NULL
#define FCEUMM_REGION_INFO_0_PT_PT NULL
#define OPTION_VAL_AUTO_PT_PT "Automático"
#define OPTION_VAL_NTSC_PT_PT NULL
#define OPTION_VAL_PAL_PT_PT NULL
#define OPTION_VAL_DENDY_PT_PT NULL
#define FCEUMM_GAME_GENIE_LABEL_PT_PT NULL
#define FCEUMM_GAME_GENIE_INFO_0_PT_PT NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_PT_PT NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_PT_PT NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_PT_PT NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_PT_PT NULL
#define FCEUMM_ASPECT_LABEL_PT_PT "Proporção do ecrã"
#define FCEUMM_ASPECT_INFO_0_PT_PT NULL
#define OPTION_VAL_8_7_PAR_PT_PT NULL
#define OPTION_VAL_4_3_PT_PT NULL
#define OPTION_VAL_PP_PT_PT NULL
#define FCEUMM_OVERSCAN_LABEL_PT_PT NULL
#define FCEUMM_OVERSCAN_INFO_0_PT_PT NULL
#define FCEUMM_OVERSCAN_H_LABEL_PT_PT NULL
#define FCEUMM_OVERSCAN_H_INFO_0_PT_PT NULL
#define FCEUMM_OVERSCAN_V_LABEL_PT_PT NULL
#define FCEUMM_OVERSCAN_V_INFO_0_PT_PT NULL
#define FCEUMM_PALETTE_LABEL_PT_PT NULL
#define FCEUMM_PALETTE_INFO_0_PT_PT NULL
#define OPTION_VAL_DEFAULT_PT_PT NULL
#define OPTION_VAL_ASQREALC_PT_PT NULL
#define OPTION_VAL_NINTENDO_VC_PT_PT NULL
#define OPTION_VAL_RGB_PT_PT NULL
#define OPTION_VAL_YUV_V3_PT_PT NULL
#define OPTION_VAL_UNSATURATED_FINAL_PT_PT NULL
#define OPTION_VAL_SONY_CXA2025AS_US_PT_PT NULL
#define OPTION_VAL_BMF_FINAL2_PT_PT NULL
#define OPTION_VAL_BMF_FINAL3_PT_PT NULL
#define OPTION_VAL_SMOOTH_FBX_PT_PT NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_PT_PT NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_PT_PT NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_PT_PT NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_PT_PT NULL
#define OPTION_VAL_NESCAP_PT_PT NULL
#define OPTION_VAL_WAVEBEAM_PT_PT NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_PT_PT NULL
#define OPTION_VAL_MAGNUM_FBX_PT_PT NULL
#define OPTION_VAL_SMOOTH_V2_FBX_PT_PT NULL
#define OPTION_VAL_NES_CLASSIC_FBX_PT_PT NULL
#define OPTION_VAL_RAW_PT_PT NULL
#define OPTION_VAL_CUSTOM_PT_PT "Personalizar"
#define FCEUMM_NTSC_FILTER_LABEL_PT_PT NULL
#define FCEUMM_NTSC_FILTER_INFO_0_PT_PT NULL
#define OPTION_VAL_COMPOSITE_PT_PT NULL
#define OPTION_VAL_SVIDEO_PT_PT NULL
#define OPTION_VAL_RGB_O9_PT_PT NULL
#define OPTION_VAL_MONOCHROME_PT_PT "Monocromático"
#define FCEUMM_SNDQUALITY_LABEL_PT_PT NULL
#define FCEUMM_SNDQUALITY_INFO_0_PT_PT NULL
#define OPTION_VAL_LOW_PT_PT NULL
#define OPTION_VAL_HIGH_PT_PT NULL
#define OPTION_VAL_VERY_HIGH_PT_PT NULL
#define FCEUMM_SNDLOWPASS_LABEL_PT_PT NULL
#define FCEUMM_SNDLOWPASS_INFO_0_PT_PT NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_PT_PT NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_PT_PT NULL
#define OPTION_VAL_01_MS_DELAY_PT_PT NULL
#define OPTION_VAL_02_MS_DELAY_PT_PT NULL
#define OPTION_VAL_03_MS_DELAY_PT_PT NULL
#define OPTION_VAL_04_MS_DELAY_PT_PT NULL
#define OPTION_VAL_05_MS_DELAY_PT_PT NULL
#define OPTION_VAL_06_MS_DELAY_PT_PT NULL
#define OPTION_VAL_07_MS_DELAY_PT_PT NULL
#define OPTION_VAL_08_MS_DELAY_PT_PT NULL
#define OPTION_VAL_09_MS_DELAY_PT_PT NULL
#define OPTION_VAL_10_MS_DELAY_PT_PT NULL
#define OPTION_VAL_11_MS_DELAY_PT_PT NULL
#define OPTION_VAL_12_MS_DELAY_PT_PT NULL
#define OPTION_VAL_13_MS_DELAY_PT_PT NULL
#define OPTION_VAL_14_MS_DELAY_PT_PT NULL
#define OPTION_VAL_15_MS_DELAY_PT_PT NULL
#define OPTION_VAL_16_MS_DELAY_PT_PT NULL
#define OPTION_VAL_17_MS_DELAY_PT_PT NULL
#define OPTION_VAL_18_MS_DELAY_PT_PT NULL
#define OPTION_VAL_19_MS_DELAY_PT_PT NULL
#define OPTION_VAL_20_MS_DELAY_PT_PT NULL
#define OPTION_VAL_21_MS_DELAY_PT_PT NULL
#define OPTION_VAL_22_MS_DELAY_PT_PT NULL
#define OPTION_VAL_23_MS_DELAY_PT_PT NULL
#define OPTION_VAL_24_MS_DELAY_PT_PT NULL
#define OPTION_VAL_25_MS_DELAY_PT_PT NULL
#define OPTION_VAL_26_MS_DELAY_PT_PT NULL
#define OPTION_VAL_27_MS_DELAY_PT_PT NULL
#define OPTION_VAL_28_MS_DELAY_PT_PT NULL
#define OPTION_VAL_29_MS_DELAY_PT_PT NULL
#define OPTION_VAL_30_MS_DELAY_PT_PT NULL
#define OPTION_VAL_31_MS_DELAY_PT_PT NULL
#define OPTION_VAL_32_MS_DELAY_PT_PT NULL
#define FCEUMM_SWAPDUTY_LABEL_PT_PT NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_PT_PT NULL
#define FCEUMM_SWAPDUTY_INFO_0_PT_PT NULL
#define FCEUMM_SNDVOLUME_LABEL_PT_PT NULL
#define FCEUMM_SNDVOLUME_INFO_0_PT_PT NULL
#define OPTION_VAL_0_PT_PT NULL
#define OPTION_VAL_1_PT_PT NULL
#define OPTION_VAL_2_PT_PT NULL
#define OPTION_VAL_3_PT_PT NULL
#define OPTION_VAL_4_PT_PT NULL
#define OPTION_VAL_5_PT_PT NULL
#define OPTION_VAL_6_PT_PT NULL
#define OPTION_VAL_7_PT_PT NULL
#define OPTION_VAL_8_PT_PT NULL
#define OPTION_VAL_9_PT_PT NULL
#define OPTION_VAL_10_PT_PT NULL
#define FCEUMM_APU_1_LABEL_PT_PT NULL
#define FCEUMM_APU_1_LABEL_CAT_PT_PT NULL
#define FCEUMM_APU_1_INFO_0_PT_PT NULL
#define FCEUMM_APU_2_LABEL_PT_PT NULL
#define FCEUMM_APU_2_LABEL_CAT_PT_PT NULL
#define FCEUMM_APU_2_INFO_0_PT_PT NULL
#define FCEUMM_APU_3_LABEL_PT_PT NULL
#define FCEUMM_APU_3_LABEL_CAT_PT_PT NULL
#define FCEUMM_APU_3_INFO_0_PT_PT NULL
#define FCEUMM_APU_4_LABEL_PT_PT NULL
#define FCEUMM_APU_4_LABEL_CAT_PT_PT NULL
#define FCEUMM_APU_4_INFO_0_PT_PT NULL
#define FCEUMM_APU_5_LABEL_PT_PT NULL
#define FCEUMM_APU_5_LABEL_CAT_PT_PT NULL
#define FCEUMM_APU_5_INFO_0_PT_PT NULL
#define FCEUMM_TURBO_ENABLE_LABEL_PT_PT NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_PT_PT NULL
#define OPTION_VAL_NONE_PT_PT "Nenhum"
#define OPTION_VAL_PLAYER_1_PT_PT NULL
#define OPTION_VAL_PLAYER_2_PT_PT NULL
#define OPTION_VAL_BOTH_PT_PT NULL
#define FCEUMM_TURBO_DELAY_LABEL_PT_PT NULL
#define FCEUMM_TURBO_DELAY_INFO_0_PT_PT NULL
#define FCEUMM_ZAPPER_MODE_LABEL_PT_PT NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_PT_PT NULL
#define OPTION_VAL_CLIGHTGUN_PT_PT NULL
#define OPTION_VAL_STLIGHTGUN_PT_PT NULL
#define OPTION_VAL_TOUCHSCREEN_PT_PT NULL
#define OPTION_VAL_MOUSE_PT_PT NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_PT_PT NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_PT_PT NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_PT_PT NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_PT_PT NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_PT_PT NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_PT_PT NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_PT_PT NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_PT_PT NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_PT_PT NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_PT_PT NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_PT_PT NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_PT_PT NULL
#define FCEUMM_OVERCLOCKING_LABEL_PT_PT NULL
#define FCEUMM_OVERCLOCKING_INFO_0_PT_PT NULL
#define OPTION_VAL_2X_POSTRENDER_PT_PT NULL
#define OPTION_VAL_2X_VBLANK_PT_PT NULL
#define FCEUMM_RAMSTATE_LABEL_PT_PT NULL
#define FCEUMM_RAMSTATE_INFO_0_PT_PT NULL
#define OPTION_VAL_FILL_FF_PT_PT NULL
#define OPTION_VAL_FILL_00_PT_PT NULL
#define OPTION_VAL_RANDOM_PT_PT NULL

struct retro_core_option_v2_category option_cats_pt_pt[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_PT_PT,
      CATEGORY_SYSTEM_INFO_0_PT_PT
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_PT_PT,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_PT_PT
#else
      CATEGORY_VIDEO_INFO_1_PT_PT
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_PT_PT,
      CATEGORY_AUDIO_INFO_0_PT_PT
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_PT_PT,
      CATEGORY_INPUT_INFO_0_PT_PT
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_PT_PT,
      CATEGORY_HACKS_INFO_0_PT_PT
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_PT_PT,
      CATEGORY_DIP_SWITCH_INFO_0_PT_PT
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_pt_pt[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_PT_PT,
      NULL,
      FCEUMM_REGION_INFO_0_PT_PT,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_PT_PT },
         { "NTSC",  OPTION_VAL_NTSC_PT_PT },
         { "PAL",   OPTION_VAL_PAL_PT_PT },
         { "Dendy", OPTION_VAL_DENDY_PT_PT },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_PT_PT,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_PT_PT,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_PT_PT,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_PT_PT,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_PT_PT,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_PT_PT,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_PT_PT,
      NULL,
      FCEUMM_ASPECT_INFO_0_PT_PT,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_PT_PT },
         { "4:3",     OPTION_VAL_4_3_PT_PT },
         { "PP",      OPTION_VAL_PP_PT_PT },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_PT_PT,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_PT_PT,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_PT_PT,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_PT_PT,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_PT_PT,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_PT_PT,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_PT_PT,
      NULL,
      FCEUMM_PALETTE_INFO_0_PT_PT,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_PT_PT },
         { "asqrealc",             OPTION_VAL_ASQREALC_PT_PT },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_PT_PT },
         { "rgb",                  OPTION_VAL_RGB_PT_PT },
         { "yuv-v3",               OPTION_VAL_YUV_V3_PT_PT },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_PT_PT },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_PT_PT },
         { "pal",                  OPTION_VAL_PAL_PT_PT },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_PT_PT },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_PT_PT },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_PT_PT },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_PT_PT },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_PT_PT },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_PT_PT },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_PT_PT },
         { "nescap",               OPTION_VAL_NESCAP_PT_PT },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_PT_PT },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_PT_PT },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_PT_PT },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_PT_PT },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_PT_PT },
         { "raw",                  OPTION_VAL_RAW_PT_PT },
         { "custom",               OPTION_VAL_CUSTOM_PT_PT },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_PT_PT,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_PT_PT,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_PT_PT },
         { "svideo",     OPTION_VAL_SVIDEO_PT_PT },
         { "rgb",        OPTION_VAL_RGB_O9_PT_PT },
         { "monochrome", OPTION_VAL_MONOCHROME_PT_PT },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_PT_PT,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_PT_PT,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_PT_PT },
         { "High",      OPTION_VAL_HIGH_PT_PT },
         { "Very High", OPTION_VAL_VERY_HIGH_PT_PT },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_PT_PT,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_PT_PT,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_PT_PT,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_PT_PT,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_PT_PT },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_PT_PT },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_PT_PT },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_PT_PT },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_PT_PT },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_PT_PT },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_PT_PT },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_PT_PT },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_PT_PT },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_PT_PT },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_PT_PT },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_PT_PT },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_PT_PT },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_PT_PT },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_PT_PT },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_PT_PT },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_PT_PT },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_PT_PT },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_PT_PT },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_PT_PT },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_PT_PT },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_PT_PT },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_PT_PT },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_PT_PT },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_PT_PT },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_PT_PT },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_PT_PT },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_PT_PT },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_PT_PT },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_PT_PT },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_PT_PT },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_PT_PT },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_PT_PT,
      FCEUMM_SWAPDUTY_LABEL_CAT_PT_PT,
      FCEUMM_SWAPDUTY_INFO_0_PT_PT,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_PT_PT,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_PT_PT,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_PT_PT },
         { "1",  OPTION_VAL_1_PT_PT },
         { "2",  OPTION_VAL_2_PT_PT },
         { "3",  OPTION_VAL_3_PT_PT },
         { "4",  OPTION_VAL_4_PT_PT },
         { "5",  OPTION_VAL_5_PT_PT },
         { "6",  OPTION_VAL_6_PT_PT },
         { "7",  OPTION_VAL_7_PT_PT },
         { "8",  OPTION_VAL_8_PT_PT },
         { "9",  OPTION_VAL_9_PT_PT },
         { "10", OPTION_VAL_10_PT_PT },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_PT_PT,
      FCEUMM_APU_1_LABEL_CAT_PT_PT,
      FCEUMM_APU_1_INFO_0_PT_PT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_PT_PT,
      FCEUMM_APU_2_LABEL_CAT_PT_PT,
      FCEUMM_APU_2_INFO_0_PT_PT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_PT_PT,
      FCEUMM_APU_3_LABEL_CAT_PT_PT,
      FCEUMM_APU_3_INFO_0_PT_PT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_PT_PT,
      FCEUMM_APU_4_LABEL_CAT_PT_PT,
      FCEUMM_APU_4_INFO_0_PT_PT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_PT_PT,
      FCEUMM_APU_5_LABEL_CAT_PT_PT,
      FCEUMM_APU_5_INFO_0_PT_PT,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_PT_PT,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_PT_PT,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_PT_PT },
         { "Player 1", OPTION_VAL_PLAYER_1_PT_PT },
         { "Player 2", OPTION_VAL_PLAYER_2_PT_PT },
         { "Both",     OPTION_VAL_BOTH_PT_PT },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_PT_PT,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_PT_PT,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_PT_PT,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_PT_PT,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_PT_PT },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_PT_PT },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_PT_PT },
         { "mouse",       OPTION_VAL_MOUSE_PT_PT },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_PT_PT,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_PT_PT,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_PT_PT,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_PT_PT,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_PT_PT,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_PT_PT,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_PT_PT,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_PT_PT,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_PT_PT,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_PT_PT,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_PT_PT,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_PT_PT,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_PT_PT,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_PT_PT,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_PT_PT },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_PT_PT },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_PT_PT,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_PT_PT,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_PT_PT },
         { "fill $00", OPTION_VAL_FILL_00_PT_PT },
         { "random",   OPTION_VAL_RANDOM_PT_PT },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_pt_pt = {
   option_cats_pt_pt,
   option_defs_pt_pt
};

/* RETRO_LANGUAGE_RO */

#define CATEGORY_SYSTEM_LABEL_RO NULL
#define CATEGORY_SYSTEM_INFO_0_RO NULL
#define CATEGORY_VIDEO_LABEL_RO NULL
#define CATEGORY_VIDEO_INFO_0_RO NULL
#define CATEGORY_VIDEO_INFO_1_RO NULL
#define CATEGORY_AUDIO_LABEL_RO NULL
#define CATEGORY_AUDIO_INFO_0_RO NULL
#define CATEGORY_INPUT_LABEL_RO NULL
#define CATEGORY_INPUT_INFO_0_RO NULL
#define CATEGORY_HACKS_LABEL_RO NULL
#define CATEGORY_HACKS_INFO_0_RO NULL
#define CATEGORY_DIP_SWITCH_LABEL_RO NULL
#define CATEGORY_DIP_SWITCH_INFO_0_RO NULL
#define FCEUMM_REGION_LABEL_RO NULL
#define FCEUMM_REGION_INFO_0_RO NULL
#define OPTION_VAL_AUTO_RO NULL
#define OPTION_VAL_NTSC_RO NULL
#define OPTION_VAL_PAL_RO NULL
#define OPTION_VAL_DENDY_RO NULL
#define FCEUMM_GAME_GENIE_LABEL_RO NULL
#define FCEUMM_GAME_GENIE_INFO_0_RO NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_RO NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_RO NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_RO NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_RO NULL
#define FCEUMM_ASPECT_LABEL_RO NULL
#define FCEUMM_ASPECT_INFO_0_RO NULL
#define OPTION_VAL_8_7_PAR_RO NULL
#define OPTION_VAL_4_3_RO NULL
#define OPTION_VAL_PP_RO NULL
#define FCEUMM_OVERSCAN_LABEL_RO NULL
#define FCEUMM_OVERSCAN_INFO_0_RO NULL
#define FCEUMM_OVERSCAN_H_LABEL_RO NULL
#define FCEUMM_OVERSCAN_H_INFO_0_RO NULL
#define FCEUMM_OVERSCAN_V_LABEL_RO NULL
#define FCEUMM_OVERSCAN_V_INFO_0_RO NULL
#define FCEUMM_PALETTE_LABEL_RO NULL
#define FCEUMM_PALETTE_INFO_0_RO NULL
#define OPTION_VAL_DEFAULT_RO NULL
#define OPTION_VAL_ASQREALC_RO NULL
#define OPTION_VAL_NINTENDO_VC_RO NULL
#define OPTION_VAL_RGB_RO NULL
#define OPTION_VAL_YUV_V3_RO NULL
#define OPTION_VAL_UNSATURATED_FINAL_RO NULL
#define OPTION_VAL_SONY_CXA2025AS_US_RO NULL
#define OPTION_VAL_BMF_FINAL2_RO NULL
#define OPTION_VAL_BMF_FINAL3_RO NULL
#define OPTION_VAL_SMOOTH_FBX_RO NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_RO NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_RO NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_RO NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_RO NULL
#define OPTION_VAL_NESCAP_RO NULL
#define OPTION_VAL_WAVEBEAM_RO NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_RO NULL
#define OPTION_VAL_MAGNUM_FBX_RO NULL
#define OPTION_VAL_SMOOTH_V2_FBX_RO NULL
#define OPTION_VAL_NES_CLASSIC_FBX_RO NULL
#define OPTION_VAL_RAW_RO NULL
#define OPTION_VAL_CUSTOM_RO NULL
#define FCEUMM_NTSC_FILTER_LABEL_RO NULL
#define FCEUMM_NTSC_FILTER_INFO_0_RO NULL
#define OPTION_VAL_COMPOSITE_RO NULL
#define OPTION_VAL_SVIDEO_RO NULL
#define OPTION_VAL_RGB_O9_RO NULL
#define OPTION_VAL_MONOCHROME_RO NULL
#define FCEUMM_SNDQUALITY_LABEL_RO NULL
#define FCEUMM_SNDQUALITY_INFO_0_RO NULL
#define OPTION_VAL_LOW_RO NULL
#define OPTION_VAL_HIGH_RO NULL
#define OPTION_VAL_VERY_HIGH_RO NULL
#define FCEUMM_SNDLOWPASS_LABEL_RO NULL
#define FCEUMM_SNDLOWPASS_INFO_0_RO NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_RO NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_RO NULL
#define OPTION_VAL_01_MS_DELAY_RO NULL
#define OPTION_VAL_02_MS_DELAY_RO NULL
#define OPTION_VAL_03_MS_DELAY_RO NULL
#define OPTION_VAL_04_MS_DELAY_RO NULL
#define OPTION_VAL_05_MS_DELAY_RO NULL
#define OPTION_VAL_06_MS_DELAY_RO NULL
#define OPTION_VAL_07_MS_DELAY_RO NULL
#define OPTION_VAL_08_MS_DELAY_RO NULL
#define OPTION_VAL_09_MS_DELAY_RO NULL
#define OPTION_VAL_10_MS_DELAY_RO NULL
#define OPTION_VAL_11_MS_DELAY_RO NULL
#define OPTION_VAL_12_MS_DELAY_RO NULL
#define OPTION_VAL_13_MS_DELAY_RO NULL
#define OPTION_VAL_14_MS_DELAY_RO NULL
#define OPTION_VAL_15_MS_DELAY_RO NULL
#define OPTION_VAL_16_MS_DELAY_RO NULL
#define OPTION_VAL_17_MS_DELAY_RO NULL
#define OPTION_VAL_18_MS_DELAY_RO NULL
#define OPTION_VAL_19_MS_DELAY_RO NULL
#define OPTION_VAL_20_MS_DELAY_RO NULL
#define OPTION_VAL_21_MS_DELAY_RO NULL
#define OPTION_VAL_22_MS_DELAY_RO NULL
#define OPTION_VAL_23_MS_DELAY_RO NULL
#define OPTION_VAL_24_MS_DELAY_RO NULL
#define OPTION_VAL_25_MS_DELAY_RO NULL
#define OPTION_VAL_26_MS_DELAY_RO NULL
#define OPTION_VAL_27_MS_DELAY_RO NULL
#define OPTION_VAL_28_MS_DELAY_RO NULL
#define OPTION_VAL_29_MS_DELAY_RO NULL
#define OPTION_VAL_30_MS_DELAY_RO NULL
#define OPTION_VAL_31_MS_DELAY_RO NULL
#define OPTION_VAL_32_MS_DELAY_RO NULL
#define FCEUMM_SWAPDUTY_LABEL_RO NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_RO NULL
#define FCEUMM_SWAPDUTY_INFO_0_RO NULL
#define FCEUMM_SNDVOLUME_LABEL_RO NULL
#define FCEUMM_SNDVOLUME_INFO_0_RO NULL
#define OPTION_VAL_0_RO NULL
#define OPTION_VAL_1_RO NULL
#define OPTION_VAL_2_RO NULL
#define OPTION_VAL_3_RO NULL
#define OPTION_VAL_4_RO NULL
#define OPTION_VAL_5_RO NULL
#define OPTION_VAL_6_RO NULL
#define OPTION_VAL_7_RO NULL
#define OPTION_VAL_8_RO NULL
#define OPTION_VAL_9_RO NULL
#define OPTION_VAL_10_RO NULL
#define FCEUMM_APU_1_LABEL_RO NULL
#define FCEUMM_APU_1_LABEL_CAT_RO NULL
#define FCEUMM_APU_1_INFO_0_RO NULL
#define FCEUMM_APU_2_LABEL_RO NULL
#define FCEUMM_APU_2_LABEL_CAT_RO NULL
#define FCEUMM_APU_2_INFO_0_RO NULL
#define FCEUMM_APU_3_LABEL_RO NULL
#define FCEUMM_APU_3_LABEL_CAT_RO NULL
#define FCEUMM_APU_3_INFO_0_RO NULL
#define FCEUMM_APU_4_LABEL_RO NULL
#define FCEUMM_APU_4_LABEL_CAT_RO NULL
#define FCEUMM_APU_4_INFO_0_RO NULL
#define FCEUMM_APU_5_LABEL_RO NULL
#define FCEUMM_APU_5_LABEL_CAT_RO NULL
#define FCEUMM_APU_5_INFO_0_RO NULL
#define FCEUMM_TURBO_ENABLE_LABEL_RO NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_RO NULL
#define OPTION_VAL_NONE_RO NULL
#define OPTION_VAL_PLAYER_1_RO NULL
#define OPTION_VAL_PLAYER_2_RO NULL
#define OPTION_VAL_BOTH_RO NULL
#define FCEUMM_TURBO_DELAY_LABEL_RO NULL
#define FCEUMM_TURBO_DELAY_INFO_0_RO NULL
#define FCEUMM_ZAPPER_MODE_LABEL_RO NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_RO NULL
#define OPTION_VAL_CLIGHTGUN_RO NULL
#define OPTION_VAL_STLIGHTGUN_RO NULL
#define OPTION_VAL_TOUCHSCREEN_RO NULL
#define OPTION_VAL_MOUSE_RO NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_RO NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_RO NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_RO NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_RO NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_RO NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_RO NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_RO NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_RO NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_RO NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_RO NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_RO NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_RO NULL
#define FCEUMM_OVERCLOCKING_LABEL_RO NULL
#define FCEUMM_OVERCLOCKING_INFO_0_RO NULL
#define OPTION_VAL_2X_POSTRENDER_RO NULL
#define OPTION_VAL_2X_VBLANK_RO NULL
#define FCEUMM_RAMSTATE_LABEL_RO NULL
#define FCEUMM_RAMSTATE_INFO_0_RO NULL
#define OPTION_VAL_FILL_FF_RO NULL
#define OPTION_VAL_FILL_00_RO NULL
#define OPTION_VAL_RANDOM_RO NULL

struct retro_core_option_v2_category option_cats_ro[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_RO,
      CATEGORY_SYSTEM_INFO_0_RO
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_RO,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_RO
#else
      CATEGORY_VIDEO_INFO_1_RO
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_RO,
      CATEGORY_AUDIO_INFO_0_RO
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_RO,
      CATEGORY_INPUT_INFO_0_RO
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_RO,
      CATEGORY_HACKS_INFO_0_RO
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_RO,
      CATEGORY_DIP_SWITCH_INFO_0_RO
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_ro[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_RO,
      NULL,
      FCEUMM_REGION_INFO_0_RO,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_RO },
         { "NTSC",  OPTION_VAL_NTSC_RO },
         { "PAL",   OPTION_VAL_PAL_RO },
         { "Dendy", OPTION_VAL_DENDY_RO },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_RO,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_RO,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_RO,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_RO,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_RO,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_RO,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_RO,
      NULL,
      FCEUMM_ASPECT_INFO_0_RO,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_RO },
         { "4:3",     OPTION_VAL_4_3_RO },
         { "PP",      OPTION_VAL_PP_RO },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_RO,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_RO,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_RO,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_RO,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_RO,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_RO,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_RO,
      NULL,
      FCEUMM_PALETTE_INFO_0_RO,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_RO },
         { "asqrealc",             OPTION_VAL_ASQREALC_RO },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_RO },
         { "rgb",                  OPTION_VAL_RGB_RO },
         { "yuv-v3",               OPTION_VAL_YUV_V3_RO },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_RO },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_RO },
         { "pal",                  OPTION_VAL_PAL_RO },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_RO },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_RO },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_RO },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_RO },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_RO },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_RO },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_RO },
         { "nescap",               OPTION_VAL_NESCAP_RO },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_RO },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_RO },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_RO },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_RO },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_RO },
         { "raw",                  OPTION_VAL_RAW_RO },
         { "custom",               OPTION_VAL_CUSTOM_RO },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_RO,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_RO,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_RO },
         { "svideo",     OPTION_VAL_SVIDEO_RO },
         { "rgb",        OPTION_VAL_RGB_O9_RO },
         { "monochrome", OPTION_VAL_MONOCHROME_RO },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_RO,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_RO,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_RO },
         { "High",      OPTION_VAL_HIGH_RO },
         { "Very High", OPTION_VAL_VERY_HIGH_RO },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_RO,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_RO,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_RO,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_RO,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_RO },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_RO },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_RO },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_RO },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_RO },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_RO },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_RO },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_RO },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_RO },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_RO },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_RO },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_RO },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_RO },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_RO },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_RO },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_RO },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_RO },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_RO },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_RO },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_RO },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_RO },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_RO },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_RO },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_RO },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_RO },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_RO },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_RO },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_RO },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_RO },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_RO },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_RO },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_RO },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_RO,
      FCEUMM_SWAPDUTY_LABEL_CAT_RO,
      FCEUMM_SWAPDUTY_INFO_0_RO,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_RO,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_RO,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_RO },
         { "1",  OPTION_VAL_1_RO },
         { "2",  OPTION_VAL_2_RO },
         { "3",  OPTION_VAL_3_RO },
         { "4",  OPTION_VAL_4_RO },
         { "5",  OPTION_VAL_5_RO },
         { "6",  OPTION_VAL_6_RO },
         { "7",  OPTION_VAL_7_RO },
         { "8",  OPTION_VAL_8_RO },
         { "9",  OPTION_VAL_9_RO },
         { "10", OPTION_VAL_10_RO },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_RO,
      FCEUMM_APU_1_LABEL_CAT_RO,
      FCEUMM_APU_1_INFO_0_RO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_RO,
      FCEUMM_APU_2_LABEL_CAT_RO,
      FCEUMM_APU_2_INFO_0_RO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_RO,
      FCEUMM_APU_3_LABEL_CAT_RO,
      FCEUMM_APU_3_INFO_0_RO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_RO,
      FCEUMM_APU_4_LABEL_CAT_RO,
      FCEUMM_APU_4_INFO_0_RO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_RO,
      FCEUMM_APU_5_LABEL_CAT_RO,
      FCEUMM_APU_5_INFO_0_RO,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_RO,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_RO,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_RO },
         { "Player 1", OPTION_VAL_PLAYER_1_RO },
         { "Player 2", OPTION_VAL_PLAYER_2_RO },
         { "Both",     OPTION_VAL_BOTH_RO },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_RO,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_RO,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_RO,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_RO,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_RO },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_RO },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_RO },
         { "mouse",       OPTION_VAL_MOUSE_RO },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_RO,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_RO,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_RO,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_RO,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_RO,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_RO,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_RO,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_RO,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_RO,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_RO,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_RO,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_RO,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_RO,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_RO,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_RO },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_RO },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_RO,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_RO,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_RO },
         { "fill $00", OPTION_VAL_FILL_00_RO },
         { "random",   OPTION_VAL_RANDOM_RO },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_ro = {
   option_cats_ro,
   option_defs_ro
};

/* RETRO_LANGUAGE_RU */

#define CATEGORY_SYSTEM_LABEL_RU "Системные"
#define CATEGORY_SYSTEM_INFO_0_RU NULL
#define CATEGORY_VIDEO_LABEL_RU "Видео"
#define CATEGORY_VIDEO_INFO_0_RU NULL
#define CATEGORY_VIDEO_INFO_1_RU NULL
#define CATEGORY_AUDIO_LABEL_RU "Аудио"
#define CATEGORY_AUDIO_INFO_0_RU NULL
#define CATEGORY_INPUT_LABEL_RU "Ввод"
#define CATEGORY_INPUT_INFO_0_RU NULL
#define CATEGORY_HACKS_LABEL_RU "Хаки эмуляции"
#define CATEGORY_HACKS_INFO_0_RU "Настройка разгона процессора и параметров точности эмуляции, влияющих на производительность и совместимость."
#define CATEGORY_DIP_SWITCH_LABEL_RU "DIP-переключатели"
#define CATEGORY_DIP_SWITCH_INFO_0_RU NULL
#define FCEUMM_REGION_LABEL_RU "Регион"
#define FCEUMM_REGION_INFO_0_RU "Принудительно использовать для ядра тайминги NTSC, PAL или Dendy."
#define OPTION_VAL_AUTO_RU "Авто"
#define OPTION_VAL_NTSC_RU NULL
#define OPTION_VAL_PAL_RU NULL
#define OPTION_VAL_DENDY_RU NULL
#define FCEUMM_GAME_GENIE_LABEL_RU "Расширение Game Genie (требуется перезапуск)"
#define FCEUMM_GAME_GENIE_INFO_0_RU "Включает эмуляцию картриджа расширения Game Genie, позволяющего вводить чит-коды при запуске игр. Файл образа Game Genie 'gamegenie.nes' должен присутствовать в системном каталоге фронтенда. Не поддерживается для игр с аркад или FDS."
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_RU "Вкл. расширенные настройки системы"
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_RU "Показывать расширенные системные настройки."
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_RU "Вкл. расширенные настройки звука"
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_RU "Показывать расширенные настройки звука."
#define FCEUMM_ASPECT_LABEL_RU "Соотношение сторон"
#define FCEUMM_ASPECT_INFO_0_RU "Выбор предпочтительного соотношения сторон."
#define OPTION_VAL_8_7_PAR_RU NULL
#define OPTION_VAL_4_3_RU NULL
#define OPTION_VAL_PP_RU NULL
#define FCEUMM_OVERSCAN_LABEL_RU "Кадрировать вылеты развёртки"
#define FCEUMM_OVERSCAN_INFO_0_RU "Убирает по 8 пикселей вылета развёртки с каждой из сторон экрана."
#define FCEUMM_OVERSCAN_H_LABEL_RU "Кадрировать вылеты развёртки по горизонтали"
#define FCEUMM_OVERSCAN_H_INFO_0_RU "Убирает по 8 пикселей с левой и правой сторон экрана для имитации вылетов развёртки стандартных ЭЛТ-телевизоров."
#define FCEUMM_OVERSCAN_V_LABEL_RU "Кадрировать вылеты развёртки по вертикали"
#define FCEUMM_OVERSCAN_V_INFO_0_RU "Убирает по 8 пикселей с верхней и нижней границ экрана для имитации вылетов развёртки стандартных ЭЛТ-телевизоров."
#define FCEUMM_PALETTE_LABEL_RU "Цветовая палитра"
#define FCEUMM_PALETTE_INFO_0_RU "Выбор предустановленной палитры, кастомной палитры 64x3 из файла или палитры в формате raw (для использования совместно с шейдером nes-decoder)."
#define OPTION_VAL_DEFAULT_RU "Стандартная"
#define OPTION_VAL_ASQREALC_RU NULL
#define OPTION_VAL_NINTENDO_VC_RU NULL
#define OPTION_VAL_RGB_RU NULL
#define OPTION_VAL_YUV_V3_RU NULL
#define OPTION_VAL_UNSATURATED_FINAL_RU NULL
#define OPTION_VAL_SONY_CXA2025AS_US_RU NULL
#define OPTION_VAL_BMF_FINAL2_RU NULL
#define OPTION_VAL_BMF_FINAL3_RU NULL
#define OPTION_VAL_SMOOTH_FBX_RU NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_RU NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_RU NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_RU NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_RU NULL
#define OPTION_VAL_NESCAP_RU NULL
#define OPTION_VAL_WAVEBEAM_RU NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_RU NULL
#define OPTION_VAL_MAGNUM_FBX_RU NULL
#define OPTION_VAL_SMOOTH_V2_FBX_RU NULL
#define OPTION_VAL_NES_CLASSIC_FBX_RU NULL
#define OPTION_VAL_RAW_RU NULL
#define OPTION_VAL_CUSTOM_RU "Кастомная"
#define FCEUMM_NTSC_FILTER_LABEL_RU "NTSC-фильтр"
#define FCEUMM_NTSC_FILTER_INFO_0_RU NULL
#define OPTION_VAL_COMPOSITE_RU "Композитный"
#define OPTION_VAL_SVIDEO_RU NULL
#define OPTION_VAL_RGB_O9_RU NULL
#define OPTION_VAL_MONOCHROME_RU "Монохромный"
#define FCEUMM_SNDQUALITY_LABEL_RU "Качество звука"
#define FCEUMM_SNDQUALITY_INFO_0_RU "Включает более высокое качество звука. Повышает системные требования."
#define OPTION_VAL_LOW_RU "Низкое"
#define OPTION_VAL_HIGH_RU "Высокое"
#define OPTION_VAL_VERY_HIGH_RU "Очень высокое"
#define FCEUMM_SNDLOWPASS_LABEL_RU NULL
#define FCEUMM_SNDLOWPASS_INFO_0_RU NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_RU NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_RU NULL
#define OPTION_VAL_01_MS_DELAY_RU NULL
#define OPTION_VAL_02_MS_DELAY_RU NULL
#define OPTION_VAL_03_MS_DELAY_RU NULL
#define OPTION_VAL_04_MS_DELAY_RU NULL
#define OPTION_VAL_05_MS_DELAY_RU NULL
#define OPTION_VAL_06_MS_DELAY_RU NULL
#define OPTION_VAL_07_MS_DELAY_RU NULL
#define OPTION_VAL_08_MS_DELAY_RU NULL
#define OPTION_VAL_09_MS_DELAY_RU NULL
#define OPTION_VAL_10_MS_DELAY_RU NULL
#define OPTION_VAL_11_MS_DELAY_RU NULL
#define OPTION_VAL_12_MS_DELAY_RU NULL
#define OPTION_VAL_13_MS_DELAY_RU NULL
#define OPTION_VAL_14_MS_DELAY_RU NULL
#define OPTION_VAL_15_MS_DELAY_RU NULL
#define OPTION_VAL_16_MS_DELAY_RU NULL
#define OPTION_VAL_17_MS_DELAY_RU NULL
#define OPTION_VAL_18_MS_DELAY_RU NULL
#define OPTION_VAL_19_MS_DELAY_RU NULL
#define OPTION_VAL_20_MS_DELAY_RU NULL
#define OPTION_VAL_21_MS_DELAY_RU NULL
#define OPTION_VAL_22_MS_DELAY_RU NULL
#define OPTION_VAL_23_MS_DELAY_RU NULL
#define OPTION_VAL_24_MS_DELAY_RU NULL
#define OPTION_VAL_25_MS_DELAY_RU NULL
#define OPTION_VAL_26_MS_DELAY_RU NULL
#define OPTION_VAL_27_MS_DELAY_RU NULL
#define OPTION_VAL_28_MS_DELAY_RU NULL
#define OPTION_VAL_29_MS_DELAY_RU NULL
#define OPTION_VAL_30_MS_DELAY_RU NULL
#define OPTION_VAL_31_MS_DELAY_RU NULL
#define OPTION_VAL_32_MS_DELAY_RU NULL
#define FCEUMM_SWAPDUTY_LABEL_RU "Поменять рабочие циклы аудио"
#define FCEUMM_SWAPDUTY_LABEL_CAT_RU "Поменять рабочие циклы"
#define FCEUMM_SWAPDUTY_INFO_0_RU "Имитирует звучание фамиклонов с обратным порядком битов в рабочих циклах каналов прямоугольной волны."
#define FCEUMM_SNDVOLUME_LABEL_RU "Общая громкость"
#define FCEUMM_SNDVOLUME_INFO_0_RU "Изменение общего уровня громкости."
#define OPTION_VAL_0_RU NULL
#define OPTION_VAL_1_RU NULL
#define OPTION_VAL_2_RU NULL
#define OPTION_VAL_3_RU NULL
#define OPTION_VAL_4_RU NULL
#define OPTION_VAL_5_RU NULL
#define OPTION_VAL_6_RU NULL
#define OPTION_VAL_7_RU NULL
#define OPTION_VAL_8_RU NULL
#define OPTION_VAL_9_RU NULL
#define OPTION_VAL_10_RU NULL
#define FCEUMM_APU_1_LABEL_RU "Аудиоканал 1 (прямоугольный 1)"
#define FCEUMM_APU_1_LABEL_CAT_RU "Канал 1 (прямоугольный 1)"
#define FCEUMM_APU_1_INFO_0_RU "Включает или отключает вывод звука для генератора прямоугольной волны 1."
#define FCEUMM_APU_2_LABEL_RU "Аудиоканал 2 (прямоугольный 2)"
#define FCEUMM_APU_2_LABEL_CAT_RU "Канал 2 (прямоугольный 2)"
#define FCEUMM_APU_2_INFO_0_RU "Включает или отключает вывод звука для генератора прямоугольной волны 2."
#define FCEUMM_APU_3_LABEL_RU "Аудиоканал 3 (треугольный)"
#define FCEUMM_APU_3_LABEL_CAT_RU "Канал 3 (треугольный)"
#define FCEUMM_APU_3_INFO_0_RU "Включает или отключает вывод звука для генератора треугольной волны."
#define FCEUMM_APU_4_LABEL_RU "Аудиоканал 4 (шумовой)"
#define FCEUMM_APU_4_LABEL_CAT_RU "Канал 4 (шумовой)"
#define FCEUMM_APU_4_INFO_0_RU "Включает или отключает вывод звука для генератора шума."
#define FCEUMM_APU_5_LABEL_RU "Аудиоканал 5 (PCM)"
#define FCEUMM_APU_5_LABEL_CAT_RU "Канал 5 (PCM)"
#define FCEUMM_APU_5_INFO_0_RU "Включает или отключает вывод звука для канала дельта-модуляции."
#define FCEUMM_TURBO_ENABLE_LABEL_RU "Включить турбо"
#define FCEUMM_TURBO_ENABLE_INFO_0_RU "Включает или отключает турбо-кнопки."
#define OPTION_VAL_NONE_RU "Отключено"
#define OPTION_VAL_PLAYER_1_RU "Игрок 1"
#define OPTION_VAL_PLAYER_2_RU "Игрок 2"
#define OPTION_VAL_BOTH_RU "Оба"
#define FCEUMM_TURBO_DELAY_LABEL_RU "Задержка турбо (в кадрах)"
#define FCEUMM_TURBO_DELAY_INFO_0_RU "Частота повтора в кадрах для турбо-кнопок."
#define FCEUMM_ZAPPER_MODE_LABEL_RU "Режим Zapper"
#define FCEUMM_ZAPPER_MODE_INFO_0_RU NULL
#define OPTION_VAL_CLIGHTGUN_RU NULL
#define OPTION_VAL_STLIGHTGUN_RU NULL
#define OPTION_VAL_TOUCHSCREEN_RU "Сенсорный экран"
#define OPTION_VAL_MOUSE_RU "Мышь"
#define FCEUMM_SHOW_CROSSHAIR_LABEL_RU "Показывать прицел Zapper"
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_RU NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_RU "Допуск Zapper"
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_RU "Устанавливает, сколько пикселей из области прицела будет находится на цели."
#define FCEUMM_ZAPPER_TRIGGER_LABEL_RU NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_RU NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_RU NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_RU NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_RU "Разрешить противоположные направления"
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_RU "Разрешает комбинации кнопок ВВЕРХ+ВНИЗ или ВЛЕВО+ВПРАВО, одновременное нажатие которых может вызывать различные эффекты в играх."
#define FCEUMM_NOSPRITELIMIT_LABEL_RU "Без ограничения спрайтов"
#define FCEUMM_NOSPRITELIMIT_INFO_0_RU "Убирает аппаратное ограничение в 8 спрайтов на линию развёртки. Уменьшает мерцание спрайтов, но может приводить к графическим багам в играх, использующих ограничение для создания эффектов."
#define FCEUMM_OVERCLOCKING_LABEL_RU "Разгон"
#define FCEUMM_OVERCLOCKING_INFO_0_RU "Включает или отключает разгон для устранения замедления в некоторых играх. Метод Postrender лучше совместим с большинством игр, VBlank более эффективен для таких игр как Contra Force."
#define OPTION_VAL_2X_POSTRENDER_RU NULL
#define OPTION_VAL_2X_VBLANK_RU NULL
#define FCEUMM_RAMSTATE_LABEL_RU "Заполнять ОЗУ при включении (перезапуск)"
#define FCEUMM_RAMSTATE_INFO_0_RU "Заполнять оперативную память при включении питания. Некоторые игры используют начальные значения ОЗУ для генерации случайных чисел."
#define OPTION_VAL_FILL_FF_RU NULL
#define OPTION_VAL_FILL_00_RU NULL
#define OPTION_VAL_RANDOM_RU "Случайно"

struct retro_core_option_v2_category option_cats_ru[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_RU,
      CATEGORY_SYSTEM_INFO_0_RU
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_RU,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_RU
#else
      CATEGORY_VIDEO_INFO_1_RU
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_RU,
      CATEGORY_AUDIO_INFO_0_RU
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_RU,
      CATEGORY_INPUT_INFO_0_RU
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_RU,
      CATEGORY_HACKS_INFO_0_RU
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_RU,
      CATEGORY_DIP_SWITCH_INFO_0_RU
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_ru[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_RU,
      NULL,
      FCEUMM_REGION_INFO_0_RU,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_RU },
         { "NTSC",  OPTION_VAL_NTSC_RU },
         { "PAL",   OPTION_VAL_PAL_RU },
         { "Dendy", OPTION_VAL_DENDY_RU },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_RU,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_RU,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_RU,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_RU,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_RU,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_RU,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_RU,
      NULL,
      FCEUMM_ASPECT_INFO_0_RU,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_RU },
         { "4:3",     OPTION_VAL_4_3_RU },
         { "PP",      OPTION_VAL_PP_RU },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_RU,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_RU,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_RU,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_RU,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_RU,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_RU,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_RU,
      NULL,
      FCEUMM_PALETTE_INFO_0_RU,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_RU },
         { "asqrealc",             OPTION_VAL_ASQREALC_RU },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_RU },
         { "rgb",                  OPTION_VAL_RGB_RU },
         { "yuv-v3",               OPTION_VAL_YUV_V3_RU },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_RU },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_RU },
         { "pal",                  OPTION_VAL_PAL_RU },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_RU },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_RU },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_RU },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_RU },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_RU },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_RU },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_RU },
         { "nescap",               OPTION_VAL_NESCAP_RU },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_RU },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_RU },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_RU },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_RU },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_RU },
         { "raw",                  OPTION_VAL_RAW_RU },
         { "custom",               OPTION_VAL_CUSTOM_RU },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_RU,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_RU,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_RU },
         { "svideo",     OPTION_VAL_SVIDEO_RU },
         { "rgb",        OPTION_VAL_RGB_O9_RU },
         { "monochrome", OPTION_VAL_MONOCHROME_RU },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_RU,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_RU,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_RU },
         { "High",      OPTION_VAL_HIGH_RU },
         { "Very High", OPTION_VAL_VERY_HIGH_RU },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_RU,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_RU,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_RU,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_RU,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_RU },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_RU },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_RU },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_RU },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_RU },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_RU },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_RU },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_RU },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_RU },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_RU },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_RU },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_RU },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_RU },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_RU },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_RU },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_RU },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_RU },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_RU },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_RU },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_RU },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_RU },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_RU },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_RU },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_RU },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_RU },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_RU },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_RU },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_RU },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_RU },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_RU },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_RU },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_RU },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_RU,
      FCEUMM_SWAPDUTY_LABEL_CAT_RU,
      FCEUMM_SWAPDUTY_INFO_0_RU,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_RU,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_RU,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_RU },
         { "1",  OPTION_VAL_1_RU },
         { "2",  OPTION_VAL_2_RU },
         { "3",  OPTION_VAL_3_RU },
         { "4",  OPTION_VAL_4_RU },
         { "5",  OPTION_VAL_5_RU },
         { "6",  OPTION_VAL_6_RU },
         { "7",  OPTION_VAL_7_RU },
         { "8",  OPTION_VAL_8_RU },
         { "9",  OPTION_VAL_9_RU },
         { "10", OPTION_VAL_10_RU },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_RU,
      FCEUMM_APU_1_LABEL_CAT_RU,
      FCEUMM_APU_1_INFO_0_RU,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_RU,
      FCEUMM_APU_2_LABEL_CAT_RU,
      FCEUMM_APU_2_INFO_0_RU,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_RU,
      FCEUMM_APU_3_LABEL_CAT_RU,
      FCEUMM_APU_3_INFO_0_RU,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_RU,
      FCEUMM_APU_4_LABEL_CAT_RU,
      FCEUMM_APU_4_INFO_0_RU,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_RU,
      FCEUMM_APU_5_LABEL_CAT_RU,
      FCEUMM_APU_5_INFO_0_RU,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_RU,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_RU,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_RU },
         { "Player 1", OPTION_VAL_PLAYER_1_RU },
         { "Player 2", OPTION_VAL_PLAYER_2_RU },
         { "Both",     OPTION_VAL_BOTH_RU },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_RU,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_RU,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_RU,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_RU,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_RU },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_RU },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_RU },
         { "mouse",       OPTION_VAL_MOUSE_RU },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_RU,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_RU,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_RU,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_RU,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_RU,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_RU,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_RU,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_RU,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_RU,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_RU,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_RU,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_RU,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_RU,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_RU,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_RU },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_RU },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_RU,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_RU,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_RU },
         { "fill $00", OPTION_VAL_FILL_00_RU },
         { "random",   OPTION_VAL_RANDOM_RU },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_ru = {
   option_cats_ru,
   option_defs_ru
};

/* RETRO_LANGUAGE_SI */

#define CATEGORY_SYSTEM_LABEL_SI NULL
#define CATEGORY_SYSTEM_INFO_0_SI NULL
#define CATEGORY_VIDEO_LABEL_SI NULL
#define CATEGORY_VIDEO_INFO_0_SI NULL
#define CATEGORY_VIDEO_INFO_1_SI NULL
#define CATEGORY_AUDIO_LABEL_SI NULL
#define CATEGORY_AUDIO_INFO_0_SI NULL
#define CATEGORY_INPUT_LABEL_SI NULL
#define CATEGORY_INPUT_INFO_0_SI NULL
#define CATEGORY_HACKS_LABEL_SI NULL
#define CATEGORY_HACKS_INFO_0_SI NULL
#define CATEGORY_DIP_SWITCH_LABEL_SI NULL
#define CATEGORY_DIP_SWITCH_INFO_0_SI NULL
#define FCEUMM_REGION_LABEL_SI NULL
#define FCEUMM_REGION_INFO_0_SI NULL
#define OPTION_VAL_AUTO_SI NULL
#define OPTION_VAL_NTSC_SI NULL
#define OPTION_VAL_PAL_SI NULL
#define OPTION_VAL_DENDY_SI NULL
#define FCEUMM_GAME_GENIE_LABEL_SI NULL
#define FCEUMM_GAME_GENIE_INFO_0_SI NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_SI NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_SI NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_SI NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_SI NULL
#define FCEUMM_ASPECT_LABEL_SI NULL
#define FCEUMM_ASPECT_INFO_0_SI NULL
#define OPTION_VAL_8_7_PAR_SI NULL
#define OPTION_VAL_4_3_SI NULL
#define OPTION_VAL_PP_SI NULL
#define FCEUMM_OVERSCAN_LABEL_SI NULL
#define FCEUMM_OVERSCAN_INFO_0_SI NULL
#define FCEUMM_OVERSCAN_H_LABEL_SI NULL
#define FCEUMM_OVERSCAN_H_INFO_0_SI NULL
#define FCEUMM_OVERSCAN_V_LABEL_SI NULL
#define FCEUMM_OVERSCAN_V_INFO_0_SI NULL
#define FCEUMM_PALETTE_LABEL_SI NULL
#define FCEUMM_PALETTE_INFO_0_SI NULL
#define OPTION_VAL_DEFAULT_SI NULL
#define OPTION_VAL_ASQREALC_SI NULL
#define OPTION_VAL_NINTENDO_VC_SI NULL
#define OPTION_VAL_RGB_SI NULL
#define OPTION_VAL_YUV_V3_SI NULL
#define OPTION_VAL_UNSATURATED_FINAL_SI NULL
#define OPTION_VAL_SONY_CXA2025AS_US_SI NULL
#define OPTION_VAL_BMF_FINAL2_SI NULL
#define OPTION_VAL_BMF_FINAL3_SI NULL
#define OPTION_VAL_SMOOTH_FBX_SI NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_SI NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_SI NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_SI NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_SI NULL
#define OPTION_VAL_NESCAP_SI NULL
#define OPTION_VAL_WAVEBEAM_SI NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_SI NULL
#define OPTION_VAL_MAGNUM_FBX_SI NULL
#define OPTION_VAL_SMOOTH_V2_FBX_SI NULL
#define OPTION_VAL_NES_CLASSIC_FBX_SI NULL
#define OPTION_VAL_RAW_SI NULL
#define OPTION_VAL_CUSTOM_SI NULL
#define FCEUMM_NTSC_FILTER_LABEL_SI NULL
#define FCEUMM_NTSC_FILTER_INFO_0_SI NULL
#define OPTION_VAL_COMPOSITE_SI NULL
#define OPTION_VAL_SVIDEO_SI NULL
#define OPTION_VAL_RGB_O9_SI NULL
#define OPTION_VAL_MONOCHROME_SI NULL
#define FCEUMM_SNDQUALITY_LABEL_SI NULL
#define FCEUMM_SNDQUALITY_INFO_0_SI NULL
#define OPTION_VAL_LOW_SI NULL
#define OPTION_VAL_HIGH_SI NULL
#define OPTION_VAL_VERY_HIGH_SI NULL
#define FCEUMM_SNDLOWPASS_LABEL_SI NULL
#define FCEUMM_SNDLOWPASS_INFO_0_SI NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_SI NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_SI NULL
#define OPTION_VAL_01_MS_DELAY_SI NULL
#define OPTION_VAL_02_MS_DELAY_SI NULL
#define OPTION_VAL_03_MS_DELAY_SI NULL
#define OPTION_VAL_04_MS_DELAY_SI NULL
#define OPTION_VAL_05_MS_DELAY_SI NULL
#define OPTION_VAL_06_MS_DELAY_SI NULL
#define OPTION_VAL_07_MS_DELAY_SI NULL
#define OPTION_VAL_08_MS_DELAY_SI NULL
#define OPTION_VAL_09_MS_DELAY_SI NULL
#define OPTION_VAL_10_MS_DELAY_SI NULL
#define OPTION_VAL_11_MS_DELAY_SI NULL
#define OPTION_VAL_12_MS_DELAY_SI NULL
#define OPTION_VAL_13_MS_DELAY_SI NULL
#define OPTION_VAL_14_MS_DELAY_SI NULL
#define OPTION_VAL_15_MS_DELAY_SI NULL
#define OPTION_VAL_16_MS_DELAY_SI NULL
#define OPTION_VAL_17_MS_DELAY_SI NULL
#define OPTION_VAL_18_MS_DELAY_SI NULL
#define OPTION_VAL_19_MS_DELAY_SI NULL
#define OPTION_VAL_20_MS_DELAY_SI NULL
#define OPTION_VAL_21_MS_DELAY_SI NULL
#define OPTION_VAL_22_MS_DELAY_SI NULL
#define OPTION_VAL_23_MS_DELAY_SI NULL
#define OPTION_VAL_24_MS_DELAY_SI NULL
#define OPTION_VAL_25_MS_DELAY_SI NULL
#define OPTION_VAL_26_MS_DELAY_SI NULL
#define OPTION_VAL_27_MS_DELAY_SI NULL
#define OPTION_VAL_28_MS_DELAY_SI NULL
#define OPTION_VAL_29_MS_DELAY_SI NULL
#define OPTION_VAL_30_MS_DELAY_SI NULL
#define OPTION_VAL_31_MS_DELAY_SI NULL
#define OPTION_VAL_32_MS_DELAY_SI NULL
#define FCEUMM_SWAPDUTY_LABEL_SI NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_SI NULL
#define FCEUMM_SWAPDUTY_INFO_0_SI NULL
#define FCEUMM_SNDVOLUME_LABEL_SI NULL
#define FCEUMM_SNDVOLUME_INFO_0_SI NULL
#define OPTION_VAL_0_SI NULL
#define OPTION_VAL_1_SI NULL
#define OPTION_VAL_2_SI NULL
#define OPTION_VAL_3_SI NULL
#define OPTION_VAL_4_SI NULL
#define OPTION_VAL_5_SI NULL
#define OPTION_VAL_6_SI NULL
#define OPTION_VAL_7_SI NULL
#define OPTION_VAL_8_SI NULL
#define OPTION_VAL_9_SI NULL
#define OPTION_VAL_10_SI NULL
#define FCEUMM_APU_1_LABEL_SI NULL
#define FCEUMM_APU_1_LABEL_CAT_SI NULL
#define FCEUMM_APU_1_INFO_0_SI NULL
#define FCEUMM_APU_2_LABEL_SI NULL
#define FCEUMM_APU_2_LABEL_CAT_SI NULL
#define FCEUMM_APU_2_INFO_0_SI NULL
#define FCEUMM_APU_3_LABEL_SI NULL
#define FCEUMM_APU_3_LABEL_CAT_SI NULL
#define FCEUMM_APU_3_INFO_0_SI NULL
#define FCEUMM_APU_4_LABEL_SI NULL
#define FCEUMM_APU_4_LABEL_CAT_SI NULL
#define FCEUMM_APU_4_INFO_0_SI NULL
#define FCEUMM_APU_5_LABEL_SI NULL
#define FCEUMM_APU_5_LABEL_CAT_SI NULL
#define FCEUMM_APU_5_INFO_0_SI NULL
#define FCEUMM_TURBO_ENABLE_LABEL_SI NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_SI NULL
#define OPTION_VAL_NONE_SI NULL
#define OPTION_VAL_PLAYER_1_SI NULL
#define OPTION_VAL_PLAYER_2_SI NULL
#define OPTION_VAL_BOTH_SI NULL
#define FCEUMM_TURBO_DELAY_LABEL_SI NULL
#define FCEUMM_TURBO_DELAY_INFO_0_SI NULL
#define FCEUMM_ZAPPER_MODE_LABEL_SI NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_SI NULL
#define OPTION_VAL_CLIGHTGUN_SI NULL
#define OPTION_VAL_STLIGHTGUN_SI NULL
#define OPTION_VAL_TOUCHSCREEN_SI NULL
#define OPTION_VAL_MOUSE_SI NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_SI NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_SI NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_SI NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_SI NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_SI NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_SI NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_SI NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_SI NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_SI NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_SI NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_SI NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_SI NULL
#define FCEUMM_OVERCLOCKING_LABEL_SI NULL
#define FCEUMM_OVERCLOCKING_INFO_0_SI NULL
#define OPTION_VAL_2X_POSTRENDER_SI NULL
#define OPTION_VAL_2X_VBLANK_SI NULL
#define FCEUMM_RAMSTATE_LABEL_SI NULL
#define FCEUMM_RAMSTATE_INFO_0_SI NULL
#define OPTION_VAL_FILL_FF_SI NULL
#define OPTION_VAL_FILL_00_SI NULL
#define OPTION_VAL_RANDOM_SI NULL

struct retro_core_option_v2_category option_cats_si[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_SI,
      CATEGORY_SYSTEM_INFO_0_SI
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_SI,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_SI
#else
      CATEGORY_VIDEO_INFO_1_SI
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_SI,
      CATEGORY_AUDIO_INFO_0_SI
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_SI,
      CATEGORY_INPUT_INFO_0_SI
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_SI,
      CATEGORY_HACKS_INFO_0_SI
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_SI,
      CATEGORY_DIP_SWITCH_INFO_0_SI
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_si[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_SI,
      NULL,
      FCEUMM_REGION_INFO_0_SI,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_SI },
         { "NTSC",  OPTION_VAL_NTSC_SI },
         { "PAL",   OPTION_VAL_PAL_SI },
         { "Dendy", OPTION_VAL_DENDY_SI },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_SI,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_SI,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_SI,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_SI,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_SI,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_SI,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_SI,
      NULL,
      FCEUMM_ASPECT_INFO_0_SI,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_SI },
         { "4:3",     OPTION_VAL_4_3_SI },
         { "PP",      OPTION_VAL_PP_SI },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_SI,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_SI,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_SI,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_SI,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_SI,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_SI,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_SI,
      NULL,
      FCEUMM_PALETTE_INFO_0_SI,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_SI },
         { "asqrealc",             OPTION_VAL_ASQREALC_SI },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_SI },
         { "rgb",                  OPTION_VAL_RGB_SI },
         { "yuv-v3",               OPTION_VAL_YUV_V3_SI },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_SI },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_SI },
         { "pal",                  OPTION_VAL_PAL_SI },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_SI },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_SI },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_SI },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_SI },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_SI },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_SI },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_SI },
         { "nescap",               OPTION_VAL_NESCAP_SI },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_SI },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_SI },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_SI },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_SI },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_SI },
         { "raw",                  OPTION_VAL_RAW_SI },
         { "custom",               OPTION_VAL_CUSTOM_SI },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_SI,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_SI,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_SI },
         { "svideo",     OPTION_VAL_SVIDEO_SI },
         { "rgb",        OPTION_VAL_RGB_O9_SI },
         { "monochrome", OPTION_VAL_MONOCHROME_SI },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_SI,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_SI,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_SI },
         { "High",      OPTION_VAL_HIGH_SI },
         { "Very High", OPTION_VAL_VERY_HIGH_SI },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_SI,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_SI,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_SI,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_SI,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_SI },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_SI },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_SI },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_SI },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_SI },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_SI },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_SI },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_SI },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_SI },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_SI },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_SI },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_SI },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_SI },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_SI },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_SI },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_SI },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_SI },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_SI },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_SI },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_SI },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_SI },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_SI },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_SI },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_SI },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_SI },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_SI },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_SI },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_SI },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_SI },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_SI },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_SI },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_SI },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_SI,
      FCEUMM_SWAPDUTY_LABEL_CAT_SI,
      FCEUMM_SWAPDUTY_INFO_0_SI,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_SI,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_SI,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_SI },
         { "1",  OPTION_VAL_1_SI },
         { "2",  OPTION_VAL_2_SI },
         { "3",  OPTION_VAL_3_SI },
         { "4",  OPTION_VAL_4_SI },
         { "5",  OPTION_VAL_5_SI },
         { "6",  OPTION_VAL_6_SI },
         { "7",  OPTION_VAL_7_SI },
         { "8",  OPTION_VAL_8_SI },
         { "9",  OPTION_VAL_9_SI },
         { "10", OPTION_VAL_10_SI },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_SI,
      FCEUMM_APU_1_LABEL_CAT_SI,
      FCEUMM_APU_1_INFO_0_SI,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_SI,
      FCEUMM_APU_2_LABEL_CAT_SI,
      FCEUMM_APU_2_INFO_0_SI,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_SI,
      FCEUMM_APU_3_LABEL_CAT_SI,
      FCEUMM_APU_3_INFO_0_SI,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_SI,
      FCEUMM_APU_4_LABEL_CAT_SI,
      FCEUMM_APU_4_INFO_0_SI,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_SI,
      FCEUMM_APU_5_LABEL_CAT_SI,
      FCEUMM_APU_5_INFO_0_SI,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_SI,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_SI,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_SI },
         { "Player 1", OPTION_VAL_PLAYER_1_SI },
         { "Player 2", OPTION_VAL_PLAYER_2_SI },
         { "Both",     OPTION_VAL_BOTH_SI },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_SI,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_SI,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_SI,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_SI,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_SI },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_SI },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_SI },
         { "mouse",       OPTION_VAL_MOUSE_SI },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_SI,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_SI,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_SI,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_SI,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_SI,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_SI,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_SI,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_SI,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_SI,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_SI,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_SI,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_SI,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_SI,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_SI,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_SI },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_SI },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_SI,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_SI,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_SI },
         { "fill $00", OPTION_VAL_FILL_00_SI },
         { "random",   OPTION_VAL_RANDOM_SI },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_si = {
   option_cats_si,
   option_defs_si
};

/* RETRO_LANGUAGE_SK */

#define CATEGORY_SYSTEM_LABEL_SK "Systém"
#define CATEGORY_SYSTEM_INFO_0_SK NULL
#define CATEGORY_VIDEO_LABEL_SK NULL
#define CATEGORY_VIDEO_INFO_0_SK NULL
#define CATEGORY_VIDEO_INFO_1_SK NULL
#define CATEGORY_AUDIO_LABEL_SK "Zvuk"
#define CATEGORY_AUDIO_INFO_0_SK NULL
#define CATEGORY_INPUT_LABEL_SK "Vstup"
#define CATEGORY_INPUT_INFO_0_SK NULL
#define CATEGORY_HACKS_LABEL_SK "Emulačné hacky"
#define CATEGORY_HACKS_INFO_0_SK NULL
#define CATEGORY_DIP_SWITCH_LABEL_SK "DIP prepínače"
#define CATEGORY_DIP_SWITCH_INFO_0_SK NULL
#define FCEUMM_REGION_LABEL_SK "Región"
#define FCEUMM_REGION_INFO_0_SK NULL
#define OPTION_VAL_AUTO_SK NULL
#define OPTION_VAL_NTSC_SK NULL
#define OPTION_VAL_PAL_SK NULL
#define OPTION_VAL_DENDY_SK NULL
#define FCEUMM_GAME_GENIE_LABEL_SK NULL
#define FCEUMM_GAME_GENIE_INFO_0_SK NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_SK NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_SK NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_SK NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_SK NULL
#define FCEUMM_ASPECT_LABEL_SK "Pomer strán"
#define FCEUMM_ASPECT_INFO_0_SK NULL
#define OPTION_VAL_8_7_PAR_SK NULL
#define OPTION_VAL_4_3_SK NULL
#define OPTION_VAL_PP_SK NULL
#define FCEUMM_OVERSCAN_LABEL_SK "Orezať overscan"
#define FCEUMM_OVERSCAN_INFO_0_SK NULL
#define FCEUMM_OVERSCAN_H_LABEL_SK NULL
#define FCEUMM_OVERSCAN_H_INFO_0_SK NULL
#define FCEUMM_OVERSCAN_V_LABEL_SK NULL
#define FCEUMM_OVERSCAN_V_INFO_0_SK NULL
#define FCEUMM_PALETTE_LABEL_SK "Farebná paleta"
#define FCEUMM_PALETTE_INFO_0_SK NULL
#define OPTION_VAL_DEFAULT_SK "Predvolené"
#define OPTION_VAL_ASQREALC_SK NULL
#define OPTION_VAL_NINTENDO_VC_SK NULL
#define OPTION_VAL_RGB_SK NULL
#define OPTION_VAL_YUV_V3_SK NULL
#define OPTION_VAL_UNSATURATED_FINAL_SK NULL
#define OPTION_VAL_SONY_CXA2025AS_US_SK NULL
#define OPTION_VAL_BMF_FINAL2_SK NULL
#define OPTION_VAL_BMF_FINAL3_SK NULL
#define OPTION_VAL_SMOOTH_FBX_SK NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_SK NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_SK NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_SK NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_SK NULL
#define OPTION_VAL_NESCAP_SK NULL
#define OPTION_VAL_WAVEBEAM_SK NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_SK NULL
#define OPTION_VAL_MAGNUM_FBX_SK NULL
#define OPTION_VAL_SMOOTH_V2_FBX_SK NULL
#define OPTION_VAL_NES_CLASSIC_FBX_SK NULL
#define OPTION_VAL_RAW_SK "Čisté"
#define OPTION_VAL_CUSTOM_SK "Vlastné"
#define FCEUMM_NTSC_FILTER_LABEL_SK NULL
#define FCEUMM_NTSC_FILTER_INFO_0_SK NULL
#define OPTION_VAL_COMPOSITE_SK "Kompozit"
#define OPTION_VAL_SVIDEO_SK NULL
#define OPTION_VAL_RGB_O9_SK NULL
#define OPTION_VAL_MONOCHROME_SK "Čiernobiele"
#define FCEUMM_SNDQUALITY_LABEL_SK "Kvalita zvuku"
#define FCEUMM_SNDQUALITY_INFO_0_SK NULL
#define OPTION_VAL_LOW_SK "Nízke"
#define OPTION_VAL_HIGH_SK "Vysoké"
#define OPTION_VAL_VERY_HIGH_SK "Veľmi vysoké"
#define FCEUMM_SNDLOWPASS_LABEL_SK NULL
#define FCEUMM_SNDLOWPASS_INFO_0_SK NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_SK NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_SK NULL
#define OPTION_VAL_01_MS_DELAY_SK NULL
#define OPTION_VAL_02_MS_DELAY_SK NULL
#define OPTION_VAL_03_MS_DELAY_SK NULL
#define OPTION_VAL_04_MS_DELAY_SK NULL
#define OPTION_VAL_05_MS_DELAY_SK NULL
#define OPTION_VAL_06_MS_DELAY_SK NULL
#define OPTION_VAL_07_MS_DELAY_SK NULL
#define OPTION_VAL_08_MS_DELAY_SK NULL
#define OPTION_VAL_09_MS_DELAY_SK NULL
#define OPTION_VAL_10_MS_DELAY_SK NULL
#define OPTION_VAL_11_MS_DELAY_SK NULL
#define OPTION_VAL_12_MS_DELAY_SK NULL
#define OPTION_VAL_13_MS_DELAY_SK NULL
#define OPTION_VAL_14_MS_DELAY_SK NULL
#define OPTION_VAL_15_MS_DELAY_SK NULL
#define OPTION_VAL_16_MS_DELAY_SK NULL
#define OPTION_VAL_17_MS_DELAY_SK NULL
#define OPTION_VAL_18_MS_DELAY_SK NULL
#define OPTION_VAL_19_MS_DELAY_SK NULL
#define OPTION_VAL_20_MS_DELAY_SK NULL
#define OPTION_VAL_21_MS_DELAY_SK NULL
#define OPTION_VAL_22_MS_DELAY_SK NULL
#define OPTION_VAL_23_MS_DELAY_SK NULL
#define OPTION_VAL_24_MS_DELAY_SK NULL
#define OPTION_VAL_25_MS_DELAY_SK NULL
#define OPTION_VAL_26_MS_DELAY_SK NULL
#define OPTION_VAL_27_MS_DELAY_SK NULL
#define OPTION_VAL_28_MS_DELAY_SK NULL
#define OPTION_VAL_29_MS_DELAY_SK NULL
#define OPTION_VAL_30_MS_DELAY_SK NULL
#define OPTION_VAL_31_MS_DELAY_SK NULL
#define OPTION_VAL_32_MS_DELAY_SK NULL
#define FCEUMM_SWAPDUTY_LABEL_SK NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_SK NULL
#define FCEUMM_SWAPDUTY_INFO_0_SK NULL
#define FCEUMM_SNDVOLUME_LABEL_SK NULL
#define FCEUMM_SNDVOLUME_INFO_0_SK NULL
#define OPTION_VAL_0_SK NULL
#define OPTION_VAL_1_SK NULL
#define OPTION_VAL_2_SK NULL
#define OPTION_VAL_3_SK NULL
#define OPTION_VAL_4_SK NULL
#define OPTION_VAL_5_SK NULL
#define OPTION_VAL_6_SK NULL
#define OPTION_VAL_7_SK NULL
#define OPTION_VAL_8_SK NULL
#define OPTION_VAL_9_SK NULL
#define OPTION_VAL_10_SK NULL
#define FCEUMM_APU_1_LABEL_SK NULL
#define FCEUMM_APU_1_LABEL_CAT_SK NULL
#define FCEUMM_APU_1_INFO_0_SK NULL
#define FCEUMM_APU_2_LABEL_SK NULL
#define FCEUMM_APU_2_LABEL_CAT_SK NULL
#define FCEUMM_APU_2_INFO_0_SK NULL
#define FCEUMM_APU_3_LABEL_SK NULL
#define FCEUMM_APU_3_LABEL_CAT_SK NULL
#define FCEUMM_APU_3_INFO_0_SK NULL
#define FCEUMM_APU_4_LABEL_SK NULL
#define FCEUMM_APU_4_LABEL_CAT_SK NULL
#define FCEUMM_APU_4_INFO_0_SK NULL
#define FCEUMM_APU_5_LABEL_SK NULL
#define FCEUMM_APU_5_LABEL_CAT_SK NULL
#define FCEUMM_APU_5_INFO_0_SK NULL
#define FCEUMM_TURBO_ENABLE_LABEL_SK "Povoliť Turbo"
#define FCEUMM_TURBO_ENABLE_INFO_0_SK NULL
#define OPTION_VAL_NONE_SK "Žiadne"
#define OPTION_VAL_PLAYER_1_SK "Hráč 1"
#define OPTION_VAL_PLAYER_2_SK "Hráč 2"
#define OPTION_VAL_BOTH_SK "Obe"
#define FCEUMM_TURBO_DELAY_LABEL_SK NULL
#define FCEUMM_TURBO_DELAY_INFO_0_SK NULL
#define FCEUMM_ZAPPER_MODE_LABEL_SK NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_SK NULL
#define OPTION_VAL_CLIGHTGUN_SK NULL
#define OPTION_VAL_STLIGHTGUN_SK NULL
#define OPTION_VAL_TOUCHSCREEN_SK "Dotykový displej"
#define OPTION_VAL_MOUSE_SK "Myš"
#define FCEUMM_SHOW_CROSSHAIR_LABEL_SK NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_SK NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_SK NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_SK NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_SK NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_SK NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_SK NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_SK NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_SK NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_SK NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_SK NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_SK NULL
#define FCEUMM_OVERCLOCKING_LABEL_SK "Pretaktovať"
#define FCEUMM_OVERCLOCKING_INFO_0_SK NULL
#define OPTION_VAL_2X_POSTRENDER_SK NULL
#define OPTION_VAL_2X_VBLANK_SK NULL
#define FCEUMM_RAMSTATE_LABEL_SK NULL
#define FCEUMM_RAMSTATE_INFO_0_SK NULL
#define OPTION_VAL_FILL_FF_SK NULL
#define OPTION_VAL_FILL_00_SK NULL
#define OPTION_VAL_RANDOM_SK "Náhodné"

struct retro_core_option_v2_category option_cats_sk[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_SK,
      CATEGORY_SYSTEM_INFO_0_SK
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_SK,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_SK
#else
      CATEGORY_VIDEO_INFO_1_SK
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_SK,
      CATEGORY_AUDIO_INFO_0_SK
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_SK,
      CATEGORY_INPUT_INFO_0_SK
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_SK,
      CATEGORY_HACKS_INFO_0_SK
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_SK,
      CATEGORY_DIP_SWITCH_INFO_0_SK
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_sk[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_SK,
      NULL,
      FCEUMM_REGION_INFO_0_SK,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_SK },
         { "NTSC",  OPTION_VAL_NTSC_SK },
         { "PAL",   OPTION_VAL_PAL_SK },
         { "Dendy", OPTION_VAL_DENDY_SK },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_SK,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_SK,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_SK,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_SK,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_SK,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_SK,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_SK,
      NULL,
      FCEUMM_ASPECT_INFO_0_SK,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_SK },
         { "4:3",     OPTION_VAL_4_3_SK },
         { "PP",      OPTION_VAL_PP_SK },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_SK,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_SK,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_SK,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_SK,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_SK,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_SK,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_SK,
      NULL,
      FCEUMM_PALETTE_INFO_0_SK,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_SK },
         { "asqrealc",             OPTION_VAL_ASQREALC_SK },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_SK },
         { "rgb",                  OPTION_VAL_RGB_SK },
         { "yuv-v3",               OPTION_VAL_YUV_V3_SK },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_SK },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_SK },
         { "pal",                  OPTION_VAL_PAL_SK },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_SK },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_SK },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_SK },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_SK },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_SK },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_SK },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_SK },
         { "nescap",               OPTION_VAL_NESCAP_SK },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_SK },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_SK },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_SK },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_SK },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_SK },
         { "raw",                  OPTION_VAL_RAW_SK },
         { "custom",               OPTION_VAL_CUSTOM_SK },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_SK,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_SK,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_SK },
         { "svideo",     OPTION_VAL_SVIDEO_SK },
         { "rgb",        OPTION_VAL_RGB_O9_SK },
         { "monochrome", OPTION_VAL_MONOCHROME_SK },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_SK,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_SK,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_SK },
         { "High",      OPTION_VAL_HIGH_SK },
         { "Very High", OPTION_VAL_VERY_HIGH_SK },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_SK,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_SK,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_SK,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_SK,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_SK },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_SK },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_SK },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_SK },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_SK },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_SK },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_SK },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_SK },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_SK },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_SK },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_SK },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_SK },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_SK },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_SK },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_SK },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_SK },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_SK },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_SK },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_SK },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_SK },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_SK },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_SK },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_SK },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_SK },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_SK },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_SK },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_SK },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_SK },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_SK },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_SK },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_SK },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_SK },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_SK,
      FCEUMM_SWAPDUTY_LABEL_CAT_SK,
      FCEUMM_SWAPDUTY_INFO_0_SK,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_SK,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_SK,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_SK },
         { "1",  OPTION_VAL_1_SK },
         { "2",  OPTION_VAL_2_SK },
         { "3",  OPTION_VAL_3_SK },
         { "4",  OPTION_VAL_4_SK },
         { "5",  OPTION_VAL_5_SK },
         { "6",  OPTION_VAL_6_SK },
         { "7",  OPTION_VAL_7_SK },
         { "8",  OPTION_VAL_8_SK },
         { "9",  OPTION_VAL_9_SK },
         { "10", OPTION_VAL_10_SK },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_SK,
      FCEUMM_APU_1_LABEL_CAT_SK,
      FCEUMM_APU_1_INFO_0_SK,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_SK,
      FCEUMM_APU_2_LABEL_CAT_SK,
      FCEUMM_APU_2_INFO_0_SK,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_SK,
      FCEUMM_APU_3_LABEL_CAT_SK,
      FCEUMM_APU_3_INFO_0_SK,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_SK,
      FCEUMM_APU_4_LABEL_CAT_SK,
      FCEUMM_APU_4_INFO_0_SK,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_SK,
      FCEUMM_APU_5_LABEL_CAT_SK,
      FCEUMM_APU_5_INFO_0_SK,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_SK,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_SK,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_SK },
         { "Player 1", OPTION_VAL_PLAYER_1_SK },
         { "Player 2", OPTION_VAL_PLAYER_2_SK },
         { "Both",     OPTION_VAL_BOTH_SK },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_SK,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_SK,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_SK,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_SK,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_SK },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_SK },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_SK },
         { "mouse",       OPTION_VAL_MOUSE_SK },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_SK,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_SK,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_SK,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_SK,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_SK,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_SK,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_SK,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_SK,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_SK,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_SK,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_SK,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_SK,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_SK,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_SK,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_SK },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_SK },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_SK,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_SK,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_SK },
         { "fill $00", OPTION_VAL_FILL_00_SK },
         { "random",   OPTION_VAL_RANDOM_SK },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_sk = {
   option_cats_sk,
   option_defs_sk
};

/* RETRO_LANGUAGE_SR */

#define CATEGORY_SYSTEM_LABEL_SR NULL
#define CATEGORY_SYSTEM_INFO_0_SR NULL
#define CATEGORY_VIDEO_LABEL_SR NULL
#define CATEGORY_VIDEO_INFO_0_SR NULL
#define CATEGORY_VIDEO_INFO_1_SR NULL
#define CATEGORY_AUDIO_LABEL_SR "Zvuk"
#define CATEGORY_AUDIO_INFO_0_SR NULL
#define CATEGORY_INPUT_LABEL_SR "Ulaz"
#define CATEGORY_INPUT_INFO_0_SR NULL
#define CATEGORY_HACKS_LABEL_SR NULL
#define CATEGORY_HACKS_INFO_0_SR NULL
#define CATEGORY_DIP_SWITCH_LABEL_SR NULL
#define CATEGORY_DIP_SWITCH_INFO_0_SR NULL
#define FCEUMM_REGION_LABEL_SR NULL
#define FCEUMM_REGION_INFO_0_SR NULL
#define OPTION_VAL_AUTO_SR NULL
#define OPTION_VAL_NTSC_SR NULL
#define OPTION_VAL_PAL_SR NULL
#define OPTION_VAL_DENDY_SR NULL
#define FCEUMM_GAME_GENIE_LABEL_SR NULL
#define FCEUMM_GAME_GENIE_INFO_0_SR NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_SR NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_SR NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_SR NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_SR NULL
#define FCEUMM_ASPECT_LABEL_SR "Odnos ekrana"
#define FCEUMM_ASPECT_INFO_0_SR NULL
#define OPTION_VAL_8_7_PAR_SR NULL
#define OPTION_VAL_4_3_SR NULL
#define OPTION_VAL_PP_SR NULL
#define FCEUMM_OVERSCAN_LABEL_SR NULL
#define FCEUMM_OVERSCAN_INFO_0_SR NULL
#define FCEUMM_OVERSCAN_H_LABEL_SR NULL
#define FCEUMM_OVERSCAN_H_INFO_0_SR NULL
#define FCEUMM_OVERSCAN_V_LABEL_SR NULL
#define FCEUMM_OVERSCAN_V_INFO_0_SR NULL
#define FCEUMM_PALETTE_LABEL_SR NULL
#define FCEUMM_PALETTE_INFO_0_SR NULL
#define OPTION_VAL_DEFAULT_SR NULL
#define OPTION_VAL_ASQREALC_SR NULL
#define OPTION_VAL_NINTENDO_VC_SR NULL
#define OPTION_VAL_RGB_SR NULL
#define OPTION_VAL_YUV_V3_SR NULL
#define OPTION_VAL_UNSATURATED_FINAL_SR NULL
#define OPTION_VAL_SONY_CXA2025AS_US_SR NULL
#define OPTION_VAL_BMF_FINAL2_SR NULL
#define OPTION_VAL_BMF_FINAL3_SR NULL
#define OPTION_VAL_SMOOTH_FBX_SR NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_SR NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_SR NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_SR NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_SR NULL
#define OPTION_VAL_NESCAP_SR NULL
#define OPTION_VAL_WAVEBEAM_SR NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_SR NULL
#define OPTION_VAL_MAGNUM_FBX_SR NULL
#define OPTION_VAL_SMOOTH_V2_FBX_SR NULL
#define OPTION_VAL_NES_CLASSIC_FBX_SR NULL
#define OPTION_VAL_RAW_SR NULL
#define OPTION_VAL_CUSTOM_SR NULL
#define FCEUMM_NTSC_FILTER_LABEL_SR NULL
#define FCEUMM_NTSC_FILTER_INFO_0_SR NULL
#define OPTION_VAL_COMPOSITE_SR NULL
#define OPTION_VAL_SVIDEO_SR NULL
#define OPTION_VAL_RGB_O9_SR NULL
#define OPTION_VAL_MONOCHROME_SR NULL
#define FCEUMM_SNDQUALITY_LABEL_SR NULL
#define FCEUMM_SNDQUALITY_INFO_0_SR NULL
#define OPTION_VAL_LOW_SR NULL
#define OPTION_VAL_HIGH_SR NULL
#define OPTION_VAL_VERY_HIGH_SR NULL
#define FCEUMM_SNDLOWPASS_LABEL_SR NULL
#define FCEUMM_SNDLOWPASS_INFO_0_SR NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_SR NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_SR NULL
#define OPTION_VAL_01_MS_DELAY_SR NULL
#define OPTION_VAL_02_MS_DELAY_SR NULL
#define OPTION_VAL_03_MS_DELAY_SR NULL
#define OPTION_VAL_04_MS_DELAY_SR NULL
#define OPTION_VAL_05_MS_DELAY_SR NULL
#define OPTION_VAL_06_MS_DELAY_SR NULL
#define OPTION_VAL_07_MS_DELAY_SR NULL
#define OPTION_VAL_08_MS_DELAY_SR NULL
#define OPTION_VAL_09_MS_DELAY_SR NULL
#define OPTION_VAL_10_MS_DELAY_SR NULL
#define OPTION_VAL_11_MS_DELAY_SR NULL
#define OPTION_VAL_12_MS_DELAY_SR NULL
#define OPTION_VAL_13_MS_DELAY_SR NULL
#define OPTION_VAL_14_MS_DELAY_SR NULL
#define OPTION_VAL_15_MS_DELAY_SR NULL
#define OPTION_VAL_16_MS_DELAY_SR NULL
#define OPTION_VAL_17_MS_DELAY_SR NULL
#define OPTION_VAL_18_MS_DELAY_SR NULL
#define OPTION_VAL_19_MS_DELAY_SR NULL
#define OPTION_VAL_20_MS_DELAY_SR NULL
#define OPTION_VAL_21_MS_DELAY_SR NULL
#define OPTION_VAL_22_MS_DELAY_SR NULL
#define OPTION_VAL_23_MS_DELAY_SR NULL
#define OPTION_VAL_24_MS_DELAY_SR NULL
#define OPTION_VAL_25_MS_DELAY_SR NULL
#define OPTION_VAL_26_MS_DELAY_SR NULL
#define OPTION_VAL_27_MS_DELAY_SR NULL
#define OPTION_VAL_28_MS_DELAY_SR NULL
#define OPTION_VAL_29_MS_DELAY_SR NULL
#define OPTION_VAL_30_MS_DELAY_SR NULL
#define OPTION_VAL_31_MS_DELAY_SR NULL
#define OPTION_VAL_32_MS_DELAY_SR NULL
#define FCEUMM_SWAPDUTY_LABEL_SR NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_SR NULL
#define FCEUMM_SWAPDUTY_INFO_0_SR NULL
#define FCEUMM_SNDVOLUME_LABEL_SR NULL
#define FCEUMM_SNDVOLUME_INFO_0_SR NULL
#define OPTION_VAL_0_SR NULL
#define OPTION_VAL_1_SR NULL
#define OPTION_VAL_2_SR NULL
#define OPTION_VAL_3_SR NULL
#define OPTION_VAL_4_SR NULL
#define OPTION_VAL_5_SR NULL
#define OPTION_VAL_6_SR NULL
#define OPTION_VAL_7_SR NULL
#define OPTION_VAL_8_SR NULL
#define OPTION_VAL_9_SR NULL
#define OPTION_VAL_10_SR NULL
#define FCEUMM_APU_1_LABEL_SR NULL
#define FCEUMM_APU_1_LABEL_CAT_SR NULL
#define FCEUMM_APU_1_INFO_0_SR NULL
#define FCEUMM_APU_2_LABEL_SR NULL
#define FCEUMM_APU_2_LABEL_CAT_SR NULL
#define FCEUMM_APU_2_INFO_0_SR NULL
#define FCEUMM_APU_3_LABEL_SR NULL
#define FCEUMM_APU_3_LABEL_CAT_SR NULL
#define FCEUMM_APU_3_INFO_0_SR NULL
#define FCEUMM_APU_4_LABEL_SR NULL
#define FCEUMM_APU_4_LABEL_CAT_SR NULL
#define FCEUMM_APU_4_INFO_0_SR NULL
#define FCEUMM_APU_5_LABEL_SR NULL
#define FCEUMM_APU_5_LABEL_CAT_SR NULL
#define FCEUMM_APU_5_INFO_0_SR NULL
#define FCEUMM_TURBO_ENABLE_LABEL_SR NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_SR NULL
#define OPTION_VAL_NONE_SR NULL
#define OPTION_VAL_PLAYER_1_SR NULL
#define OPTION_VAL_PLAYER_2_SR NULL
#define OPTION_VAL_BOTH_SR NULL
#define FCEUMM_TURBO_DELAY_LABEL_SR NULL
#define FCEUMM_TURBO_DELAY_INFO_0_SR NULL
#define FCEUMM_ZAPPER_MODE_LABEL_SR NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_SR NULL
#define OPTION_VAL_CLIGHTGUN_SR NULL
#define OPTION_VAL_STLIGHTGUN_SR NULL
#define OPTION_VAL_TOUCHSCREEN_SR NULL
#define OPTION_VAL_MOUSE_SR NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_SR NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_SR NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_SR NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_SR NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_SR NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_SR NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_SR NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_SR NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_SR NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_SR NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_SR NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_SR NULL
#define FCEUMM_OVERCLOCKING_LABEL_SR NULL
#define FCEUMM_OVERCLOCKING_INFO_0_SR NULL
#define OPTION_VAL_2X_POSTRENDER_SR NULL
#define OPTION_VAL_2X_VBLANK_SR NULL
#define FCEUMM_RAMSTATE_LABEL_SR NULL
#define FCEUMM_RAMSTATE_INFO_0_SR NULL
#define OPTION_VAL_FILL_FF_SR NULL
#define OPTION_VAL_FILL_00_SR NULL
#define OPTION_VAL_RANDOM_SR NULL

struct retro_core_option_v2_category option_cats_sr[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_SR,
      CATEGORY_SYSTEM_INFO_0_SR
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_SR,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_SR
#else
      CATEGORY_VIDEO_INFO_1_SR
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_SR,
      CATEGORY_AUDIO_INFO_0_SR
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_SR,
      CATEGORY_INPUT_INFO_0_SR
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_SR,
      CATEGORY_HACKS_INFO_0_SR
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_SR,
      CATEGORY_DIP_SWITCH_INFO_0_SR
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_sr[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_SR,
      NULL,
      FCEUMM_REGION_INFO_0_SR,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_SR },
         { "NTSC",  OPTION_VAL_NTSC_SR },
         { "PAL",   OPTION_VAL_PAL_SR },
         { "Dendy", OPTION_VAL_DENDY_SR },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_SR,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_SR,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_SR,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_SR,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_SR,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_SR,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_SR,
      NULL,
      FCEUMM_ASPECT_INFO_0_SR,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_SR },
         { "4:3",     OPTION_VAL_4_3_SR },
         { "PP",      OPTION_VAL_PP_SR },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_SR,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_SR,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_SR,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_SR,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_SR,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_SR,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_SR,
      NULL,
      FCEUMM_PALETTE_INFO_0_SR,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_SR },
         { "asqrealc",             OPTION_VAL_ASQREALC_SR },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_SR },
         { "rgb",                  OPTION_VAL_RGB_SR },
         { "yuv-v3",               OPTION_VAL_YUV_V3_SR },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_SR },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_SR },
         { "pal",                  OPTION_VAL_PAL_SR },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_SR },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_SR },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_SR },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_SR },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_SR },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_SR },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_SR },
         { "nescap",               OPTION_VAL_NESCAP_SR },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_SR },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_SR },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_SR },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_SR },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_SR },
         { "raw",                  OPTION_VAL_RAW_SR },
         { "custom",               OPTION_VAL_CUSTOM_SR },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_SR,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_SR,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_SR },
         { "svideo",     OPTION_VAL_SVIDEO_SR },
         { "rgb",        OPTION_VAL_RGB_O9_SR },
         { "monochrome", OPTION_VAL_MONOCHROME_SR },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_SR,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_SR,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_SR },
         { "High",      OPTION_VAL_HIGH_SR },
         { "Very High", OPTION_VAL_VERY_HIGH_SR },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_SR,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_SR,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_SR,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_SR,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_SR },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_SR },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_SR },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_SR },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_SR },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_SR },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_SR },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_SR },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_SR },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_SR },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_SR },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_SR },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_SR },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_SR },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_SR },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_SR },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_SR },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_SR },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_SR },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_SR },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_SR },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_SR },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_SR },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_SR },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_SR },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_SR },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_SR },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_SR },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_SR },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_SR },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_SR },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_SR },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_SR,
      FCEUMM_SWAPDUTY_LABEL_CAT_SR,
      FCEUMM_SWAPDUTY_INFO_0_SR,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_SR,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_SR,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_SR },
         { "1",  OPTION_VAL_1_SR },
         { "2",  OPTION_VAL_2_SR },
         { "3",  OPTION_VAL_3_SR },
         { "4",  OPTION_VAL_4_SR },
         { "5",  OPTION_VAL_5_SR },
         { "6",  OPTION_VAL_6_SR },
         { "7",  OPTION_VAL_7_SR },
         { "8",  OPTION_VAL_8_SR },
         { "9",  OPTION_VAL_9_SR },
         { "10", OPTION_VAL_10_SR },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_SR,
      FCEUMM_APU_1_LABEL_CAT_SR,
      FCEUMM_APU_1_INFO_0_SR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_SR,
      FCEUMM_APU_2_LABEL_CAT_SR,
      FCEUMM_APU_2_INFO_0_SR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_SR,
      FCEUMM_APU_3_LABEL_CAT_SR,
      FCEUMM_APU_3_INFO_0_SR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_SR,
      FCEUMM_APU_4_LABEL_CAT_SR,
      FCEUMM_APU_4_INFO_0_SR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_SR,
      FCEUMM_APU_5_LABEL_CAT_SR,
      FCEUMM_APU_5_INFO_0_SR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_SR,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_SR,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_SR },
         { "Player 1", OPTION_VAL_PLAYER_1_SR },
         { "Player 2", OPTION_VAL_PLAYER_2_SR },
         { "Both",     OPTION_VAL_BOTH_SR },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_SR,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_SR,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_SR,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_SR,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_SR },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_SR },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_SR },
         { "mouse",       OPTION_VAL_MOUSE_SR },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_SR,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_SR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_SR,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_SR,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_SR,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_SR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_SR,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_SR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_SR,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_SR,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_SR,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_SR,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_SR,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_SR,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_SR },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_SR },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_SR,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_SR,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_SR },
         { "fill $00", OPTION_VAL_FILL_00_SR },
         { "random",   OPTION_VAL_RANDOM_SR },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_sr = {
   option_cats_sr,
   option_defs_sr
};

/* RETRO_LANGUAGE_SV */

#define CATEGORY_SYSTEM_LABEL_SV NULL
#define CATEGORY_SYSTEM_INFO_0_SV NULL
#define CATEGORY_VIDEO_LABEL_SV "Bild"
#define CATEGORY_VIDEO_INFO_0_SV NULL
#define CATEGORY_VIDEO_INFO_1_SV NULL
#define CATEGORY_AUDIO_LABEL_SV "Ljud"
#define CATEGORY_AUDIO_INFO_0_SV NULL
#define CATEGORY_INPUT_LABEL_SV "Inmatning"
#define CATEGORY_INPUT_INFO_0_SV NULL
#define CATEGORY_HACKS_LABEL_SV NULL
#define CATEGORY_HACKS_INFO_0_SV NULL
#define CATEGORY_DIP_SWITCH_LABEL_SV NULL
#define CATEGORY_DIP_SWITCH_INFO_0_SV NULL
#define FCEUMM_REGION_LABEL_SV NULL
#define FCEUMM_REGION_INFO_0_SV NULL
#define OPTION_VAL_AUTO_SV "Automatiskt"
#define OPTION_VAL_NTSC_SV NULL
#define OPTION_VAL_PAL_SV NULL
#define OPTION_VAL_DENDY_SV NULL
#define FCEUMM_GAME_GENIE_LABEL_SV NULL
#define FCEUMM_GAME_GENIE_INFO_0_SV NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_SV NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_SV NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_SV NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_SV NULL
#define FCEUMM_ASPECT_LABEL_SV "Bildförhållande"
#define FCEUMM_ASPECT_INFO_0_SV NULL
#define OPTION_VAL_8_7_PAR_SV NULL
#define OPTION_VAL_4_3_SV NULL
#define OPTION_VAL_PP_SV NULL
#define FCEUMM_OVERSCAN_LABEL_SV NULL
#define FCEUMM_OVERSCAN_INFO_0_SV NULL
#define FCEUMM_OVERSCAN_H_LABEL_SV NULL
#define FCEUMM_OVERSCAN_H_INFO_0_SV NULL
#define FCEUMM_OVERSCAN_V_LABEL_SV NULL
#define FCEUMM_OVERSCAN_V_INFO_0_SV NULL
#define FCEUMM_PALETTE_LABEL_SV "Färgpalett"
#define FCEUMM_PALETTE_INFO_0_SV NULL
#define OPTION_VAL_DEFAULT_SV NULL
#define OPTION_VAL_ASQREALC_SV NULL
#define OPTION_VAL_NINTENDO_VC_SV NULL
#define OPTION_VAL_RGB_SV NULL
#define OPTION_VAL_YUV_V3_SV NULL
#define OPTION_VAL_UNSATURATED_FINAL_SV NULL
#define OPTION_VAL_SONY_CXA2025AS_US_SV NULL
#define OPTION_VAL_BMF_FINAL2_SV NULL
#define OPTION_VAL_BMF_FINAL3_SV NULL
#define OPTION_VAL_SMOOTH_FBX_SV NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_SV NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_SV NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_SV NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_SV NULL
#define OPTION_VAL_NESCAP_SV NULL
#define OPTION_VAL_WAVEBEAM_SV NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_SV NULL
#define OPTION_VAL_MAGNUM_FBX_SV NULL
#define OPTION_VAL_SMOOTH_V2_FBX_SV NULL
#define OPTION_VAL_NES_CLASSIC_FBX_SV NULL
#define OPTION_VAL_RAW_SV NULL
#define OPTION_VAL_CUSTOM_SV "Anpassad"
#define FCEUMM_NTSC_FILTER_LABEL_SV NULL
#define FCEUMM_NTSC_FILTER_INFO_0_SV NULL
#define OPTION_VAL_COMPOSITE_SV "Komposit"
#define OPTION_VAL_SVIDEO_SV NULL
#define OPTION_VAL_RGB_O9_SV NULL
#define OPTION_VAL_MONOCHROME_SV "Monokrom"
#define FCEUMM_SNDQUALITY_LABEL_SV "Ljudkvalitet"
#define FCEUMM_SNDQUALITY_INFO_0_SV NULL
#define OPTION_VAL_LOW_SV "Låg"
#define OPTION_VAL_HIGH_SV "Hög"
#define OPTION_VAL_VERY_HIGH_SV NULL
#define FCEUMM_SNDLOWPASS_LABEL_SV NULL
#define FCEUMM_SNDLOWPASS_INFO_0_SV NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_SV NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_SV NULL
#define OPTION_VAL_01_MS_DELAY_SV NULL
#define OPTION_VAL_02_MS_DELAY_SV NULL
#define OPTION_VAL_03_MS_DELAY_SV NULL
#define OPTION_VAL_04_MS_DELAY_SV NULL
#define OPTION_VAL_05_MS_DELAY_SV NULL
#define OPTION_VAL_06_MS_DELAY_SV NULL
#define OPTION_VAL_07_MS_DELAY_SV NULL
#define OPTION_VAL_08_MS_DELAY_SV NULL
#define OPTION_VAL_09_MS_DELAY_SV NULL
#define OPTION_VAL_10_MS_DELAY_SV NULL
#define OPTION_VAL_11_MS_DELAY_SV NULL
#define OPTION_VAL_12_MS_DELAY_SV NULL
#define OPTION_VAL_13_MS_DELAY_SV NULL
#define OPTION_VAL_14_MS_DELAY_SV NULL
#define OPTION_VAL_15_MS_DELAY_SV NULL
#define OPTION_VAL_16_MS_DELAY_SV NULL
#define OPTION_VAL_17_MS_DELAY_SV NULL
#define OPTION_VAL_18_MS_DELAY_SV NULL
#define OPTION_VAL_19_MS_DELAY_SV NULL
#define OPTION_VAL_20_MS_DELAY_SV NULL
#define OPTION_VAL_21_MS_DELAY_SV NULL
#define OPTION_VAL_22_MS_DELAY_SV NULL
#define OPTION_VAL_23_MS_DELAY_SV NULL
#define OPTION_VAL_24_MS_DELAY_SV NULL
#define OPTION_VAL_25_MS_DELAY_SV NULL
#define OPTION_VAL_26_MS_DELAY_SV NULL
#define OPTION_VAL_27_MS_DELAY_SV NULL
#define OPTION_VAL_28_MS_DELAY_SV NULL
#define OPTION_VAL_29_MS_DELAY_SV NULL
#define OPTION_VAL_30_MS_DELAY_SV NULL
#define OPTION_VAL_31_MS_DELAY_SV NULL
#define OPTION_VAL_32_MS_DELAY_SV NULL
#define FCEUMM_SWAPDUTY_LABEL_SV NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_SV NULL
#define FCEUMM_SWAPDUTY_INFO_0_SV NULL
#define FCEUMM_SNDVOLUME_LABEL_SV NULL
#define FCEUMM_SNDVOLUME_INFO_0_SV NULL
#define OPTION_VAL_0_SV "0 %"
#define OPTION_VAL_1_SV "10 %"
#define OPTION_VAL_2_SV "20 %"
#define OPTION_VAL_3_SV "30 %"
#define OPTION_VAL_4_SV "40 %"
#define OPTION_VAL_5_SV "50 %"
#define OPTION_VAL_6_SV "60 %"
#define OPTION_VAL_7_SV "70 %"
#define OPTION_VAL_8_SV "80 %"
#define OPTION_VAL_9_SV "90 %"
#define OPTION_VAL_10_SV "100 %"
#define FCEUMM_APU_1_LABEL_SV NULL
#define FCEUMM_APU_1_LABEL_CAT_SV NULL
#define FCEUMM_APU_1_INFO_0_SV NULL
#define FCEUMM_APU_2_LABEL_SV NULL
#define FCEUMM_APU_2_LABEL_CAT_SV NULL
#define FCEUMM_APU_2_INFO_0_SV NULL
#define FCEUMM_APU_3_LABEL_SV NULL
#define FCEUMM_APU_3_LABEL_CAT_SV NULL
#define FCEUMM_APU_3_INFO_0_SV NULL
#define FCEUMM_APU_4_LABEL_SV NULL
#define FCEUMM_APU_4_LABEL_CAT_SV NULL
#define FCEUMM_APU_4_INFO_0_SV NULL
#define FCEUMM_APU_5_LABEL_SV NULL
#define FCEUMM_APU_5_LABEL_CAT_SV NULL
#define FCEUMM_APU_5_INFO_0_SV NULL
#define FCEUMM_TURBO_ENABLE_LABEL_SV NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_SV NULL
#define OPTION_VAL_NONE_SV "Ingen"
#define OPTION_VAL_PLAYER_1_SV "Spelare 1"
#define OPTION_VAL_PLAYER_2_SV "Spelare 2"
#define OPTION_VAL_BOTH_SV "Båda"
#define FCEUMM_TURBO_DELAY_LABEL_SV NULL
#define FCEUMM_TURBO_DELAY_INFO_0_SV NULL
#define FCEUMM_ZAPPER_MODE_LABEL_SV NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_SV NULL
#define OPTION_VAL_CLIGHTGUN_SV NULL
#define OPTION_VAL_STLIGHTGUN_SV NULL
#define OPTION_VAL_TOUCHSCREEN_SV "Pekskärm"
#define OPTION_VAL_MOUSE_SV "Mus"
#define FCEUMM_SHOW_CROSSHAIR_LABEL_SV NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_SV NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_SV NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_SV NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_SV NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_SV NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_SV NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_SV NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_SV NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_SV NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_SV NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_SV NULL
#define FCEUMM_OVERCLOCKING_LABEL_SV NULL
#define FCEUMM_OVERCLOCKING_INFO_0_SV NULL
#define OPTION_VAL_2X_POSTRENDER_SV NULL
#define OPTION_VAL_2X_VBLANK_SV NULL
#define FCEUMM_RAMSTATE_LABEL_SV NULL
#define FCEUMM_RAMSTATE_INFO_0_SV NULL
#define OPTION_VAL_FILL_FF_SV NULL
#define OPTION_VAL_FILL_00_SV NULL
#define OPTION_VAL_RANDOM_SV NULL

struct retro_core_option_v2_category option_cats_sv[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_SV,
      CATEGORY_SYSTEM_INFO_0_SV
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_SV,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_SV
#else
      CATEGORY_VIDEO_INFO_1_SV
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_SV,
      CATEGORY_AUDIO_INFO_0_SV
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_SV,
      CATEGORY_INPUT_INFO_0_SV
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_SV,
      CATEGORY_HACKS_INFO_0_SV
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_SV,
      CATEGORY_DIP_SWITCH_INFO_0_SV
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_sv[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_SV,
      NULL,
      FCEUMM_REGION_INFO_0_SV,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_SV },
         { "NTSC",  OPTION_VAL_NTSC_SV },
         { "PAL",   OPTION_VAL_PAL_SV },
         { "Dendy", OPTION_VAL_DENDY_SV },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_SV,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_SV,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_SV,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_SV,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_SV,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_SV,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_SV,
      NULL,
      FCEUMM_ASPECT_INFO_0_SV,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_SV },
         { "4:3",     OPTION_VAL_4_3_SV },
         { "PP",      OPTION_VAL_PP_SV },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_SV,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_SV,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_SV,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_SV,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_SV,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_SV,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_SV,
      NULL,
      FCEUMM_PALETTE_INFO_0_SV,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_SV },
         { "asqrealc",             OPTION_VAL_ASQREALC_SV },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_SV },
         { "rgb",                  OPTION_VAL_RGB_SV },
         { "yuv-v3",               OPTION_VAL_YUV_V3_SV },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_SV },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_SV },
         { "pal",                  OPTION_VAL_PAL_SV },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_SV },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_SV },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_SV },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_SV },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_SV },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_SV },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_SV },
         { "nescap",               OPTION_VAL_NESCAP_SV },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_SV },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_SV },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_SV },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_SV },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_SV },
         { "raw",                  OPTION_VAL_RAW_SV },
         { "custom",               OPTION_VAL_CUSTOM_SV },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_SV,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_SV,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_SV },
         { "svideo",     OPTION_VAL_SVIDEO_SV },
         { "rgb",        OPTION_VAL_RGB_O9_SV },
         { "monochrome", OPTION_VAL_MONOCHROME_SV },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_SV,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_SV,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_SV },
         { "High",      OPTION_VAL_HIGH_SV },
         { "Very High", OPTION_VAL_VERY_HIGH_SV },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_SV,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_SV,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_SV,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_SV,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_SV },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_SV },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_SV },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_SV },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_SV },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_SV },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_SV },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_SV },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_SV },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_SV },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_SV },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_SV },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_SV },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_SV },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_SV },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_SV },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_SV },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_SV },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_SV },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_SV },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_SV },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_SV },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_SV },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_SV },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_SV },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_SV },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_SV },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_SV },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_SV },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_SV },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_SV },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_SV },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_SV,
      FCEUMM_SWAPDUTY_LABEL_CAT_SV,
      FCEUMM_SWAPDUTY_INFO_0_SV,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_SV,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_SV,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_SV },
         { "1",  OPTION_VAL_1_SV },
         { "2",  OPTION_VAL_2_SV },
         { "3",  OPTION_VAL_3_SV },
         { "4",  OPTION_VAL_4_SV },
         { "5",  OPTION_VAL_5_SV },
         { "6",  OPTION_VAL_6_SV },
         { "7",  OPTION_VAL_7_SV },
         { "8",  OPTION_VAL_8_SV },
         { "9",  OPTION_VAL_9_SV },
         { "10", OPTION_VAL_10_SV },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_SV,
      FCEUMM_APU_1_LABEL_CAT_SV,
      FCEUMM_APU_1_INFO_0_SV,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_SV,
      FCEUMM_APU_2_LABEL_CAT_SV,
      FCEUMM_APU_2_INFO_0_SV,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_SV,
      FCEUMM_APU_3_LABEL_CAT_SV,
      FCEUMM_APU_3_INFO_0_SV,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_SV,
      FCEUMM_APU_4_LABEL_CAT_SV,
      FCEUMM_APU_4_INFO_0_SV,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_SV,
      FCEUMM_APU_5_LABEL_CAT_SV,
      FCEUMM_APU_5_INFO_0_SV,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_SV,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_SV,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_SV },
         { "Player 1", OPTION_VAL_PLAYER_1_SV },
         { "Player 2", OPTION_VAL_PLAYER_2_SV },
         { "Both",     OPTION_VAL_BOTH_SV },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_SV,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_SV,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_SV,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_SV,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_SV },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_SV },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_SV },
         { "mouse",       OPTION_VAL_MOUSE_SV },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_SV,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_SV,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_SV,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_SV,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_SV,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_SV,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_SV,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_SV,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_SV,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_SV,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_SV,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_SV,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_SV,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_SV,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_SV },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_SV },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_SV,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_SV,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_SV },
         { "fill $00", OPTION_VAL_FILL_00_SV },
         { "random",   OPTION_VAL_RANDOM_SV },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_sv = {
   option_cats_sv,
   option_defs_sv
};

/* RETRO_LANGUAGE_TR */

#define CATEGORY_SYSTEM_LABEL_TR "Sistem"
#define CATEGORY_SYSTEM_INFO_0_TR NULL
#define CATEGORY_VIDEO_LABEL_TR NULL
#define CATEGORY_VIDEO_INFO_0_TR NULL
#define CATEGORY_VIDEO_INFO_1_TR NULL
#define CATEGORY_AUDIO_LABEL_TR "Ses"
#define CATEGORY_AUDIO_INFO_0_TR NULL
#define CATEGORY_INPUT_LABEL_TR "Girdi"
#define CATEGORY_INPUT_INFO_0_TR NULL
#define CATEGORY_HACKS_LABEL_TR "Emülatör Geliştirmeleri"
#define CATEGORY_HACKS_INFO_0_TR "Düşük seviyeli performansı ve uyumluluğu etkileyen işlemci hız aşırtma ve taklit doğruluğu ayarlarını değiştirin."
#define CATEGORY_DIP_SWITCH_LABEL_TR "DIP Anahtarları"
#define CATEGORY_DIP_SWITCH_INFO_0_TR NULL
#define FCEUMM_REGION_LABEL_TR "Bölge"
#define FCEUMM_REGION_INFO_0_TR "Çekirdeği NTSC, PAL veya Dendy bölge zamanlamaları kullanmaya zorlayın."
#define OPTION_VAL_AUTO_TR "Otomatik"
#define OPTION_VAL_NTSC_TR NULL
#define OPTION_VAL_PAL_TR NULL
#define OPTION_VAL_DENDY_TR NULL
#define FCEUMM_GAME_GENIE_LABEL_TR "Game Genie Eklentisi (Yeniden Başlatılmalı)"
#define FCEUMM_GAME_GENIE_INFO_0_TR "Oyunlar başlatılırken hile kodlarının girilmesine izin vererek Game Genie eklenti kartuşunun taklidini etkinleştirin. Game Genie ROM dosyası 'gamegenie.nes' ön ucun sistem dizininde bulunmalıdır. FDS veya arcade içeriği için geçerli değildir."
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_TR "Gelişmiş Sistem Seçeneklerini Göster"
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_TR "Gelişmiş sistem seçeneklerini ve ince ayarları göster."
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_TR "Gelişmiş Ses Seçeneklerini Göster"
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_TR "Gelişmiş ses seçeneklerini ve ince ayarları gösterin."
#define FCEUMM_ASPECT_LABEL_TR "En Boy Oranı"
#define FCEUMM_ASPECT_INFO_0_TR "Tercih edilen en boy oranını seçin."
#define OPTION_VAL_8_7_PAR_TR NULL
#define OPTION_VAL_4_3_TR NULL
#define OPTION_VAL_PP_TR "Mükemmel Piksel"
#define FCEUMM_OVERSCAN_LABEL_TR "Aşırı Taramayı Kırp"
#define FCEUMM_OVERSCAN_INFO_0_TR "Ekranın her tarafından 8 piksel fazla taramayı kaldırır."
#define FCEUMM_OVERSCAN_H_LABEL_TR "Yatay Aşırı Taramayı Kırp"
#define FCEUMM_OVERSCAN_H_INFO_0_TR "Standart CRT televizyonlarda görülen aşırı taramayı taklit etmek için ekranın sol ve sağ taraflarındaki 8 pikseli kaldırır."
#define FCEUMM_OVERSCAN_V_LABEL_TR "Dikey Aşırı Taramayı Kırp"
#define FCEUMM_OVERSCAN_V_INFO_0_TR "Standart CRT televizyonlarda görülen aşırı taramayı taklit etmek için ekranın üstünden ve altından 8 pikseli kaldırır."
#define FCEUMM_PALETTE_LABEL_TR "Renk Paleti"
#define FCEUMM_PALETTE_INFO_0_TR "Önceden oluşturulmuş paletler, dosyadan veya ham formattan özel bir 64x3 palet (bir nes-kod çözücü gölgelendiricisi kullanması gerekir) arasından seçim yapın."
#define OPTION_VAL_DEFAULT_TR "Varsayılan"
#define OPTION_VAL_ASQREALC_TR NULL
#define OPTION_VAL_NINTENDO_VC_TR NULL
#define OPTION_VAL_RGB_TR NULL
#define OPTION_VAL_YUV_V3_TR NULL
#define OPTION_VAL_UNSATURATED_FINAL_TR NULL
#define OPTION_VAL_SONY_CXA2025AS_US_TR NULL
#define OPTION_VAL_BMF_FINAL2_TR NULL
#define OPTION_VAL_BMF_FINAL3_TR NULL
#define OPTION_VAL_SMOOTH_FBX_TR NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_TR NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_TR NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_TR NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_TR NULL
#define OPTION_VAL_NESCAP_TR NULL
#define OPTION_VAL_WAVEBEAM_TR NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_TR NULL
#define OPTION_VAL_MAGNUM_FBX_TR NULL
#define OPTION_VAL_SMOOTH_V2_FBX_TR NULL
#define OPTION_VAL_NES_CLASSIC_FBX_TR NULL
#define OPTION_VAL_RAW_TR "İşlenmemiş"
#define OPTION_VAL_CUSTOM_TR "Özel"
#define FCEUMM_NTSC_FILTER_LABEL_TR "NTSC Filtresi"
#define FCEUMM_NTSC_FILTER_INFO_0_TR NULL
#define OPTION_VAL_COMPOSITE_TR NULL
#define OPTION_VAL_SVIDEO_TR NULL
#define OPTION_VAL_RGB_O9_TR NULL
#define OPTION_VAL_MONOCHROME_TR "Tek Renk"
#define FCEUMM_SNDQUALITY_LABEL_TR "Ses Kalitesi"
#define FCEUMM_SNDQUALITY_INFO_0_TR "Daha yüksek kaliteli sesi etkinleştirin. Performans gereksinimlerini artırır."
#define OPTION_VAL_LOW_TR "Düşük"
#define OPTION_VAL_HIGH_TR "Yüksek"
#define OPTION_VAL_VERY_HIGH_TR "Çok Yüksek"
#define FCEUMM_SNDLOWPASS_LABEL_TR NULL
#define FCEUMM_SNDLOWPASS_INFO_0_TR NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_TR NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_TR NULL
#define OPTION_VAL_01_MS_DELAY_TR NULL
#define OPTION_VAL_02_MS_DELAY_TR NULL
#define OPTION_VAL_03_MS_DELAY_TR NULL
#define OPTION_VAL_04_MS_DELAY_TR NULL
#define OPTION_VAL_05_MS_DELAY_TR NULL
#define OPTION_VAL_06_MS_DELAY_TR NULL
#define OPTION_VAL_07_MS_DELAY_TR NULL
#define OPTION_VAL_08_MS_DELAY_TR NULL
#define OPTION_VAL_09_MS_DELAY_TR NULL
#define OPTION_VAL_10_MS_DELAY_TR NULL
#define OPTION_VAL_11_MS_DELAY_TR NULL
#define OPTION_VAL_12_MS_DELAY_TR NULL
#define OPTION_VAL_13_MS_DELAY_TR NULL
#define OPTION_VAL_14_MS_DELAY_TR NULL
#define OPTION_VAL_15_MS_DELAY_TR NULL
#define OPTION_VAL_16_MS_DELAY_TR NULL
#define OPTION_VAL_17_MS_DELAY_TR NULL
#define OPTION_VAL_18_MS_DELAY_TR NULL
#define OPTION_VAL_19_MS_DELAY_TR NULL
#define OPTION_VAL_20_MS_DELAY_TR NULL
#define OPTION_VAL_21_MS_DELAY_TR NULL
#define OPTION_VAL_22_MS_DELAY_TR NULL
#define OPTION_VAL_23_MS_DELAY_TR NULL
#define OPTION_VAL_24_MS_DELAY_TR NULL
#define OPTION_VAL_25_MS_DELAY_TR NULL
#define OPTION_VAL_26_MS_DELAY_TR NULL
#define OPTION_VAL_27_MS_DELAY_TR NULL
#define OPTION_VAL_28_MS_DELAY_TR NULL
#define OPTION_VAL_29_MS_DELAY_TR NULL
#define OPTION_VAL_30_MS_DELAY_TR NULL
#define OPTION_VAL_31_MS_DELAY_TR NULL
#define OPTION_VAL_32_MS_DELAY_TR NULL
#define FCEUMM_SWAPDUTY_LABEL_TR "Ses Görev Döngülerini Değiştir"
#define FCEUMM_SWAPDUTY_LABEL_CAT_TR "Görev Döngülerini Değiştir"
#define FCEUMM_SWAPDUTY_INFO_0_TR "Darbe dalgası kanalları görev döngüsü bitleri ters çevrilmiş Famiklon üstünden gelen sesi taklit eder."
#define FCEUMM_SNDVOLUME_LABEL_TR "Ana Ses Seviyesi"
#define FCEUMM_SNDVOLUME_INFO_0_TR "Ana ses seviyesini değiştirin."
#define OPTION_VAL_0_TR "%0"
#define OPTION_VAL_1_TR "%10"
#define OPTION_VAL_2_TR "%20"
#define OPTION_VAL_3_TR "%30"
#define OPTION_VAL_4_TR "%40"
#define OPTION_VAL_5_TR "%50"
#define OPTION_VAL_6_TR "%60"
#define OPTION_VAL_7_TR "%70"
#define OPTION_VAL_8_TR "%80"
#define OPTION_VAL_9_TR "%90"
#define OPTION_VAL_10_TR "%100"
#define FCEUMM_APU_1_LABEL_TR "Ses Kanalı 1 (Kare 1)"
#define FCEUMM_APU_1_LABEL_CAT_TR "Kanal 1 (Kare 1)"
#define FCEUMM_APU_1_INFO_0_TR "Darbe dalgası üreteci ses çıkışı 1 için etkinleştirir veya devre dışı bırakır."
#define FCEUMM_APU_2_LABEL_TR "Ses Kanalı 2 (Kare 2)"
#define FCEUMM_APU_2_LABEL_CAT_TR "Kanal 2 (Kare 2)"
#define FCEUMM_APU_2_INFO_0_TR "Darbe dalgası üreteci ses çıkışı 2 için etkinleştirir veya devre dışı bırakır."
#define FCEUMM_APU_3_LABEL_TR "Ses Kanalı 3 (Üçgen)"
#define FCEUMM_APU_3_LABEL_CAT_TR "Kanal 3 (Üçgen)"
#define FCEUMM_APU_3_INFO_0_TR "Üçgen dalga üreteci ses çıkışını etkinleştirir veya devre dışı bırakır."
#define FCEUMM_APU_4_LABEL_TR "Ses Kanalı 4 (Gürültü)"
#define FCEUMM_APU_4_LABEL_CAT_TR "Ses Kanalı 4 (Gürültü)"
#define FCEUMM_APU_4_INFO_0_TR "Gürültü oluşturucu ses çıkışını etkinleştirir veya devre dışı bırakır."
#define FCEUMM_APU_5_LABEL_TR "Ses Kanalı 5 (PCM)"
#define FCEUMM_APU_5_LABEL_CAT_TR "Kanal 5 (PCM)"
#define FCEUMM_APU_5_INFO_0_TR "Delta modülasyon kanalı ses çıkışını etkinleştirir veya devre dışı bırakır."
#define FCEUMM_TURBO_ENABLE_LABEL_TR "Turbo Etkinleştir"
#define FCEUMM_TURBO_ENABLE_INFO_0_TR "Turbo düğmelerini etkinleştirir veya devre dışı bırakır."
#define OPTION_VAL_NONE_TR "Hiçbiri"
#define OPTION_VAL_PLAYER_1_TR "1. Oyuncu"
#define OPTION_VAL_PLAYER_2_TR "2. Oyuncu"
#define OPTION_VAL_BOTH_TR "İkiside"
#define FCEUMM_TURBO_DELAY_LABEL_TR "Turbo Gecikmesi (kare içinde)"
#define FCEUMM_TURBO_DELAY_INFO_0_TR "Karelerdeki turbo düğmelerin tekrar hızı."
#define FCEUMM_ZAPPER_MODE_LABEL_TR "Zapper Kipi"
#define FCEUMM_ZAPPER_MODE_INFO_0_TR NULL
#define OPTION_VAL_CLIGHTGUN_TR NULL
#define OPTION_VAL_STLIGHTGUN_TR NULL
#define OPTION_VAL_TOUCHSCREEN_TR "Dokunmatik Ekran"
#define OPTION_VAL_MOUSE_TR "Fare"
#define FCEUMM_SHOW_CROSSHAIR_LABEL_TR "Zapper Nişangahı Göster"
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_TR NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_TR "Zapper Toleransı"
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_TR "Hedef alandan kaç pikselin hedefte olduğunu ayarlar."
#define FCEUMM_ZAPPER_TRIGGER_LABEL_TR NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_TR NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_TR NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_TR NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_TR "Karşı Yönlere İzin Ver"
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_TR "Bazı oyunlarda farklı efektler oluşturabilen aynı anda YUKARI+AŞAĞI veya SOL+SAĞ düğme kombinasyonlarına izin verir."
#define FCEUMM_NOSPRITELIMIT_LABEL_TR "Hareket Sınırını Kaldır"
#define FCEUMM_NOSPRITELIMIT_INFO_0_TR "Tarama satırı başına 8 donanım sınırını kaldırır. Bu, hareketli grafiğin titremesini azaltır, ancak bazıları bunu efektler için kullandığından bazı oyunların bozulmasına neden olabilir."
#define FCEUMM_OVERCLOCKING_LABEL_TR "Hız Aşırtma"
#define FCEUMM_OVERCLOCKING_INFO_0_TR "Bazı oyunlarda yavaşlamayı azaltabilen hız aşırtmayı etkinleştirir veya devre dışı bırakır. Postrender yöntemi her oyunla daha uyumludur, Vblank Contra Force gibi oyunlarda daha etkilidir."
#define OPTION_VAL_2X_POSTRENDER_TR NULL
#define OPTION_VAL_2X_VBLANK_TR NULL
#define FCEUMM_RAMSTATE_LABEL_TR "RAM Açılışta Doldur (Yeniden Başlatılmalı)"
#define FCEUMM_RAMSTATE_INFO_0_TR "Açılışta RAM değerleri. Bazı oyunlar, örnek olarak rastgele sayı üretimi için ilk RAM değerlerine güvenir."
#define OPTION_VAL_FILL_FF_TR NULL
#define OPTION_VAL_FILL_00_TR NULL
#define OPTION_VAL_RANDOM_TR "Rastgele"

struct retro_core_option_v2_category option_cats_tr[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_TR,
      CATEGORY_SYSTEM_INFO_0_TR
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_TR,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_TR
#else
      CATEGORY_VIDEO_INFO_1_TR
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_TR,
      CATEGORY_AUDIO_INFO_0_TR
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_TR,
      CATEGORY_INPUT_INFO_0_TR
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_TR,
      CATEGORY_HACKS_INFO_0_TR
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_TR,
      CATEGORY_DIP_SWITCH_INFO_0_TR
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_tr[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_TR,
      NULL,
      FCEUMM_REGION_INFO_0_TR,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_TR },
         { "NTSC",  OPTION_VAL_NTSC_TR },
         { "PAL",   OPTION_VAL_PAL_TR },
         { "Dendy", OPTION_VAL_DENDY_TR },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_TR,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_TR,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_TR,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_TR,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_TR,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_TR,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_TR,
      NULL,
      FCEUMM_ASPECT_INFO_0_TR,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_TR },
         { "4:3",     OPTION_VAL_4_3_TR },
         { "PP",      OPTION_VAL_PP_TR },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_TR,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_TR,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_TR,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_TR,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_TR,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_TR,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_TR,
      NULL,
      FCEUMM_PALETTE_INFO_0_TR,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_TR },
         { "asqrealc",             OPTION_VAL_ASQREALC_TR },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_TR },
         { "rgb",                  OPTION_VAL_RGB_TR },
         { "yuv-v3",               OPTION_VAL_YUV_V3_TR },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_TR },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_TR },
         { "pal",                  OPTION_VAL_PAL_TR },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_TR },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_TR },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_TR },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_TR },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_TR },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_TR },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_TR },
         { "nescap",               OPTION_VAL_NESCAP_TR },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_TR },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_TR },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_TR },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_TR },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_TR },
         { "raw",                  OPTION_VAL_RAW_TR },
         { "custom",               OPTION_VAL_CUSTOM_TR },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_TR,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_TR,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_TR },
         { "svideo",     OPTION_VAL_SVIDEO_TR },
         { "rgb",        OPTION_VAL_RGB_O9_TR },
         { "monochrome", OPTION_VAL_MONOCHROME_TR },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_TR,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_TR,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_TR },
         { "High",      OPTION_VAL_HIGH_TR },
         { "Very High", OPTION_VAL_VERY_HIGH_TR },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_TR,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_TR,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_TR,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_TR,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_TR },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_TR },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_TR },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_TR },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_TR },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_TR },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_TR },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_TR },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_TR },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_TR },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_TR },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_TR },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_TR },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_TR },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_TR },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_TR },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_TR },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_TR },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_TR },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_TR },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_TR },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_TR },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_TR },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_TR },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_TR },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_TR },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_TR },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_TR },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_TR },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_TR },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_TR },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_TR },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_TR,
      FCEUMM_SWAPDUTY_LABEL_CAT_TR,
      FCEUMM_SWAPDUTY_INFO_0_TR,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_TR,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_TR,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_TR },
         { "1",  OPTION_VAL_1_TR },
         { "2",  OPTION_VAL_2_TR },
         { "3",  OPTION_VAL_3_TR },
         { "4",  OPTION_VAL_4_TR },
         { "5",  OPTION_VAL_5_TR },
         { "6",  OPTION_VAL_6_TR },
         { "7",  OPTION_VAL_7_TR },
         { "8",  OPTION_VAL_8_TR },
         { "9",  OPTION_VAL_9_TR },
         { "10", OPTION_VAL_10_TR },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_TR,
      FCEUMM_APU_1_LABEL_CAT_TR,
      FCEUMM_APU_1_INFO_0_TR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_TR,
      FCEUMM_APU_2_LABEL_CAT_TR,
      FCEUMM_APU_2_INFO_0_TR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_TR,
      FCEUMM_APU_3_LABEL_CAT_TR,
      FCEUMM_APU_3_INFO_0_TR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_TR,
      FCEUMM_APU_4_LABEL_CAT_TR,
      FCEUMM_APU_4_INFO_0_TR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_TR,
      FCEUMM_APU_5_LABEL_CAT_TR,
      FCEUMM_APU_5_INFO_0_TR,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_TR,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_TR,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_TR },
         { "Player 1", OPTION_VAL_PLAYER_1_TR },
         { "Player 2", OPTION_VAL_PLAYER_2_TR },
         { "Both",     OPTION_VAL_BOTH_TR },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_TR,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_TR,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_TR,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_TR,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_TR },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_TR },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_TR },
         { "mouse",       OPTION_VAL_MOUSE_TR },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_TR,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_TR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_TR,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_TR,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_TR,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_TR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_TR,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_TR,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_TR,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_TR,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_TR,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_TR,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_TR,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_TR,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_TR },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_TR },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_TR,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_TR,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_TR },
         { "fill $00", OPTION_VAL_FILL_00_TR },
         { "random",   OPTION_VAL_RANDOM_TR },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_tr = {
   option_cats_tr,
   option_defs_tr
};

/* RETRO_LANGUAGE_UK */

#define CATEGORY_SYSTEM_LABEL_UK "Система"
#define CATEGORY_SYSTEM_INFO_0_UK NULL
#define CATEGORY_VIDEO_LABEL_UK "Відео"
#define CATEGORY_VIDEO_INFO_0_UK NULL
#define CATEGORY_VIDEO_INFO_1_UK NULL
#define CATEGORY_AUDIO_LABEL_UK "Аудіо"
#define CATEGORY_AUDIO_INFO_0_UK NULL
#define CATEGORY_INPUT_LABEL_UK "Ввід"
#define CATEGORY_INPUT_INFO_0_UK NULL
#define CATEGORY_HACKS_LABEL_UK "Гаки емуляції"
#define CATEGORY_HACKS_INFO_0_UK "Налаштування розгону процесора та точності емуляції, що впливають на низькорівневу продуктивність та сумісність."
#define CATEGORY_DIP_SWITCH_LABEL_UK NULL
#define CATEGORY_DIP_SWITCH_INFO_0_UK NULL
#define FCEUMM_REGION_LABEL_UK "Регіон"
#define FCEUMM_REGION_INFO_0_UK NULL
#define OPTION_VAL_AUTO_UK "Авто"
#define OPTION_VAL_NTSC_UK NULL
#define OPTION_VAL_PAL_UK NULL
#define OPTION_VAL_DENDY_UK NULL
#define FCEUMM_GAME_GENIE_LABEL_UK NULL
#define FCEUMM_GAME_GENIE_INFO_0_UK NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_UK NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_UK NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_UK NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_UK NULL
#define FCEUMM_ASPECT_LABEL_UK "Співвідношення сторін"
#define FCEUMM_ASPECT_INFO_0_UK NULL
#define OPTION_VAL_8_7_PAR_UK NULL
#define OPTION_VAL_4_3_UK NULL
#define OPTION_VAL_PP_UK NULL
#define FCEUMM_OVERSCAN_LABEL_UK NULL
#define FCEUMM_OVERSCAN_INFO_0_UK NULL
#define FCEUMM_OVERSCAN_H_LABEL_UK NULL
#define FCEUMM_OVERSCAN_H_INFO_0_UK NULL
#define FCEUMM_OVERSCAN_V_LABEL_UK NULL
#define FCEUMM_OVERSCAN_V_INFO_0_UK NULL
#define FCEUMM_PALETTE_LABEL_UK "Кольорова палітра"
#define FCEUMM_PALETTE_INFO_0_UK NULL
#define OPTION_VAL_DEFAULT_UK "За замовчуванням"
#define OPTION_VAL_ASQREALC_UK NULL
#define OPTION_VAL_NINTENDO_VC_UK NULL
#define OPTION_VAL_RGB_UK NULL
#define OPTION_VAL_YUV_V3_UK NULL
#define OPTION_VAL_UNSATURATED_FINAL_UK NULL
#define OPTION_VAL_SONY_CXA2025AS_US_UK NULL
#define OPTION_VAL_BMF_FINAL2_UK NULL
#define OPTION_VAL_BMF_FINAL3_UK NULL
#define OPTION_VAL_SMOOTH_FBX_UK NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_UK NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_UK NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_UK NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_UK NULL
#define OPTION_VAL_NESCAP_UK NULL
#define OPTION_VAL_WAVEBEAM_UK NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_UK NULL
#define OPTION_VAL_MAGNUM_FBX_UK NULL
#define OPTION_VAL_SMOOTH_V2_FBX_UK NULL
#define OPTION_VAL_NES_CLASSIC_FBX_UK NULL
#define OPTION_VAL_RAW_UK NULL
#define OPTION_VAL_CUSTOM_UK "Користувацька"
#define FCEUMM_NTSC_FILTER_LABEL_UK NULL
#define FCEUMM_NTSC_FILTER_INFO_0_UK NULL
#define OPTION_VAL_COMPOSITE_UK "Композитний"
#define OPTION_VAL_SVIDEO_UK NULL
#define OPTION_VAL_RGB_O9_UK NULL
#define OPTION_VAL_MONOCHROME_UK "Монотонні"
#define FCEUMM_SNDQUALITY_LABEL_UK NULL
#define FCEUMM_SNDQUALITY_INFO_0_UK NULL
#define OPTION_VAL_LOW_UK NULL
#define OPTION_VAL_HIGH_UK NULL
#define OPTION_VAL_VERY_HIGH_UK NULL
#define FCEUMM_SNDLOWPASS_LABEL_UK NULL
#define FCEUMM_SNDLOWPASS_INFO_0_UK NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_UK NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_UK NULL
#define OPTION_VAL_01_MS_DELAY_UK NULL
#define OPTION_VAL_02_MS_DELAY_UK NULL
#define OPTION_VAL_03_MS_DELAY_UK NULL
#define OPTION_VAL_04_MS_DELAY_UK NULL
#define OPTION_VAL_05_MS_DELAY_UK NULL
#define OPTION_VAL_06_MS_DELAY_UK NULL
#define OPTION_VAL_07_MS_DELAY_UK NULL
#define OPTION_VAL_08_MS_DELAY_UK NULL
#define OPTION_VAL_09_MS_DELAY_UK NULL
#define OPTION_VAL_10_MS_DELAY_UK NULL
#define OPTION_VAL_11_MS_DELAY_UK NULL
#define OPTION_VAL_12_MS_DELAY_UK NULL
#define OPTION_VAL_13_MS_DELAY_UK NULL
#define OPTION_VAL_14_MS_DELAY_UK NULL
#define OPTION_VAL_15_MS_DELAY_UK NULL
#define OPTION_VAL_16_MS_DELAY_UK NULL
#define OPTION_VAL_17_MS_DELAY_UK NULL
#define OPTION_VAL_18_MS_DELAY_UK NULL
#define OPTION_VAL_19_MS_DELAY_UK NULL
#define OPTION_VAL_20_MS_DELAY_UK NULL
#define OPTION_VAL_21_MS_DELAY_UK NULL
#define OPTION_VAL_22_MS_DELAY_UK NULL
#define OPTION_VAL_23_MS_DELAY_UK NULL
#define OPTION_VAL_24_MS_DELAY_UK NULL
#define OPTION_VAL_25_MS_DELAY_UK NULL
#define OPTION_VAL_26_MS_DELAY_UK NULL
#define OPTION_VAL_27_MS_DELAY_UK NULL
#define OPTION_VAL_28_MS_DELAY_UK NULL
#define OPTION_VAL_29_MS_DELAY_UK NULL
#define OPTION_VAL_30_MS_DELAY_UK NULL
#define OPTION_VAL_31_MS_DELAY_UK NULL
#define OPTION_VAL_32_MS_DELAY_UK NULL
#define FCEUMM_SWAPDUTY_LABEL_UK NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_UK NULL
#define FCEUMM_SWAPDUTY_INFO_0_UK NULL
#define FCEUMM_SNDVOLUME_LABEL_UK NULL
#define FCEUMM_SNDVOLUME_INFO_0_UK NULL
#define OPTION_VAL_0_UK NULL
#define OPTION_VAL_1_UK NULL
#define OPTION_VAL_2_UK NULL
#define OPTION_VAL_3_UK NULL
#define OPTION_VAL_4_UK NULL
#define OPTION_VAL_5_UK NULL
#define OPTION_VAL_6_UK NULL
#define OPTION_VAL_7_UK NULL
#define OPTION_VAL_8_UK NULL
#define OPTION_VAL_9_UK NULL
#define OPTION_VAL_10_UK NULL
#define FCEUMM_APU_1_LABEL_UK NULL
#define FCEUMM_APU_1_LABEL_CAT_UK NULL
#define FCEUMM_APU_1_INFO_0_UK NULL
#define FCEUMM_APU_2_LABEL_UK NULL
#define FCEUMM_APU_2_LABEL_CAT_UK NULL
#define FCEUMM_APU_2_INFO_0_UK NULL
#define FCEUMM_APU_3_LABEL_UK NULL
#define FCEUMM_APU_3_LABEL_CAT_UK NULL
#define FCEUMM_APU_3_INFO_0_UK NULL
#define FCEUMM_APU_4_LABEL_UK NULL
#define FCEUMM_APU_4_LABEL_CAT_UK NULL
#define FCEUMM_APU_4_INFO_0_UK NULL
#define FCEUMM_APU_5_LABEL_UK NULL
#define FCEUMM_APU_5_LABEL_CAT_UK NULL
#define FCEUMM_APU_5_INFO_0_UK NULL
#define FCEUMM_TURBO_ENABLE_LABEL_UK NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_UK NULL
#define OPTION_VAL_NONE_UK NULL
#define OPTION_VAL_PLAYER_1_UK NULL
#define OPTION_VAL_PLAYER_2_UK NULL
#define OPTION_VAL_BOTH_UK NULL
#define FCEUMM_TURBO_DELAY_LABEL_UK NULL
#define FCEUMM_TURBO_DELAY_INFO_0_UK NULL
#define FCEUMM_ZAPPER_MODE_LABEL_UK NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_UK NULL
#define OPTION_VAL_CLIGHTGUN_UK NULL
#define OPTION_VAL_STLIGHTGUN_UK NULL
#define OPTION_VAL_TOUCHSCREEN_UK NULL
#define OPTION_VAL_MOUSE_UK "Миша"
#define FCEUMM_SHOW_CROSSHAIR_LABEL_UK NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_UK NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_UK NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_UK NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_UK NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_UK NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_UK NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_UK NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_UK "Дозволити протилежні напрямки"
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_UK NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_UK NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_UK NULL
#define FCEUMM_OVERCLOCKING_LABEL_UK NULL
#define FCEUMM_OVERCLOCKING_INFO_0_UK NULL
#define OPTION_VAL_2X_POSTRENDER_UK NULL
#define OPTION_VAL_2X_VBLANK_UK NULL
#define FCEUMM_RAMSTATE_LABEL_UK NULL
#define FCEUMM_RAMSTATE_INFO_0_UK NULL
#define OPTION_VAL_FILL_FF_UK NULL
#define OPTION_VAL_FILL_00_UK NULL
#define OPTION_VAL_RANDOM_UK "Випадково"

struct retro_core_option_v2_category option_cats_uk[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_UK,
      CATEGORY_SYSTEM_INFO_0_UK
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_UK,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_UK
#else
      CATEGORY_VIDEO_INFO_1_UK
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_UK,
      CATEGORY_AUDIO_INFO_0_UK
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_UK,
      CATEGORY_INPUT_INFO_0_UK
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_UK,
      CATEGORY_HACKS_INFO_0_UK
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_UK,
      CATEGORY_DIP_SWITCH_INFO_0_UK
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_uk[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_UK,
      NULL,
      FCEUMM_REGION_INFO_0_UK,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_UK },
         { "NTSC",  OPTION_VAL_NTSC_UK },
         { "PAL",   OPTION_VAL_PAL_UK },
         { "Dendy", OPTION_VAL_DENDY_UK },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_UK,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_UK,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_UK,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_UK,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_UK,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_UK,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_UK,
      NULL,
      FCEUMM_ASPECT_INFO_0_UK,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_UK },
         { "4:3",     OPTION_VAL_4_3_UK },
         { "PP",      OPTION_VAL_PP_UK },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_UK,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_UK,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_UK,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_UK,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_UK,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_UK,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_UK,
      NULL,
      FCEUMM_PALETTE_INFO_0_UK,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_UK },
         { "asqrealc",             OPTION_VAL_ASQREALC_UK },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_UK },
         { "rgb",                  OPTION_VAL_RGB_UK },
         { "yuv-v3",               OPTION_VAL_YUV_V3_UK },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_UK },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_UK },
         { "pal",                  OPTION_VAL_PAL_UK },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_UK },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_UK },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_UK },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_UK },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_UK },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_UK },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_UK },
         { "nescap",               OPTION_VAL_NESCAP_UK },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_UK },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_UK },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_UK },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_UK },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_UK },
         { "raw",                  OPTION_VAL_RAW_UK },
         { "custom",               OPTION_VAL_CUSTOM_UK },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_UK,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_UK,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_UK },
         { "svideo",     OPTION_VAL_SVIDEO_UK },
         { "rgb",        OPTION_VAL_RGB_O9_UK },
         { "monochrome", OPTION_VAL_MONOCHROME_UK },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_UK,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_UK,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_UK },
         { "High",      OPTION_VAL_HIGH_UK },
         { "Very High", OPTION_VAL_VERY_HIGH_UK },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_UK,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_UK,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_UK,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_UK,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_UK },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_UK },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_UK },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_UK },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_UK },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_UK },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_UK },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_UK },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_UK },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_UK },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_UK },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_UK },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_UK },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_UK },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_UK },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_UK },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_UK },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_UK },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_UK },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_UK },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_UK },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_UK },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_UK },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_UK },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_UK },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_UK },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_UK },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_UK },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_UK },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_UK },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_UK },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_UK },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_UK,
      FCEUMM_SWAPDUTY_LABEL_CAT_UK,
      FCEUMM_SWAPDUTY_INFO_0_UK,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_UK,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_UK,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_UK },
         { "1",  OPTION_VAL_1_UK },
         { "2",  OPTION_VAL_2_UK },
         { "3",  OPTION_VAL_3_UK },
         { "4",  OPTION_VAL_4_UK },
         { "5",  OPTION_VAL_5_UK },
         { "6",  OPTION_VAL_6_UK },
         { "7",  OPTION_VAL_7_UK },
         { "8",  OPTION_VAL_8_UK },
         { "9",  OPTION_VAL_9_UK },
         { "10", OPTION_VAL_10_UK },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_UK,
      FCEUMM_APU_1_LABEL_CAT_UK,
      FCEUMM_APU_1_INFO_0_UK,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_UK,
      FCEUMM_APU_2_LABEL_CAT_UK,
      FCEUMM_APU_2_INFO_0_UK,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_UK,
      FCEUMM_APU_3_LABEL_CAT_UK,
      FCEUMM_APU_3_INFO_0_UK,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_UK,
      FCEUMM_APU_4_LABEL_CAT_UK,
      FCEUMM_APU_4_INFO_0_UK,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_UK,
      FCEUMM_APU_5_LABEL_CAT_UK,
      FCEUMM_APU_5_INFO_0_UK,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_UK,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_UK,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_UK },
         { "Player 1", OPTION_VAL_PLAYER_1_UK },
         { "Player 2", OPTION_VAL_PLAYER_2_UK },
         { "Both",     OPTION_VAL_BOTH_UK },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_UK,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_UK,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_UK,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_UK,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_UK },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_UK },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_UK },
         { "mouse",       OPTION_VAL_MOUSE_UK },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_UK,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_UK,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_UK,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_UK,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_UK,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_UK,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_UK,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_UK,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_UK,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_UK,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_UK,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_UK,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_UK,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_UK,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_UK },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_UK },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_UK,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_UK,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_UK },
         { "fill $00", OPTION_VAL_FILL_00_UK },
         { "random",   OPTION_VAL_RANDOM_UK },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_uk = {
   option_cats_uk,
   option_defs_uk
};

/* RETRO_LANGUAGE_VAL */

#define CATEGORY_SYSTEM_LABEL_VAL "Sistema"
#define CATEGORY_SYSTEM_INFO_0_VAL NULL
#define CATEGORY_VIDEO_LABEL_VAL "Vídeo"
#define CATEGORY_VIDEO_INFO_0_VAL NULL
#define CATEGORY_VIDEO_INFO_1_VAL NULL
#define CATEGORY_AUDIO_LABEL_VAL "Àudio"
#define CATEGORY_AUDIO_INFO_0_VAL NULL
#define CATEGORY_INPUT_LABEL_VAL "Entrada"
#define CATEGORY_INPUT_INFO_0_VAL NULL
#define CATEGORY_HACKS_LABEL_VAL NULL
#define CATEGORY_HACKS_INFO_0_VAL NULL
#define CATEGORY_DIP_SWITCH_LABEL_VAL NULL
#define CATEGORY_DIP_SWITCH_INFO_0_VAL NULL
#define FCEUMM_REGION_LABEL_VAL "Regió"
#define FCEUMM_REGION_INFO_0_VAL NULL
#define OPTION_VAL_AUTO_VAL "Selecció automàtica"
#define OPTION_VAL_NTSC_VAL NULL
#define OPTION_VAL_PAL_VAL NULL
#define OPTION_VAL_DENDY_VAL NULL
#define FCEUMM_GAME_GENIE_LABEL_VAL NULL
#define FCEUMM_GAME_GENIE_INFO_0_VAL NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_VAL NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_VAL NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_VAL NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_VAL NULL
#define FCEUMM_ASPECT_LABEL_VAL "Relació d'aspecte"
#define FCEUMM_ASPECT_INFO_0_VAL NULL
#define OPTION_VAL_8_7_PAR_VAL NULL
#define OPTION_VAL_4_3_VAL NULL
#define OPTION_VAL_PP_VAL NULL
#define FCEUMM_OVERSCAN_LABEL_VAL NULL
#define FCEUMM_OVERSCAN_INFO_0_VAL NULL
#define FCEUMM_OVERSCAN_H_LABEL_VAL NULL
#define FCEUMM_OVERSCAN_H_INFO_0_VAL NULL
#define FCEUMM_OVERSCAN_V_LABEL_VAL NULL
#define FCEUMM_OVERSCAN_V_INFO_0_VAL NULL
#define FCEUMM_PALETTE_LABEL_VAL NULL
#define FCEUMM_PALETTE_INFO_0_VAL NULL
#define OPTION_VAL_DEFAULT_VAL NULL
#define OPTION_VAL_ASQREALC_VAL NULL
#define OPTION_VAL_NINTENDO_VC_VAL NULL
#define OPTION_VAL_RGB_VAL NULL
#define OPTION_VAL_YUV_V3_VAL NULL
#define OPTION_VAL_UNSATURATED_FINAL_VAL NULL
#define OPTION_VAL_SONY_CXA2025AS_US_VAL NULL
#define OPTION_VAL_BMF_FINAL2_VAL NULL
#define OPTION_VAL_BMF_FINAL3_VAL NULL
#define OPTION_VAL_SMOOTH_FBX_VAL NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_VAL NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_VAL NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_VAL NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_VAL NULL
#define OPTION_VAL_NESCAP_VAL NULL
#define OPTION_VAL_WAVEBEAM_VAL NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_VAL NULL
#define OPTION_VAL_MAGNUM_FBX_VAL NULL
#define OPTION_VAL_SMOOTH_V2_FBX_VAL NULL
#define OPTION_VAL_NES_CLASSIC_FBX_VAL NULL
#define OPTION_VAL_RAW_VAL NULL
#define OPTION_VAL_CUSTOM_VAL NULL
#define FCEUMM_NTSC_FILTER_LABEL_VAL NULL
#define FCEUMM_NTSC_FILTER_INFO_0_VAL NULL
#define OPTION_VAL_COMPOSITE_VAL NULL
#define OPTION_VAL_SVIDEO_VAL NULL
#define OPTION_VAL_RGB_O9_VAL NULL
#define OPTION_VAL_MONOCHROME_VAL NULL
#define FCEUMM_SNDQUALITY_LABEL_VAL NULL
#define FCEUMM_SNDQUALITY_INFO_0_VAL NULL
#define OPTION_VAL_LOW_VAL NULL
#define OPTION_VAL_HIGH_VAL NULL
#define OPTION_VAL_VERY_HIGH_VAL NULL
#define FCEUMM_SNDLOWPASS_LABEL_VAL NULL
#define FCEUMM_SNDLOWPASS_INFO_0_VAL NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_VAL NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_VAL NULL
#define OPTION_VAL_01_MS_DELAY_VAL NULL
#define OPTION_VAL_02_MS_DELAY_VAL NULL
#define OPTION_VAL_03_MS_DELAY_VAL NULL
#define OPTION_VAL_04_MS_DELAY_VAL NULL
#define OPTION_VAL_05_MS_DELAY_VAL NULL
#define OPTION_VAL_06_MS_DELAY_VAL NULL
#define OPTION_VAL_07_MS_DELAY_VAL NULL
#define OPTION_VAL_08_MS_DELAY_VAL NULL
#define OPTION_VAL_09_MS_DELAY_VAL NULL
#define OPTION_VAL_10_MS_DELAY_VAL NULL
#define OPTION_VAL_11_MS_DELAY_VAL NULL
#define OPTION_VAL_12_MS_DELAY_VAL NULL
#define OPTION_VAL_13_MS_DELAY_VAL NULL
#define OPTION_VAL_14_MS_DELAY_VAL NULL
#define OPTION_VAL_15_MS_DELAY_VAL NULL
#define OPTION_VAL_16_MS_DELAY_VAL NULL
#define OPTION_VAL_17_MS_DELAY_VAL NULL
#define OPTION_VAL_18_MS_DELAY_VAL NULL
#define OPTION_VAL_19_MS_DELAY_VAL NULL
#define OPTION_VAL_20_MS_DELAY_VAL NULL
#define OPTION_VAL_21_MS_DELAY_VAL NULL
#define OPTION_VAL_22_MS_DELAY_VAL NULL
#define OPTION_VAL_23_MS_DELAY_VAL NULL
#define OPTION_VAL_24_MS_DELAY_VAL NULL
#define OPTION_VAL_25_MS_DELAY_VAL NULL
#define OPTION_VAL_26_MS_DELAY_VAL NULL
#define OPTION_VAL_27_MS_DELAY_VAL NULL
#define OPTION_VAL_28_MS_DELAY_VAL NULL
#define OPTION_VAL_29_MS_DELAY_VAL NULL
#define OPTION_VAL_30_MS_DELAY_VAL NULL
#define OPTION_VAL_31_MS_DELAY_VAL NULL
#define OPTION_VAL_32_MS_DELAY_VAL NULL
#define FCEUMM_SWAPDUTY_LABEL_VAL NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_VAL NULL
#define FCEUMM_SWAPDUTY_INFO_0_VAL NULL
#define FCEUMM_SNDVOLUME_LABEL_VAL NULL
#define FCEUMM_SNDVOLUME_INFO_0_VAL NULL
#define OPTION_VAL_0_VAL NULL
#define OPTION_VAL_1_VAL NULL
#define OPTION_VAL_2_VAL NULL
#define OPTION_VAL_3_VAL NULL
#define OPTION_VAL_4_VAL NULL
#define OPTION_VAL_5_VAL NULL
#define OPTION_VAL_6_VAL NULL
#define OPTION_VAL_7_VAL NULL
#define OPTION_VAL_8_VAL NULL
#define OPTION_VAL_9_VAL NULL
#define OPTION_VAL_10_VAL NULL
#define FCEUMM_APU_1_LABEL_VAL NULL
#define FCEUMM_APU_1_LABEL_CAT_VAL NULL
#define FCEUMM_APU_1_INFO_0_VAL NULL
#define FCEUMM_APU_2_LABEL_VAL NULL
#define FCEUMM_APU_2_LABEL_CAT_VAL NULL
#define FCEUMM_APU_2_INFO_0_VAL NULL
#define FCEUMM_APU_3_LABEL_VAL NULL
#define FCEUMM_APU_3_LABEL_CAT_VAL NULL
#define FCEUMM_APU_3_INFO_0_VAL NULL
#define FCEUMM_APU_4_LABEL_VAL NULL
#define FCEUMM_APU_4_LABEL_CAT_VAL NULL
#define FCEUMM_APU_4_INFO_0_VAL NULL
#define FCEUMM_APU_5_LABEL_VAL NULL
#define FCEUMM_APU_5_LABEL_CAT_VAL NULL
#define FCEUMM_APU_5_INFO_0_VAL NULL
#define FCEUMM_TURBO_ENABLE_LABEL_VAL NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_VAL NULL
#define OPTION_VAL_NONE_VAL "Cap"
#define OPTION_VAL_PLAYER_1_VAL NULL
#define OPTION_VAL_PLAYER_2_VAL NULL
#define OPTION_VAL_BOTH_VAL NULL
#define FCEUMM_TURBO_DELAY_LABEL_VAL NULL
#define FCEUMM_TURBO_DELAY_INFO_0_VAL NULL
#define FCEUMM_ZAPPER_MODE_LABEL_VAL NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_VAL NULL
#define OPTION_VAL_CLIGHTGUN_VAL NULL
#define OPTION_VAL_STLIGHTGUN_VAL NULL
#define OPTION_VAL_TOUCHSCREEN_VAL NULL
#define OPTION_VAL_MOUSE_VAL NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_VAL NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_VAL NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_VAL NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_VAL NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_VAL NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_VAL NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_VAL NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_VAL NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_VAL NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_VAL NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_VAL NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_VAL NULL
#define FCEUMM_OVERCLOCKING_LABEL_VAL NULL
#define FCEUMM_OVERCLOCKING_INFO_0_VAL NULL
#define OPTION_VAL_2X_POSTRENDER_VAL NULL
#define OPTION_VAL_2X_VBLANK_VAL NULL
#define FCEUMM_RAMSTATE_LABEL_VAL NULL
#define FCEUMM_RAMSTATE_INFO_0_VAL NULL
#define OPTION_VAL_FILL_FF_VAL NULL
#define OPTION_VAL_FILL_00_VAL NULL
#define OPTION_VAL_RANDOM_VAL NULL

struct retro_core_option_v2_category option_cats_val[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_VAL,
      CATEGORY_SYSTEM_INFO_0_VAL
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_VAL,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_VAL
#else
      CATEGORY_VIDEO_INFO_1_VAL
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_VAL,
      CATEGORY_AUDIO_INFO_0_VAL
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_VAL,
      CATEGORY_INPUT_INFO_0_VAL
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_VAL,
      CATEGORY_HACKS_INFO_0_VAL
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_VAL,
      CATEGORY_DIP_SWITCH_INFO_0_VAL
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_val[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_VAL,
      NULL,
      FCEUMM_REGION_INFO_0_VAL,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_VAL },
         { "NTSC",  OPTION_VAL_NTSC_VAL },
         { "PAL",   OPTION_VAL_PAL_VAL },
         { "Dendy", OPTION_VAL_DENDY_VAL },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_VAL,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_VAL,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_VAL,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_VAL,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_VAL,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_VAL,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_VAL,
      NULL,
      FCEUMM_ASPECT_INFO_0_VAL,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_VAL },
         { "4:3",     OPTION_VAL_4_3_VAL },
         { "PP",      OPTION_VAL_PP_VAL },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_VAL,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_VAL,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_VAL,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_VAL,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_VAL,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_VAL,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_VAL,
      NULL,
      FCEUMM_PALETTE_INFO_0_VAL,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_VAL },
         { "asqrealc",             OPTION_VAL_ASQREALC_VAL },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_VAL },
         { "rgb",                  OPTION_VAL_RGB_VAL },
         { "yuv-v3",               OPTION_VAL_YUV_V3_VAL },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_VAL },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_VAL },
         { "pal",                  OPTION_VAL_PAL_VAL },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_VAL },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_VAL },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_VAL },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_VAL },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_VAL },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_VAL },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_VAL },
         { "nescap",               OPTION_VAL_NESCAP_VAL },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_VAL },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_VAL },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_VAL },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_VAL },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_VAL },
         { "raw",                  OPTION_VAL_RAW_VAL },
         { "custom",               OPTION_VAL_CUSTOM_VAL },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_VAL,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_VAL,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_VAL },
         { "svideo",     OPTION_VAL_SVIDEO_VAL },
         { "rgb",        OPTION_VAL_RGB_O9_VAL },
         { "monochrome", OPTION_VAL_MONOCHROME_VAL },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_VAL,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_VAL,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_VAL },
         { "High",      OPTION_VAL_HIGH_VAL },
         { "Very High", OPTION_VAL_VERY_HIGH_VAL },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_VAL,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_VAL,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_VAL,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_VAL,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_VAL },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_VAL },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_VAL },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_VAL },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_VAL },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_VAL },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_VAL },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_VAL },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_VAL },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_VAL },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_VAL },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_VAL },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_VAL },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_VAL },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_VAL },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_VAL },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_VAL },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_VAL },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_VAL },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_VAL },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_VAL },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_VAL },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_VAL },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_VAL },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_VAL },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_VAL },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_VAL },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_VAL },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_VAL },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_VAL },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_VAL },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_VAL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_VAL,
      FCEUMM_SWAPDUTY_LABEL_CAT_VAL,
      FCEUMM_SWAPDUTY_INFO_0_VAL,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_VAL,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_VAL,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_VAL },
         { "1",  OPTION_VAL_1_VAL },
         { "2",  OPTION_VAL_2_VAL },
         { "3",  OPTION_VAL_3_VAL },
         { "4",  OPTION_VAL_4_VAL },
         { "5",  OPTION_VAL_5_VAL },
         { "6",  OPTION_VAL_6_VAL },
         { "7",  OPTION_VAL_7_VAL },
         { "8",  OPTION_VAL_8_VAL },
         { "9",  OPTION_VAL_9_VAL },
         { "10", OPTION_VAL_10_VAL },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_VAL,
      FCEUMM_APU_1_LABEL_CAT_VAL,
      FCEUMM_APU_1_INFO_0_VAL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_VAL,
      FCEUMM_APU_2_LABEL_CAT_VAL,
      FCEUMM_APU_2_INFO_0_VAL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_VAL,
      FCEUMM_APU_3_LABEL_CAT_VAL,
      FCEUMM_APU_3_INFO_0_VAL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_VAL,
      FCEUMM_APU_4_LABEL_CAT_VAL,
      FCEUMM_APU_4_INFO_0_VAL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_VAL,
      FCEUMM_APU_5_LABEL_CAT_VAL,
      FCEUMM_APU_5_INFO_0_VAL,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_VAL,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_VAL,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_VAL },
         { "Player 1", OPTION_VAL_PLAYER_1_VAL },
         { "Player 2", OPTION_VAL_PLAYER_2_VAL },
         { "Both",     OPTION_VAL_BOTH_VAL },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_VAL,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_VAL,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_VAL,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_VAL,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_VAL },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_VAL },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_VAL },
         { "mouse",       OPTION_VAL_MOUSE_VAL },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_VAL,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_VAL,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_VAL,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_VAL,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_VAL,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_VAL,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_VAL,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_VAL,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_VAL,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_VAL,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_VAL,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_VAL,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_VAL,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_VAL,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_VAL },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_VAL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_VAL,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_VAL,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_VAL },
         { "fill $00", OPTION_VAL_FILL_00_VAL },
         { "random",   OPTION_VAL_RANDOM_VAL },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_val = {
   option_cats_val,
   option_defs_val
};

/* RETRO_LANGUAGE_VN */

#define CATEGORY_SYSTEM_LABEL_VN "Hệ thống"
#define CATEGORY_SYSTEM_INFO_0_VN NULL
#define CATEGORY_VIDEO_LABEL_VN "Hình ảnh"
#define CATEGORY_VIDEO_INFO_0_VN NULL
#define CATEGORY_VIDEO_INFO_1_VN NULL
#define CATEGORY_AUDIO_LABEL_VN "Âm thanh"
#define CATEGORY_AUDIO_INFO_0_VN NULL
#define CATEGORY_INPUT_LABEL_VN "Input Driver"
#define CATEGORY_INPUT_INFO_0_VN NULL
#define CATEGORY_HACKS_LABEL_VN "Hacks giả lập"
#define CATEGORY_HACKS_INFO_0_VN NULL
#define CATEGORY_DIP_SWITCH_LABEL_VN NULL
#define CATEGORY_DIP_SWITCH_INFO_0_VN NULL
#define FCEUMM_REGION_LABEL_VN "Khu vực"
#define FCEUMM_REGION_INFO_0_VN NULL
#define OPTION_VAL_AUTO_VN "Tự động"
#define OPTION_VAL_NTSC_VN NULL
#define OPTION_VAL_PAL_VN NULL
#define OPTION_VAL_DENDY_VN NULL
#define FCEUMM_GAME_GENIE_LABEL_VN NULL
#define FCEUMM_GAME_GENIE_INFO_0_VN NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_VN NULL
#define FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_VN NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_VN NULL
#define FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_VN NULL
#define FCEUMM_ASPECT_LABEL_VN NULL
#define FCEUMM_ASPECT_INFO_0_VN NULL
#define OPTION_VAL_8_7_PAR_VN NULL
#define OPTION_VAL_4_3_VN NULL
#define OPTION_VAL_PP_VN NULL
#define FCEUMM_OVERSCAN_LABEL_VN NULL
#define FCEUMM_OVERSCAN_INFO_0_VN NULL
#define FCEUMM_OVERSCAN_H_LABEL_VN NULL
#define FCEUMM_OVERSCAN_H_INFO_0_VN NULL
#define FCEUMM_OVERSCAN_V_LABEL_VN NULL
#define FCEUMM_OVERSCAN_V_INFO_0_VN NULL
#define FCEUMM_PALETTE_LABEL_VN "Bảng màu"
#define FCEUMM_PALETTE_INFO_0_VN NULL
#define OPTION_VAL_DEFAULT_VN NULL
#define OPTION_VAL_ASQREALC_VN NULL
#define OPTION_VAL_NINTENDO_VC_VN NULL
#define OPTION_VAL_RGB_VN NULL
#define OPTION_VAL_YUV_V3_VN NULL
#define OPTION_VAL_UNSATURATED_FINAL_VN NULL
#define OPTION_VAL_SONY_CXA2025AS_US_VN NULL
#define OPTION_VAL_BMF_FINAL2_VN NULL
#define OPTION_VAL_BMF_FINAL3_VN NULL
#define OPTION_VAL_SMOOTH_FBX_VN NULL
#define OPTION_VAL_COMPOSITE_DIRECT_FBX_VN NULL
#define OPTION_VAL_PVM_STYLE_D93_FBX_VN NULL
#define OPTION_VAL_NTSC_HARDWARE_FBX_VN NULL
#define OPTION_VAL_NES_CLASSIC_FBX_FS_VN NULL
#define OPTION_VAL_NESCAP_VN NULL
#define OPTION_VAL_WAVEBEAM_VN NULL
#define OPTION_VAL_DIGITAL_PRIME_FBX_VN NULL
#define OPTION_VAL_MAGNUM_FBX_VN NULL
#define OPTION_VAL_SMOOTH_V2_FBX_VN NULL
#define OPTION_VAL_NES_CLASSIC_FBX_VN NULL
#define OPTION_VAL_RAW_VN NULL
#define OPTION_VAL_CUSTOM_VN "Tùy chỉnh"
#define FCEUMM_NTSC_FILTER_LABEL_VN NULL
#define FCEUMM_NTSC_FILTER_INFO_0_VN NULL
#define OPTION_VAL_COMPOSITE_VN "Kết hợp"
#define OPTION_VAL_SVIDEO_VN NULL
#define OPTION_VAL_RGB_O9_VN NULL
#define OPTION_VAL_MONOCHROME_VN NULL
#define FCEUMM_SNDQUALITY_LABEL_VN NULL
#define FCEUMM_SNDQUALITY_INFO_0_VN NULL
#define OPTION_VAL_LOW_VN NULL
#define OPTION_VAL_HIGH_VN NULL
#define OPTION_VAL_VERY_HIGH_VN NULL
#define FCEUMM_SNDLOWPASS_LABEL_VN NULL
#define FCEUMM_SNDLOWPASS_INFO_0_VN NULL
#define FCEUMM_SNDSTEREODELAY_LABEL_VN NULL
#define FCEUMM_SNDSTEREODELAY_INFO_0_VN NULL
#define OPTION_VAL_01_MS_DELAY_VN NULL
#define OPTION_VAL_02_MS_DELAY_VN NULL
#define OPTION_VAL_03_MS_DELAY_VN NULL
#define OPTION_VAL_04_MS_DELAY_VN NULL
#define OPTION_VAL_05_MS_DELAY_VN NULL
#define OPTION_VAL_06_MS_DELAY_VN NULL
#define OPTION_VAL_07_MS_DELAY_VN NULL
#define OPTION_VAL_08_MS_DELAY_VN NULL
#define OPTION_VAL_09_MS_DELAY_VN NULL
#define OPTION_VAL_10_MS_DELAY_VN NULL
#define OPTION_VAL_11_MS_DELAY_VN NULL
#define OPTION_VAL_12_MS_DELAY_VN NULL
#define OPTION_VAL_13_MS_DELAY_VN NULL
#define OPTION_VAL_14_MS_DELAY_VN NULL
#define OPTION_VAL_15_MS_DELAY_VN NULL
#define OPTION_VAL_16_MS_DELAY_VN NULL
#define OPTION_VAL_17_MS_DELAY_VN NULL
#define OPTION_VAL_18_MS_DELAY_VN NULL
#define OPTION_VAL_19_MS_DELAY_VN NULL
#define OPTION_VAL_20_MS_DELAY_VN NULL
#define OPTION_VAL_21_MS_DELAY_VN NULL
#define OPTION_VAL_22_MS_DELAY_VN NULL
#define OPTION_VAL_23_MS_DELAY_VN NULL
#define OPTION_VAL_24_MS_DELAY_VN NULL
#define OPTION_VAL_25_MS_DELAY_VN NULL
#define OPTION_VAL_26_MS_DELAY_VN NULL
#define OPTION_VAL_27_MS_DELAY_VN NULL
#define OPTION_VAL_28_MS_DELAY_VN NULL
#define OPTION_VAL_29_MS_DELAY_VN NULL
#define OPTION_VAL_30_MS_DELAY_VN NULL
#define OPTION_VAL_31_MS_DELAY_VN NULL
#define OPTION_VAL_32_MS_DELAY_VN NULL
#define FCEUMM_SWAPDUTY_LABEL_VN NULL
#define FCEUMM_SWAPDUTY_LABEL_CAT_VN NULL
#define FCEUMM_SWAPDUTY_INFO_0_VN NULL
#define FCEUMM_SNDVOLUME_LABEL_VN NULL
#define FCEUMM_SNDVOLUME_INFO_0_VN NULL
#define OPTION_VAL_0_VN NULL
#define OPTION_VAL_1_VN NULL
#define OPTION_VAL_2_VN NULL
#define OPTION_VAL_3_VN NULL
#define OPTION_VAL_4_VN NULL
#define OPTION_VAL_5_VN NULL
#define OPTION_VAL_6_VN NULL
#define OPTION_VAL_7_VN NULL
#define OPTION_VAL_8_VN NULL
#define OPTION_VAL_9_VN NULL
#define OPTION_VAL_10_VN NULL
#define FCEUMM_APU_1_LABEL_VN NULL
#define FCEUMM_APU_1_LABEL_CAT_VN NULL
#define FCEUMM_APU_1_INFO_0_VN NULL
#define FCEUMM_APU_2_LABEL_VN NULL
#define FCEUMM_APU_2_LABEL_CAT_VN NULL
#define FCEUMM_APU_2_INFO_0_VN NULL
#define FCEUMM_APU_3_LABEL_VN NULL
#define FCEUMM_APU_3_LABEL_CAT_VN NULL
#define FCEUMM_APU_3_INFO_0_VN NULL
#define FCEUMM_APU_4_LABEL_VN NULL
#define FCEUMM_APU_4_LABEL_CAT_VN NULL
#define FCEUMM_APU_4_INFO_0_VN NULL
#define FCEUMM_APU_5_LABEL_VN NULL
#define FCEUMM_APU_5_LABEL_CAT_VN NULL
#define FCEUMM_APU_5_INFO_0_VN NULL
#define FCEUMM_TURBO_ENABLE_LABEL_VN NULL
#define FCEUMM_TURBO_ENABLE_INFO_0_VN NULL
#define OPTION_VAL_NONE_VN "Không"
#define OPTION_VAL_PLAYER_1_VN NULL
#define OPTION_VAL_PLAYER_2_VN NULL
#define OPTION_VAL_BOTH_VN NULL
#define FCEUMM_TURBO_DELAY_LABEL_VN NULL
#define FCEUMM_TURBO_DELAY_INFO_0_VN NULL
#define FCEUMM_ZAPPER_MODE_LABEL_VN NULL
#define FCEUMM_ZAPPER_MODE_INFO_0_VN NULL
#define OPTION_VAL_CLIGHTGUN_VN NULL
#define OPTION_VAL_STLIGHTGUN_VN NULL
#define OPTION_VAL_TOUCHSCREEN_VN NULL
#define OPTION_VAL_MOUSE_VN NULL
#define FCEUMM_SHOW_CROSSHAIR_LABEL_VN NULL
#define FCEUMM_SHOW_CROSSHAIR_INFO_0_VN NULL
#define FCEUMM_ZAPPER_TOLERANCE_LABEL_VN NULL
#define FCEUMM_ZAPPER_TOLERANCE_INFO_0_VN NULL
#define FCEUMM_ZAPPER_TRIGGER_LABEL_VN NULL
#define FCEUMM_ZAPPER_TRIGGER_INFO_0_VN NULL
#define FCEUMM_ZAPPER_SENSOR_LABEL_VN NULL
#define FCEUMM_ZAPPER_SENSOR_INFO_0_VN NULL
#define FCEUMM_UP_DOWN_ALLOWED_LABEL_VN NULL
#define FCEUMM_UP_DOWN_ALLOWED_INFO_0_VN NULL
#define FCEUMM_NOSPRITELIMIT_LABEL_VN NULL
#define FCEUMM_NOSPRITELIMIT_INFO_0_VN NULL
#define FCEUMM_OVERCLOCKING_LABEL_VN NULL
#define FCEUMM_OVERCLOCKING_INFO_0_VN NULL
#define OPTION_VAL_2X_POSTRENDER_VN NULL
#define OPTION_VAL_2X_VBLANK_VN NULL
#define FCEUMM_RAMSTATE_LABEL_VN NULL
#define FCEUMM_RAMSTATE_INFO_0_VN NULL
#define OPTION_VAL_FILL_FF_VN NULL
#define OPTION_VAL_FILL_00_VN NULL
#define OPTION_VAL_RANDOM_VN NULL

struct retro_core_option_v2_category option_cats_vn[] = {
   {
      "system",
      CATEGORY_SYSTEM_LABEL_VN,
      CATEGORY_SYSTEM_INFO_0_VN
   },
   {
      "video",
      CATEGORY_VIDEO_LABEL_VN,
#ifdef HAVE_NTSC_FILTER
      CATEGORY_VIDEO_INFO_0_VN
#else
      CATEGORY_VIDEO_INFO_1_VN
#endif
   },
   {
      "audio",
      CATEGORY_AUDIO_LABEL_VN,
      CATEGORY_AUDIO_INFO_0_VN
   },
   {
      "input",
      CATEGORY_INPUT_LABEL_VN,
      CATEGORY_INPUT_INFO_0_VN
   },
   {
      "hacks",
      CATEGORY_HACKS_LABEL_VN,
      CATEGORY_HACKS_INFO_0_VN
   },
   {
      "dip_switch",
      CATEGORY_DIP_SWITCH_LABEL_VN,
      CATEGORY_DIP_SWITCH_INFO_0_VN
   },
   { NULL, NULL, NULL },
};
struct retro_core_option_v2_definition option_defs_vn[] = {
   {
      "fceumm_region",
      FCEUMM_REGION_LABEL_VN,
      NULL,
      FCEUMM_REGION_INFO_0_VN,
      NULL,
      "system",
      {
         { "Auto",  OPTION_VAL_AUTO_VN },
         { "NTSC",  OPTION_VAL_NTSC_VN },
         { "PAL",   OPTION_VAL_PAL_VN },
         { "Dendy", OPTION_VAL_DENDY_VN },
         { NULL, NULL },
      },
      "Auto",
   },
   {
      "fceumm_game_genie",
      FCEUMM_GAME_GENIE_LABEL_VN,
      NULL,
      FCEUMM_GAME_GENIE_INFO_0_VN,
      NULL,
      "system",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_show_adv_system_options",
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_LABEL_VN,
      NULL,
      FCEUMM_SHOW_ADV_SYSTEM_OPTIONS_INFO_0_VN,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_show_adv_sound_options",
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_LABEL_VN,
      NULL,
      FCEUMM_SHOW_ADV_SOUND_OPTIONS_INFO_0_VN,
      NULL,
      NULL,
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled"
   },
   {
      "fceumm_aspect",
      FCEUMM_ASPECT_LABEL_VN,
      NULL,
      FCEUMM_ASPECT_INFO_0_VN,
      NULL,
      "video",
      {
         { "8:7 PAR", OPTION_VAL_8_7_PAR_VN },
         { "4:3",     OPTION_VAL_4_3_VN },
         { "PP",      OPTION_VAL_PP_VN },
         { NULL, NULL },
      },
      "8:7 PAR",
   },
#ifdef PSP /* overscan options */
   {
      "fceumm_overscan",
      FCEUMM_OVERSCAN_LABEL_VN,
      NULL,
      FCEUMM_OVERSCAN_INFO_0_VN,
      NULL,
      "video",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#else
   {
      "fceumm_overscan_h",
      FCEUMM_OVERSCAN_H_LABEL_VN,
      NULL,
      FCEUMM_OVERSCAN_H_INFO_0_VN,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overscan_v",
      FCEUMM_OVERSCAN_V_LABEL_VN,
      NULL,
      FCEUMM_OVERSCAN_V_INFO_0_VN,
      NULL,
      "video",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "enabled",
   },
#endif /* overscan options */
   {
      "fceumm_palette",
      FCEUMM_PALETTE_LABEL_VN,
      NULL,
      FCEUMM_PALETTE_INFO_0_VN,
      NULL,
      "video",
      {
         { "default",              OPTION_VAL_DEFAULT_VN },
         { "asqrealc",             OPTION_VAL_ASQREALC_VN },
         { "nintendo-vc",          OPTION_VAL_NINTENDO_VC_VN },
         { "rgb",                  OPTION_VAL_RGB_VN },
         { "yuv-v3",               OPTION_VAL_YUV_V3_VN },
         { "unsaturated-final",    OPTION_VAL_UNSATURATED_FINAL_VN },
         { "sony-cxa2025as-us",    OPTION_VAL_SONY_CXA2025AS_US_VN },
         { "pal",                  OPTION_VAL_PAL_VN },
         { "bmf-final2",           OPTION_VAL_BMF_FINAL2_VN },
         { "bmf-final3",           OPTION_VAL_BMF_FINAL3_VN },
         { "smooth-fbx",           OPTION_VAL_SMOOTH_FBX_VN },
         { "composite-direct-fbx", OPTION_VAL_COMPOSITE_DIRECT_FBX_VN },
         { "pvm-style-d93-fbx",    OPTION_VAL_PVM_STYLE_D93_FBX_VN },
         { "ntsc-hardware-fbx",    OPTION_VAL_NTSC_HARDWARE_FBX_VN },
         { "nes-classic-fbx-fs",   OPTION_VAL_NES_CLASSIC_FBX_FS_VN },
         { "nescap",               OPTION_VAL_NESCAP_VN },
         { "wavebeam",             OPTION_VAL_WAVEBEAM_VN },
         { "digital-prime-fbx",    OPTION_VAL_DIGITAL_PRIME_FBX_VN },
         { "magnum-fbx",           OPTION_VAL_MAGNUM_FBX_VN },
         { "smooth-v2-fbx",        OPTION_VAL_SMOOTH_V2_FBX_VN },
         { "nes-classic-fbx",      OPTION_VAL_NES_CLASSIC_FBX_VN },
         { "raw",                  OPTION_VAL_RAW_VN },
         { "custom",               OPTION_VAL_CUSTOM_VN },
         { NULL, NULL },
      },
      "default",
   },
#ifdef HAVE_NTSC_FILTER
   {
      "fceumm_ntsc_filter",
      FCEUMM_NTSC_FILTER_LABEL_VN,
      NULL,
      FCEUMM_NTSC_FILTER_INFO_0_VN,
      NULL,
      "video",
      {
         { "disabled",   NULL },
         { "composite",  OPTION_VAL_COMPOSITE_VN },
         { "svideo",     OPTION_VAL_SVIDEO_VN },
         { "rgb",        OPTION_VAL_RGB_O9_VN },
         { "monochrome", OPTION_VAL_MONOCHROME_VN },
         { NULL, NULL },
      },
      "disabled"
   },
#endif
   {
      "fceumm_sndquality",
      FCEUMM_SNDQUALITY_LABEL_VN,
      NULL,
      FCEUMM_SNDQUALITY_INFO_0_VN,
      NULL,
      "audio",
      {
         { "Low",       OPTION_VAL_LOW_VN },
         { "High",      OPTION_VAL_HIGH_VN },
         { "Very High", OPTION_VAL_VERY_HIGH_VN },
         { NULL, NULL },
      },
      "Low",
   },
   {
      "fceumm_sndlowpass",
      FCEUMM_SNDLOWPASS_LABEL_VN,
      NULL,
      FCEUMM_SNDLOWPASS_INFO_0_VN,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndstereodelay",
      FCEUMM_SNDSTEREODELAY_LABEL_VN,
      NULL,
      FCEUMM_SNDSTEREODELAY_INFO_0_VN,
      NULL,
      "audio",
      {
         { "disabled",    NULL },
         { "01_ms_delay", OPTION_VAL_01_MS_DELAY_VN },
         { "02_ms_delay", OPTION_VAL_02_MS_DELAY_VN },
         { "03_ms_delay", OPTION_VAL_03_MS_DELAY_VN },
         { "04_ms_delay", OPTION_VAL_04_MS_DELAY_VN },
         { "05_ms_delay", OPTION_VAL_05_MS_DELAY_VN },
         { "06_ms_delay", OPTION_VAL_06_MS_DELAY_VN },
         { "07_ms_delay", OPTION_VAL_07_MS_DELAY_VN },
         { "08_ms_delay", OPTION_VAL_08_MS_DELAY_VN },
         { "09_ms_delay", OPTION_VAL_09_MS_DELAY_VN },
         { "10_ms_delay", OPTION_VAL_10_MS_DELAY_VN },
         { "11_ms_delay", OPTION_VAL_11_MS_DELAY_VN },
         { "12_ms_delay", OPTION_VAL_12_MS_DELAY_VN },
         { "13_ms_delay", OPTION_VAL_13_MS_DELAY_VN },
         { "14_ms_delay", OPTION_VAL_14_MS_DELAY_VN },
         { "15_ms_delay", OPTION_VAL_15_MS_DELAY_VN },
         { "16_ms_delay", OPTION_VAL_16_MS_DELAY_VN },
         { "17_ms_delay", OPTION_VAL_17_MS_DELAY_VN },
         { "18_ms_delay", OPTION_VAL_18_MS_DELAY_VN },
         { "19_ms_delay", OPTION_VAL_19_MS_DELAY_VN },
         { "20_ms_delay", OPTION_VAL_20_MS_DELAY_VN },
         { "21_ms_delay", OPTION_VAL_21_MS_DELAY_VN },
         { "22_ms_delay", OPTION_VAL_22_MS_DELAY_VN },
         { "23_ms_delay", OPTION_VAL_23_MS_DELAY_VN },
         { "24_ms_delay", OPTION_VAL_24_MS_DELAY_VN },
         { "25_ms_delay", OPTION_VAL_25_MS_DELAY_VN },
         { "26_ms_delay", OPTION_VAL_26_MS_DELAY_VN },
         { "27_ms_delay", OPTION_VAL_27_MS_DELAY_VN },
         { "28_ms_delay", OPTION_VAL_28_MS_DELAY_VN },
         { "29_ms_delay", OPTION_VAL_29_MS_DELAY_VN },
         { "30_ms_delay", OPTION_VAL_30_MS_DELAY_VN },
         { "31_ms_delay", OPTION_VAL_31_MS_DELAY_VN },
         { "32_ms_delay", OPTION_VAL_32_MS_DELAY_VN },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_swapduty",
      FCEUMM_SWAPDUTY_LABEL_VN,
      FCEUMM_SWAPDUTY_LABEL_CAT_VN,
      FCEUMM_SWAPDUTY_INFO_0_VN,
      NULL,
      "audio",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_sndvolume",
      FCEUMM_SNDVOLUME_LABEL_VN,
      NULL,
      FCEUMM_SNDVOLUME_INFO_0_VN,
      NULL,
      "audio",
      {
         { "0",  OPTION_VAL_0_VN },
         { "1",  OPTION_VAL_1_VN },
         { "2",  OPTION_VAL_2_VN },
         { "3",  OPTION_VAL_3_VN },
         { "4",  OPTION_VAL_4_VN },
         { "5",  OPTION_VAL_5_VN },
         { "6",  OPTION_VAL_6_VN },
         { "7",  OPTION_VAL_7_VN },
         { "8",  OPTION_VAL_8_VN },
         { "9",  OPTION_VAL_9_VN },
         { "10", OPTION_VAL_10_VN },
         { NULL, NULL },
      },
      "7",
   },
   {
      "fceumm_apu_1",
      FCEUMM_APU_1_LABEL_VN,
      FCEUMM_APU_1_LABEL_CAT_VN,
      FCEUMM_APU_1_INFO_0_VN,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_2",
      FCEUMM_APU_2_LABEL_VN,
      FCEUMM_APU_2_LABEL_CAT_VN,
      FCEUMM_APU_2_INFO_0_VN,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_3",
      FCEUMM_APU_3_LABEL_VN,
      FCEUMM_APU_3_LABEL_CAT_VN,
      FCEUMM_APU_3_INFO_0_VN,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_4",
      FCEUMM_APU_4_LABEL_VN,
      FCEUMM_APU_4_LABEL_CAT_VN,
      FCEUMM_APU_4_INFO_0_VN,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_apu_5",
      FCEUMM_APU_5_LABEL_VN,
      FCEUMM_APU_5_LABEL_CAT_VN,
      FCEUMM_APU_5_INFO_0_VN,
      NULL,
      "audio",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_turbo_enable",
      FCEUMM_TURBO_ENABLE_LABEL_VN,
      NULL,
      FCEUMM_TURBO_ENABLE_INFO_0_VN,
      NULL,
      "input",
      {
         { "None",     OPTION_VAL_NONE_VN },
         { "Player 1", OPTION_VAL_PLAYER_1_VN },
         { "Player 2", OPTION_VAL_PLAYER_2_VN },
         { "Both",     OPTION_VAL_BOTH_VN },
         { NULL, NULL },
      },
      "None",
   },
   {
      "fceumm_turbo_delay",
      FCEUMM_TURBO_DELAY_LABEL_VN,
      NULL,
      FCEUMM_TURBO_DELAY_INFO_0_VN,
      NULL,
      "input",
      {
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "5",  NULL },
         { "10", NULL },
         { "15", NULL },
         { "30", NULL },
         { "60", NULL },
         { NULL, NULL },
      },
      "3",
   },
   {
      "fceumm_zapper_mode",
      FCEUMM_ZAPPER_MODE_LABEL_VN,
      NULL,
      FCEUMM_ZAPPER_MODE_INFO_0_VN,
      NULL,
      "input",
      {
         { "clightgun",   OPTION_VAL_CLIGHTGUN_VN },
         { "stlightgun",  OPTION_VAL_STLIGHTGUN_VN },
         { "touchscreen", OPTION_VAL_TOUCHSCREEN_VN },
         { "mouse",       OPTION_VAL_MOUSE_VN },
         { NULL, NULL },
      },
      "clightgun",
   },
   {
      "fceumm_show_crosshair",
      FCEUMM_SHOW_CROSSHAIR_LABEL_VN,
      NULL,
      FCEUMM_SHOW_CROSSHAIR_INFO_0_VN,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_tolerance",
      FCEUMM_ZAPPER_TOLERANCE_LABEL_VN,
      NULL,
      FCEUMM_ZAPPER_TOLERANCE_INFO_0_VN,
      NULL,
      "input",
      {
         { "0",  NULL },
         { "1",  NULL },
         { "2",  NULL },
         { "3",  NULL },
         { "4",  NULL },
         { "5",  NULL },
         { "6",  NULL },
         { "7",  NULL },
         { "8",  NULL },
         { "9",  NULL },
         { "10", NULL },
         { "11", NULL },
         { "12", NULL },
         { "13", NULL },
         { "14", NULL },
         { "15", NULL },
         { "16", NULL },
         { "17", NULL },
         { "18", NULL },
         { "19", NULL },
         { "20", NULL },
         { NULL, NULL },
      },
      "6",
   },
   {
      "fceumm_zapper_trigger",
      FCEUMM_ZAPPER_TRIGGER_LABEL_VN,
      NULL,
      FCEUMM_ZAPPER_TRIGGER_INFO_0_VN,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_zapper_sensor",
      FCEUMM_ZAPPER_SENSOR_LABEL_VN,
      NULL,
      FCEUMM_ZAPPER_SENSOR_INFO_0_VN,
      NULL,
      "input",
      {
         { "enabled",  NULL },
         { "disabled", NULL },
         { NULL, NULL },
      },
      "enabled",
   },
   {
      "fceumm_up_down_allowed",
      FCEUMM_UP_DOWN_ALLOWED_LABEL_VN,
      NULL,
      FCEUMM_UP_DOWN_ALLOWED_INFO_0_VN,
      NULL,
      "input",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_nospritelimit",
      FCEUMM_NOSPRITELIMIT_LABEL_VN,
      NULL,
      FCEUMM_NOSPRITELIMIT_INFO_0_VN,
      NULL,
      "hacks",
      {
         { "disabled", NULL },
         { "enabled",  NULL },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_overclocking",
      FCEUMM_OVERCLOCKING_LABEL_VN,
      NULL,
      FCEUMM_OVERCLOCKING_INFO_0_VN,
      NULL,
      "hacks",
      {
         { "disabled",      NULL },
         { "2x-Postrender", OPTION_VAL_2X_POSTRENDER_VN },
         { "2x-VBlank",     OPTION_VAL_2X_VBLANK_VN },
         { NULL, NULL },
      },
      "disabled",
   },
   {
      "fceumm_ramstate",
      FCEUMM_RAMSTATE_LABEL_VN,
      NULL,
      FCEUMM_RAMSTATE_INFO_0_VN,
      NULL,
      "hacks",
      {
         { "fill $ff", OPTION_VAL_FILL_FF_VN },
         { "fill $00", OPTION_VAL_FILL_00_VN },
         { "random",   OPTION_VAL_RANDOM_VN },
         { NULL, NULL },
      },
      "fill $ff",
   },
   { NULL, NULL, NULL, NULL, NULL, NULL, {{0}}, NULL },
};
struct retro_core_options_v2 options_vn = {
   option_cats_vn,
   option_defs_vn
};


#ifdef __cplusplus
}
#endif

#endif