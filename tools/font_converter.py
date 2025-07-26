from PIL import Image, ImageDraw, ImageFont
from tkinter import Tk, Label, Entry, StringVar, Button, Frame, Canvas, filedialog, ttk, Checkbutton, IntVar
import os
import re

################################ - Font format - ################################
#
# font:
# |
# ├── glyph_bitmap[] -> 8 bit array containing the bitmap data for all glyph
# |
# └── glyph_data[] -> struct that contains all the data to correctly draw the glyph
#
######################## - Explanation of glyph_bitmap[] - #######################
# First, let's see an example : '!'
#
# we are going to convert glyph_bitmap[] bytes to binary :
# 11111111,
# 11111111,
# 11000111,
# 11100000,
#
# then we rearrange them :
#  [3 bits wide]
#       111
#       111
#       111
# [9    111   We clearly reconize '!' character
# bits  111
# tall] 111
#       000
#       111
#       111
#       (000000)
#
# Second example with '0' :
# 0x30,0x04,0x07,0x09,0x00,0x07,
# 0x7D,0xFB,0xBF,0x7E,0xFD,0xFB,0xFF,0x7C,
#
# - width = 0x07 = 7
# - height = 0x09 = 9
# - data[n] = 0x7D,0xFB,0xBF,0x7E,0xFD,0xFB,0xFF,0x7C
#
# in binary :
# 1111101
# 11111011
# 10111111
# 1111110
# 11111101
# 11111011
# 11111111
# 1111100
#
# We see that everything is not aligned so we add zeros ON THE LEFT :
# ->01111101
#   11111011
#   10111111
# ->01111110
#   11111101
#   11111011
#   11111111
# ->01111100

# Next, we rearrange the bits :
#    [ 7 bits wide]
#       0111110
#       1111110
#       1110111
# [9    1110111
# bits  1110111     we can reconize '0' (if you squint a loooot)
# tall] 1110111
#       1110111
#       1111111
#       0111110
#       (0)
#
# And that's basically how characters are encoded using this tool

# Example usage (defaults parameters)
list_char_ranges_init = "32-126, 160-255"
font_size_init = 11

font_path = ("arial.ttf")  # Replace with your TTF font path

# Variables to track panning
start_x = 0
start_y = 0

def get_char_list():
    list_char = []
    for intervals in list_char_ranges.get().split(','):
        first = intervals.split('-')[0]
        # we check if we the user input is a single char or an interval
        try:
            second = intervals.split('-')[1]
        except IndexError:
            list_char.append(int(first))
        else:
            second = intervals.split('-')[1]
            for char in range(int(first), int(second) + 1):
                list_char.append(char)
    return list_char

def find_bounding_box(image):
    pixels = image.load()
    width, height = image.size
    x_min, y_min = width, height
    x_max, y_max = 0, 0

    for y in range(height):
        for x in range(width):
            if pixels[x, y] >= 1:  # Looking for 'on' pixels
                x_min = min(x_min, x)
                y_min = min(y_min, y)
                x_max = max(x_max, x)
                y_max = max(y_max, y)

    if x_min > x_max or y_min > y_max:  # No target pixels found
        return None
    return (x_min, y_min, x_max+1, y_max+1)

