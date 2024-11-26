from PIL import Image, ImageDraw, ImageFont, ImageTk
from tkinter import Tk, Label, Entry, StringVar, Button, Frame, Canvas, filedialog, ttk, Checkbutton, IntVar
import os

############################### - Data structure - ###############################
#
# 0 : Character Code
# 1 : Adjusted Y Offset
# 2 : Width
# 3 : Height
# 4 : xOffset
# 5 : xDelta (the distance to move the cursor. Effective width of the character.)
# 6 : Data[n]
# 
# example :
# // '0' in DejaVuSans18
#
#  0    1    2    3    4    5
#  |    |    |    |    |    |
# 0x30,0x01,0x09,0x0D,0x01,0x0B,
# 0x3E,0x3F,0x98,0xD8,0x3C,0x1E,0x0F,0x07,0x83,0xC1,0xE0,0xD8,0xCF,0xE3,0xE0,
#   \___|____|____|____|____|____|____|____|____|____|____|____|____|___/
#                                  |
#                                  6
# 
########################### - Explanation of Data[n] - ###########################
# First, let's see an example : '!'
# 0x21,0x04,0x03,0x09,0x00,0x03,
# 0xFF,0xFF,0xC7,0xE0,
#
# - width = 0x03 = 3
# - height = 0x09 = 9
# - data[n] = 0xFF,0xFF,0xC7,0xE0
# 
# we are going to convert hex data[n] bytes to binary :
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

treshold_init = 110 # tip : lower if too thin letters / missing pixel

first_char_init = 32
last_char_init = 255

list_char_exclude_init = "127-160, 192-221"

# Example usage (defaults parameters)
font_name_init = "arial"
font_path = ("arial.ttf")  # Replace with your TTF font path
font_size_init = 11

# Variables to track panning
start_x = 0
start_y = 0

def get_char_list():
    list_char_exclude_int = []
    if list_char_exclude.get() != '':
        for intervals in list_char_exclude.get().split(','):
            first = intervals.split('-')[0]
            # we check if we the user input is a single char or an interval
            try:
                second = intervals.split('-')[1]
            except IndexError:
                list_char_exclude_int.append(int(first))
            else:
                second = intervals.split('-')[1]
                for char in range(int(first), int(second)):
                    list_char_exclude_int.append(char)

    list_char = []
    for char_code in range(int(first_char.get()), int(last_char.get())):
        if char_code not in list_char_exclude_int:
            list_char.append(char_code)

    return list_char    

def pre_compute_x_delta_space(treshold, font_size, pil_font):
    # pre-compute the x_delta_space for control/space char
    image_control_char = Image.new("L", (font_size * 2, font_size * 2), 0)
    draw_control_char = ImageDraw.Draw(image_control_char)
    draw_control_char.text((0, 0), chr(1), font=pil_font, fill=255)
    pixels = image_control_char.load()  # Load the pixel data into a pixel access object
    for x in range(image_control_char.width):
        for y in range(image_control_char.height):
            if pixels[x, y] >= treshold:  # play with the treshold value to get the best quality
                pixels[x, y] = 1  # Set the pixel to white
            else:
                pixels[x, y] = 0  # Set the pixel to black

    x0, y0, x1, y1 = find_bounding_box(image_control_char)  # Get bounding box
    return (x1 - x0)

def find_bounding_box(image):
    pixels = image.load()
    width, height = image.size
    x_min, y_min = width, height
    x_max, y_max = 0, 0

    for y in range(height):
        for x in range(width):
            if pixels[x, y] == 1:  # Looking for 'on' pixels
                x_min = min(x_min, x)
                y_min = min(y_min, y)
                x_max = max(x_max, x)
                y_max = max(y_max, y)

    if x_min > x_max or y_min > y_max:  # No target pixels found
        return None
    return (x_min, y_min, x_max+1, y_max+1)

