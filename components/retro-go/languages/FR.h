// This file contains about every string that is displayed in the GUI of retro-go

// rg_system.c
#define TEXT_App_unresponsive "L'application a plante...appuyez longuement sur select pour quitter" // "App unresponsive... Hold MENU to quit!"

#define TEXT_Reset_all_settings "Reset parametres" // "Reset all settings"
#define TEXT_Reboot_to_factory "Redemarrer usine" // "Reboot to factory "
#define TEXT_Reboot_to_launcher "Relancer launcher" // "Reboot to launcher"

#define TEXT_Recovery_mode "Mode de récupération" // "Recovery mode"
#define TEXT_System_Panic "Plantage systeme!" // "System Panic!"
// end of rg_system.c


// main.c
#define TEXT_Show "Montrer" // "Show"
#define TEXT_Hide "Cacher" // "Hide"

#define TEXT_Tabs_Visibility "Visibilitee onglets" // "Tabs Visibility"

// scroll modes
#define TEXT_Center "centrer" // "Center"
#define TEXT_Paging "Paging" // "Paging"

// start screen
#define TEXT_Auto "Auto" // "Auto"
#define TEXT_Carousel "Carousel" // "Carousel"
#define TEXT_Browser "Browser" // "Browser"

// preview
#define TEXT_None "Aucun     " // "None      "
#define TEXT_Cover_Save "Cover,Save" // "Cover,Save"
#define TEXT_Save_Cover "Save,Cover" // "Save,Cover"
#define TEXT_Cover_only "Cover only" // "Cover only"
#define TEXT_Save_only "Save only " // "Save only "

// startup app
#define TEXT_Last_game "Dernier jeu" // "Last game"
#define TEXT_Launcher "Launcher" // "Launcher"

// launcher options
#define TEXT_Launcher_Options "Options du launcher" // "Launcher Options"
#define TEXT_Color_theme "Couleurs" // "Color theme "
#define TEXT_Preview "apercu     " // "Preview     "
#define TEXT_Scroll_mode "Mode defilement" // "Scroll mode "
#define TEXT_Start_screen "Ecran demarrage" // "Start screen"
#define TEXT_Hide_tabs "Cacher onglet" // "Hide tabs   "
#define TEXT_File_server "Serveur fichier" // "File server "
#define TEXT_Startup_app "App demarrage" // "Startup app "

#define TEXT_Build_CRC_cache "Build CRC cache" // "Build CRC cache"
#define TEXT_Check_for_updates "Check for updates" // "Check for updates"
#define TEXT_HTTP_Server_Busy "HTTP Server Busy..." // "HTTP Server Busy..."

#define TEXT_HTTP_SD_Card_Error "Erreur carte SD" // "SD Card Error"
#define TEXT_HTTP_Storage_mount_failed "Erreur montage carte SD.\nVerifiez que la carte est en FAT32." // "Storage mount failed.\nMake sure the card is FAT32."
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
#define TEXT_Welcome_to_Retro_Go "Bienvenue sur Retro-Go!" // "Welcome to Retro-Go!"
#define TEXT_Place_roms_in_folder "Placer les ROMS dans le dossier: %s" // "Place roms in folder: %s"
#define TEXT_With_file_extension "Avec l'extension: %s" // "With file extension: %s"
#define TEXT_You_can_hide_this_tab "Vous pouvez chacher cet onglet dans le menu" // "You can hide this tab in the menu"

#define TEXT_File_not_found "Fichier non present" // "File not found"

// rom options
#define TEXT_ROM_Name "Nom" // "Name"
#define TEXT_ROM_Folder "Dossier" // "Folder"
#define TEXT_ROM_Size "Taille" // "Size"
#define TEXT_ROM_CRC32 "CRC32" // "CRC32"
#define TEXT_ROM_Delete_file "Supprimer fichier" // "Delete file"
#define TEXT_ROM_Close "Fermer" // "Close"
#define TEXT_File_properties "Propriete" // "File properties"
#define TEXT_Delete_selected_file "Supprimer fichier?" // "Delete selected file?"

// in-game menu
#define TEXT_Resume_game "Reprendre partie" // "Resume game"
#define TEXT_New_game "Nouvelle partie" // "New game    "
#define TEXT_Del_favorite "supp favori" // "Del favorite"
#define TEXT_Add_favorite "Ajout favori" // "Add favorite"
#define TEXT_Delete_save "Supp sauvegarde" // "Delete save"
#define TEXT_Properties "Propriete" // "Properties"
#define TEXT_Resume "Reprendre" // "Resume"
#define TEXT_Delete_save_question "Supp sauvegarde?" // "Delete save?"
#define TEXT_Delete_sram_file "Supp fichier SRAM?" // "Delete sram file?"

#define TEXT_Application_not_present "Application '%s' (%s) not present, skipping" // "Application '%s' (%s) not present, skipping"
// end of applications.c