def load_ttf_font(font_path, font_size):
    # Load the TTF font
    enforce_font_size = enforce_font_size_bool.get()
    pil_font = ImageFont.truetype(font_path, font_size)

    font_name = ' '.join(pil_font.getname())
    font_data = []

    for char_code in get_char_list():
        char = chr(char_code)

        image = Image.new("1", (font_size * 2, font_size * 2), 0) # generate mono bmp, 0 = black color
        draw = ImageDraw.Draw(image)
        # Draw at pos 1 otherwise some glyphs are clipped. we remove the added offset below
        draw.text((1, 0), char, font=pil_font, fill=255)

        bbox = find_bounding_box(image)  # Get bounding box

        if bbox is None: # control character / space
            width, height = 0, 0
            offset_x, offset_y = 0, 0
        else:
            x0, y0, x1, y1 = bbox
            width, height = x1 - x0, y1 - y0
            offset_x, offset_y = x0, y0
            if offset_x:
                offset_x -= 1

        try: # Get the real glyph width including padding on the right that the box will remove
            adv_w = int(draw.textlength(char, font=pil_font))
            adv_w = max(adv_w, width + offset_x)
        except:
            adv_w = width + offset_x

        # Shift or crop glyphs that would be drawn beyond font_size. Most glyphs are not affected by this.
        # If enforce_font_size is false, then max_height will be calculated at the end and the font might
        # be taller than requested.
        if enforce_font_size and offset_y + height > font_size:
            print(f"    font_size exceeded: {offset_y+height}")
            if font_size - height >= 0:
                offset_y = font_size - height
            else:
                offset_y = 0
                height = font_size

        # Extract bitmap data
        cropped_image = image.crop(bbox)
        bitmap = []
        row = 0
        i = 0
        for y in range(height):
            for x in range(width):
                if i == 8:
                    bitmap.append(row)
                    row = 0
                    i = 0
                pixel = 1 if cropped_image.getpixel((x, y)) else 0
                row = (row << 1) | pixel
                i += 1
        bitmap.append(row << 8-i) # to "fill" with zero the remaining empty bits
        bitmap = bitmap[0:int((width * height + 7) / 8)]

        # Create glyph entry
        glyph_data = {
            "char_code": char_code,
            "ofs_y": int(offset_y),
            "box_w": int(width),
            "box_h": int(height),
            "ofs_x": int(offset_x),
            "adv_w": int(adv_w),
            "bitmap": bitmap,
        }
        font_data.append(glyph_data)

    # The font render glyphs at font_size but they can shift them up or down which will cause the max_height
    # to exceed font_size. It's not desirable to remove the padding entirely (the "enforce" option above), 
    # but there are some things we can do to reduce the discrepency without affecting the look.
    max_height = max(g["ofs_y"] + g["box_h"] for g in font_data)
    if max_height > font_size:
        min_ofs_y = min((g["ofs_y"] if g["box_h"] > 0 else 1000) for g in font_data)
        for key, glyph in enumerate(font_data):
            offset = glyph["ofs_y"]
            # If there's a consistent excess of top padding across all glyphs, we can remove it
            if min_ofs_y > 0 and offset >= min_ofs_y:
                offset -= min_ofs_y
            # In some fonts like Vera and DejaVu we can shift _ and | to gain an extra pixel
            if chr(glyph["char_code"]) in ["_", "|"] and offset + glyph["box_h"] > font_size and offset > 0:
                offset -= 1
            font_data[key]["ofs_y"] = offset

        max_height = max(g["ofs_y"] + g["box_h"] for g in font_data)

    print(f"Glyphs: {len(font_data)}, font_size: {font_size}, max_height: {max_height}")

    return (font_name, font_size, font_data)

def load_c_font(file_path):
    # Load the C font
    font_name = "Unknown"
    font_size = 0
    font_data = []

    with open(file_path, 'r', encoding='UTF-8') as file:
        text = file.read()
        text = re.sub('//.*?$|/\*.*?\*/', '', text, flags=re.S|re.MULTILINE)
        text = re.sub('[\n\r\t\s]+', ' ', text)
        # FIXME: Handle parse errors...
        if m := re.search('\.name\s*=\s*"(.+)",', text):
            font_name = m.group(1)
        if m := re.search('\.height\s*=\s*(\d+),', text):
            font_size = int(m.group(1))
        if m := re.search('\.data\s*=\s*\{(.+?)\}', text):
            hexdata = [int(h, base=16) for h in re.findall('0x[0-9A-Fa-f]{2}', text)]

    while len(hexdata):
        char_code = hexdata[0] | (hexdata[1] << 8)
        if not char_code:
            break
        ofs_y = hexdata[2]
        box_w = hexdata[3]
        box_h = hexdata[4]
        ofs_x = hexdata[5]
        adv_w = hexdata[6]
        bitmap = hexdata[7:int((box_w * box_h + 7) / 8) + 7]

        glyph_data = {
            "char_code": char_code,
            "ofs_y": ofs_y,
            "box_w": box_w,
            "box_h": box_h,
            "ofs_x": ofs_x,
            "adv_w": adv_w,
            "bitmap": bitmap,
        }
        font_data.append(glyph_data)

        hexdata = hexdata[7 + len(bitmap):]

    return (font_name, font_size, font_data)

