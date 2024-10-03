// This file contains about every string that is displayed in the GUI of retro-go

// rg_system.c
#define TEXT_App_unresponsive "App unresponsive... Hold MENU to quit!" // "App unresponsive... Hold MENU to quit!"

#define TEXT_Reset_all_settings "Reset all settings" // "Reset all settings"
#define TEXT_Reboot_to_factory "Reboot to factory " // "Reboot to factory "
#define TEXT_Reboot_to_launcher "Reboot to launcher" // "Reboot to launcher"

#define TEXT_Recovery_mode "Recovery mode" // "Recovery mode"
#define TEXT_System_Panic "System Panic!" // "System Panic!"
// end of rg_system.c


// main.c
#define TEXT_Show "Show" // "Show"
#define TEXT_Hide "Hide" // "Hide"

#define TEXT_Tabs_Visibility "Tabs Visibility" // "Tabs Visibility"

// scroll modes
#define TEXT_Center "Center" // "Center"
#define TEXT_Paging "Paging" // "Paging"

// start screen
#define TEXT_Auto "Auto" // "Auto"
#define TEXT_Carousel "Carousel" // "Carousel"
#define TEXT_Browser "Browser" // "Browser"

// preview
#define TEXT_None "None      " // "None      "
#define TEXT_Cover_Save "Cover,Save" // "Cover,Save"
#define TEXT_Save_Cover "Save,Cover" // "Save,Cover"
#define TEXT_Cover_only "Cover only" // "Cover only"
#define TEXT_Save_only "Save only " // "Save only "

// startup app
#define TEXT_Last_game "Last game" // "Last game"
#define TEXT_Launcher "Launcher" // "Launcher"

// launcher options
#define TEXT_Launcher_Options "Launcher Options" // "Launcher Options"
#define TEXT_Color_theme "Color theme " // "Color theme "
#define TEXT_Preview "Preview     " // "Preview     "
#define TEXT_Scroll_mode "Scroll mode " // "Scroll mode "
#define TEXT_Start_screen "Start screen" // "Start screen"
#define TEXT_Hide_tabs "Hide tabs   " // "Hide tabs   "
#define TEXT_File_server "File server " // "File server "
#define TEXT_Startup_app "Startup app " // "Startup app "

#define TEXT_Build_CRC_cache "Build CRC cache" // "Build CRC cache"
#define TEXT_Check_for_updates "Check for updates" // "Check for updates"
#define TEXT_HTTP_Server_Busy "HTTP Server Busy..." // "HTTP Server Busy..."

#define TEXT_HTTP_SD_Card_Error "SD Card Error" // "SD Card Error"
#define TEXT_HTTP_Storage_mount_failed "Storage mount failed.\nMake sure the card is FAT32." // "Storage mount failed.\nMake sure the card is FAT32."
// end of main.c


// applications.c
#define TEXT_Found_subdirectory "Found subdirectory '%s'" // "Found subdirectory '%s'"
#define TEXT_Ran_out_of_memory_file_scanning_stopped_at "Ran out of memory, file scanning stopped at %d entries ..." // "Ran out of memory, file scanning stopped at %d entries ..."
#define TEXT_Initializing_application "Initializing application '%s' (%s)" // "Initializing application '%s' (%s)"
#define TEXT_Failed_to_allocate_crc_cache "Failed to allocate crc_cache!" // "Failed to allocate crc_cache!"
#define TEXT_Loaded_CRC_cache "Loaded CRC cache (entries: %d)" // "Loaded CRC cache (entries: %d)"
#define TEXT_Saving_CRC_cache "Saving CRC cache..." // "Saving CRC cache..."

#define TEXT_Adding_to_cache "Adding %08X => %08X to cache (new total: %d)" // "Adding %08X => %08X to cache (new total: %d)""
#define TEXT_Updating_to_cache "Updating %08X => %08X to cache (total: %d)" // "Updating %08X => %08X to cache (total: %d)"

#define TEXT_Scanning "Scanning %s %d/%d" // "Scanning %s %d/%d"

// message when no rom
#define TEXT_Welcome_to_Retro_Go "Welcome to Retro-Go!" // "Welcome to Retro-Go!"
#define TEXT_Place_roms_in_folder "Place roms in folder: %s" // "Place roms in folder: %s"
#define TEXT_With_file_extension "With file extension: %s" // "With file extension: %s"
#define TEXT_You_can_hide_this_tab "You can hide this tab in the menu" // "You can hide this tab in the menu"

