# Theming Retro-Go

This document should document what are themes, how they're structured, and how to make them.

Example themes can be found in the [themes](/themes/) folder of this project.


## Theme Structure

A theme is a folder placed in `sd:/retro-go/themes` containing the following files:

````
/retro-go/themes
└── example
    ├── background_*.png
    ├── banner_*.png
    ├── logo_*.png
    ├── preview.png
    └── theme.json
````

| Name | Format | Description | Required |
|--|--|--|--|
| theme.json | JSON | Contains the theme metadata (description, author, colors, etc) | Yes |
| preview.png | PNG 160x120 | Theme preview to be displayed in the theme selector | No |
| background_X.png | PNG 320x240 | Launcher backgrounds where X is the name of the launcher tab | No |
| banner_X.png | PNG 272x24 | Launcher banners where X is the name of the launcher tab | No |
| logo_X.png | PNG 46x50 | Launcher logos where X is the name of the launcher tab | No |


### theme.json

The `theme.json` file contains the colors used by the theme, as well as some author meta data.

All fields are optional but it is not recommended to omit individual values because the fallback value that will be used is subject to change between retro-go versions. You are free to omit entire sections, however. For example if you only wish to retheme the launcher but not the dialogs, or vice-versa.

Colors are RGB565 and can be represented as integers or hex strings. The special value `transparent` is also accepted in some places.

<details>
  <summary>View example theme.json</summary>

````json
{
    "description": "Default Retro-Go Theme",
    "website": "https://github.com/ducalex/retro-go/",
    "author": "ducalex",
    "dialog": {
        "__comment": "This section contains global dialog colors",
        "background": "0x0010",
        "foreground": "0xFFFF",
        "border": "0x6B4D",
        "header": "0xFFFF",
        "scrollbar": "0xFFFF",
        "shadow": "none",
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
        "list_selected_fg": "0x07E0"
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
</details>


### Images

It is highly recommended to keep the image files sizes as small as possible to ensure good loading speed. This can be achieved by using the lowest bit depth possible when saving your PNG file. Tools like [pngquant](https://pngquant.org/) can also help!

Magenta (rgb(255, 0, 255) / 0xF81F) is used as the transparency color in some situations.