def generate_font_data():
    if font_path.endswith(".c"):
        font_name, font_size, font_data = load_c_font(font_path)
    else:
        font_name, font_size, font_data = load_ttf_font(font_path, int(font_height_input.get()))

    window.title(f"Font preview: {font_name} {font_size}")
    font_height_input.set(font_size)

    max_height = max(font_size, max(g["ofs_y"] + g["box_h"] for g in font_data))
    bounding_box = bounding_box_bool.get()

    canvas.delete("all")
    offset_x_1 = 1
    offset_y_1 = 1

    for glyph_data in font_data:
        offset_y = glyph_data["ofs_y"]
        width = glyph_data["box_w"]
        height = glyph_data["box_h"]
        offset_x = glyph_data["ofs_x"]
        adv_w = glyph_data["adv_w"]

        if offset_x_1+adv_w+1 > canva_width:
            offset_x_1 = 1
            offset_y_1 += max_height + 1

        byte_index = 0
        byte_value = 0
        bit_index = 0
        for y in range(height):
            for x in range(width):
                if bit_index == 0:
                    byte_value = glyph_data["bitmap"][byte_index]
                    byte_index += 1
                if byte_value & (1 << 7-bit_index):
                    canvas.create_rectangle((x+offset_x_1+offset_x)*p_size, (y+offset_y_1+offset_y)*p_size, (x+offset_x_1+offset_x)*p_size+p_size, (y+offset_y_1+offset_y)*p_size+p_size,fill="white")
                bit_index += 1
                bit_index %= 8

        if bounding_box:
            canvas.create_rectangle((offset_x_1+offset_x)*p_size, (offset_y_1+offset_y)*p_size, (width+offset_x_1+offset_x)*p_size, (height+offset_y_1+offset_y)*p_size, width=1, outline="red", fill='') # bounding box
            canvas.create_rectangle((offset_x_1)*p_size, (offset_y_1)*p_size, (offset_x_1+adv_w)*p_size, (offset_y_1+max_height)*p_size, width=1, outline='blue', fill='')

        offset_x_1 += adv_w + 1

    return (font_name, font_size, font_data)

def save_font_data():
    font_name, font_size, font_data = generate_font_data()

    filename = filedialog.asksaveasfilename(
        title='Save Font',
        initialdir=os.getcwd(),
        initialfile=f"{font_name.replace('-', '_').replace(' ', '')}{font_size}",
        defaultextension=".c",
        filetypes=(('Retro-Go Font', '*.c'), ('All files', '*.*')))

    if filename:
        with open(filename, 'w', encoding='UTF-8') as f:
            f.write(generate_c_font(font_name, font_size, font_data))

def generate_c_font(font_name, font_size, font_data):
    normalized_name = f"{font_name.replace('-', '_').replace(' ', '')}{font_size}"
    max_height = max(font_size, max(g["ofs_y"] + g["box_h"] for g in font_data))
    memory_usage = sum(len(g["bitmap"]) + 8 for g in font_data)

    file_data = "#include \"../rg_gui.h\"\n\n"
    file_data += "// File generated with font_converter.py (https://github.com/ducalex/retro-go/tree/dev/tools)\n\n"
    file_data += f"// Font           : {font_name}\n"
    file_data += f"// Point Size     : {font_size}\n"
    file_data += f"// Memory usage   : {memory_usage} bytes\n"
    file_data += f"// # characters   : {len(font_data)}\n\n"
    file_data += f"const rg_font_t font_{normalized_name} = {{\n"
    file_data += f"    .name = \"{font_name}\",\n"
    file_data += f"    .type = 1,\n"
    file_data += f"    .width = 0,\n"
    file_data += f"    .height = {max_height},\n"
    file_data += f"    .chars = {len(font_data)},\n"
    file_data += f"    .data = {{\n"
    for glyph in font_data:
        char_code = glyph['char_code']
        header_data = [char_code & 0xFF, char_code >> 8, glyph['ofs_y'], glyph['box_w'],
                        glyph['box_h'], glyph['ofs_x'], glyph['adv_w']]
        file_data += f"        /* U+{char_code:04X} '{chr(char_code)}' */\n        "
        file_data += ", ".join([f"0x{byte:02X}" for byte in header_data])
        file_data += f",\n        "
        if len(glyph["bitmap"]) > 0:
            file_data += ", ".join([f"0x{byte:02X}" for byte in glyph["bitmap"]])
            file_data += f","
        file_data += "\n"
    file_data += "\n"
    file_data += "        // Terminator\n"
    file_data += "        0x00, 0x00,\n"
    file_data += "    },\n"
    file_data += "};\n"

    return file_data