def generate_font_data():
    font_name = font_name_input.get()
    font_size = int(font_height_input.get())
    # Load the TTF font
    pil_font = ImageFont.truetype(font_path, font_size)

    # Initialize the font data structure
    font_data = []
    memory_usage = 0
    num_characters = 0

    canvas.delete("all")
    treshold = int(treshold_input.get())
    offset_x_1 = 1
    offset_y_1 = 1

    x_delta_space = pre_compute_x_delta_space(treshold, font_size, pil_font)

    for char_code in get_char_list():
        char = chr(char_code)
        print("char : " + char)
        # Render character to an image and get its bounding box
        image = Image.new("L", (font_size * 2, font_size * 2), 0)
        draw = ImageDraw.Draw(image)
        draw.text((0, 0), char, font=pil_font, fill=255)


        ### - MOST IMPORTANT PART - ###
        # this step convert 8bit grayscale image to 1bit color (0 or 1)
        # TODO : make it better (or can we ?)

        pixels = image.load()  # Load the pixel data into a pixel access object
        for x in range(image.width):
            for y in range(image.height):
                if pixels[x, y] >= treshold:  # play with the treshold value to get the best quality
                    pixels[x, y] = 1  # Set the pixel to white
                else:
                    pixels[x, y] = 0  # Set the pixel to black

        ### - END OF MOST IMPORTANT PART - ###


        bbox = find_bounding_box(image)  # Get bounding box

        if bbox is None: # control character / space
            # Create glyph entry
            glyph_data = {
                "char_code": char_code,
                "y_offset": 0,
                "width": 0,
                "height": 0,
                "x_offset": 0,
                "x_delta": x_delta_space,
                "data": [],
            }
            font_data.append(glyph_data)
            num_characters += 1
            continue  # Skip if character has no valid bounding box
        
        x0, y0, x1, y1 = bbox
        width, height = x1 - x0, y1 - y0
        offset_x, offset_y = x0, y0

        # Crop the image to the bounding box
        cropped_image = image.crop(bbox)

        # Extract bitmap data
        bitmap = []
        row = 0
        i = 0

        if bounding_box_bool.get():
            canvas.create_rectangle((offset_x_1)*pixel_size, (offset_y_1+offset_y)*pixel_size, (width+offset_x_1)*pixel_size, (height+offset_y_1+offset_y)*pixel_size, width=1, outline="blue",fill='') # bounding box

        for y in range(height):
            for x in range(width):
                pixel = cropped_image.getpixel((x, y))
                if i >= 8:
                    bitmap.append(row)
                    row = 0
                    i = 0
                row = (row << 1) | pixel
                if pixel:
                    canvas.create_rectangle((x+offset_x_1)*pixel_size, (y+offset_y_1+offset_y)*pixel_size, (x+offset_x_1)*pixel_size+pixel_size, (y+offset_y_1+offset_y)*pixel_size+pixel_size,fill="white")
                i += 1

        row = row << 8-i # to "fill" with zero the remaining empty bits
        bitmap.append(row)

        if offset_x_1+2*width+6 <= canva_width:
            offset_x_1 += width + 2
        else:
            offset_x_1 = 1
            offset_y_1 += font_size + 2

        # Create glyph entry
        glyph_data = {
            "char_code": char_code,
            "y_offset": offset_y,
            "width": width,
            "height": height,
            "x_offset": offset_x+2, # FIXME make it better
            "x_delta": width,
            "data": bitmap,
        }
        font_data.append(glyph_data)

        # Update memory usage and character count
        memory_usage += len(bitmap) + 6  # 6 bytes for the header per character
        num_characters += 1

    # find max width/height
    max_width, max_height = (0,0)
    for glyph in font_data:
        max_width = max(glyph['width'], max_width)
        max_height = max(glyph['height'], max_height)

    # Generate header
    header = {
    "char_width": max_width,
    "char_height": max_height,
    }

    save_file(font_name, {
        "header": header,
        "glyphs": font_data,
        "memory_usage": memory_usage,
        "num_characters": num_characters,
    })

def save_file(font_name, font_data):
    with open (font_name+font_height_input.get()+".c", 'w', encoding='ISO8859-1') as f:
        # Output header
        f.write("#include \"../rg_gui.h\"\n\n")
        f.write("// This file was generated using font_converter.py\n")
        f.write("// Checkout https://github.com/ducalex/retro-go/tree/dev/tools for more informations on the format\n")
        f.write(f"// Font           : {font_name}\n")
        f.write(f"// Point Size     : {font_height_input.get()}\n")
        f.write(f"// Treshold Value : {treshold_input.get()}\n")
        f.write(f"// Memory usage   : {font_data['memory_usage']} bytes\n")
        f.write(f"// # characters   : {font_data['num_characters']}\n\n")
        f.write(f"const rg_font_t font_{font_name.replace('-', '_')+font_height_input.get()} = ")
        f.write("{\n")
        f.write(f"    .name = \"{font_name}\",\n")
        f.write( "    .type = 1,\n")
        f.write(f"    .width = {font_data['header']['char_width']},\n")
        f.write(f"    .height = {font_data['header']['char_height']+3},\n")
        f.write(f"    .chars = {font_data['num_characters']},\n")
        f.write( "    .data = {\n")

        # output glyph data
        for glyph in font_data["glyphs"]:
            f.write(f"        // '{chr(glyph['char_code'])}'\n")
            f.write(f"        0x{glyph['char_code']:02X},0x{glyph['y_offset']:02X},0x{glyph['width']:02X},"
                    f"0x{glyph['height']:02X},0x{glyph['x_offset']:02X},0x{glyph['x_delta']:02X},\n        ")
            f.write(",".join([f"0x{byte:02X}" for byte in glyph["data"]]))
            if glyph['data'] == []:
                f.write("\n")
            else:
                f.write(",\n")

        f.write("\n    // Terminator\n")
        f.write("    0xFF,\n")
        f.write("  },\n")
        f.write("};\n")