// rg_gui.c
#define TEXT_Failed_to_load_theme_JSON_from "Erreur de chargement du theme JSON de '%s'!\n" // "Failed to load theme JSON from '%s'!\n"
#define TEXT_Theme_set_to "Theme appliqué : '%s'!\n" // "Theme set to '%s'!\n"
#define TEXT_Font_set_to "Police appliqué : points=%d, scaling=%.2f\n" // "Font set to: points=%d, scaling=%.2f\n"
#define TEXT_SPEED_BUSY "VIT: %d%% (%d %d) / BUSY: %d%%" // "SPEED: %d%% (%d %d) / BUSY: %d%%"
#define TEXT_Using_built_in_theme "Usage du theme par défault!\n" // "Using built-in theme!\n"
#define TEXT_Folder_is_empty "Le dossier est vide." // "Folder is empty."

#define TEXT_Yes "Oui" // "yes"
#define TEXT_No "Non" // "No "
#define TEXT_OK "OK" // "OK"

#define TEXT_On "On " // "On "
#define TEXT_Off "Off  " // "Off  "
#define TEXT_Horiz "Horiz" // "Horiz"
#define TEXT_Vert "Vert " // "Vert "
#define TEXT_Both "Tout" // "Both "

#define TEXT_Fit "Ajuster" // "Fit "
#define TEXT_Full "Remplir " // "Full "
#define TEXT_Zoom "Zoomer" // "Zoom"

// Led options
#define TEXT_LED_options "Options LED" // "LED options"
#define TEXT_System_activity "Activite systeme" // "System activity"
#define TEXT_Disk_activity "Activite stockage" // "Disk activity"
#define TEXT_Low_battery "Battery basse" // "Low battery"

#define TEXT_Default "Default" // "Default"
#define TEXT_none "Aucun" // "none"
#define TEXT_none1 "<Aucun>" // "<None>"

// Wifi
#define TEXT_Not_connected "Non connecte" // "Not connected"
#define TEXT_Connecting "Connection..." // "Connecting..."
#define TEXT_Disconnecting "Disconnection..." // "Disconnecting..."
#define TEXT_empty "(vide)" // "(empty)"
#define TEXT_WiFi_Profile "Profil Wi-Fi" // "Wi-Fi Profile"
#define TEXT_WiFi_AP "Wi-Fi AP" // "Wi-Fi AP"
#define TEXT_WiFi_informations "Demarrer point d'acces?\n\nSSID: retro-go\nPassword: retro-go\n\nAdresse: http://192.168.4.1/" // "Start access point?\n\nSSID: retro-go\nPassword: retro-go\n\nBrowse: http://192.168.4.1/"
#define TEXT_WiFi_enable "Activer Wi-Fi" // "Wi-Fi enable "
#define TEXT_WiFi_access_point "Point acces WiFi" // "Wi-Fi access point"
#define TEXT_Network "Reseau   " // "Network   "
#define TEXT_IP_address "Adresse IP" // "IP address"
#define TEXT_WiFi_Options "Options Wifi" // "Wifi Options"

// retro-go settings
#define TEXT_Brightness "Luminosite" // "Brightness"
#define TEXT_Volume "Volume    " // "Volume    "
#define TEXT_Audio_out "Sortie audio" // "Audio out "
#define TEXT_Font_type "Police" // "Font type "
#define TEXT_Theme "Theme" // "Theme     "
#define TEXT_Show_clock "Horloge" // "Show clock"
#define TEXT_Timezone "Fuseau  " // "Timezone  "

// app dettings
#define TEXT_Scaling "Format" // "Scaling"
#define TEXT_Factor " Factor" // " Factor"
#define TEXT_Filter "Filtre" // "Filter"
#define TEXT_Border "Bordure" // "Border"
#define TEXT_Speed "Vitesse" // "Speed"

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
#define TEXT_Network "Network   " // "Network   "
#define TEXT_Local_time "Local time" // "Local time"
#define TEXT_Timezone "Timezone  " // "Timezone  "
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
#define TEXT_Slot_last_used "Emplacement %d (dernier utilisé)" // "Slot %d (last used)"
#define TEXT_Slot "Emplacement %d" // "Slot %d"
#define TEXT_Slot_is_empty "Emplacement %d vide" // "Slot %d is empty"
#define TEXT_Slot0 "Emplacement 0" // "Slot 0"
#define TEXT_Slot1 "Emplacement 1" // "Slot 1"
#define TEXT_Slot2 "Emplacement 2" // "Slot 2"
#define TEXT_Slot3 "Emplacement 3" // "Slot 3"

// game menu
#define TEXT_Save_Continue "Sauver et continuer" // "Save & Continue"
#define TEXT_Save_Quit "Sauver et quitter" // "Save & Quit    "
#define TEXT_Load_game "Charger partie" // "Load game      "
#define TEXT_Reset "Reset          " // "Reset          "
#define TEXT_Netplay "Netplay " // "Netplay "
#define TEXT_About "Infos   " // "About   "
#define TEXT_Quit "Quitter " // "Quit    "

#define TEXT_Retro_Go "Retro-Go" // "Retro-Go"

#define TEXT_Soft_reset "Soft reset" // "Soft reset"
#define TEXT_Hard_reset "Hard reset" // "Hard reset"

#define TEXT_Reset_Emulation "Reset Emulation?" // "Reset Emulation?"
#define TEXT_Save "Sauver" // "Save"
#define TEXT_Load "Charger" // "Load"
// end of rg_gui.c