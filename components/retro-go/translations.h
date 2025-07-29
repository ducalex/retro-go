#include "rg_localization.h"

static const char *language_names[RG_LANG_MAX] = {"English", "Francais", "中文"};

static const char *translations[][RG_LANG_MAX] =
{
    {
        [RG_LANG_EN] = "Never",
        [RG_LANG_FR] = "Jamais",
        [RG_LANG_CHS] = "从不",
    },
    {
        [RG_LANG_EN] = "Always",
        [RG_LANG_FR] = "Toujours",
        [RG_LANG_CHS] = "总是",
    },
    {
        [RG_LANG_EN] = "Composite",
        [RG_LANG_FR] = "Composite",
        [RG_LANG_CHS] = "复合",
    },
    {
        [RG_LANG_EN] = "NES Classic",
        [RG_LANG_FR] = "NES Classic",
        [RG_LANG_CHS] = "NES经典",
    },
    {
        [RG_LANG_EN] = "NTSC",
        [RG_LANG_FR] = "NTSC",
        [RG_LANG_CHS] = "NTSC",
    },
    {
        [RG_LANG_EN] = "PVM",
        [RG_LANG_FR] = "PVM",
        [RG_LANG_CHS] = "PVM",
    },
    {
        [RG_LANG_EN] = "Smooth",
        [RG_LANG_FR] = "Lisser",
        [RG_LANG_CHS] = "平滑",
    },
    {
        [RG_LANG_EN] = "To start, try: 1 or * or #",
        [RG_LANG_FR] = "Pour commencer, 1 ou * ou #",
        [RG_LANG_CHS] = "开始请按: 1 或 * 或 #",
    },
    {
	    [RG_LANG_EN] = "Full",
	    [RG_LANG_FR] = "Complet",
        [RG_LANG_CHS] = "完整",
    },
    {
        [RG_LANG_EN] = "Yes",
        [RG_LANG_FR] = "Oui",
        [RG_LANG_CHS] = "是",
    },
    {
        [RG_LANG_EN] = "Select file",
        [RG_LANG_FR] = "Choisissez un fichier",
        [RG_LANG_CHS] = "选择文件",
    },
    {
        [RG_LANG_EN] = "Language",
        [RG_LANG_FR] = "Langue",
        [RG_LANG_CHS] = "语言",
    },
    {
        [RG_LANG_EN] = "Language changed!",
        [RG_LANG_FR] = "Changement de langue",
        [RG_LANG_CHS] = "语言已更改!",
    },
    {
        [RG_LANG_EN] = "For these changes to take effect you must restart your device.\nrestart now?",
        [RG_LANG_FR] = "Pour que ces changements prennent effet, vous devez redémmarer votre appareil.\nRedémmarer maintenant ?",
        [RG_LANG_CHS] = "要使更改生效，必须重启设备。\n立即重启?",
    },
    {
        [RG_LANG_EN] = "Wi-Fi profile",
        [RG_LANG_FR] = "Profil Wi-Fi",
        [RG_LANG_CHS] = "Wi-Fi配置",
    },
    {
        [RG_LANG_EN] = "Language",
        [RG_LANG_FR] = "Langue",
        [RG_LANG_CHS] = "语言",
    },
    {
        [RG_LANG_EN] = "Options",
        [RG_LANG_FR] = "Options",
        [RG_LANG_CHS] = "选项",
    },
    {
        [RG_LANG_EN] = "About Retro-Go",
        [RG_LANG_FR] = "A propos de Retro-go",
        [RG_LANG_CHS] = "关于Retro-Go",
    },
    {
        [RG_LANG_EN] = "Reset all settings?",
        [RG_LANG_FR] = "Reset tous les paramètres",
        [RG_LANG_CHS] = "重置所有设置?",
    },
    {
        [RG_LANG_EN] = "Initializing...",
        [RG_LANG_FR] = "Initialisation...",
        [RG_LANG_CHS] = "初始化中...",
    },
    {
        [RG_LANG_EN] = "Host Game (P1)",
        [RG_LANG_FR] = "Host Game (P1)",
        [RG_LANG_CHS] = "主机游戏(P1)",
    },
    {
        [RG_LANG_EN] = "Find Game (P2)",
        [RG_LANG_FR] = "Find Game (P2)",
        [RG_LANG_CHS] = "加入游戏(P2)",
    },
    {
        [RG_LANG_EN] = "Netplay",
        [RG_LANG_FR] = "Netplay",
        [RG_LANG_CHS] = "联机游戏",
    },
    {
        [RG_LANG_EN] = "ROMs not identical. Continue?",
        [RG_LANG_FR] = "ROMs not identical. Continue?",
        [RG_LANG_CHS] = "ROM文件不一致。继续?",
    },
    {
        [RG_LANG_EN] = "Exchanging info...",
        [RG_LANG_FR] = "Exchanging info...",
        [RG_LANG_CHS] = "交换信息中...",
    },
    {
        [RG_LANG_EN] = "Unable to find host!",
        [RG_LANG_FR] = "Unable to find host!",
        [RG_LANG_CHS] = "无法找到主机!",
    },
    {
        [RG_LANG_EN] = "Connection failed!",
        [RG_LANG_FR] = "Connection failed!",
        [RG_LANG_CHS] = "连接失败!",
    },
    {
        [RG_LANG_EN] = "Waiting for peer...",
        [RG_LANG_FR] = "Waiting for peer...",
        [RG_LANG_CHS] = "等待对方...",
    },
    {
        [RG_LANG_EN] = "Unknown status...",
        [RG_LANG_FR] = "Unknown status...",
        [RG_LANG_CHS] = "未知状态...",
    },
    {
        [RG_LANG_EN] = "On",
        [RG_LANG_FR] = "On",
        [RG_LANG_CHS] = "开",
    },
    {
        [RG_LANG_EN] = "Keyboard",
        [RG_LANG_FR] = "Clavier",
        [RG_LANG_CHS] = "键盘",
    },
    {
        [RG_LANG_EN] = "Joystick",
        [RG_LANG_FR] = "Joystick",
        [RG_LANG_CHS] = "摇杆",
    },
    {
        [RG_LANG_EN] = "Input",
        [RG_LANG_FR] = "Entrée",
        [RG_LANG_CHS] = "输入",
    },
    {
        [RG_LANG_EN] = "Crop",
        [RG_LANG_FR] = "Couper",
        [RG_LANG_CHS] = "裁剪",
    },
    {
        [RG_LANG_EN] = "BIOS file missing!",
        [RG_LANG_FR] = "Fichier BIOS manquant",
        [RG_LANG_CHS] = "缺少BIOS文件!",
    },
    {
        [RG_LANG_EN] = "YM2612 audio ",
        [RG_LANG_FR] = "YM2612 audio ",
        [RG_LANG_CHS] = "YM2612音频",
    },
    {
        [RG_LANG_EN] = "SN76489 audio",
        [RG_LANG_FR] = "SN76489 audio",
        [RG_LANG_CHS] = "SN76489音频",
    },
    {
        [RG_LANG_EN] = "Z80 emulation",
        [RG_LANG_FR] = "Emulation Z80",
        [RG_LANG_CHS] = "Z80模拟",
    },
    {
        [RG_LANG_EN] = "Launcher options",
        [RG_LANG_FR] = "Options du lanceur",
        [RG_LANG_CHS] = "启动器选项",
    },
    {
        [RG_LANG_EN] = "Emulator options",
        [RG_LANG_FR] = "Options emulateur",
        [RG_LANG_CHS] = "模拟器选项",
    },
    {
        [RG_LANG_EN] = "Date",
        [RG_LANG_FR] = "Date",
        [RG_LANG_CHS] = "日期",
    },
    {
        [RG_LANG_EN] = "Files:",
        [RG_LANG_FR] = "Fichiers:",
        [RG_LANG_CHS] = "文件:",
    },
    {
        [RG_LANG_EN] = "Download complete!",
        [RG_LANG_FR] = "Téléchargement terminé",
        [RG_LANG_CHS] = "下载完成!",
    },
    {
        [RG_LANG_EN] = "Reboot to flash?",
        [RG_LANG_FR] = "Redémarrer",
        [RG_LANG_CHS] = "重启刷机?",
    },
    {
        [RG_LANG_EN] = "Available Releases",
        [RG_LANG_FR] = "Maj disponible",
        [RG_LANG_CHS] = "可用更新",
    },
    {
        [RG_LANG_EN] = "Received empty list!",
        [RG_LANG_FR] = "Liste vide reçue",
        [RG_LANG_CHS] = "收到空列表!",
    },
    {
        [RG_LANG_EN] = "Gamma Boost",
        [RG_LANG_FR] = "Boost Gamma",
        [RG_LANG_CHS] = "伽马增强",
    },
    {
        [RG_LANG_EN] = "Day",
        [RG_LANG_FR] = "Jour",
        [RG_LANG_CHS] = "天",
    },
    {
        [RG_LANG_EN] = "Hour",
        [RG_LANG_FR] = "Heure",
        [RG_LANG_CHS] = "小时",
    },
    {
        [RG_LANG_EN] = "Min",
        [RG_LANG_FR] = "Min",
        [RG_LANG_CHS] = "分钟",
    },
    {
        [RG_LANG_EN] = "Sec",
        [RG_LANG_FR] = "Sec",
        [RG_LANG_CHS] = "秒",
    },
    {
        [RG_LANG_EN] = "Sync",
        [RG_LANG_FR] = "Synchro",
        [RG_LANG_CHS] = "同步",
    },
    {
        [RG_LANG_EN] = "RTC config",
        [RG_LANG_FR] = "Congig RTC",
        [RG_LANG_CHS] = "RTC配置",
    },
    {
        [RG_LANG_EN] = "Palette",
        [RG_LANG_FR] = "Palette",
        [RG_LANG_CHS] = "调色板",
    },
    {
        [RG_LANG_EN] = "RTC config",
        [RG_LANG_FR] = "Config RTC",
        [RG_LANG_CHS] = "RTC配置",
    },
    {
        [RG_LANG_EN] = "SRAM autosave",
        [RG_LANG_FR] = "Sauvegarde auto SRAM",
        [RG_LANG_CHS] = "SRAM自动保存",
    },
    {
        [RG_LANG_EN] = "Enable BIOS",
        [RG_LANG_FR] = "Activer BIOS",
        [RG_LANG_CHS] = "启用BIOS",
    },
    {
        [RG_LANG_EN] = "Name",
        [RG_LANG_FR] = "Nom",
        [RG_LANG_CHS] = "名称",
    },
    {
        [RG_LANG_EN] = "Artist",
        [RG_LANG_FR] = "Artiste",
        [RG_LANG_CHS] = "艺术家",
    },
    {
        [RG_LANG_EN] = "Copyright",
        [RG_LANG_FR] = "Copyright",
        [RG_LANG_CHS] = "版权",
    },
    {
        [RG_LANG_EN] = "Playing",
        [RG_LANG_FR] = "Playing",
        [RG_LANG_CHS] = "播放中",
    },
    {
        [RG_LANG_EN] = "Palette",
        [RG_LANG_FR] = "Palette",
        [RG_LANG_CHS] = "调色板",
    },
    {
        [RG_LANG_EN] = "Overscan",
        [RG_LANG_FR] = "Overscan",
        [RG_LANG_CHS] = "过扫描",
    },
    {
        [RG_LANG_EN] = "Crop sides",
        [RG_LANG_FR] = "Couper les côtés",
        [RG_LANG_CHS] = "裁剪边距",
    },
    {
        [RG_LANG_EN] = "Sprite limit",
        [RG_LANG_FR] = "Limite de sprite",
        [RG_LANG_CHS] = "Sprite limit",
    },
    {
        [RG_LANG_EN] = "Overscan",
        [RG_LANG_FR] = "Overscan",
        [RG_LANG_CHS] = "过扫描",
    },
    {
        [RG_LANG_EN] = "Palette",
        [RG_LANG_FR] = "Palette",
        [RG_LANG_CHS] = "调色板",
    },
    {
        [RG_LANG_EN] = "Profile",
        [RG_LANG_FR] = "Profil",
        [RG_LANG_CHS] = "配置文件",
    },
    {
        [RG_LANG_EN] = "<profile name>",
        [RG_LANG_FR] = "<nom du profil>",
        [RG_LANG_CHS] = "<配置名称>",
    },
    {
        [RG_LANG_EN] = "Controls",
        [RG_LANG_FR] = "Contrôles",
        [RG_LANG_CHS] = "控制",
    },
    {
        [RG_LANG_EN] = "Audio enable",
        [RG_LANG_FR] = "Activer audio",
        [RG_LANG_CHS] = "启用音频",
    },
    {
        [RG_LANG_EN] = "Audio filter",
        [RG_LANG_FR] = "Filtre audio",
        [RG_LANG_CHS] = "音频滤镜",
    },


    // rg_gui.c
    {
        [RG_LANG_EN] = "Folder is empty.",
        [RG_LANG_FR] = "Le dossier est vide.",
        [RG_LANG_CHS] = "文件夹为空。",
    },
    {
        [RG_LANG_EN] = "No",
        [RG_LANG_FR] = "Non",
        [RG_LANG_CHS] = "否",
    },
    {
        [RG_LANG_EN] = "OK",
        [RG_LANG_FR] = "OK",
        [RG_LANG_CHS] = "确定",
    },
    {
        [RG_LANG_EN] = "On",
        [RG_LANG_FR] = "On",
        [RG_LANG_CHS] = "开",
    },
    {
        [RG_LANG_EN] = "Off",
        [RG_LANG_FR] = "Off",
        [RG_LANG_CHS] = "关",
    },
    {
        [RG_LANG_EN] = "Horiz",
        [RG_LANG_FR] = "Horiz",
        [RG_LANG_CHS] = "水平",
    },
    {
        [RG_LANG_EN] = "Vert",
        [RG_LANG_FR] = "Vert ",
        [RG_LANG_CHS] = "垂直",
    },
    {
        [RG_LANG_EN] = "Both",
        [RG_LANG_FR] = "Tout",
        [RG_LANG_CHS] = "全部",
    },
    {
        [RG_LANG_EN] = "Fit",
        [RG_LANG_FR] = "Ajuster",
        [RG_LANG_CHS] = "适应",
    },
    {
        [RG_LANG_EN] = "Full ",
        [RG_LANG_FR] = "Remplir ",
        [RG_LANG_CHS] = "填充",
    },
    {
        [RG_LANG_EN] = "Zoom",
        [RG_LANG_FR] = "Zoomer",
        [RG_LANG_CHS] = "缩放",
    },

    // Led options
    {
        [RG_LANG_EN] = "LED options",
        [RG_LANG_FR] = "Options LED",
        [RG_LANG_CHS] = "LED选项",
    },
    {
        [RG_LANG_EN] = "System activity",
        [RG_LANG_FR] = "Activité système",
        [RG_LANG_CHS] = "系统活动",
    },
    {
        [RG_LANG_EN] = "Disk activity",
        [RG_LANG_FR] = "Activité stockage",
        [RG_LANG_CHS] = "磁盘活动",
    },
    {
        [RG_LANG_EN] = "Low battery",
        [RG_LANG_FR] = "Batterie basse",
        [RG_LANG_CHS] = "低电量",
    },
    {
        [RG_LANG_EN] = "Default",
        [RG_LANG_FR] = "Défaut",
        [RG_LANG_CHS] = "默认",
    },
    {
        [RG_LANG_EN] = "<None>",
        [RG_LANG_FR] = "<Aucun>",
        [RG_LANG_CHS] = "<无>",
    },

    // Wifi
    {
        [RG_LANG_EN] = "Not connected",
        [RG_LANG_FR] = "Non connecté",
        [RG_LANG_CHS] = "未连接",
    },
    {
        [RG_LANG_EN] = "Connecting...",
        [RG_LANG_FR] = "Connection...",
        [RG_LANG_CHS] = "连接中...",
    },
    {
        [RG_LANG_EN] = "Disconnecting...",
        [RG_LANG_FR] = "Disconnection...",
        [RG_LANG_CHS] = "断开中...",
    },
    {
        [RG_LANG_EN] = "(empty)",
        [RG_LANG_FR] = "(vide)",
        [RG_LANG_CHS] = "(空)",
    },
    {
        [RG_LANG_EN] = "Wi-Fi AP",
        [RG_LANG_FR] = "Wi-Fi AP",
        [RG_LANG_CHS] = "Wi-Fi热点",
    },
    {
        [RG_LANG_EN] = "Start access point?\n\nSSID: retro-go\nPassword: retro-go\n\nBrowse: http://192.168.4.1/",
        [RG_LANG_FR] = "Démarrer point d'accès?\n\nSSID: retro-go\nPassword: retro-go\n\nAdresse: http://192.168.4.1/",
        [RG_LANG_CHS] = "启动热点?\n\nSSID: retro-go\n密码: retro-go\n\n访问: http://192.168.4.1/",
    },
    {
        [RG_LANG_EN] = "Wi-Fi enable",
        [RG_LANG_FR] = "Activer Wi-Fi",
        [RG_LANG_CHS] = "启用Wi-Fi",
    },
    {
        [RG_LANG_EN] = "Wi-Fi access point",
        [RG_LANG_FR] = "Point d'accès WiFi",
        [RG_LANG_CHS] = "Wi-Fi热点",
    },
    {
        [RG_LANG_EN] = "Network",
        [RG_LANG_FR] = "Réseau",
        [RG_LANG_CHS] = "网络",
    },
    {
        [RG_LANG_EN] = "IP address",
        [RG_LANG_FR] = "Adresse IP",
        [RG_LANG_CHS] = "IP地址",
    },

    // retro-go settings
    {
        [RG_LANG_EN] = "Brightness",
        [RG_LANG_FR] = "Luminosité",
        [RG_LANG_CHS] = "亮度",
    },
    {
        [RG_LANG_EN] = "Volume",
        [RG_LANG_FR] = "Volume",
        [RG_LANG_CHS] = "音量",
    },
    {
        [RG_LANG_EN] = "Audio out",
        [RG_LANG_FR] = "Sortie audio",
        [RG_LANG_CHS] = "音频输出",
    },
    {
        [RG_LANG_EN] = "Font type",
        [RG_LANG_FR] = "Police",
        [RG_LANG_CHS] = "字体类型",
    },
    {
        [RG_LANG_EN] = "Theme",
        [RG_LANG_FR] = "Thème",
        [RG_LANG_CHS] = "主题",
    },
    {
        [RG_LANG_EN] = "Show clock",
        [RG_LANG_FR] = "Horloge",
        [RG_LANG_CHS] = "显示时钟",
    },
    {
        [RG_LANG_EN] = "Timezone",
        [RG_LANG_FR] = "Fuseau",
        [RG_LANG_CHS] = "时区",
    },
    {
        [RG_LANG_EN] = "Wi-Fi options",
        [RG_LANG_FR] = "Options Wi-Fi",
        [RG_LANG_CHS] = "Wi-Fi选项",
    },

    // app dettings
    {
        [RG_LANG_EN] = "Scaling",
        [RG_LANG_FR] = "Format",
        [RG_LANG_CHS] = "缩放",
    },
    {
        [RG_LANG_EN] = "Factor",
        [RG_LANG_FR] = "Factor",
        [RG_LANG_CHS] = "比例",
    },
    {
        [RG_LANG_EN] = "Filter",
        [RG_LANG_FR] = "Filtre",
        [RG_LANG_CHS] = "滤镜",
    },
    {
        [RG_LANG_EN] = "Border",
        [RG_LANG_FR] = "Bordure",
        [RG_LANG_CHS] = "边框",
    },
    {
        [RG_LANG_EN] = "Speed",
        [RG_LANG_FR] = "Vitesse",
        [RG_LANG_CHS] = "速度",
    },

    // about menu
    {
        [RG_LANG_EN] = "Version",
        [RG_LANG_FR] = "Version",
        [RG_LANG_CHS] = "版本",
    },
    {
        [RG_LANG_EN] = "Date",
        [RG_LANG_FR] = "Date",
        [RG_LANG_CHS] = "日期",
    },
    {
        [RG_LANG_EN] = "Target",
        [RG_LANG_FR] = "Appareil",
        [RG_LANG_CHS] = "目标设备",
    },
    {
        [RG_LANG_EN] = "Website",
        [RG_LANG_FR] = "Site Web",
        [RG_LANG_CHS] = "网站",
    },
    {
        [RG_LANG_EN] = "Options",
        [RG_LANG_FR] = "Options",
        [RG_LANG_CHS] = "选项",
    },
    {
        [RG_LANG_EN] = "View credits",
        [RG_LANG_FR] = "Credits",
        [RG_LANG_CHS] = "查看贡献者",
    },
    {
        [RG_LANG_EN] = "Debug menu",
        [RG_LANG_FR] = "Menu debug",
        [RG_LANG_CHS] = "调试菜单",
    },
    {
        [RG_LANG_EN] = "Reset settings",
        [RG_LANG_FR] = "Reset paramètres",
        [RG_LANG_CHS] = "重置设置",
    },

    // save slot
    {
        [RG_LANG_EN] = "Slot 0",
        [RG_LANG_FR] = "Emplacement 0",
        [RG_LANG_CHS] = "存档位0",
    },
    {
        [RG_LANG_EN] = "Slot 1",
        [RG_LANG_FR] = "Emplacement 1",
        [RG_LANG_CHS] = "存档位1",
    },
    {
        [RG_LANG_EN] = "Slot 2",
        [RG_LANG_FR] = "Emplacement 2",
        [RG_LANG_CHS] = "存档位2",
    },
    {
        [RG_LANG_EN] = "Slot 3",
        [RG_LANG_FR] = "Emplacement 3",
        [RG_LANG_CHS] = "存档位3",
    },

    // game menu
    {
        [RG_LANG_EN] = "Save & Continue",
        [RG_LANG_FR] = "Sauver et continuer",
        [RG_LANG_CHS] = "保存并继续",
    },
    {
        [RG_LANG_EN] = "Save & Quit",
        [RG_LANG_FR] = "Sauver et quitter",
        [RG_LANG_CHS] = "保存并退出",
    },
    {
        [RG_LANG_EN] = "Load game",
        [RG_LANG_FR] = "Charger partie",
        [RG_LANG_CHS] = "加载游戏",
    },
    {
        [RG_LANG_EN] = "Reset",
        [RG_LANG_FR] = "Reset",
        [RG_LANG_CHS] = "重置",
    },
    {
        [RG_LANG_EN] = "Netplay",
        [RG_LANG_FR] = "Netplay",
        [RG_LANG_CHS] = "联机游戏",
    },
    {
        [RG_LANG_EN] = "About",
        [RG_LANG_FR] = "Infos",
        [RG_LANG_CHS] = "关于",
    },
    {
        [RG_LANG_EN] = "Quit",
        [RG_LANG_FR] = "Quitter",
        [RG_LANG_CHS] = "退出",
    },
    {
        [RG_LANG_EN] = "Soft reset",
        [RG_LANG_FR] = "Soft reset",
        [RG_LANG_CHS] = "软重启",
    },
    {
        [RG_LANG_EN] = "Hard reset",
        [RG_LANG_FR] = "Hard reset",
        [RG_LANG_CHS] = "硬重启",
    },

    {
        [RG_LANG_EN] = "Reset Emulation?",
        [RG_LANG_FR] = "Reset Emulation?",
        [RG_LANG_CHS] = "重置模拟器?",
    },
    {
        [RG_LANG_EN] = "Save",
        [RG_LANG_FR] = "Sauver",
        [RG_LANG_CHS] = "保存",
    },
    {
        [RG_LANG_EN] = "Load",
        [RG_LANG_FR] = "Charger",
        [RG_LANG_CHS] = "加载",
    },
    // end of rg_gui.c


    // main.c
    {
        [RG_LANG_EN] = "Show",
        [RG_LANG_FR] = "Montrer",
        [RG_LANG_CHS] = "显示",
    },
    {
        [RG_LANG_EN] = "Hide",
        [RG_LANG_FR] = "Cacher",
        [RG_LANG_CHS] = "隐藏",
    },
    {
        [RG_LANG_EN] = "Tabs Visibility",
        [RG_LANG_FR] = "Visibilitée onglets",
        [RG_LANG_CHS] = "标签页可见性",
    },

    // scroll modes
    {
        [RG_LANG_EN] = "Center",
        [RG_LANG_FR] = "centrer",
        [RG_LANG_CHS] = "居中",
    },
    {
        [RG_LANG_EN] = "Paging",
        [RG_LANG_FR] = "Paging",
        [RG_LANG_CHS] = "分页",
    },

    // start screen
    {
        [RG_LANG_EN] = "Auto",
        [RG_LANG_FR] = "Auto",
        [RG_LANG_CHS] = "自动",
    },
    {
        [RG_LANG_EN] = "Carousel",
        [RG_LANG_FR] = "Carousel",
        [RG_LANG_CHS] = "轮播",
    },
    {
        [RG_LANG_EN] = "Browser",
        [RG_LANG_FR] = "Browser",
        [RG_LANG_CHS] = "浏览器",
    },

    // preview
    {
        [RG_LANG_EN] = "None",
        [RG_LANG_FR] = "Aucun",
        [RG_LANG_CHS] = "无",
    },
    {
        [RG_LANG_EN] = "Cover,Save",
        [RG_LANG_FR] = "Cover,Save",
        [RG_LANG_CHS] = "封面,存档",
    },
    {
        [RG_LANG_EN] = "Save,Cover",
        [RG_LANG_FR] = "Save,Cover",
        [RG_LANG_CHS] = "存档,封面",
    },
    {
        [RG_LANG_EN] = "Cover only",
        [RG_LANG_FR] = "Cover only",
        [RG_LANG_CHS] = "仅封面",
    },
    {
        [RG_LANG_EN] = "Save only",
        [RG_LANG_FR] = "Save only",
        [RG_LANG_CHS] = "仅存档",
    },

    // startup app
    {
        [RG_LANG_EN] = "Last game",
        [RG_LANG_FR] = "Dernier jeu",
        [RG_LANG_CHS] = "最后启动的游戏",
    },
    {
        [RG_LANG_EN] = "Launcher",
        [RG_LANG_FR] = "Launcher",
        [RG_LANG_CHS] = "启动器",
    },

    // launcher options
    {
        [RG_LANG_EN] = "Launcher Options",
        [RG_LANG_FR] = "Options du lanceur",
        [RG_LANG_CHS] = "启动器选项",
    },
    {
        [RG_LANG_EN] = "Color theme",
        [RG_LANG_FR] = "Couleurs",
        [RG_LANG_CHS] = "颜色主题",
    },
    {
        [RG_LANG_EN] = "Preview",
        [RG_LANG_FR] = "Aperçu",
        [RG_LANG_CHS] = "预览",
    },
    {
        [RG_LANG_EN] = "Scroll mode",
        [RG_LANG_FR] = "Mode défilement",
        [RG_LANG_CHS] = "滚动模式",
    },
    {
        [RG_LANG_EN] = "Start screen",
        [RG_LANG_FR] = "Ecran démarrage",
        [RG_LANG_CHS] = "启动画面",
    },
    {
        [RG_LANG_EN] = "Hide tabs",
        [RG_LANG_FR] = "Cacher onglet",
        [RG_LANG_CHS] = "隐藏标签页",
    },
    {
        [RG_LANG_EN] = "File server",
        [RG_LANG_FR] = "Serveur fichier",
        [RG_LANG_CHS] = "文件服务器",
    },
    {
        [RG_LANG_EN] = "Startup app",
        [RG_LANG_FR] = "App démarrage",
        [RG_LANG_CHS] = "启动应用",
    },
    {
        [RG_LANG_EN] = "Build CRC cache",
        [RG_LANG_FR] = "Build CRC cache",
        [RG_LANG_CHS] = "构建CRC缓存",
    },
    {
        [RG_LANG_EN] = "Check for updates",
        [RG_LANG_FR] = "Verifier mise à jour",
        [RG_LANG_CHS] = "检查更新",
    },
    {
        [RG_LANG_EN] = "HTTP Server Busy...",
        [RG_LANG_FR] = "Server Web ...",
        [RG_LANG_CHS] = "HTTP服务器忙...",
    },
    {
        [RG_LANG_EN] = "SD Card Error",
        [RG_LANG_FR] = "Erreur carte SD",
        [RG_LANG_CHS] = "SD卡错误",
    },
    {
        [RG_LANG_EN] = "Storage mount failed.\nMake sure the card is FAT32.",
        [RG_LANG_FR] = "Erreur carte SD.\nLa carte est bien en FAT32 ?",
        [RG_LANG_CHS] = "存储挂载失败。\n请确保卡是FAT32格式。",
    },
    // end of main.c


    // applications.c
    {
        [RG_LANG_EN] = "Scanning %s %d/%d",
        [RG_LANG_FR] = "Scan %s %d/%d",
        [RG_LANG_CHS] = "扫描中 %s %d/%d",
    },
    // message when no rom
    {
        [RG_LANG_EN] = "Welcome to Retro-Go!",
        [RG_LANG_FR] = "Bienvenue sur Retro-Go!",
        [RG_LANG_CHS] = "欢迎使用Retro-Go!",
    },
    {
        [RG_LANG_EN] = "Place roms in folder: %s",
        [RG_LANG_FR] = "Placer les ROMS dans le dossier: %s",
        [RG_LANG_CHS] = "请将ROM放入文件夹: %s",
    },
    {
        [RG_LANG_EN] = "With file extension: %s",
        [RG_LANG_FR] = "Avec l'extension: %s",
        [RG_LANG_CHS] = "文件扩展名: %s",
    },
    {
        [RG_LANG_EN] = "You can hide this tab in the menu",
        [RG_LANG_FR] = "Vous pouvez cacher cet onglet dans le menu",
        [RG_LANG_CHS] = "可在菜单中隐藏此标签页",
    },
    {
        [RG_LANG_EN] = "You have no %s games",
        [RG_LANG_FR] = "Vous n'avez pas de jeux %s",
        [RG_LANG_CHS] = "没有%s游戏",
    },
    {
        [RG_LANG_EN] = "File not found",
        [RG_LANG_FR] = "Fichier non présent",
        [RG_LANG_CHS] = "文件未找到",
    },

    // rom options
    {
        [RG_LANG_EN] = "Name",
        [RG_LANG_FR] = "Nom",
        [RG_LANG_CHS] = "名称",
    },
    {
        [RG_LANG_EN] = "Folder",
        [RG_LANG_FR] = "Dossier",
        [RG_LANG_CHS] = "文件夹",
    },
    {
        [RG_LANG_EN] = "Size",
        [RG_LANG_FR] = "Taille",
        [RG_LANG_CHS] = "大小",
    },
    {
        [RG_LANG_EN] = "CRC32",
        [RG_LANG_FR] = "CRC32",
        [RG_LANG_CHS] = "CRC32",
    },
    {
        [RG_LANG_EN] = "Delete file",
        [RG_LANG_FR] = "Supprimer fichier",
        [RG_LANG_CHS] = "删除文件",
    },
    {
        [RG_LANG_EN] = "Close",
        [RG_LANG_FR] = "Fermer",
        [RG_LANG_CHS] = "关闭",
    },
    {
        [RG_LANG_EN] = "File properties",
        [RG_LANG_FR] = "Propriétés fichier",
        [RG_LANG_CHS] = "文件属性",
    },
    {
        [RG_LANG_EN] = "Delete selected file?",
        [RG_LANG_FR] = "Supprimer fichier?",
        [RG_LANG_CHS] = "删除选定文件?",
    },


    // in-game menu
    {
        [RG_LANG_EN] = "Resume game",
        [RG_LANG_FR] = "Reprendre partie",
        [RG_LANG_CHS] = "继续游戏",
    },
    {
        [RG_LANG_EN] = "New game",
        [RG_LANG_FR] = "Nouvelle partie",
        [RG_LANG_CHS] = "新游戏",
    },
    {
        [RG_LANG_EN] = "Del favorite",
        [RG_LANG_FR] = "supp favori",
        [RG_LANG_CHS] = "删除收藏",
    },
    {
        [RG_LANG_EN] = "Add favorite",
        [RG_LANG_FR] = "Ajouter favori",
        [RG_LANG_CHS] = "添加收藏",
    },
    {
        [RG_LANG_EN] = "Delete save",
        [RG_LANG_FR] = "Supp sauvegarde",
        [RG_LANG_CHS] = "删除存档",
    },
    {
        [RG_LANG_EN] = "Properties",
        [RG_LANG_FR] = "Propriétés",
        [RG_LANG_CHS] = "属性",
    },
    {
        [RG_LANG_EN] = "Resume",
        [RG_LANG_FR] = "Reprendre",
        [RG_LANG_CHS] = "继续",
    },
    {
        [RG_LANG_EN] = "Delete save?",
        [RG_LANG_FR] = "Supp sauvegarde?",
        [RG_LANG_CHS] = "删除存档?",
    },
    {
        [RG_LANG_EN] = "Delete sram file?",
        [RG_LANG_FR] = "Supp fichier SRAM?",
        [RG_LANG_CHS] = "删除SRAM文件?",
    },
    // end of applications.c


    // rg_system.c
    {
        [RG_LANG_EN] = "App unresponsive... Hold MENU to quit!",
        [RG_LANG_FR] = "Plantage... MENU pour quitter",
        [RG_LANG_CHS] = "应用无响应...长按MENU退出!",
    },
    {
        [RG_LANG_EN] = "Reset all settings",
        [RG_LANG_FR] = "Reset paramètres",
        [RG_LANG_CHS] = "重置所有设置",
    },
    {
        [RG_LANG_EN] = "Reboot to factory ",
        [RG_LANG_FR] = "Redémarrer usine",
        [RG_LANG_CHS] = "恢复出厂设置",
    },
    {
        [RG_LANG_EN] = "Reboot to launcher",
        [RG_LANG_FR] = "Relancer launcher",
        [RG_LANG_CHS] = "重启到启动器",
    },
    {
        [RG_LANG_EN] = "Recovery mode",
        [RG_LANG_FR] = "Mode de recupération",
        [RG_LANG_CHS] = "恢复模式",
    },
    {
        [RG_LANG_EN] = "System Panic!",
        [RG_LANG_FR] = "Plantage système!",
        [RG_LANG_CHS] = "系统崩溃!",
    },
    {
        [RG_LANG_EN] = "Save failed",
        [RG_LANG_FR] = "Erreur sauvegarde",
        [RG_LANG_CHS] = "保存失败",
    },
    // end of rg_system.c
};