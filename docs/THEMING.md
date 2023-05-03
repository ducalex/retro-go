# Theming Retro-Go

This document should document what are themes, how they're structured, and how to make them.


## Theme Structure

A theme is a folder placed in `sd:/retro-go/themes` containing the following files:

| name | Format | Description | Required |
|--|--|--|--|
| theme.json | JSON | Contains the theme metadata (description, author, colors, etc) | Yes |
| preview.png | PNG 160x120 | Theme preview to be displayed in the theme selector | No |
| background_X.png | PNG 320x240 | Launcher backgrounds where X is the name of the launcher tab (see launcher/main/images) | No |
| banner_X.png | PNG 272x24 | Launcher banners where X is the name of the launcher tab (see launcher/main/images) | No |
| logo_X.png | PNG 46x50 | Launcher logos where X is the name of the launcher tab (see launcher/main/images) | No |

### theme.json

All fields are optional (you'll have to dig in the source if you need to know the default value...).

<!-- FIXME: We should settle on a specific endianness and document it... -->
<!-- FIXME: Document where transparency is applicable, also make review code to ensure it's correctly converted to C_TRANSPARENT where needed. -->
<!-- FIXME2: Instead of -1 maybe we should accept the string "transparent", it's more explicit. -->
Colors are RGB565 and can be represented as integers or hex strings. The special value `-1` means transparent.

````json
{
    "description": "Theme description",
    "website": "https://example.com/retro-go-theme",
    "author": "John Smith",
    "dialog": {
        "__comment": "This section contains global dialog colors",
        "foreground": "0xFFFF",
        "background": "0xFFFF",
        "border": "0xFFFF",
        "header": "0xFFFF",
        "scrollbar": "0xFFFF",
        "item_standard": "0xFFFF",
        "item_disabled": "0xFFFF",
    },
    "launcher_1": {
        "__comment": "This section contains launcher theme variant 1",
        "list_standard_bg": "0xFFFF",
        "list_standard_fg": "0xFFFF",
        "list_selected_bg": "0xFFFF",
        "list_selected_fg": "0xFFFF",
    },
    "launcher_2": {
        "__comment": "This section contains launcher theme variant 2",
        "list_standard_bg": "0xFFFF",
        "list_standard_fg": "0xFFFF",
        "list_selected_bg": "0xFFFF",
        "list_selected_fg": "0xFFFF",
    },
    "launcher_3": {
        "__comment": "This section contains launcher theme variant 3",
        "list_standard_bg": "0xFFFF",
        "list_standard_fg": "0xFFFF",
        "list_selected_bg": "0xFFFF",
        "list_selected_fg": "0xFFFF",
    },
    "launcher_4": {
        "__comment": "This section contains launcher theme variant 4",
        "list_standard_bg": "0xFFFF",
        "list_standard_fg": "0xFFFF",
        "list_selected_bg": "0xFFFF",
        "list_selected_fg": "0xFFFF",
    }
}
````
