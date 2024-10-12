#include "rg_localization.h"

int rg_language = 0;

Translation translations[] = {
    // rg_gui.c
    {"Folder is empty.", "Le dossier est vide."},

    {"yes", "Oui"},
    {"No ", "Non"},
    {"OK", "OK"},

    {"On ", "On "},
    {"Off  ", "Off  "},
    {"Horiz", "Horiz"},
    {"Vert ", "Vert "},
    {"Both ", "Tout"},

    {"Fit ", "Ajuster"},
    {"Full ", "Remplir "},
    {"Zoom", "Zoomer"},

    // Led options
    {"LED options", "Options LED"},
    {"System activity", "Activite systeme"},
    {"Disk activity", "Activite stockage"},
    {"Low battery", "Battery basse"},

    {"Default", "Default"},
    {"none", "Aucun"},
    {"<None>", "<Aucun>"},

    // Wifi
    {"Not connected", "Non connecte"},
    {"Connecting...", "Connection..."},
    {"Disconnecting...", "Disconnection..."},
    {"(empty)", "(vide)"},
    {"Wi-Fi Profile", "Profil Wi-Fi"},
    {"Wi-Fi AP", "Wi-Fi AP"},
    {"Start access point?\n\nSSID: retro-go\nPassword: retro-go\n\nBrowse: http://192.168.4.1/", "Demarrer point d'acces?\n\nSSID: retro-go\nPassword: retro-go\n\nAdresse: http://192.168.4.1/"},
    {"Wi-Fi enable ",  "Activer Wi-Fi"},
    {"Wi-Fi access point", "Point d'acces WiFi"},
    {"Network   ", "Reseau   "},
    {"IP address", "Adresse IP"},
    {"Wifi Options", "Options Wifi"},

    // retro-go settings
    {"Brightness", "Luminosite"},
    {"Volume    ", "Volume    "},
    {"Audio out ", "Sortie audio"},
    {"Font type ", "Police"},
    {"Theme     ", "Theme"},
    {"Show clock", "Horloge"},
    {"Timezone  ", "Fuseau  "},
    {"Wi-Fi options", "Options Wi-Fi"},

    // app dettings
    {"Scaling", "Format"},
    {" Factor", " Factor"},
    {"Filter", "Filtre"},
    {"Border", "Bordure"},
    {"Speed", "Vitesse"},

    // about menu
    {"Version", "Version"},
    {"Date   ", "Date"},
    {"Target ", "Cible"},
    {"Website", "Site Web"},
    {"Options ", "Options"},
    {"View credits", "Credits"},
    {"Debug menu", "Menu debug"},
    {"Reset settings", "Reset parametres"},

    // save slot
    {"Slot 0", "Emplacement 0"},
    {"Slot 1", "Emplacement 1"},
    {"Slot 2", "Emplacement 2"},
    {"Slot 3", "Emplacement 3"},

    // game menu
    {"Save & Continue", "Sauver et continuer"},
    {"Save & Quit    ", "Sauver et quitter"},
    {"Load game      ", "Charger partie"},
    {"Reset          ", "Reset          "},
    {"Netplay ", "Netplay "},
    {"About   ", "Infos   "},
    {"Quit    ", "Quitter "},

    {"Soft reset", "Soft reset"},
    {"Hard reset", "Hard reset"},

    {"Reset Emulation?", "Reset Emulation?"},
    {"Save", "Sauver"},
    {"Load", "Charger"},
    // end of rg_gui.c


    // main.c
    {"Show", "Montrer"},
    {"Hide", "Cacher"},

    {"Tabs Visibility", "Visibilitee onglets"},

    // scroll modes
    {"Center", "centrer"},
    {"Paging", "Paging"},

    // start screen
    {"Auto", "Auto" },
    {"Carousel", "Carousel"},
    {"Browser", "Browser"},

    // preview
    {"None      ", "Aucun     "},
    {"Cover,Save", "Cover,Save"},
    {"Save,Cover", "Save,Cover"},
    {"Cover only", "Cover only"},
    {"Save only ", "Save only "},

    // startup app
    {"Last game", "Dernier jeu"},
    {"Launcher", "Launcher"},

    // launcher options
    {"Launcher Options", "Options du launcher"},
    {"Color theme ", "Couleurs"},
    {"Preview     ", "Apercu     "},
    {"Scroll mode ", "Mode defilement"},
    {"Start screen", "Ecran demarrage"},
    {"Hide tabs   ", "Cacher onglet"},
    {"File server ", "Serveur fichier"},
    {"Startup app ", "App demarrage"},

    {"Build CRC cache", "Build CRC cache"},
    {"Check for updates", "Check for updates"},
    {"HTTP Server Busy...", "Server Web ..."},

    {"SD Card Error", "Erreur carte SD"},
    {"Storage mount failed.\nMake sure the card is FAT32.", "Erreur montage SD.\nVerifiez que la carte est en FAT32."},
    // end of main.c


    // applications.c
    {"Scanning %s %d/%d", "Scan %s %d/%d"},

    // message when no rom
    {"Welcome to Retro-Go!", "Bienvenue sur Retro-Go!"},
    {"Place roms in folder: %s", "Placer les ROMS dans le dossier: %s"},
    {"With file extension: %s", "Avec l'extension: %s"},
    {"You can hide this tab in the menu", "Vous pouvez cacher cet onglet dans le menu"},
    {"You have no %s games", "Vous n'avez pas de jeux %s"},

    {"File not found", "Fichier non present"},

    // rom options
    {"Name", "Nom"},
    {"Folder", "Dossier"},
    {"Size", "Taille"},
    {"CRC32", "CRC32"},
    {"Delete file", "Supprimer fichier"},
    {"Close", "Fermer"},
    {"File properties", "Propriete"},
    {"Delete selected file?", "Supprimer fichier?"},


    // in-game menu
    {"Resume game", "Reprendre partie"},
    {"New game    ", "Nouvelle partie"},
    {"Del favorite", "supp favori"},
    {"Add favorite", "Ajouter favori"},
    {"Delete save", "Supp sauvegarde"},
    {"Properties", "Proprietes"},
    {"Resume", "Reprendre"},
    {"Delete save?", "Supp sauvegarde?"},
    {"Delete sram file?", "Supp fichier SRAM?"},
    // end of applications.c


    // rg_system.c
    {"App unresponsive... Hold MENU to quit!", "Plantage... MENU pour quitter"},

    {"Reset all settings", "Reset parametres"},
    {"Reboot to factory ", "Redemarrer usine"},
    {"Reboot to launcher", "Relancer launcher"},

    {"Recovery mode", "Mode de recuperation"},
    {"System Panic!", "Plantage systeme!"},

    {"Save failed", "Erreur sauvegarde"},
    // end of rg_system.c


    {NULL, NULL}  // End of array
};

#define RG_LANGUAGE_MAX 2


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
    for (int i = 0; translations[i].msg != NULL; i++) {
        if (strcmp(translations[i].msg, text) == 0) {
            switch (rg_language)
            {
            case 0:
                return translations[i].msg;  // Return the english string
                break;
            
            case 1:
                return translations[i].fr;  // Return the french string
                break;
            }
        }
    }
    return text;
}