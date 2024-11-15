# Theming Retro-Go

This document should document what are themes, how they're structured, and how to make them.


## Theme Structure

A theme is a folder placed in `sd:/retro-go/themes` containing the following files:

| Name | Format | Description | Required |
|--|--|--|--|
| theme.json | JSON | Contains the theme metadata (description, author, colors, etc) | Yes |
| preview.png | PNG 160x120 | Theme preview to be displayed in the theme selector | No |
| background_X.png | PNG 320x240 | Launcher backgrounds where X is the name of the launcher tab (see launcher/main/images) | No |
| banner_X.png | PNG 272x24 | Launcher banners where X is the name of the launcher tab (see launcher/main/images) | No |
| logo_X.png | PNG 46x50 | Launcher logos where X is the name of the launcher tab (see launcher/main/images) | No |

### theme.json

All fields are optional (you'll have to dig in the source if you need to know the default value...).

Colors are RGB565 and can be represented as integers or hex strings. The special value `transparent` is also accepted in some places.

````json
{
    "description": "Theme description",
    "website": "https://example.com/retro-go-theme",
    "author": "John Smith",
    "dialog": {
        "__comment": "This section contains global dialog colors",
        "background": "0x0010",
        "foreground": "0xFFFF",
        "border": "0x6B4D",
        "header": "0xFFFF",
        "scrollbar": "0xFFFF",
        "item_standard": "0xFFFF",
        "item_disabled": "0x8410",
        "item_message": "0xBDF7"
    },
    "launcher_1": {
        "__comment": "This section contains launcher colors variant 1",
        "background": "0x0000",
        "foreground": "0xFFDE",
        "list_standard_bg": "transparent",
        "list_standard_fg": "0x8410",
        "list_selected_bg": "transparent",
        "list_selected_fg": "0xFFFF"
    },
    "launcher_2": {
        "__comment": "This section contains launcher colors variant 2",
        "background": "0x0000",
        "foreground": "0xFFDE",
        "list_standard_bg": "transparent",
        "list_standard_fg": "0x8410",
        "list_selected_bg": "transparent",
        "list_selected_fg": "0xFFFF"
    },
    "launcher_3": {
        "__comment": "This section contains launcher colors variant 3",
        "background": "0x0000",
        "foreground": "0xFFDE",
        "list_standard_bg": "transparent",
        "list_standard_fg": "0x8410",
        "list_selected_bg": "0xFFFF",
        "list_selected_fg": "0x0000"
    },
    "launcher_4": {
        "__comment": "This section contains launcher colors variant 4",
        "background": "0x0000",
        "foreground": "0xFFDE",
        "list_standard_bg": "transparent",
        "list_standard_fg": "0xAD55",
        "list_selected_bg": "0xFFFF",
        "list_selected_fg": "0x0000"
    }
}
````

Important: If retro-go refuses to load your theme, please run your theme.json through a JSON validator to make sure the format is correct (JSON is quite strict regarding quotes or trailing commas for example).
