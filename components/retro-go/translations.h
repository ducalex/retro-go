#include "rg_localization.h"

static const char *language_names[RG_LANG_MAX] = {
    [RG_LANG_EN] = "English",
    [RG_LANG_FR] = "Francais",
    [RG_LANG_DE] = "Deutsch",
};

static const char *translations[][RG_LANG_MAX] =
{
    {
        [RG_LANG_EN] = "Never",
        [RG_LANG_FR] = "Jamais",
        [RG_LANG_DE] = "Niemals",
    },
    {
        [RG_LANG_EN] = "Always",
        [RG_LANG_FR] = "Toujours",
        [RG_LANG_DE] = "Immer",
    },
    {
        [RG_LANG_EN] = "Composite",
        [RG_LANG_FR] = "Composite",
        [RG_LANG_DE] = "Composite",
    },
    {
        [RG_LANG_EN] = "NES Classic",
        [RG_LANG_FR] = "NES Classic",
        [RG_LANG_DE] = "NES Classic",
    },
    {
        [RG_LANG_EN] = "NTSC",
        [RG_LANG_FR] = "NTSC",
        [RG_LANG_DE] = "NTSC",
    },
    {
        [RG_LANG_EN] = "PVM",
        [RG_LANG_FR] = "PVM",
        [RG_LANG_DE] = "PVM",
    },
    {
        [RG_LANG_EN] = "Smooth",
        [RG_LANG_FR] = "Lisser",
        [RG_LANG_DE] = "Glatt",
    },
    {
        [RG_LANG_EN] = "To start, try: 1 or * or #",
        [RG_LANG_FR] = "Pour commencer, 1 ou * ou #",
        [RG_LANG_DE] = "Zum Starten, versuche 1 oder * oder #",
    },
    {
	    [RG_LANG_EN] = "Full",
	    [RG_LANG_FR] = "Complet",
        [RG_LANG_DE] = "Voll",
    },
    {
        [RG_LANG_EN] = "Yes",
        [RG_LANG_FR] = "Oui",
        [RG_LANG_DE] = "Ja",
    },
    {
        [RG_LANG_EN] = "Select file",
        [RG_LANG_FR] = "Choisissez un fichier",
        [RG_LANG_DE] = "Datei auswählen",
    },
    {
        [RG_LANG_EN] = "Language",
        [RG_LANG_FR] = "Langue",
        [RG_LANG_DE] = "Sprache",
    },
    {
        [RG_LANG_EN] = "Language changed!",
        [RG_LANG_FR] = "Changement de langue",
        [RG_LANG_DE] = "Sprache geändert!",
    },
    {
        [RG_LANG_EN] = "For these changes to take effect you must restart your device.\nrestart now?",
        [RG_LANG_FR] = "Pour que ces changements prennent effet, vous devez redémarrer votre appareil.\nRedémarrer maintenant ?",
        [RG_LANG_DE] = "Um die Änderungen anzuwenden muss das Gerät neu gestartet werden. \nJetzt neustarten?",
    },
    {
        [RG_LANG_EN] = "Wi-Fi profile",
        [RG_LANG_FR] = "Profil Wi-Fi",
        [RG_LANG_DE] = "Wi-Fi Profil",
    },
    {
        [RG_LANG_EN] = "Language",
        [RG_LANG_FR] = "Langue",
        [RG_LANG_DE] = "Sprache",
    },
    {
        [RG_LANG_EN] = "Options",
        [RG_LANG_FR] = "Options",
        [RG_LANG_DE] = "Optionen",
    },
    {
        [RG_LANG_EN] = "About Retro-Go",
        [RG_LANG_FR] = "A propos de Retro-go",
        [RG_LANG_DE] = "Über Retro-Go",
    },
    {
        [RG_LANG_EN] = "Reset all settings?",
        [RG_LANG_FR] = "Reset tous les paramètres",
        [RG_LANG_DE] = "Alle Einstellungen zurücksetzen?",
    },
    {
        [RG_LANG_EN] = "Initializing...",
        [RG_LANG_FR] = "Initialisation...",
        [RG_LANG_DE] = "Initialisierung...",
    },
    {
        [RG_LANG_EN] = "Host Game (P1)",
        [RG_LANG_FR] = "Host Game (P1)",
        [RG_LANG_DE] = "Hoste Spiel (P1)",
    },
    {
        [RG_LANG_EN] = "Find Game (P2)",
        [RG_LANG_FR] = "Find Game (P2)",
        [RG_LANG_DE] = "Finde Spiel (P2)",
    },
    {
        [RG_LANG_EN] = "Netplay",
        [RG_LANG_FR] = "Netplay",
        [RG_LANG_DE] = "Netplay",
    },
    {
        [RG_LANG_EN] = "ROMs not identical. Continue?",
        [RG_LANG_FR] = "ROMs not identical. Continue?",
        [RG_LANG_DE] = "ROMs nicht identisch. Weiter?",
    },
    {
        [RG_LANG_EN] = "Exchanging info...",
        [RG_LANG_FR] = "Exchanging info...",
        [RG_LANG_DE] = "Informationsaustausch...",
    },
    {
        [RG_LANG_EN] = "Unable to find host!",
        [RG_LANG_FR] = "Unable to find host!",
        [RG_LANG_DE] = "Kein Host gefunden!",
    },
    {
        [RG_LANG_EN] = "Connection failed!",
        [RG_LANG_FR] = "Connection failed!",
        [RG_LANG_DE] = "Verbindung fehlgeschlagen!",
    },
    {
        [RG_LANG_EN] = "Waiting for peer...",
        [RG_LANG_FR] = "Waiting for peer...",
        [RG_LANG_DE] = "Warte auf peer",
    },
    {
        [RG_LANG_EN] = "Unknown status...",
        [RG_LANG_FR] = "Unknown status...",
        [RG_LANG_DE] = "Unbekannter Status...",
    },
    {
        [RG_LANG_EN] = "On",
        [RG_LANG_FR] = "On",
        [RG_LANG_DE] = "An",
    },
    {
        [RG_LANG_EN] = "Keyboard",
        [RG_LANG_FR] = "Clavier",
        [RG_LANG_DE] = "Keyboard",
    },
    {
        [RG_LANG_EN] = "Joystick",
        [RG_LANG_FR] = "Joystick",
        [RG_LANG_DE] = "Joystick",
    },
    {
        [RG_LANG_EN] = "Input",
        [RG_LANG_FR] = "Entrée",
        [RG_LANG_DE] = "Eingabe",
    },
    {
        [RG_LANG_EN] = "Crop",
        [RG_LANG_FR] = "Couper",
        [RG_LANG_DE] = "Zuschneiden",
    },
    {
        [RG_LANG_EN] = "BIOS file missing!",
        [RG_LANG_FR] = "Fichier BIOS manquant",
        [RG_LANG_DE] = "BIOS-Datei fehlt!",
    },
    {
        [RG_LANG_EN] = "YM2612 audio ",
        [RG_LANG_FR] = "YM2612 audio ",
        [RG_LANG_DE] = "YM2612 audio ",
    },
    {
        [RG_LANG_EN] = "SN76489 audio",
        [RG_LANG_FR] = "SN76489 audio",
        [RG_LANG_DE] = "SN76489 audio",
    },
    {
        [RG_LANG_EN] = "Z80 emulation",
        [RG_LANG_FR] = "Emulation Z80",
        [RG_LANG_DE] = "Z80 emulation",
    },
    {
        [RG_LANG_EN] = "Launcher options",
        [RG_LANG_FR] = "Options du lanceur",
        [RG_LANG_DE] = "Launcher Optionen",
    },
    {
        [RG_LANG_EN] = "Emulator options",
        [RG_LANG_FR] = "Options émulateur",
        [RG_LANG_DE] = "Emulator Optionen",
    },
    {
        [RG_LANG_EN] = "Date",
        [RG_LANG_FR] = "Date",
        [RG_LANG_DE] = "Datum",
    },
    {
        [RG_LANG_EN] = "Files:",
        [RG_LANG_FR] = "Fichiers:",
        [RG_LANG_DE] = "Dateien",
    },
    {
        [RG_LANG_EN] = "Download complete!",
        [RG_LANG_FR] = "Téléchargement terminé",
        [RG_LANG_DE] = "Download abgeschlossen!",
    },
    {
        [RG_LANG_EN] = "Reboot to flash?",
        [RG_LANG_FR] = "Redémarrer",
        [RG_LANG_DE] = "Neustarten um zu flashen",
    },
    {
        [RG_LANG_EN] = "Available Releases",
        [RG_LANG_FR] = "Maj disponible",
        [RG_LANG_DE] = "Verfügbare Releases",
    },
    {
        [RG_LANG_EN] = "Received empty list!",
        [RG_LANG_FR] = "Liste vide reçue",
        [RG_LANG_DE] = "Leere Liste empfangen!",
    },
    {
        [RG_LANG_EN] = "Gamma Boost",
        [RG_LANG_FR] = "Boost Gamma",
        [RG_LANG_DE] = "Gamma Boost",
    },
    {
        [RG_LANG_EN] = "Day",
        [RG_LANG_FR] = "Jour",
        [RG_LANG_DE] = "Tag",
    },
    {
        [RG_LANG_EN] = "Hour",
        [RG_LANG_FR] = "Heure",
        [RG_LANG_DE] = "Stunde",
    },
    {
        [RG_LANG_EN] = "Min",
        [RG_LANG_FR] = "Min",
        [RG_LANG_DE] = "Min",
    },
    {
        [RG_LANG_EN] = "Sec",
        [RG_LANG_FR] = "Sec",
        [RG_LANG_DE] = "Sek",
    },
    {
        [RG_LANG_EN] = "Sync",
        [RG_LANG_FR] = "Synchro",
        [RG_LANG_DE] = "Synchronisieren",
    },
    {
        [RG_LANG_EN] = "RTC config",
        [RG_LANG_FR] = "Config RTC",
        [RG_LANG_DE] = "RTC Einstellung",
    },
    {
        [RG_LANG_EN] = "Palette",
        [RG_LANG_FR] = "Palette",
        [RG_LANG_DE] = "Palette",
    },
    {
        [RG_LANG_EN] = "RTC config",
        [RG_LANG_FR] = "Config RTC",
        [RG_LANG_DE] = "RTC Einstellung",
    },
    {
        [RG_LANG_EN] = "SRAM autosave",
        [RG_LANG_FR] = "Sauvegarde auto SRAM",
        [RG_LANG_DE] = "SRAM autospeichern",
    },
    {
        [RG_LANG_EN] = "Enable BIOS",
        [RG_LANG_FR] = "Activer BIOS",
        [RG_LANG_DE] = "BIOS aktivieren",
    },
    {
        [RG_LANG_EN] = "Name",
        [RG_LANG_FR] = "Nom",
        [RG_LANG_DE] = "Name",
    },
    {
        [RG_LANG_EN] = "Artist",
        [RG_LANG_FR] = "Artiste",
        [RG_LANG_DE] = "Künstler",
    },
    {
        [RG_LANG_EN] = "Copyright",
        [RG_LANG_FR] = "Copyright",
        [RG_LANG_DE] = "Copyright",
    },
    {
        [RG_LANG_EN] = "Playing",
        [RG_LANG_FR] = "Playing",
        [RG_LANG_DE] = "Spielt",
    },
    {
        [RG_LANG_EN] = "Palette",
        [RG_LANG_FR] = "Palette",
        [RG_LANG_DE] = "Palette",
    },
    {
        [RG_LANG_EN] = "Overscan",
        [RG_LANG_FR] = "Overscan",
        [RG_LANG_DE] = "Overscan",
    },
    {
        [RG_LANG_EN] = "Crop sides",
        [RG_LANG_FR] = "Couper les côtés",
        [RG_LANG_DE] = "Seiten abschneiden",
    },
    {
        [RG_LANG_EN] = "Sprite limit",
        [RG_LANG_FR] = "Limite de sprite",
        [RG_LANG_DE] = "Sprite Limit",
    },
    {
        [RG_LANG_EN] = "Overscan",
        [RG_LANG_FR] = "Overscan",
        [RG_LANG_DE] = "Overscan",
    },
    {
        [RG_LANG_EN] = "Palette",
        [RG_LANG_FR] = "Palette",
        [RG_LANG_DE] = "Palette",
    },
    {
        [RG_LANG_EN] = "Profile",
        [RG_LANG_FR] = "Profil",
        [RG_LANG_DE] = "Profil",
    },
    {
        [RG_LANG_EN] = "<profile name>",
        [RG_LANG_FR] = "<nom du profil>",
        [RG_LANG_DE] = "<profil name>",
    },
    {
        [RG_LANG_EN] = "Controls",
        [RG_LANG_FR] = "Contrôles",
        [RG_LANG_DE] = "Steuerung",
    },
    {
        [RG_LANG_EN] = "Audio enable",
        [RG_LANG_FR] = "Activer audio",
        [RG_LANG_DE] = "Audio einschalten",
    },
    {
        [RG_LANG_EN] = "Audio filter",
        [RG_LANG_FR] = "Filtre audio",
        [RG_LANG_DE] = "Audio Filter",
    },


    // rg_gui.c
    {
        [RG_LANG_EN] = "Folder is empty.",
        [RG_LANG_FR] = "Le dossier est vide.",
        [RG_LANG_DE] = "Ordner ist leer",
    },
    {
        [RG_LANG_EN] = "No",
        [RG_LANG_FR] = "Non",
        [RG_LANG_DE] = "Nein",
    },
    {
        [RG_LANG_EN] = "OK",
        [RG_LANG_FR] = "OK",
        [RG_LANG_DE] = "OK",
    },
    {
        [RG_LANG_EN] = "On",
        [RG_LANG_FR] = "On",
        [RG_LANG_DE] = "An",
    },
    {
        [RG_LANG_EN] = "Off",
        [RG_LANG_FR] = "Off",
        [RG_LANG_DE] = "Aus",
    },
    {
        [RG_LANG_EN] = "Horiz",
        [RG_LANG_FR] = "Horiz",
        [RG_LANG_DE] = "Horiz",
    },
    {
        [RG_LANG_EN] = "Vert",
        [RG_LANG_FR] = "Vert ",
        [RG_LANG_DE] = "Vert",
    },
    {
        [RG_LANG_EN] = "Both",
        [RG_LANG_FR] = "Tout",
        [RG_LANG_DE] = "Beide",
    },
    {
        [RG_LANG_EN] = "Fit",
        [RG_LANG_FR] = "Ajuster",
        [RG_LANG_DE] = "Einpassen",
    },
    {
        [RG_LANG_EN] = "Full ",
        [RG_LANG_FR] = "Remplir ",
        [RG_LANG_DE] = "Voll",
    },
    {
        [RG_LANG_EN] = "Zoom",
        [RG_LANG_FR] = "Zoomer",
        [RG_LANG_DE] = "Zoom",
    },

    // Led options
    {
        [RG_LANG_EN] = "LED options",
        [RG_LANG_FR] = "Options LED",
        [RG_LANG_DE] = "LED Optionen",
    },
    {
        [RG_LANG_EN] = "System activity",
        [RG_LANG_FR] = "Activité système",
        [RG_LANG_DE] = "Systemaktivität",
    },
    {
        [RG_LANG_EN] = "Disk activity",
        [RG_LANG_FR] = "Activité stockage",
        [RG_LANG_DE] = "SD-Aktivität",
    },
    {
        [RG_LANG_EN] = "Low battery",
        [RG_LANG_FR] = "Batterie basse",
        [RG_LANG_DE] = "Schwache Batterie",
    },
    {
        [RG_LANG_EN] = "Default",
        [RG_LANG_FR] = "Défaut",
        [RG_LANG_DE] = "Standard",
    },
    {
        [RG_LANG_EN] = "<None>",
        [RG_LANG_FR] = "<Aucun>",
        [RG_LANG_DE] = "<None>",
    },

    // Wifi
    {
        [RG_LANG_EN] = "Not connected",
        [RG_LANG_FR] = "Non connecté",
        [RG_LANG_DE] = "Nicht verbunden",
    },
    {
        [RG_LANG_EN] = "Connecting...",
        [RG_LANG_FR] = "Connexion...",
        [RG_LANG_DE] = "Verbinde...",
    },
    {
        [RG_LANG_EN] = "Disconnecting...",
        [RG_LANG_FR] = "Déconnexion...",
        [RG_LANG_DE] = "Trennen...",
    },
    {
        [RG_LANG_EN] = "(empty)",
        [RG_LANG_FR] = "(vide)",
        [RG_LANG_DE] = "(leer)",
    },
    {
        [RG_LANG_EN] = "Wi-Fi AP",
        [RG_LANG_FR] = "Wi-Fi AP",
        [RG_LANG_DE] = "Wi-Fi AP",
    },
    {
        [RG_LANG_EN] = "Start access point?\n\nSSID: retro-go\nPassword: retro-go\n\nBrowse: http://192.168.4.1/",
        [RG_LANG_FR] = "Démarrer point d'accès?\n\nSSID: retro-go\nPassword: retro-go\n\nAdresse: http://192.168.4.1/",
        [RG_LANG_DE] = "Starte Hotspot?\n\nSSID: retro-go\nPasswort: retro-go\n\nAdresse: http://192.168.4.1/",
    },
    {
        [RG_LANG_EN] = "Wi-Fi enable",
        [RG_LANG_FR] = "Activer Wi-Fi",
        [RG_LANG_DE] = "Wi-Fi aktivieren",
    },
    {
        [RG_LANG_EN] = "Wi-Fi access point",
        [RG_LANG_FR] = "Point d'accès WiFi",
        [RG_LANG_DE] = "Wi-Fi Hotspot",
    },
    {
        [RG_LANG_EN] = "Network",
        [RG_LANG_FR] = "Réseau",
        [RG_LANG_DE] = "Netzwerk",
    },
    {
        [RG_LANG_EN] = "IP address",
        [RG_LANG_FR] = "Adresse IP",
        [RG_LANG_DE] = "IP-Adresse",
    },

    // retro-go settings
    {
        [RG_LANG_EN] = "Brightness",
        [RG_LANG_FR] = "Luminosité",
        [RG_LANG_DE] = "Helligkeit",
    },
    {
        [RG_LANG_EN] = "Volume",
        [RG_LANG_FR] = "Volume",
        [RG_LANG_DE] = "Lautstärke",
    },
    {
        [RG_LANG_EN] = "Audio out",
        [RG_LANG_FR] = "Sortie audio",
        [RG_LANG_DE] = "Audioausgang",
    },
    {
        [RG_LANG_EN] = "Font type",
        [RG_LANG_FR] = "Police",
        [RG_LANG_DE] = "Schriftart",
    },
    {
        [RG_LANG_EN] = "Theme",
        [RG_LANG_FR] = "Thème",
        [RG_LANG_DE] = "Thema",
    },
    {
        [RG_LANG_EN] = "Show clock",
        [RG_LANG_FR] = "Horloge",
        [RG_LANG_DE] = "Uhr anzeigen",
    },
    {
        [RG_LANG_EN] = "Timezone",
        [RG_LANG_FR] = "Fuseau",
        [RG_LANG_DE] = "Zeitzone",
    },
    {
        [RG_LANG_EN] = "Wi-Fi options",
        [RG_LANG_FR] = "Options Wi-Fi",
        [RG_LANG_DE] = "Wi-Fi Optionen",
    },

    // app settings
    {
        [RG_LANG_EN] = "Scaling",
        [RG_LANG_FR] = "Format",
        [RG_LANG_DE] = "Skalierung",
    },
    {
        [RG_LANG_EN] = "Factor",
        [RG_LANG_FR] = "Factor",
        [RG_LANG_DE] = "Faktor",
    },
    {
        [RG_LANG_EN] = "Filter",
        [RG_LANG_FR] = "Filtre",
        [RG_LANG_DE] = "Filter",
    },
    {
        [RG_LANG_EN] = "Border",
        [RG_LANG_FR] = "Bordure",
        [RG_LANG_DE] = "Rand",
    },
    {
        [RG_LANG_EN] = "Speed",
        [RG_LANG_FR] = "Vitesse",
        [RG_LANG_DE] = "Geschwindigkeit",
    },

    // about menu
    {
        [RG_LANG_EN] = "Version",
        [RG_LANG_FR] = "Version",
        [RG_LANG_DE] = "Version",
    },
    {
        [RG_LANG_EN] = "Date",
        [RG_LANG_FR] = "Date",
        [RG_LANG_DE] = "Datum",
    },
    {
        [RG_LANG_EN] = "Target",
        [RG_LANG_FR] = "Appareil",
        [RG_LANG_DE] = "Ziel",
    },
    {
        [RG_LANG_EN] = "Website",
        [RG_LANG_FR] = "Site Web",
        [RG_LANG_DE] = "Webseite",
    },
    {
        [RG_LANG_EN] = "Options",
        [RG_LANG_FR] = "Options",
        [RG_LANG_DE] = "Optionen",
    },
    {
        [RG_LANG_EN] = "View credits",
        [RG_LANG_FR] = "Credits",
        [RG_LANG_DE] = "Credits",
    },
    {
        [RG_LANG_EN] = "Debug menu",
        [RG_LANG_FR] = "Menu debug",
        [RG_LANG_DE] = "Debug Menü",
    },
    {
        [RG_LANG_EN] = "Reset settings",
        [RG_LANG_FR] = "Reset paramètres",
        [RG_LANG_DE] = "Einstellungen zurücksetzen",
    },

    // save slot
    {
        [RG_LANG_EN] = "Slot 0",
        [RG_LANG_FR] = "Emplacement 0",
        [RG_LANG_DE] = "Speicherplatz 0",
    },
    {
        [RG_LANG_EN] = "Slot 1",
        [RG_LANG_FR] = "Emplacement 1",
        [RG_LANG_DE] = "Speicherplatz 1",
    },
    {
        [RG_LANG_EN] = "Slot 2",
        [RG_LANG_FR] = "Emplacement 2",
        [RG_LANG_DE] = "Speicherplatz 2",

    },
    {
        [RG_LANG_EN] = "Slot 3",
        [RG_LANG_FR] = "Emplacement 3",
        [RG_LANG_DE] = "Speicherplatz 3",
    },

    // game menu
    {
        [RG_LANG_EN] = "Save & Continue",
        [RG_LANG_FR] = "Sauver et continuer",
        [RG_LANG_DE] = "Speichern und weiter",
    },
    {
        [RG_LANG_EN] = "Save & Quit",
        [RG_LANG_FR] = "Sauver et quitter",
        [RG_LANG_DE] = "Speichern und Verlassen",
    },
    {
        [RG_LANG_EN] = "Load game",
        [RG_LANG_FR] = "Charger partie",
        [RG_LANG_DE] = "Spiel laden",
    },
    {
        [RG_LANG_EN] = "Reset",
        [RG_LANG_FR] = "Reset",
        [RG_LANG_DE] = "Reset",
    },
    {
        [RG_LANG_EN] = "Netplay",
        [RG_LANG_FR] = "Netplay",
        [RG_LANG_DE] = "Netplay",
    },
    {
        [RG_LANG_EN] = "About",
        [RG_LANG_FR] = "Infos",
        [RG_LANG_DE] = "Infos",
    },
    {
        [RG_LANG_EN] = "Quit",
        [RG_LANG_FR] = "Quitter",
        [RG_LANG_DE] = "Verlassen",
    },
    {
        [RG_LANG_EN] = "Soft reset",
        [RG_LANG_FR] = "Soft reset",
        [RG_LANG_DE] = "Soft reset",
    },
    {
        [RG_LANG_EN] = "Hard reset",
        [RG_LANG_FR] = "Hard reset",
        [RG_LANG_DE] = "Hard reset",
    },

    {
        [RG_LANG_EN] = "Reset Emulation?",
        [RG_LANG_FR] = "Reset Emulation?",
        [RG_LANG_DE] = "Emulation zurücksetzen?",
    },
    {
        [RG_LANG_EN] = "Save",
        [RG_LANG_FR] = "Sauver",
        [RG_LANG_DE] = "Speichern",
    },
    {
        [RG_LANG_EN] = "Load",
        [RG_LANG_FR] = "Charger",
        [RG_LANG_DE] = "Laden",
    },
    // end of rg_gui.c


    // main.c
    {
        [RG_LANG_EN] = "Show",
        [RG_LANG_FR] = "Montrer",
        [RG_LANG_DE] = "Zeigen",
    },
    {
        [RG_LANG_EN] = "Hide",
        [RG_LANG_FR] = "Cacher",
        [RG_LANG_DE] = "Verstecken",
    },
    {
        [RG_LANG_EN] = "Tabs Visibility",
        [RG_LANG_FR] = "Visibilité onglets",
        [RG_LANG_DE] = "Tabs sichtbar",
    },

    // scroll modes
    {
        [RG_LANG_EN] = "Center",
        [RG_LANG_FR] = "centrer",
        [RG_LANG_DE] = "Zentriert",
    },
    {
        [RG_LANG_EN] = "Paging",
        [RG_LANG_FR] = "Paging",
        [RG_LANG_DE] = "Blättern",
    },

    // start screen
    {
        [RG_LANG_EN] = "Auto",
        [RG_LANG_FR] = "Auto",
        [RG_LANG_DE] = "Auto",
    },
    {
        [RG_LANG_EN] = "Carousel",
        [RG_LANG_FR] = "Carousel",
        [RG_LANG_DE] = "Carousel",
    },
    {
        [RG_LANG_EN] = "Browser",
        [RG_LANG_FR] = "Browser",
        [RG_LANG_DE] = "Browser",
    },

    // preview
    {
        [RG_LANG_EN] = "None",
        [RG_LANG_FR] = "Aucun",
        [RG_LANG_DE] = "Nichts",
    },
    {
        [RG_LANG_EN] = "Cover,Save",
        [RG_LANG_FR] = "Cover,Save",
        [RG_LANG_DE] = "Cover, Speicherstand",
    },
    {
        [RG_LANG_EN] = "Save,Cover",
        [RG_LANG_FR] = "Save,Cover",
        [RG_LANG_DE] = "Speicherstand, Cover",
    },
    {
        [RG_LANG_EN] = "Cover only",
        [RG_LANG_FR] = "Cover only",
        [RG_LANG_DE] = "Nur Cover",
    },
    {
        [RG_LANG_EN] = "Save only",
        [RG_LANG_FR] = "Save only",
        [RG_LANG_DE] = "Nur Save",
    },

    // startup app
    {
        [RG_LANG_EN] = "Last game",
        [RG_LANG_FR] = "Dernier jeu",
        [RG_LANG_DE] = "Letztes Spiel",
    },
    {
        [RG_LANG_EN] = "Launcher",
        [RG_LANG_FR] = "Launcher",
        [RG_LANG_DE] = "Launcher",
    },

    // launcher options
    {
        [RG_LANG_EN] = "Launcher Options",
        [RG_LANG_FR] = "Options du lanceur",
        [RG_LANG_DE] = "Launcher Optionen",
    },
    {
        [RG_LANG_EN] = "Color theme",
        [RG_LANG_FR] = "Couleurs",
        [RG_LANG_DE] = "Farbthema",
    },
    {
        [RG_LANG_EN] = "Preview",
        [RG_LANG_FR] = "Aperçu",
        [RG_LANG_DE] = "Vorschau",
    },
    {
        [RG_LANG_EN] = "Scroll mode",
        [RG_LANG_FR] = "Mode défilement",
        [RG_LANG_DE] = "Scroll-Modus",
    },
    {
        [RG_LANG_EN] = "Start screen",
        [RG_LANG_FR] = "Écran démarrage",
        [RG_LANG_DE] = "Startbildschirm",
    },
    {
        [RG_LANG_EN] = "Hide tabs",
        [RG_LANG_FR] = "Cacher onglet",
        [RG_LANG_DE] = "Tabs verstecken",
    },
    {
        [RG_LANG_EN] = "File server",
        [RG_LANG_FR] = "Serveur fichier",
        [RG_LANG_DE] = "Datei Server",
    },
    {
        [RG_LANG_EN] = "Startup app",
        [RG_LANG_FR] = "App démarrage",
        [RG_LANG_DE] = "Start-App",
    },
    {
        [RG_LANG_EN] = "Build CRC cache",
        [RG_LANG_FR] = "Build CRC cache",
        [RG_LANG_DE] = "Build CRC cache",
    },
    {
        [RG_LANG_EN] = "Check for updates",
        [RG_LANG_FR] = "Verifier mise à jour",
        [RG_LANG_DE] = "Auf Updates prüfen",
    },
    {
        [RG_LANG_EN] = "HTTP Server Busy...",
        [RG_LANG_FR] = "Server Web ...",
        [RG_LANG_DE] = "HTTP Server Busy...",
    },
    {
        [RG_LANG_EN] = "SD Card Error",
        [RG_LANG_FR] = "Erreur carte SD",
        [RG_LANG_DE] = "SD-Card Fehler!",
    },
    {
        [RG_LANG_EN] = "Storage mount failed.\nMake sure the card is FAT32.",
        [RG_LANG_FR] = "Erreur carte SD.\nLa carte est bien en FAT32 ?",
        [RG_LANG_DE] = "Speicherbereitstellung fehlgeschlagen.\nStellen Sie sicher, dass die Karte FAT32 ist.",
    },
    // end of main.c


    // applications.c
    {
        [RG_LANG_EN] = "Scanning %s %d/%d",
        [RG_LANG_FR] = "Scan %s %d/%d",
        [RG_LANG_DE] = "Scannen %s %d/%d",
    },
    // message when no rom
    {
        [RG_LANG_EN] = "Welcome to Retro-Go!",
        [RG_LANG_FR] = "Bienvenue sur Retro-Go!",
        [RG_LANG_DE] = "Wilkommen bei Retro-Go!",
    },
    {
        [RG_LANG_EN] = "Place roms in folder: %s",
        [RG_LANG_FR] = "Placer les ROMS dans le dossier: %s",
        [RG_LANG_DE] = "Lege ROMs in den Ordner: %s",
    },
    {
        [RG_LANG_EN] = "With file extension: %s",
        [RG_LANG_FR] = "Avec l'extension: %s",
        [RG_LANG_DE] = "Mit Dateiendung: %s",
    },
    {
        [RG_LANG_EN] = "You can hide this tab in the menu",
        [RG_LANG_FR] = "Vous pouvez cacher cet onglet dans le menu",
        [RG_LANG_DE] = "Sie können diesen Reiter im Menü ausblenden",
    },
    {
        [RG_LANG_EN] = "You have no %s games",
        [RG_LANG_FR] = "Vous n'avez pas de jeux %s",
        [RG_LANG_DE] = "Du hast keine %s Spiele",
    },
    {
        [RG_LANG_EN] = "File not found",
        [RG_LANG_FR] = "Fichier non présent",
        [RG_LANG_DE] = "Datei nicht gefunden",
    },

    // rom options
    {
        [RG_LANG_EN] = "Name",
        [RG_LANG_FR] = "Nom",
        [RG_LANG_DE] = "Name",
    },
    {
        [RG_LANG_EN] = "Folder",
        [RG_LANG_FR] = "Dossier",
        [RG_LANG_DE] = "Ordner",
    },
    {
        [RG_LANG_EN] = "Size",
        [RG_LANG_FR] = "Taille",
        [RG_LANG_DE] = "Größe",
    },
    {
        [RG_LANG_EN] = "CRC32",
        [RG_LANG_FR] = "CRC32",
        [RG_LANG_DE] = "CRC32",
    },
    {
        [RG_LANG_EN] = "Delete file",
        [RG_LANG_FR] = "Supprimer fichier",
        [RG_LANG_DE] = "Datei löschen",
    },
    {
        [RG_LANG_EN] = "Close",
        [RG_LANG_FR] = "Fermer",
        [RG_LANG_DE] = "Schließen",
    },
    {
        [RG_LANG_EN] = "File properties",
        [RG_LANG_FR] = "Propriétés fichier",
        [RG_LANG_DE] = "Dateieigenschaften",
    },
    {
        [RG_LANG_EN] = "Delete selected file?",
        [RG_LANG_FR] = "Supprimer fichier?",
        [RG_LANG_DE] = "Ausgewählte Datei löschen?",
    },


    // in-game menu
    {
        [RG_LANG_EN] = "Resume game",
        [RG_LANG_FR] = "Reprendre partie",
        [RG_LANG_DE] = "Fortsetzen",
    },
    {
        [RG_LANG_EN] = "New game",
        [RG_LANG_FR] = "Nouvelle partie",
        [RG_LANG_DE] = "Neues Spiel",
    },
    {
        [RG_LANG_EN] = "Del favorite",
        [RG_LANG_FR] = "supp favori",
        [RG_LANG_DE] = "Favorit löschen",
    },
    {
        [RG_LANG_EN] = "Add favorite",
        [RG_LANG_FR] = "Ajouter favori",
        [RG_LANG_DE] = "Favorit hinzufügen",
    },
    {
        [RG_LANG_EN] = "Delete save",
        [RG_LANG_FR] = "Supp sauvegarde",
        [RG_LANG_DE] = "Speicherstand löschen?",
    },
    {
        [RG_LANG_EN] = "Properties",
        [RG_LANG_FR] = "Propriétés",
        [RG_LANG_DE] = "Eigenschaften",
    },
    {
        [RG_LANG_EN] = "Resume",
        [RG_LANG_FR] = "Reprendre",
        [RG_LANG_DE] = "Fortsetzen",
    },
    {
        [RG_LANG_EN] = "Delete save?",
        [RG_LANG_FR] = "Supp sauvegarde?",
        [RG_LANG_DE] = "Speicherstand löschen?",
    },
    {
        [RG_LANG_EN] = "Delete sram file?",
        [RG_LANG_FR] = "Supp fichier SRAM?",
        [RG_LANG_DE] = "SRAM-Datei löschen?",
    },
    // end of applications.c


    // rg_system.c
    {
        [RG_LANG_EN] = "App unresponsive... Hold MENU to quit!",
        [RG_LANG_FR] = "Plantage... MENU pour quitter",
        [RG_LANG_DE] = "App reagiert nicht... Halte MENU zum Beenden",
    },
    {
        [RG_LANG_EN] = "Reset all settings",
        [RG_LANG_FR] = "Reset paramètres",
        [RG_LANG_DE] = "Einstellungen zurücksetzen",
    },
    {
        [RG_LANG_EN] = "Reboot to factory ",
        [RG_LANG_FR] = "Redémarrer usine",
        [RG_LANG_DE] = "Neustart mit Werkseinstellungen",
    },
    {
        [RG_LANG_EN] = "Reboot to launcher",
        [RG_LANG_FR] = "Relancer launcher",
        [RG_LANG_DE] = "Neustart zum Launcher",
    },
    {
        [RG_LANG_EN] = "Recovery mode",
        [RG_LANG_FR] = "Mode de récupération",
        [RG_LANG_DE] = "Wiederherstellungsmodus",
    },
    {
        [RG_LANG_EN] = "System Panic!",
        [RG_LANG_FR] = "Plantage système!",
        [RG_LANG_DE] = "Systempanik!",
    },
    {
        [RG_LANG_EN] = "Save failed",
        [RG_LANG_FR] = "Erreur sauvegarde",
        [RG_LANG_DE] = "Speichern fehlgeschlagen",
    },
    // end of rg_system.c
};