#define TEXT_File_not_found "File not found" // "File not found"

// rom options
#define TEXT_ROM_Name "Name" // "Name"
#define TEXT_ROM_Folder "Folder" // "Folder"
#define TEXT_ROM_Size "Size" // "Size"
#define TEXT_ROM_CRC32 "CRC32" // "CRC32"
#define TEXT_ROM_Delete_file "Delete file" // "Delete file"
#define TEXT_ROM_Close "Close" // "Close"
#define TEXT_File_properties "File properties" // "File properties"
#define TEXT_Delete_selected_file "Delete selected file?" // "Delete selected file?"

// in-game menu
#define TEXT_Resume_game "Resume game" // "Resume game"
#define TEXT_New_game "New game    " // "New game    "
#define TEXT_Del_favorite "Del favorite" // "Del favorite"
#define TEXT_Add_favorite "Add favorite" // "Add favorite"
#define TEXT_Delete_save "Delete save" // "Delete save"
#define TEXT_Properties "Properties" // "Properties"
#define TEXT_Resume "Resume" // "Resume"
#define TEXT_Delete_save_question "Delete save?" // "Delete save?"
#define TEXT_Delete_sram_file "Delete sram file?" // "Delete sram file?"

#define TEXT_Application_not_present "Application '%s' (%s) not present, skipping" // "Application '%s' (%s) not present, skipping"
// end of applications.c


// rg_gui.c
#define TEXT_Failed_to_load_theme_JSON_from "Failed to load theme JSON from '%s'!\n" // "Failed to load theme JSON from '%s'!\n"
#define TEXT_Theme_set_to "Theme set to '%s'!\n" // "Theme set to '%s'!\n"
#define TEXT_Font_set_to "Font set to: points=%d, scaling=%.2f\n" // "Font set to: points=%d, scaling=%.2f\n"
#define TEXT_SPEED_BUSY "SPEED: %d%% (%d %d) / BUSY: %d%%" // "SPEED: %d%% (%d %d) / BUSY: %d%%"
#define TEXT_Using_built_in_theme "Using built-in theme!\n" // "Using built-in theme!\n"
#define TEXT_Folder_is_empty "Folder is empty." // "Folder is empty."

#define TEXT_Yes "Yes" // "yes"
#define TEXT_No "No" // "No "
#define TEXT_OK "OK" // "OK"

#define TEXT_On "On " // "On "
#define TEXT_Off "Off  " // "Off  "
#define TEXT_Horiz "Horiz" // "Horiz"
#define TEXT_Vert "Vert " // "Vert "
#define TEXT_Both "Both " // "Both "

#define TEXT_Fit "Fit " // "Fit "
#define TEXT_Full "Full " // "Full "
#define TEXT_Zoom "Zoom" // "Zoom"

// Led options
#define TEXT_LED_options "LED options" // "LED options"
#define TEXT_System_activity "System activity" // "System activity"
#define TEXT_Disk_activity "Disk activity" // "Disk activity"
#define TEXT_Low_battery "Low battery" // "Low battery"

#define TEXT_Default "Default" // "Default"
#define TEXT_none "none" // "none"
#define TEXT_none1 "<none>" // "<None>"

// Wifi
#define TEXT_Not_connected "Not connected" // "Not connected"
#define TEXT_Connecting "Connecting..." // "Connecting..."
#define TEXT_Disconnecting "Disconnecting..." // "Disconnecting..."
#define TEXT_empty "(empty)" // "(empty)"
#define TEXT_WiFi_Profile "Wi-Fi Profile" // "Wi-Fi Profile"
#define TEXT_WiFi_AP "Wi-Fi AP" // "Wi-Fi AP"
#define TEXT_WiFi_informations "Start access point?\n\nSSID: retro-go\nPassword: retro-go\n\nBrowse: http://192.168.4.1/" // "Start access point?\n\nSSID: retro-go\nPassword: retro-go\n\nBrowse: http://192.168.4.1/"
#define TEXT_WiFi_enable "Wi-Fi enable " // "Wi-Fi enable "
#define TEXT_WiFi_access_point "Wi-Fi access point" // "Wi-Fi access point"
#define TEXT_Network "Network   " // "Network   "
#define TEXT_IP_address "IP address" // "IP address"
#define TEXT_WiFi_Options "Wifi Options" // "Wifi Options"

