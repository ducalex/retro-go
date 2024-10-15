#include "rg_localization.h"

int rg_language = 0;

Translation translations[] = {
    // rg_gui.c
    {
        .msg = "Folder is empty.", 
        .fr = "Le dossier est vide."},

    {
        .msg = "yes", 
        .fr = "Oui"},
    {
        .msg = "No ", 
        .fr = "Non"},
    {
        .msg = "OK", 
        .fr = "OK"},

    {
        .msg = "On ", 
        .fr = "On "},
    {
        .msg = "Off  ", 
        .fr = "Off  "},
    {
        .msg = "Horiz", 
        .fr = "Horiz"},
    {
        .msg = "Vert ", 
        .fr = "Vert "},
    {
        .msg = "Both ", 
        .fr = "Tout"},

    {
        .msg = "Fit ", 
        .fr = "Ajuster"},
    {
        .msg = "Full ", 
        .fr = "Remplir "},
    {
        .msg = "Zoom", 
        .fr = "Zoomer"},

    // Led options
    {
        .msg = "LED options", 
        .fr = "Options LED"},
    {
        .msg = "System activity", 
        .fr = "Activite systeme"},
    {
        .msg = "Disk activity", 
        .fr = "Activite stockage"},
    {
        .msg = "Low battery", 
        .fr = "Battery basse"},

    {
        .msg = "Default", 
        .fr = "Default"},
    {
        .msg = "none", 
        .fr = "Aucun"},
    {
        .msg = "<None>", 
        .fr = "<Aucun>"},

    // Wifi
    {
        .msg = "Not connected", 
        .fr = "Non connecte"},
    {
        .msg = "Connecting...", 
        .fr = "Connection..."},
    {
        .msg = "Disconnecting...", 
        .fr = "Disconnection..."},
    {
        .msg = "(empty)", 
        .fr = "(vide)"},
    {
        .msg = "Wi-Fi Profile", 
        .fr = "Profil Wi-Fi"},
    {
        .msg = "Wi-Fi AP", 
        .fr = "Wi-Fi AP"},
    {
        .msg = "Start access point?\n\nSSID: retro-go\nPassword: retro-go\n\nBrowse: http://192.168.4.1/", 
        .fr = "Demarrer point d'acces?\n\nSSID: retro-go\nPassword: retro-go\n\nAdresse: http://192.168.4.1/"},
    {
        .msg = "Wi-Fi enable ",  
        .fr = "Activer Wi-Fi"},
    {
        .msg = "Wi-Fi access point", 
        .fr = "Point d'acces WiFi"},
    {
        .msg = "Network   ", 
        .fr = "Reseau   "},
    {
        .msg = "IP address", 
        .fr = "Adresse IP"},
    {
        .msg = "Wifi Options", 
        .fr = "Options Wifi"},

    // retro-go settings
    {
        .msg = "Brightness", 
        .fr = "Luminosite"},
    {
        .msg = "Volume    ", 
        .fr = "Volume    "},
    {
        .msg = "Audio out ", 
        .fr = "Sortie audio"},
    {
        .msg = "Font type ", 
        .fr = "Police"},
    {
        .msg = "Theme     ", 
        .fr = "Theme"},
    {
        .msg = "Show clock", 
        .fr = "Horloge"},
    {
        .msg = "Timezone  ", 
        .fr = "Fuseau  "},
    {
        .msg = "Wi-Fi options", 
        .fr = "Options Wi-Fi"},

    // app dettings
    {
        .msg = "Scaling", 
        .fr = "Format"},
    {
        .msg = " Factor", 
        .fr = " Factor"},
    {
        .msg = "Filter", 
        .fr = "Filtre"},
    {
        .msg = "Border", 
        .fr = "Bordure"},
    {
        .msg = "Speed", 
        .fr = "Vitesse"},

    // about menu
    {
        .msg = "Version", 
        .fr = "Version"},
    {
        .msg = "Date   ", 
        .fr = "Date"},
    {
        .msg = "Target ", 
        .fr = "Cible"},
    {
        .msg = "Website", 
        .fr = "Site Web"},
    {
        .msg = "Options ", 
        .fr = "Options"},
    {
        .msg = "View credits", 
        .fr = "Credits"},
    {
        .msg = "Debug menu", 
        .fr = "Menu debug"},
    {
        .msg = "Reset settings", 
        .fr = "Reset parametres"},

    // save slot
    {
        .msg = "Slot 0", 
        .fr = "Emplacement 0"},
    {
        .msg = "Slot 1", 
        .fr = "Emplacement 1"},
    {
        .msg = "Slot 2", 
        .fr = "Emplacement 2"},
    {
        .msg = "Slot 3", 
        .fr = "Emplacement 3"},

    // game menu
    {
        .msg = "Save & Continue", 
        .fr = "Sauver et continuer"},
    {
        .msg = "Save & Quit    ", 
        .fr = "Sauver et quitter"},
    {
        .msg = "Load game      ", 
        .fr = "Charger partie"},
    {
        .msg = "Reset          ", 
        .fr = "Reset          "},
    {
        .msg = "Netplay ", 
        .fr = "Netplay "},
    {
        .msg = "About   ", 
        .fr = "Infos   "},
    {
        .msg = "Quit    ", 
        .fr = "Quitter "},

    {
        .msg = "Soft reset", 
        .fr = "Soft reset"},
    {
        .msg = "Hard reset", 
        .fr = "Hard reset"},

    {
        .msg = "Reset Emulation?", 
        .fr = "Reset Emulation?"},
    {
        .msg = "Save", 
        .fr = "Sauver"},
    {
        .msg = "Load", 
        .fr = "Charger"},
    // end of rg_gui.c


    // main.c
    {
        .msg = "Show", 
        .fr = "Montrer"},
    {
        .msg = "Hide", 
        .fr = "Cacher"},

    {
        .msg = "Tabs Visibility", 
        .fr = "Visibilitee onglets"},

    // scroll modes
    {
        .msg = "Center", 
        .fr = "centrer"},
    {
        .msg = "Paging", 
        .fr = "Paging"},

    // start screen
    {
        .msg = "Auto", 
        .fr = "Auto" },
    {
        .msg = "Carousel", 
        .fr = "Carousel"},
    {
        .msg = "Browser", 
        .fr = "Browser"},

    // preview
    {
        .msg = "None      ", 
        .fr = "Aucun     "},
    {
        .msg = "Cover,Save", 
        .fr = "Cover,Save"},
    {
        .msg = "Save,Cover", 
        .fr = "Save,Cover"},
    {
        .msg = "Cover only", 
        .fr = "Cover only"},
    {
        .msg = "Save only ", 
        .fr = "Save only "},

    // startup app
    {
        .msg = "Last game", 
        .fr = "Dernier jeu"},
    {
        .msg = "Launcher", 
        .fr = "Launcher"},

    // launcher options
    {
        .msg = "Launcher Options", 
        .fr = "Options du launcher"},
    {
        .msg = "Color theme ", 
        .fr = "Couleurs"},
    {
        .msg = "Preview     ", 
        .fr = "Apercu     "},
    {
        .msg = "Scroll mode ", 
        .fr = "Mode defilement"},
    {
        .msg = "Start screen", 
        .fr = "Ecran demarrage"},
    {
        .msg = "Hide tabs   ", 
        .fr = "Cacher onglet"},
    {
        .msg = "File server ", 
        .fr = "Serveur fichier"},
    {
        .msg = "Startup app ", 
        .fr = "App demarrage"},

    {
        .msg = "Build CRC cache", 
        .fr = "Build CRC cache"},
    {
        .msg = "Check for updates", 
        .fr = "Check for updates"},
    {
        .msg = "HTTP Server Busy...", 
        .fr = "Server Web ..."},

    {
        .msg = "SD Card Error", 
        .fr = "Erreur carte SD"},
    {
        .msg = "Storage mount failed.\nMake sure the card is FAT32.", 
        .fr = "Erreur montage SD.\nVerifiez que la carte est en FAT32."},
    // end of main.c


    // applications.c
    {
        .msg = "Scanning %s %d/%d", 
        .fr = "Scan %s %d/%d"},

    // message when no rom
    {
        .msg = "Welcome to Retro-Go!", 
        .fr = "Bienvenue sur Retro-Go!"},
    {
        .msg = "Place roms in folder: %s", 
        .fr = "Placer les ROMS dans le dossier: %s"},
    {
        .msg = "With file extension: %s", 
        .fr = "Avec l'extension: %s"},
    {
        .msg = "You can hide this tab in the menu", 
        .fr = "Vous pouvez cacher cet onglet dans le menu"},
    {
        .msg = "You have no %s games", 
        .fr = "Vous n'avez pas de jeux %s"},

    {
        .msg = "File not found", 
        .fr = "Fichier non present"},

    // rom options
    {
        .msg = "Name", 
        .fr = "Nom"},
    {
        .msg = "Folder", 
        .fr = "Dossier"},
    {
        .msg = "Size", 
        .fr = "Taille"},
    {
        .msg = "CRC32", 
        .fr = "CRC32"},
    {
        .msg = "Delete file", 
        .fr = "Supprimer fichier"},
    {
        .msg = "Close", 
        .fr = "Fermer"},
    {
        .msg = "File properties", 
        .fr = "Propriete"},
    {
        .msg = "Delete selected file?", 
        .fr = "Supprimer fichier?"},


    // in-game menu
    {
        .msg = "Resume game", 
        .fr = "Reprendre partie"},
    {
        .msg = "New game    ", 
        .fr = "Nouvelle partie"},
    {
        .msg = "Del favorite", 
        .fr = "supp favori"},
    {
        .msg = "Add favorite", 
        .fr = "Ajouter favori"},
    {
        .msg = "Delete save", 
        .fr = "Supp sauvegarde"},
    {
        .msg = "Properties", 
        .fr = "Proprietes"},
    {
        .msg = "Resume", 
        .fr = "Reprendre"},
    {
        .msg = "Delete save?", 
        .fr = "Supp sauvegarde?"},
    {
        .msg = "Delete sram file?", 
        .fr = "Supp fichier SRAM?"},
    // end of applications.c


    // rg_system.c
    {
        .msg = "App unresponsive... Hold MENU to quit!", 
        .fr = "Plantage... MENU pour quitter"},

    {
        .msg = "Reset all settings", 
        .fr = "Reset parametres"},
    {
        .msg = "Reboot to factory ", 
        .fr = "Redemarrer usine"},
    {
        .msg = "Reboot to launcher", 
        .fr = "Relancer launcher"},

    {
        .msg = "Recovery mode", "Mode de recuperation"},
    {
        .msg = "System Panic!", 
        .fr = "Plantage systeme!"},

    {
        .msg = "Save failed", 
        .fr = "Erreur sauvegarde"},
    // end of rg_system.c


    {NULL, NULL}  // End of array
};

int rg_localization_get_language_id(void){
    return rg_language;
}

bool rg_localization_set_language_id(int language_id){
    if (language_id < 0 || language_id > RG_LANGUAGE_MAX - 1)
        return false;

    rg_language = language_id;
    return true;
}

const char* rg_gettext(const char *text) {
    if (rg_language == 0)
        return text; // in case language == 0 == english -> return the original string

    for (int i = 0; translations[i].msg != NULL; i++) {
        if (strcmp(translations[i].msg, text) == 0) {
            switch (rg_language)
            {
            case 1:
                return translations[i].fr;  // Return the french string
                break;
            }
        }
    }
    return text;
}