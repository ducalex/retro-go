/**
 * IMPORTANT: This file must be opened and saved as ISO 8859-1 (Latin-1)!
 * Retro-Go does NOT understand UTF-8 or any other encoding.
 * If the following looks like gibberish, your encoding is wrong: � � � � � � � �
 */

#include "rg_localization.h"

static const char *language_names[RG_LANG_MAX] = {"English", "Francais"};

static const char *translations[][RG_LANG_MAX] =
{
    {
        [RG_LANG_EN] = "Never",
        [RG_LANG_FR] = "Jamais",
        [RG_LANG_ES] = "Nunca",
    },
    {
        [RG_LANG_EN] = "Always",
        [RG_LANG_FR] = "Toujours",
        [RG_LANG_ES] = "Siempre",
    },
    {
        [RG_LANG_EN] = "Composite",
        [RG_LANG_FR] = "Composite",
        [RG_LANG_ES] = "Composite",
    },
    {
        [RG_LANG_EN] = "NES Classic",
        [RG_LANG_FR] = "NES Classic",
        [RG_LANG_ES] = "NES Classic",
    },
    {
        [RG_LANG_EN] = "NTSC",
        [RG_LANG_FR] = "NTSC",
        [RG_LANG_ES] = "NTSC",
    },
    {
        [RG_LANG_EN] = "PVM",
        [RG_LANG_FR] = "PVM",
        [RG_LANG_ES] = "PVM",
    },
    {
        [RG_LANG_EN] = "Smooth",
        [RG_LANG_FR] = "Lisser",
        [RG_LANG_ES] = "Suave",
    },
    {
        [RG_LANG_EN] = "To start, try: 1 or * or #",
        [RG_LANG_FR] = "Pour commencer, 1 ou * ou #",
        [RG_LANG_ES] = "Para iniciar, 1 o * o #",
    },
    {
	    [RG_LANG_EN] = "Full",
	    [RG_LANG_FR] = "Complet",
        [RG_LANG_ES] = "Completo",
    },
    {
        [RG_LANG_EN] = "Yes",
        [RG_LANG_FR] = "Oui",
        [RG_LANG_ES] = "Si",
    },
    {
        [RG_LANG_EN] = "Select file",
        [RG_LANG_FR] = "Choisissez un fichier",
        [RG_LANG_ES] = "Seleccione un archivo",
    },
    {
        [RG_LANG_EN] = "Off",
        [RG_LANG_FR] = "Off",
        [RG_LANG_ES] = "Apagado",
    },
    {
        [RG_LANG_EN] = "Language",
        [RG_LANG_FR] = "Langue",
        [RG_LANG_ES] = "Idioma",
    },
    {
        [RG_LANG_EN] = "Language changed!",
        [RG_LANG_FR] = "Changement de langue",
        [RG_LANG_ES] = "Idioma cambiado!",
    },
    {
        [RG_LANG_EN] = "For these changes to take effect you must restart your device.\nrestart now?",
        [RG_LANG_FR] = "Pour que ces changements prennent effet, vous devez redemmarer votre appareil.\nRedemmarer maintenant ?",
        [RG_LANG_ES] = "Para que estos cambios tengan efecto, debe reiniciar su dispositivo.\n¿Reiniciar ahora?",
    },
    {
        [RG_LANG_EN] = "Wi-Fi profile",
        [RG_LANG_FR] = "Profile Wi-Fi",
        [RG_LANG_ES] = "Perfil Wi-Fi",
    },
    {
        [RG_LANG_EN] = "Language",
        [RG_LANG_FR] = "Langue",
        [RG_LANG_ES] = "Idioma",
    },
    {
        [RG_LANG_EN] = "Options",
        [RG_LANG_FR] = "Options",
        [RG_LANG_ES] = "Opciones",
    },
    {
        [RG_LANG_EN] = "About Retro-Go",
        [RG_LANG_FR] = "A propos de Retro-go",
        [RG_LANG_ES] = "Acerca de Retro-Go",
    },
    {
        [RG_LANG_EN] = "Reset all settings?",
        [RG_LANG_FR] = "Reset tous les parametres",
        [RG_LANG_ES] = "¿Restablecer todos los ajustes?",
    },
    {
        [RG_LANG_EN] = "Initializing...",
        [RG_LANG_FR] = "Initialisation...",
        [RG_LANG_ES] = "Inicializando...",
    },
    {
        [RG_LANG_EN] = "Host Game (P1)",
        [RG_LANG_FR] = "Anfitrión del juego (P1)",
        [RG_LANG_ES] = "Anfitrión (P1)",
    },
    {
        [RG_LANG_EN] = "Find Game (P2)",
        [RG_LANG_FR] = "Find Game (P2)",
        [RG_LANG_ES] = "Buscar Juego (P2)",
    },
    {
        [RG_LANG_EN] = "Netplay",
        [RG_LANG_FR] = "Netplay",
        [RG_LANG_ES] = "Netplay",

    },
    {
        [RG_LANG_EN] = "ROMs not identical. Continue?",
        [RG_LANG_FR] = "ROMs not identical. Continue?",
        [RG_LANG_ES] = "ROMs no son idénticos. ¿Continuar?",
    },
    {
        [RG_LANG_EN] = "Exchanging info...",
        [RG_LANG_FR] = "Exchanging info...".
        [RG_LANG_ES] = "Intercambiando información...",
    },
    {
        [RG_LANG_EN] = "Unable to find host!",
        [RG_LANG_FR] = "Unable to find host!",
        [RG_LANG_ES] = "¡No se puede encontrar al anfitrión!",
    },
    {
        [RG_LANG_EN] = "Connection failed!",
        [RG_LANG_FR] = "Connection failed!",
        [RG_LANG_ES] = "¡Conexión fallida!",
    },
    {
        [RG_LANG_EN] = "Waiting for peer...",
        [RG_LANG_FR] = "Waiting for peer...",
        [RG_LANG_ES] = "Esperando al compañero...",
    },
    {
        [RG_LANG_EN] = "Unknown status...",
        [RG_LANG_FR] = "Unknown status...",
        [RG_LANG_ES] = "Estado desconocido...",
    },
    {
        [RG_LANG_EN] = "On",
        [RG_LANG_FR] = "On",
        [RG_LANG_ES] = "Encendido",
    },
    {
        [RG_LANG_EN] = "Keyboard",
        [RG_LANG_FR] = "Clavier",
        [RG_LANG_ES] = "Teclado",
    },
    {
        [RG_LANG_EN] = "Joystick",
        [RG_LANG_FR] = "Joystick",
        [RG_LANG_ES] = "Joystick",
    },
    {
        [RG_LANG_EN] = "Input",
        [RG_LANG_FR] = "Entree",
        [RG_LANG_ES] = "Entrada",
    },
    {
        [RG_LANG_EN] = "Crop",
        [RG_LANG_FR] = "Couper",
        [RG_LANG_ES] = "Recortar",
    },
    {
        [RG_LANG_EN] = "BIOS file missing!",
        [RG_LANG_FR] = "Fichier BIOS manquant",
        [RG_LANG_ES] = "¡Archivo BIOS faltante!",
    },
    {
        [RG_LANG_EN] = "YM2612 audio ",
        [RG_LANG_FR] = "YM2612 audio ",
        [RG_LANG_ES] = "Audio YM2612",
    },
    {
        [RG_LANG_EN] = "SN76489 audio",
        [RG_LANG_FR] = "SN76489 audio",
        [RG_LANG_ES] = "Audio SN76489",
    },
    {
        [RG_LANG_EN] = "Z80 emulation",
        [RG_LANG_FR] = "Emulation Z80",
        [RG_LANG_ES] = "Emulación Z80",
    },
    {
        [RG_LANG_EN] = "Launcher options",
        [RG_LANG_FR] = "Options du lanceur",
        [RG_LANG_ES] = "Opciones del launcher",
    },
    {
        [RG_LANG_EN] = "Emulator options",
        [RG_LANG_FR] = "Options emulateur",
        [RG_LANG_ES] = "Opciones del emulador",
    },
    {
        [RG_LANG_EN] = "Date",
        [RG_LANG_FR] = "Date",
        [RG_LANG_ES] = "Fecha",
    },
    {
        [RG_LANG_EN] = "Files:",
        [RG_LANG_FR] = "Fichiers:",
        [RG_LANG_ES] = "Archivos:",
    },
    {
        [RG_LANG_EN] = "Download complete!",
        [RG_LANG_FR] = "Telechargement termine",
        [RG_LANG_ES] = "¡Descarga completa!",
    },
    {
        [RG_LANG_EN] = "Reboot to flash?",
        [RG_LANG_FR] = "Redemarrer",
        [RG_LANG_ES] = "¿Reiniciar para flashear?",
    },
    {
        [RG_LANG_EN] = "Available Releases",
        [RG_LANG_FR] = "Maj disponibles",
        [RG_LANG_ES] = "Actualizaciones disponibles",
    },
    {
        [RG_LANG_EN] = "Received empty list!",
        [RG_LANG_FR] = "Liste vide recue",
        [RG_LANG_ES] = "¡Lista vacía recibida!",
    },
    {
        [RG_LANG_EN] = "Gamma Boost",
        [RG_LANG_FR] = "Boost Gamma",
        [RG_LANG_ES] = "Gamma Boost",
    },
    {
        [RG_LANG_EN] = "Day",
        [RG_LANG_FR] = "Jour",
        [RG_LANG_ES] = "Día"
    },
    {
        [RG_LANG_EN] = "Hour",
        [RG_LANG_FR] = "Heure",
        [RG_LANG_ES] = "Hora"
    },
    {
        [RG_LANG_EN] = "Min",
        [RG_LANG_FR] = "Min",
        [RG_LANG_ES] = "Min"
    },
    {
        [RG_LANG_EN] = "Sec",
        [RG_LANG_FR] = "Sec",
        [RG_LANG_ES] = "Seg"
    },
    {
        [RG_LANG_EN] = "Sync",
        [RG_LANG_FR] = "Synchro",
        [RG_LANG_ES] = "Sincronizar"
    },
    {
        [RG_LANG_EN] = "RTC config",
        [RG_LANG_FR] = "Config RTC",
        [RG_LANG_ES] = "Configurar RTC"
    },
    {
        [RG_LANG_EN] = "Palette",
        [RG_LANG_FR] = "Palette",
        [RG_LANG_ES] = "Paleta"
    },
    {
        [RG_LANG_EN] = "RTC config",
        [RG_LANG_FR] = "Config RTC",
        [RG_LANG_ES] = "Configurar RTC"
    },
    {
        [RG_LANG_EN] = "SRAM autosave",
        [RG_LANG_FR] = "Sauvegarde auto SRAM",
        [RG_LANG_ES] = "Guardado automático SRAM"
    },
    {
        [RG_LANG_EN] = "Enable BIOS",
        [RG_LANG_FR] = "Activer BIOS",
        [RG_LANG_ES] = "Habilitar BIOS"
    },
    {
        [RG_LANG_EN] = "Name",
        [RG_LANG_FR] = "Nom",
        [RG_LANG_ES] = "Nombre"
    },
    {
        [RG_LANG_EN] = "Artist",
        [RG_LANG_FR] = "Artiste",
        [RG_LANG_ES] = "Artista"
    },
    {
        [RG_LANG_EN] = "Copyright",
        [RG_LANG_FR] = "Copyright",
        [RG_LANG_ES] = "Copyright"
    },
    {
        [RG_LANG_EN] = "Playing",
        [RG_LANG_FR] = "Playing",
        [RG_LANG_ES] = "Reproduciendo"
    },
    {
        [RG_LANG_EN] = "Palette",
        [RG_LANG_FR] = "Palette",
        [RG_LANG_ES] = "Paleta"
    },
    {
        [RG_LANG_EN] = "Overscan",
        [RG_LANG_FR] = "Overscan",
        [RG_LANG_ES] = "Overscan"
    },
    {
        [RG_LANG_EN] = "Crop sides",
        [RG_LANG_FR] = "Couper les cotes",
        [RG_LANG_ES] = "Recortar lados"
    },
    {
        [RG_LANG_EN] = "Sprite limit",
        [RG_LANG_FR] = "Limite de sprite",
        [RG_LANG_ES] = "Límite de sprites"
    },
    {
        [RG_LANG_EN] = "Overscan",
        [RG_LANG_FR] = "Overscan",
        [RG_LANG_ES] = "Overscan"
    },
    {
        [RG_LANG_EN] = "Palette",
        [RG_LANG_FR] = "Palette",
        [RG_LANG_ES] = "Paleta"
    },
    {
        [RG_LANG_EN] = "Profile",
        [RG_LANG_FR] = "Profile",
        [RG_LANG_ES] = "Perfil"
    },
    {
        [RG_LANG_EN] = "Controls",
        [RG_LANG_FR] = "Controles",
        [RG_LANG_ES] = "Controles"
    },
    {
        [RG_LANG_EN] = "Audio enable",
        [RG_LANG_FR] = "Activer audio",
        [RG_LANG_ES] = "Habilitar audio"
    },
    {
        [RG_LANG_EN] = "Audio filter",
        [RG_LANG_FR] = "Filtre audio",
        [RG_LANG_ES] = "Filtro de audio"
    },


    // rg_gui.c
    {
        [RG_LANG_EN] = "Folder is empty.",
        [RG_LANG_FR] = "Le dossier est vide.",
        [RG_LANG_ES] = "La carpeta está vacía.",
    },
    {
        [RG_LANG_EN] = "No",
        [RG_LANG_FR] = "Non",
        [RG_LANG_ES] = "No",
    },
    {
        [RG_LANG_EN] = "OK",
        [RG_LANG_FR] = "OK",
        [RG_LANG_ES] = "OK",
    },
    {
        [RG_LANG_EN] = "On",
        [RG_LANG_FR] = "On",
        [RG_LANG_ES] = "On",
    },
    {
        [RG_LANG_EN] = "Off",
        [RG_LANG_FR] = "Off",
        [RG_LANG_ES] = "Off",
    },
    {
        [RG_LANG_EN] = "Horiz",
        [RG_LANG_FR] = "Horiz",
        [RG_LANG_ES] = "Horiz",
    },
    {
        [RG_LANG_EN] = "Vert",
        [RG_LANG_FR] = "Vert",
        [RG_LANG_ES] = "Vert",
    },
    {
        [RG_LANG_EN] = "Both",
        [RG_LANG_FR] = "Tout",
        [RG_LANG_ES] = "Ambos",
    },
    {
        [RG_LANG_EN] = "Fit",
        [RG_LANG_FR] = "Ajuster",
        [RG_LANG_ES] = "Ajustar",
    },
    {
        [RG_LANG_EN] = "Full ",
        [RG_LANG_FR] = "Remplir ",
        [RG_LANG_ES] = "Completo ",
    },
    {
        [RG_LANG_EN] = "Zoom",
        [RG_LANG_FR] = "Zoomer",
        [RG_LANG_ES] = "Zoom",
    },

    // Led options
    {
        [RG_LANG_EN] = "LED options",
        [RG_LANG_FR] = "Options LED",
        [RG_LANG_ES] = "Opciones LED",
    },
    {
        [RG_LANG_EN] = "System activity",
        [RG_LANG_FR] = "Activite systeme",
        [RG_LANG_ES] = "Actividad del sistema",
    },
    {
        [RG_LANG_EN] = "Disk activity",
        [RG_LANG_FR] = "Activite stockage",
        [RG_LANG_ES] = "Actividad del disco",
    },
    {
        [RG_LANG_EN] = "Low battery",
        [RG_LANG_FR] = "Battery basse",
        [RG_LANG_ES] = "Batería baja",
    },
    {
        [RG_LANG_EN] = "Default",
        [RG_LANG_FR] = "Default",
        [RG_LANG_ES] = "Predeterminado",
    },
    {
        [RG_LANG_EN] = "none",
        [RG_LANG_FR] = "Aucun",
        [RG_LANG_ES] = "ninguno",
    },
    {
        [RG_LANG_EN] = "<None>",
        [RG_LANG_FR] = "<Aucun>",
        [RG_LANG_ES] = "<Ninguno>",
    },

    // Wifi
    {
        [RG_LANG_EN] = "Not connected",
        [RG_LANG_FR] = "Non connecte",
        [RG_LANG_ES] = "No conectado",
    },
    {
        [RG_LANG_EN] = "Connecting...",
        [RG_LANG_FR] = "Connection...",
        [RG_LANG_ES] = "Conectando...",
    },
    {
        [RG_LANG_EN] = "Disconnecting...",
        [RG_LANG_FR] = "Disconnection...",
        [RG_LANG_ES] = "Desconectando...",
    },
    {
        [RG_LANG_EN] = "(empty)",
        [RG_LANG_FR] = "(vide)",
        [RG_LANG_ES] = "(vacío)",
    },
    {
        [RG_LANG_EN] = "Wi-Fi AP",
        [RG_LANG_FR] = "Wi-Fi AP",
        [RG_LANG_ES] = "Wi-Fi AP",
    },
    {
        [RG_LANG_EN] = "Start access point?\n\nSSID: retro-go\nPassword: retro-go\n\nBrowse: http://192.168.4.1/",
        [RG_LANG_FR] = "Demarrer point d'acces?\n\nSSID: retro-go\nPassword: retro-go\n\nAdresse: http://192.168.4.1/",
        [RG_LANG_ES] = "¿Iniciar punto de acceso?\n\nSSID: retro-go\nContraseña: retro-go\n\nNavegar: http://192.168.4.1/"
    },
    {
        [RG_LANG_EN] = "Wi-Fi enable",
        [RG_LANG_FR] = "Activer Wi-Fi",
        [RG_LANG_ES] = "Habilitar Wi-Fi"
    },
    {
        [RG_LANG_EN] = "Wi-Fi access point",
        [RG_LANG_FR] = "Point d'acces WiFi",
        [RG_LANG_ES] = "Punto de acceso Wi-Fi"
    },
    {
        [RG_LANG_EN] = "Network",
        [RG_LANG_FR] = "Reseau",
        [RG_LANG_ES] = "Red"
    },
    {
        [RG_LANG_EN] = "IP address",
        [RG_LANG_FR] = "Adresse IP",
        [RG_LANG_ES] = "Dirección IP"
    },

    // retro-go settings
    {
        [RG_LANG_EN] = "Brightness",
        [RG_LANG_FR] = "Luminosite",
        [RG_LANG_ES] = "Brillo"
    },
    {
        [RG_LANG_EN] = "Volume",
        [RG_LANG_FR] = "Volume",
        [RG_LANG_ES] = "Volumen"
    },
    {
        [RG_LANG_EN] = "Audio out",
        [RG_LANG_FR] = "Sortie audio",
        [RG_LANG_ES] = "Salida de audio"
    },
    {
        [RG_LANG_EN] = "Font type",
        [RG_LANG_FR] = "Police",
        [RG_LANG_ES] = "Tipo de fuente"
    },
    {
        [RG_LANG_EN] = "Theme",
        [RG_LANG_FR] = "Theme",
        [RG_LANG_ES] = "Tema"
    },
    {
        [RG_LANG_EN] = "Show clock",
        [RG_LANG_FR] = "Horloge",
        [RG_LANG_ES] = "Mostrar reloj"
    },
    {
        [RG_LANG_EN] = "Timezone",
        [RG_LANG_FR] = "Fuseau",
        [RG_LANG_ES] = "Zona horaria"
    },
    {
        [RG_LANG_EN] = "Wi-Fi options",
        [RG_LANG_FR] = "Options Wi-Fi",
        [RG_LANG_ES] = "Opciones Wi-Fi"
    },

    // app dettings
    {
        [RG_LANG_EN] = "Scaling",
        [RG_LANG_FR] = "Format",
        [RG_LANG_ES] = "Escalado"
    },
    {
        [RG_LANG_EN] = "Factor",
        [RG_LANG_FR] = "Factor",
        [RG_LANG_ES] = "Factor"
    },
    {
        [RG_LANG_EN] = "Filter",
        [RG_LANG_FR] = "Filtre",
        [RG_LANG_ES] = "Filtro"
    },
    {
        [RG_LANG_EN] = "Border",
        [RG_LANG_FR] = "Bordure",
        [RG_LANG_ES] = "Borde"
    },
    {
        [RG_LANG_EN] = "Speed",
        [RG_LANG_FR] = "Vitesse",
        [RG_LANG_ES] = "Velocidad"
    },

    // about menu
    {
        [RG_LANG_EN] = "Version",
        [RG_LANG_FR] = "Version",
        [RG_LANG_ES] = "Versión"
    },
    {
        [RG_LANG_EN] = "Date",
        [RG_LANG_FR] = "Date",
        [RG_LANG_ES] = "Fecha"
    },
    {
        [RG_LANG_EN] = "Target",
        [RG_LANG_FR] = "Cible",
        [RG_LANG_ES] = "Objetivo"
    },
    {
        [RG_LANG_EN] = "Website",
        [RG_LANG_FR] = "Site Web",
        [RG_LANG_ES] = "Sitio web"
    },
    {
        [RG_LANG_EN] = "Options",
        [RG_LANG_FR] = "Options",
        [RG_LANG_ES] = "Opciones"
    },
    {
        [RG_LANG_EN] = "View credits",
        [RG_LANG_FR] = "Credits",
        [RG_LANG_ES] = "Ver créditos"
    },
    {
        [RG_LANG_EN] = "Debug menu",
        [RG_LANG_FR] = "Menu debug",
        [RG_LANG_ES] = "Menú de depuración"
    },
    {
        [RG_LANG_EN] = "Reset settings",
        [RG_LANG_FR] = "Reset parametres",
        [RG_LANG_ES] = "Restablecer ajustes"
    },

    // save slot
    {
        [RG_LANG_EN] = "Slot 0",
        [RG_LANG_FR] = "Emplacement 0",
        [RG_LANG_ES] = "Ranura 0"
    },
    {
        [RG_LANG_EN] = "Slot 1",
        [RG_LANG_FR] = "Emplacement 1",
        [RG_LANG_ES] = "Ranura 1"
    },
    {
        [RG_LANG_EN] = "Slot 2",
        [RG_LANG_FR] = "Emplacement 2",
        [RG_LANG_ES] = "Ranura 2"
    },
    {
        [RG_LANG_EN] = "Slot 3",
        [RG_LANG_FR] = "Emplacement 3",
        [RG_LANG_ES] = "Ranura 3"
    },

    // game menu
    {
        [RG_LANG_EN] = "Save & Continue",
        [RG_LANG_FR] = "Sauver et continuer",
        [RG_LANG_ES] = "Guardar y continuar"
    },
    {
        [RG_LANG_EN] = "Save & Quit",
        [RG_LANG_FR] = "Sauver et quitter",
        [RG_LANG_ES] = "Guardar y salir"
    },
    {
        [RG_LANG_EN] = "Load game",
        [RG_LANG_FR] = "Charger partie",
        [RG_LANG_ES] = "Cargar juego"
    },
    {
        [RG_LANG_EN] = "Reset",
        [RG_LANG_FR] = "Reset",
        [RG_LANG_ES] = "Reiniciar"
    },
    {
        [RG_LANG_EN] = "Netplay",
        [RG_LANG_FR] = "Netplay",
        [RG_LANG_ES] = "Netplay"
    },
    {
        [RG_LANG_EN] = "About",
        [RG_LANG_FR] = "Infos",
        [RG_LANG_ES] = "Acerca de"
    },
    {
        [RG_LANG_EN] = "Quit",
        [RG_LANG_FR] = "Quitter",
        [RG_LANG_ES] = "Salir"
    },
    {
        [RG_LANG_EN] = "Soft reset",
        [RG_LANG_FR] = "Soft reset",
        [RG_LANG_ES] = "Reinicio suave"
    },
    {
        [RG_LANG_EN] = "Hard reset",
        [RG_LANG_FR] = "Hard reset",
        [RG_LANG_ES] = "Reinicio duro"
    },

    {
        [RG_LANG_EN] = "Reset Emulation?",
        [RG_LANG_FR] = "Reset Emulation?",
        [RG_LANG_ES] = "¿Reiniciar emulación?"
    },
    {
        [RG_LANG_EN] = "Save",
        [RG_LANG_FR] = "Sauver",
        [RG_LANG_ES] = "Guardar"
    },
    {
        [RG_LANG_EN] = "Load",
        [RG_LANG_FR] = "Charger",
        [RG_LANG_ES] = "Cargar"
    },
    // end of rg_gui.c


    // main.c
    {
        [RG_LANG_EN] = "Show",
        [RG_LANG_FR] = "Montrer",
        [RG_LANG_ES] = "Mostrar"
    },
    {
        [RG_LANG_EN] = "Hide",
        [RG_LANG_FR] = "Cacher",
        [RG_LANG_ES] = "Ocultar"
    },

    // scroll modes
    {
        [RG_LANG_EN] = "Center",
        [RG_LANG_FR] = "centrer",
        [RG_LANG_ES] = "Centrar"
    },
    {
        [RG_LANG_EN] = "Paging",
        [RG_LANG_FR] = "Paging",
        [RG_LANG_ES] = "Paginación"
    },

    // start screen
    {
        [RG_LANG_EN] = "Auto",
        [RG_LANG_FR] = "Auto",
        [RG_LANG_ES] = "Auto"
    },
    {
        [RG_LANG_EN] = "Carousel",
        [RG_LANG_FR] = "Carousel",
        [RG_LANG_ES] = "Carrusel"
    },
    {
        [RG_LANG_EN] = "Browser",
        [RG_LANG_FR] = "Browser",
        [RG_LANG_ES] = "Navegador"
    },

    // preview
    {
        [RG_LANG_EN] = "None",
        [RG_LANG_FR] = "Aucun",
        [RG_LANG_ES] = "Ninguno"
    },
    {
        [RG_LANG_EN] = "Cover,Save",
        [RG_LANG_FR] = "Cover,Save",
        [RG_LANG_ES] = "Portada,Guardar"
    },
    {
        [RG_LANG_EN] = "Save,Cover",
        [RG_LANG_FR] = "Save,Cover",
        [RG_LANG_ES] = "Guardar,Portada"
    },
    {
        [RG_LANG_EN] = "Cover only",
        [RG_LANG_FR] = "Cover only",
        [RG_LANG_ES] = "Portada solamente"
    },
    {
        [RG_LANG_EN] = "Save only",
        [RG_LANG_FR] = "Save only",
        [RG_LANG_ES] = "Guardar solamente"
    },

    // startup app
    {
        [RG_LANG_EN] = "Last game",
        [RG_LANG_FR] = "Dernier jeu",
        [RG_LANG_ES] = "Último juego"
    },
    {
        [RG_LANG_EN] = "Launcher",
        [RG_LANG_FR] = "Launcher",
        [RG_LANG_ES] = "Launcher"
    },

    // launcher options
    {
        [RG_LANG_EN] = "Color theme",
        [RG_LANG_FR] = "Couleurs",
        [RG_LANG_ES] = "Tema de color"
    },
    {
        [RG_LANG_EN] = "Preview",
        [RG_LANG_FR] = "Apercu",
        [RG_LANG_ES] = "Vista previa"
    },
    {
        [RG_LANG_EN] = "Scroll mode",
        [RG_LANG_FR] = "Mode defilement",
        [RG_LANG_ES] = "Modo de desplazamiento"
    },
    {
        [RG_LANG_EN] = "Start screen",
        [RG_LANG_FR] = "Ecran demarrage",
        [RG_LANG_ES] = "Pantalla de inicio"
    },
    {
        [RG_LANG_EN] = "Hide tabs",
        [RG_LANG_FR] = "Cacher onglet",
        [RG_LANG_ES] = "Ocultar pestañas"
    },
    {
        [RG_LANG_EN] = "File server",
        [RG_LANG_FR] = "Serveur fichier",
        [RG_LANG_ES] = "Servidor de archivos"
    },
    {
        [RG_LANG_EN] = "Startup app",
        [RG_LANG_FR] = "App demarrage",
        [RG_LANG_ES] = "Aplicación de inicio"
    },
    {
        [RG_LANG_EN] = "Build CRC cache",
        [RG_LANG_FR] = "Build CRC cache",
        [RG_LANG_ES] = "Construir caché CRC"
    },
    {
        [RG_LANG_EN] = "Check for updates",
        [RG_LANG_FR] = "Check for updates",
        [RG_LANG_ES] = "Buscar actualizaciones"
    },
    {
        [RG_LANG_EN] = "HTTP Server Busy...",
        [RG_LANG_FR] = "Server Web ...",
        [RG_LANG_ES] = "Servidor HTTP ocupado..."
    },
    {
        [RG_LANG_EN] = "SD Card Error",
        [RG_LANG_FR] = "Erreur carte SD",
        [RG_LANG_ES] = "Error de tarjeta SD"
    },
    {
        [RG_LANG_EN] = "Storage mount failed.\nMake sure the card is FAT32.",
        [RG_LANG_FR] = "Erreur montage SD.\nVerifiez que la carte est en FAT32.",
        [RG_LANG_ES] = "Error de montaje de almacenamiento.\nAsegúrese de que la tarjeta esté en FAT32."
    },
    // end of main.c


    // applications.c
    {
        [RG_LANG_EN] = "Scanning %s %d/%d",
        [RG_LANG_FR] = "Scan %s %d/%d",
        [RG_LANG_ES] = "Escaneando %s %d/%d"
    },
    // message when no rom
    {
        [RG_LANG_EN] = "Welcome to Retro-Go!",
        [RG_LANG_FR] = "Bienvenue sur Retro-Go!",
        [RG_LANG_ES] = "¡Bienvenido a Retro-Go!"
    },
    {
        [RG_LANG_EN] = "Place roms in folder: %s",
        [RG_LANG_FR] = "Placer les ROMS dans le dossier: %s",
        [RG_LANG_ES] = "Coloque las ROMs en la carpeta: %s"
    },
    {
        [RG_LANG_EN] = "With file extension: %s",
        [RG_LANG_FR] = "Avec l'extension: %s",
        [RG_LANG_ES] = "Con la extensión de archivo: %s"
    },
    {
        [RG_LANG_EN] = "You can hide this tab in the menu",
        [RG_LANG_FR] = "Vous pouvez cacher cet onglet dans le menu",
        [RG_LANG_ES] = "Puede ocultar esta pestaña en el menú"
    },
    {
        [RG_LANG_EN] = "You have no %s games",
        [RG_LANG_FR] = "Vous n'avez pas de jeux %s",
        [RG_LANG_ES] = "No tienes juegos %s"
    },
    {
        [RG_LANG_EN] = "File not found",
        [RG_LANG_FR] = "Fichier non present",
        [RG_LANG_ES] = "Archivo no encontrado"
    },

    // rom options
    {
        [RG_LANG_EN] = "Name",
        [RG_LANG_FR] = "Nom",
        [RG_LANG_ES] = "Nombre"
    },
    {
        [RG_LANG_EN] = "Folder",
        [RG_LANG_FR] = "Dossier",
        [RG_LANG_ES] = "Carpeta"
    },
    {
        [RG_LANG_EN] = "Size",
        [RG_LANG_FR] = "Taille",
        [RG_LANG_ES] = "Tamaño"
    },
    {
        [RG_LANG_EN] = "CRC32",
        [RG_LANG_FR] = "CRC32",
        [RG_LANG_ES] = "CRC32"
    },
    {
        [RG_LANG_EN] = "Delete file",
        [RG_LANG_FR] = "Supprimer fichier",
        [RG_LANG_ES] = "Eliminar archivo"
    },
    {
        [RG_LANG_EN] = "Close",
        [RG_LANG_FR] = "Fermer",
        [RG_LANG_ES] = "Cerrar"
    },
    {
        [RG_LANG_EN] = "File properties",
        [RG_LANG_FR] = "Propriete",
        [RG_LANG_ES] = "Propiedades del archivo"
    },
    {
        [RG_LANG_EN] = "Delete selected file?",
        [RG_LANG_FR] = "Supprimer fichier?",
        [RG_LANG_ES] = "¿Eliminar el archivo seleccionado?"
    },


    // in-game menu
    {
        [RG_LANG_EN] = "Resume game",
        [RG_LANG_FR] = "Reprendre partie",
        [RG_LANG_ES] = "Reanudar juego"
    },
    {
        [RG_LANG_EN] = "New game",
        [RG_LANG_FR] = "Nouvelle partie",
        [RG_LANG_ES] = "Nuevo juego"
    },
    {
        [RG_LANG_EN] = "Del favorite",
        [RG_LANG_FR] = "supp favori",
        [RG_LANG_ES] = "Eliminar favorito"
    },
    {
        [RG_LANG_EN] = "Add favorite",
        [RG_LANG_FR] = "Ajouter favori",
        [RG_LANG_ES] = "Agregar favorito"
    },
    {
        [RG_LANG_EN] = "Delete save",
        [RG_LANG_FR] = "Supp sauvegarde",
        [RG_LANG_ES] = "Eliminar guardado"
    },
    {
        [RG_LANG_EN] = "Properties",
        [RG_LANG_FR] = "Proprietes",
        [RG_LANG_ES] = "Propiedades"
    },
    {
        [RG_LANG_EN] = "Resume",
        [RG_LANG_FR] = "Reprendre",
        [RG_LANG_ES] = "Reanudar"
    },
    {
        [RG_LANG_EN] = "Delete save?",
        [RG_LANG_FR] = "Supp sauvegarde?",
        [RG_LANG_ES] = "¿Eliminar guardado?"
    },
    {
        [RG_LANG_EN] = "Delete sram file?",
        [RG_LANG_FR] = "Supp fichier SRAM?",
        [RG_LANG_ES] = "¿Eliminar archivo SRAM?"
    },
    // end of applications.c


    // rg_system.c
    {
        [RG_LANG_EN] = "App unresponsive... Hold MENU to quit!",
        [RG_LANG_FR] = "Plantage... MENU pour quitter",
        [RG_LANG_ES] = "Sin respuesta... Presione MENU para salir!"
    },
    {
        [RG_LANG_EN] = "Reset all settings",
        [RG_LANG_FR] = "Reset parametres",
        [RG_LANG_ES] = "Restablecer ajustes"
    },
    {
        [RG_LANG_EN] = "Reboot to factory ",
        [RG_LANG_FR] = "Redemarrer usine",
        [RG_LANG_ES] = "Reiniciar de fábrica"
    },
    {
        [RG_LANG_EN] = "Reboot to launcher",
        [RG_LANG_FR] = "Relancer launcher",
        [RG_LANG_ES] = "Reiniciar launcher"
    },
    {
        [RG_LANG_EN] = "Recovery mode",
        [RG_LANG_FR] = "Mode de recuperation",
        [RG_LANG_ES] = "Modo de recuperación"
    },
    {
        [RG_LANG_EN] = "System Panic!",
        [RG_LANG_FR] = "Plantage systeme!",
        [RG_LANG_ES] = "¡Pánico del sistema!"
    },
    {
        [RG_LANG_EN] = "Save failed",
        [RG_LANG_FR] = "Erreur sauvegarde",
        [RG_LANG_ES] = "Error al guardar"
    },
    // end of rg_system.c
};