def select_file():
    filetypes = (
        ('True Type Font', '*.ttf'),
        ('All files', '*.*')
    )

    filename = filedialog.askopenfilename(
        title='Open a Font',
        initialdir=os.getcwd(),
        filetypes=filetypes)

    font_name_input.set(os.path.basename(filename)[:-4:])

    global font_path
    font_path = filename

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

window = Tk()
window.title("Font render")

# Get screen width and height
screen_width = window.winfo_screenwidth()
screen_height = window.winfo_screenheight()
# Set the window size to fill the entire screen
window.geometry(f"{screen_width}x{screen_height}")

pixel_size = 10 # pixel size on the renderer

canva_width = screen_width//pixel_size
canva_height = screen_height//pixel_size-16

frame = Frame(window)
frame.pack(anchor="center", padx=10, pady=2)

# choose font button (file picker)
choose_font_button = ttk.Button(frame, text='Choose font', command=select_file)
choose_font_button.pack(side="left", padx=5)

# Label and Entry for Font height
Label(frame, text="Font height", height=4).pack(side="left", padx=5)
font_height_input = StringVar(value=str(font_size_init))
Entry(frame, textvariable=font_height_input, width=4).pack(side="left", padx=5)

# Label and Entry for First Char
Label(frame, text="First Char", height=4).pack(side="left", padx=5)
first_char = StringVar(value=str(first_char_init))
Entry(frame, textvariable=first_char, width=4).pack(side="left", padx=5)

# Label and Entry for Last Char
Label(frame, text="Last Char", height=4).pack(side="left", padx=5)
last_char = StringVar(value=str(last_char_init))
Entry(frame, textvariable=last_char, width=4).pack(side="left", padx=5)

# Label and Entry for Char to exclude
Label(frame, text="Char to exclude", height=4).pack(side="left", padx=5)
list_char_exclude = StringVar(value=str(list_char_exclude_init))
Entry(frame, textvariable=list_char_exclude, width=30).pack(side="left", padx=5)

# Label and Entry for Font Name
Label(frame, text="Font name (used for output)", height=4).pack(side="left", padx=5)
font_name_input = StringVar(value=str(font_name_init))
Entry(frame, textvariable=font_name_input, width=20).pack(side="left", padx=5)

# Label and Entry for Treshold Value
Label(frame, text="Treshold Value (1-255)", height=4).pack(side="left", padx=5)
treshold_input = StringVar(value=str(treshold_init))
Entry(frame, textvariable=treshold_input, width=4).pack(side="left", padx=5)

# Variable to hold the state of the checkbox
bounding_box_bool = IntVar()  # 0 for unchecked, 1 for checked
Checkbutton(frame, text="Bounding box", variable=bounding_box_bool).pack(side="left", padx=10)

# Button to launch the font generation function
b1 = Button(frame, text="Generate", width=14, height=2, background="blue", foreground="white", command=generate_font_data)
b1.pack(side="left", padx=5)

frame = Frame(window).pack(anchor="w", padx=2, pady=2)
canvas = Canvas(frame, width=canva_width*pixel_size, height=canva_height*pixel_size, bg="black")
canvas.configure(scrollregion=(0, 0, canva_width*pixel_size, canva_height*pixel_size))
canvas.bind("<MouseWheel>", zoom)
canvas.bind("<ButtonPress-1>", start_pan)  # Start panning
canvas.bind("<B1-Motion>",pan_canvas)
canvas.focus_set()
canvas.pack(fill="both", expand=True)

window.mainloop()