def select_file():
    filename = filedialog.askopenfilename(
        title='Load Font',
        initialdir=os.getcwd(),
        filetypes=(('True Type Font', '*.ttf'), ('Retro-Go Font', '*.c'), ('All files', '*.*')))

    if filename:
        global font_path
        font_path = filename
        generate_font_data()

# Function to zoom in and out on the canvas
def zoom(event):
    scale = 1.0
    if event.delta > 0:  # Scroll up to zoom in
        scale = 1.2
    elif event.delta < 0:  # Scroll down to zoom out
        scale = 0.8

    # Get the canvas size and adjust scale based on cursor position
    canvas.scale("all", event.x, event.y, scale, scale)

    # Update the scroll region to reflect the new scale
    canvas.configure(scrollregion=canvas.bbox("all"))

def start_pan(event):
    global start_x, start_y
    # Record the current mouse position
    start_x = event.x
    start_y = event.y

def pan_canvas(event):
    global start_x, start_y
    # Calculate the distance moved
    dx = start_x - event.x
    dy = start_y - event.y

    # Scroll the canvas
    canvas.move("all", -dx, -dy)

    # Update the starting position
    start_x = event.x
    start_y = event.y


if __name__ == "__main__":
    window = Tk()
    window.title("Retro-Go Font Converter")

    # Get screen width and height
    screen_width = window.winfo_screenwidth()
    screen_height = window.winfo_screenheight()
    # Set the window size to fill the entire screen
    window.geometry(f"{screen_width}x{screen_height}")

    p_size = 8 # pixel size on the renderer

    canva_width = screen_width//p_size
    canva_height = screen_height//p_size-16

    frame = Frame(window)
    frame.pack(anchor="center", padx=10, pady=2)

    # choose font button (file picker)
    choose_font_button = ttk.Button(frame, text='Choose font', command=select_file)
    choose_font_button.pack(side="left", padx=5)

    # Label and Entry for Font height
    Label(frame, text="Font height").pack(side="left", padx=5)
    font_height_input = StringVar(value=str(font_size_init))
    Entry(frame, textvariable=font_height_input, width=4).pack(side="left", padx=5)

    # Variable to hold the state of the checkbox
    enforce_font_size_bool = IntVar()  # 0 for unchecked, 1 for checked
    Checkbutton(frame, text="Enforce size", variable=enforce_font_size_bool).pack(side="left", padx=5)

    # Label and Entry for Char ranges to include
    Label(frame, text="Ranges to include").pack(side="left", padx=5)
    list_char_ranges = StringVar(value=str(list_char_ranges_init))
    Entry(frame, textvariable=list_char_ranges, width=30).pack(side="left", padx=5)

    # Variable to hold the state of the checkbox
    bounding_box_bool = IntVar(value=1)  # 0 for unchecked, 1 for checked
    Checkbutton(frame, text="Bounding box", variable=bounding_box_bool).pack(side="left", padx=10)

    # Button to launch the font generation function
    b1 = Button(frame, text="Preview", width=14, height=2, background="blue", foreground="white", command=generate_font_data)
    b1.pack(side="left", padx=5)

    # Button to launch the font exporting function
    b1 = Button(frame, text="Save", width=14, height=2, background="blue", foreground="white", command=save_font_data)
    b1.pack(side="left", padx=5)

    frame = Frame(window).pack(anchor="w", padx=2, pady=2)
    canvas = Canvas(frame, width=canva_width*p_size, height=canva_height*p_size, bg="black")
    canvas.configure(scrollregion=(0, 0, canva_width*p_size, canva_height*p_size))
    canvas.bind("<MouseWheel>", zoom)
    canvas.bind("<ButtonPress-1>", start_pan)  # Start panning
    canvas.bind("<B1-Motion>",pan_canvas)
    canvas.focus_set()
    canvas.pack(fill="both", expand=True)

    window.mainloop()