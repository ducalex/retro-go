from PIL import Image, ImageDraw, ImageFont
from tkinter import Tk, Label, Entry, StringVar, Button, Frame, Canvas, filedialog, ttk
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

windows_x = 1280
windows_y = 720

pixel_size = 5

canva_width = windows_x//pixel_size
canva_height = windows_y//pixel_size

treshold_init = 80 # tip : lower if too thin letters / missing pixel

# Example usage
font_name_init = "OpenSans"
font_path = os.path.abspath("OpenSans-Regular.ttf")  # Replace with your TTF font path
font_size_init = 12

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

    for char_code in range(32, 255):  # ASCII printable characters
        char = chr(char_code)
        # Render character to an image and get its bounding box
        image = Image.new("L", (font_size * 2, font_size * 2), 0)
        draw = ImageDraw.Draw(image)
        draw.text((0, 0), char, font=pil_font, fill=255)
        bbox = draw.textbbox((0, 0), char, font=pil_font)  # Get bounding box
        if bbox is None:
            continue  # Skip if character has no valid bounding box
        
        x0, y0, x1, y1 = bbox
        width, height = x1 - x0, y1 - y0
        offset_x, offset_y = x0, y0

        # Crop the image to the bounding box
        cropped_image = image.crop(bbox)

        # Extract bitmap data
        bitmap = []
        print("char : " + char)
        row = 0
        i = 0
        for y in range(height):
            for x in range(width):
                pixel = cropped_image.getpixel((x, y))
                if i >= 8:
                    bitmap.append(row)
                    row = 0
                    i = 0
                row = (row << 1) | (1 if pixel > treshold else 0) # play with the condition value to get the best quality
                if pixel > treshold:
                    canvas.create_rectangle((x+offset_x_1)*pixel_size, (y+offset_y_1)*pixel_size, (x+offset_x_1)*pixel_size+pixel_size, (y+offset_y_1)*pixel_size+pixel_size,fill="white")
                i += 1

        row = row << 8-i # to "fill" with zero the remaining empty bits
        bitmap.append(row)

        if offset_x_1+2*width+6 <= canva_width:
            offset_x_1 += width + 2
        else:
            offset_x_1 = 1
            offset_y_1 += font_size + 1

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

    # Generate header
    header = {
        "char_width": 0x00,  # Marker for proportional font
        "char_height": font_size,
        "num_chars": 0x00,
    }

    max_width = 0
    for glyph in font_data:
        if glyph['width'] > max_width:
            max_width = glyph['width']
    print(f"Max width : {max_width}")

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
        f.write(f"// Font         : {font_name}\n")
        f.write(f"// Point Size   : 12\n")
        f.write(f"// Memory usage : {font_data['memory_usage']} bytes\n")
        f.write(f"// # characters : {font_data['num_characters']}\n\n")
        f.write(f"const rg_font_t font_VeraBold12 = ")
        f.write("{\n")
        f.write("    .type = 1,\n")
        f.write(f"    .name = \"{font_name}\",\n")
        f.write(f"    .width = {font_data['header']['char_width']},\n")
        f.write(f"    .height = {font_data['header']['char_height']+3},\n")
        f.write(f"    .chars = {font_data['num_characters']},\n")
        f.write("    .data = {\n")

        for glyph in font_data["glyphs"]:
            f.write(f"        // '{chr(glyph['char_code'])}'\n")
            f.write(f"        0x{glyph['char_code']:02X},0x{glyph['y_offset']:02X},0x{glyph['width']:02X},"
                    f"0x{glyph['height']:02X},0x{glyph['x_offset']:02X},0x{glyph['x_delta']:02X},\n        ")
            f.write(",".join([f"0x{byte:02X}" for byte in glyph["data"]]) + ",\n")

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
        initialdir='/',
        filetypes=filetypes)

    font_name_input.set(os.path.basename(filename)[:-4:])

    global font_path
    font_path = filename


window = Tk()
frame = Frame(window)
frame.pack(padx=20, pady=10)

lab1 = Label(frame, text="Font render")
lab1.grid(row=0, column=0, columnspan=8, padx=2, pady=2)

# choose font button
choose_font_button = ttk.Button(frame, text='Choose font', command=select_file)
choose_font_button.grid(row=1, column=0, padx=2, pady=2)


labelText=StringVar()
labelText.set("Font height")
labelDir=Label(frame, textvariable=labelText, height=4)
labelDir.grid(row=1, column=1, padx=2, pady=2)

font_height_input=StringVar(None)
font_height_input.set(str(font_size_init))
entree=Entry(frame,textvariable=font_height_input,width=20)
entree.grid(row=1, column=2, padx=2, pady=2)


labelText_2=StringVar()
labelText_2.set("Font name (used for output)")
labelDir_2=Label(frame, textvariable=labelText_2, height=4)
labelDir_2.grid(row=1, column=3, padx=2, pady=2)

font_name_input=StringVar(None)
font_name_input.set(str(font_name_init))
entree_1=Entry(frame,textvariable=font_name_input,width=20)
entree_1.grid(row=1, column=4, padx=2, pady=2)


labelText_1=StringVar()
labelText_1.set("Treshold Value (1-255)")
labelDir_1=Label(frame, textvariable=labelText_1, height=4)
labelDir_1.grid(row=1, column=5, padx=2, pady=2)

treshold_input=StringVar(None)
treshold_input.set(str(treshold_init))
entree_2=Entry(frame,textvariable=treshold_input,width=20)
entree_2.grid(row=1, column=6, padx=2, pady=2)

canvas = Canvas(frame, width=canva_width*pixel_size, height=canva_height*pixel_size, bg="black")

b1 = Button(frame, text="generate", width=14, height=2, background="blue", foreground="white", command=generate_font_data)
b1.grid(row=1, column=7, padx=2, pady=2)

canvas.focus_set()
canvas.grid(row=3, column=0, columnspan=8)
window.mainloop()