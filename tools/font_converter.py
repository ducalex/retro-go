from PIL import Image, ImageDraw, ImageFont
from tkinter import Tk, Label, Entry, StringVar, Button, Frame, Canvas, filedialog, ttk, Checkbutton, IntVar
import os

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

def generate_font_data():
    font_size = int(font_height_input.get())
    bounding_box = bounding_box_bool.get()
    enforce_font_size = enforce_font_size_bool.get()

    # Load the TTF font
    pil_font = ImageFont.truetype(font_path, font_size)
    font_name = ' '.join(pil_font.getname())

    window.title(f"Font preview: {font_name} {font_size}")

    # Initialize the font data structure
    font_data = []
    bitmap_data = dict()
    memory_usage = 0
    max_height = 0

    canvas.delete("all")
    offset_x_1 = 1
    offset_y_1 = 1

    for char_code in get_char_list():
        char = chr(char_code)
        print(f"Processing character: {char} ({char_code})")

        image = Image.new("1", (font_size * 2, font_size * 2), 0) # generate mono bmp, 0 = black color
        draw = ImageDraw.Draw(image)
        draw.text((1, 0), char, font=pil_font, fill=255)

        bbox = find_bounding_box(image)  # Get bounding box

        if bbox is None: # control character / space
            width, height = 0, 0
            offset_x, offset_y = 0, 0
            try:
                adv_w = draw.textlength(char, font=pil_font) + 1
            except:
                adv_w = 0
        else:
            x0, y0, x1, y1 = bbox
            width, height = x1 - x0, y1 - y0
            offset_x, offset_y = x0, y0
            adv_w = width + offset_x

        if offset_x_1+adv_w+1 > canva_width:
            offset_x_1 = 1
            offset_y_1 += font_size + font_size//3

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
                pixel = cropped_image.getpixel((x, y))
                if i == 8:
                    bitmap.append(row)
                    row = 0
                    i = 0
                pixel = 1 if cropped_image.getpixel((x, y)) else 0
                row = (row << 1) | pixel
                if pixel:
                    canvas.create_rectangle((x+offset_x_1+offset_x)*p_size, (y+offset_y_1+offset_y)*p_size, (x+offset_x_1+offset_x)*p_size+p_size, (y+offset_y_1+offset_y)*p_size+p_size,fill="white")
                i += 1

        row = row << 8-i # to "fill" with zero the remaining empty bits
        bitmap.append(row)

        if bounding_box:
            canvas.create_rectangle((offset_x_1+offset_x)*p_size, (offset_y_1+offset_y)*p_size, (width+offset_x_1+offset_x)*p_size, (height+offset_y_1+offset_y)*p_size, width=1, outline="red", fill='') # bounding box
            canvas.create_rectangle((offset_x_1)*p_size, (offset_y_1)*p_size, (offset_x_1+adv_w)*p_size, (offset_y_1+offset_y+height)*p_size, width=1, outline='blue', fill='')

        offset_x_1 += adv_w + 1

        # Create glyph entry
        glyph_data = {
            "char_code": char_code,
            "ofs_y": int(offset_y),
            "box_w": int(width),
            "box_h": int(height),
            "ofs_x": int(offset_x),
            "adv_w": int(adv_w)
        }
        font_data.append(glyph_data)

        bitmap = bitmap[0:int((width * height + 7) / 8)]
        bitmap_data[char_code] = bitmap

        # Update memory usage
        memory_usage += len(bitmap) + 8  # 8 bytes for the header per glyph
        max_height = max(height + offset_y, max_height)

    return (font_name, font_size, {
        "bitmap": bitmap_data,
        "glyphs": font_data,
        "memory_usage": memory_usage,
        "max_height": max_height
    })

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
            f.write(generate_file(font_name, font_size, font_data))

def generate_file(font_name, font_size, font_data):
    normalized_name = f"{font_name.replace('-', '_').replace(' ', '')}{font_size}"

    file_data = "#include \"../rg_gui.h\"\n\n"
    file_data += "// File generated with font_converter.py (https://github.com/ducalex/retro-go/tree/dev/tools)\n\n"
    file_data += f"// Font           : {font_name}\n"
    file_data += f"// Point Size     : {font_size}\n"
    file_data += f"// Memory usage   : {font_data['memory_usage']} bytes\n"
    file_data += f"// # characters   : {len(font_data['glyphs'])}\n\n"
    file_data += f"const rg_font_t font_{normalized_name} = {{\n"
    file_data += f"    .name = \"{font_name}\",\n"
    file_data += f"    .type = 1,\n"
    file_data += f"    .width = 0,\n"
    file_data += f"    .height = {font_data['max_height']},\n"
    file_data += f"    .chars = {len(font_data['glyphs'])},\n"
    file_data += f"    .data = {{\n"
    for glyph in font_data["glyphs"]:
        char_code = glyph['char_code']
        header_data = [char_code & 0xFF, char_code >> 8, glyph['ofs_y'], glyph['box_w'],
                        glyph['box_h'], glyph['ofs_x'], glyph['adv_w']]
        bitmap_data = font_data["bitmap"][char_code]
        file_data += f"        /* U+{char_code:04X} '{chr(char_code)}' */\n        "
        file_data += ", ".join([f"0x{byte:02X}" for byte in header_data])
        file_data += f",\n        "
        if len(bitmap_data) > 0:
            file_data += ", ".join([f"0x{byte:02X}" for byte in bitmap_data])
            file_data += f","
        file_data += "\n"
    file_data += "\n"
    file_data += "        // Terminator\n"
    file_data += "        0x00, 0x00,\n"
    file_data += "    };\n"
    file_data += "};\n"

    return file_data

def select_file():
    filename = filedialog.askopenfilename(
        title='Load Font',
        initialdir=os.getcwd(),
        filetypes=(('True Type Font', '*.ttf'), ('All files', '*.*')))

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
    bounding_box_bool = IntVar()  # 0 for unchecked, 1 for checked
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