// retro-go settings
#define TEXT_Brightness "Brightness" // "Brightness"
#define TEXT_Volume "Volume    " // "Volume    "
#define TEXT_Audio_out "Audio out " // "Audio out "
#define TEXT_Font_type "Font type " // "Font type "
#define TEXT_Theme "Theme" // "Theme     "
#define TEXT_Show_clock "Show clock" // "Show clock"
#define TEXT_Timezone "Timezone  " // "Timezone  "

// app dettings
#define TEXT_Scaling "Scaling" // "Scaling"
#define TEXT_Factor " Factor" // " Factor"
#define TEXT_Filter "Filter" // "Filter"
#define TEXT_Border "Border" // "Border"
#define TEXT_Speed "Speed" // "Speed"

// about menu
#define TEXT_Version "Version" // "Version"
#define TEXT_Date "Date   " // "Date   "
#define TEXT_Target "Target " // "Target "
#define TEXT_Website "Website" // "Website"
#define TEXT_Options "Options " // "Options "
#define TEXT_Debug_menu "Debug menu" // "Debug menu"
#define TEXT_Reset_settings "Reset settings" // "Reset settings"
#define TEXT_About_Retro_Go "About Retro-Go" // "About Retro-Go"
#define TEXT_Credits "Credits" // "Credits"
#define TEXT_Reset_all_settings_question "Reset all settings?" // "Reset all settings?"

// debug menu
#define TEXT_Screen_res "Screen res" // "Screen res"
#define TEXT_Source_res "Source res" // "Source res"
#define TEXT_Scaled_res "Scaled res" // "Scaled res"
#define TEXT_Stack_HWM "Stack HWM " // "Stack HWM "
#define TEXT_Heap_free "Heap free " // "Heap free "
#define TEXT_Block_free "Block free" // "Block free"
#define TEXT_App_name "App name  " // "App name  "
#define TEXT_Local_time "Local time" // "Local time"
#define TEXT_Uptime "Uptime    " // "Uptime    "
#define TEXT_Battery "Battery   " // "Battery   "
#define TEXT_Blit_time "Blit time " // "Blit time "

#define TEXT_Overclock "Overclock" // "Overclock"
#define TEXT_Reboot_to_firmware "Reboot to firmware" // "Reboot to firmware"
#define TEXT_Clear_cache "Clear cache    " // "Clear cache    "
#define TEXT_Save_screenshot "Save screenshot" // "Save screenshot"
#define TEXT_Save_trace "Save trace" // "Save trace"
#define TEXT_Cheats "Cheats    " // "Cheats    "
#define TEXT_Crash "Crash     " // "Crash     "
#define TEXT_Log_debug "Log=debug " // "Log=debug "

#define TEXT_Debugging "Debugging" // "Debugging"

// save slot
#define TEXT_Slot_last_used "Slot %d (last used)" // "Slot %d (last used)"
#define TEXT_Slot "Slot %d" // "Slot %d"
#define TEXT_Slot_is_empty "Slot %d is empty" // "Slot %d is empty"
#define TEXT_Slot0 "Slot 0" // "Slot 0"
#define TEXT_Slot1 "Slot 1" // "Slot 1"
#define TEXT_Slot2 "Slot 2" // "Slot 2"
#define TEXT_Slot3 "Slot 3" // "Slot 3"

// game menu
#define TEXT_Save_Continue "Save & Continue" // "Save & Continue"
#define TEXT_Save_Quit "Save & Quit    " // "Save & Quit    "
#define TEXT_Load_game "Load game      " // "Load game      "
#define TEXT_Reset "Reset          " // "Reset          "
#define TEXT_Netplay "Netplay " // "Netplay "
#define TEXT_About "About   " // "About   "
#define TEXT_Quit "Quit    " // "Quit    "

#define TEXT_Retro_Go "Retro-Go" // "Retro-Go"

#define TEXT_Soft_reset "Soft reset" // "Soft reset"
#define TEXT_Hard_reset "Hard reset" // "Hard reset"

#define TEXT_Reset_Emulation "Reset Emulation?" // "Reset Emulation?"
#define TEXT_Save "Save" // "Save"
#define TEXT_Load "Load" // "Load"
// end of rg_gui.c