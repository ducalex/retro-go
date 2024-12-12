from tkinter import Tk, Button, Frame, Canvas, filedialog, ttk, Checkbutton, IntVar
import os

font_size = 14

list_bbox = [] # ((x0, y0, x1, y1), (x0, y0, x1, y1), ...) used to find the correct pixels on the canva
list_glyph_data = [] # contain font data for all glyphs

lastrect_xy = (0,0) # used for sliding function

header_start = """
/*
*  This file was generated using font_converter.py
*  Checkout https://github.com/ducalex/retro-go/tree/dev/tools for more informations on the format
*/

#include \"../rg_gui.h\"

"""

def renderCfont(byte_list):
    canvas.delete("all")

    # we also have to clear the matrix
    global rect_ids
    rect_ids = []
    for y in range(canva_height):
        line = [-1] * canva_width
        rect_ids.append(line)

    global list_bbox
    list_bbox = []

    # ov is the small pixel that stick to the mouse, we want to keep this one
    global ov
    ov = canvas.create_rectangle(0,0,pixel_size,pixel_size,fill="white")

    offset_x_1 = 1
    offset_y_1 = 1

    global list_glyph_data

    for glyph in list_glyph_data:
        bitmap_index = glyph['bitmap_index']
        offset_y = glyph['ofs_y']
        width = glyph['box_w']
        height = glyph['box_h']
        xOffset = glyph['ofs_x']
        xDelta = glyph['adv_w']

        bit_index = 0

        if bounding_box_bool.get():
            canvas.create_rectangle((offset_x_1)*pixel_size, (offset_y_1+offset_y)*pixel_size, (width+offset_x_1)*pixel_size, (height+offset_y_1+offset_y)*pixel_size, width=1, outline="blue",fill='') # bounding box

        bbox = {
            "x0": offset_x_1,
            "y0": offset_y_1+offset_y,
            "x1": width+offset_x_1,
            "y1": height+offset_y_1+offset_y,
        }

        for y in range(height):
            for x in range(width):
                if byte_list[bitmap_index] & 0b10000000: # Pixel(x,y) = 1
                    rect_ids[y+offset_y_1+offset_y][x+offset_x_1] = canvas.create_rectangle((x+offset_x_1)*pixel_size, (y+offset_y_1+offset_y)*pixel_size, (x+offset_x_1)*pixel_size+pixel_size, (y+offset_y_1+offset_y)*pixel_size+pixel_size,fill="white")

                if bit_index == 7:
                    bit_index = 0
                    bitmap_index += 1

                else:
                    byte_list[bitmap_index] = byte_list[bitmap_index] << 1 # we shift data[n] to get the next pixel on the most significant bit
                    bit_index += 1

        if offset_x_1+2*width+6 <= canva_width:
            offset_x_1 += width + 2
        else:
            offset_x_1 = 1
            offset_y_1 += font_size + 1

        list_bbox.append(bbox)

def generate_font_data():
    # Initialize the font data structure
    bitmap_data = []

    for glyph_data, bbox in zip(list_glyph_data, list_bbox):
        width, height = glyph_data['box_w'], glyph_data['box_h']
        x0, y0 = bbox['x0'], bbox['y0']
        
        # Extract bitmap data
        bitmap = []
        row = 0
        bit_count = 0

        for y in range(height):
            for x in range(width):
                # Determine pixel value
                pixel = (1 if rect_ids[y + y0][x + x0] != -1 else 0)

                # Build the row bit by bit
                row = (row << 1) | pixel
                bit_count += 1

                # Append row when full
                if bit_count == 8:
                    bitmap.append(row)
                    row = 0
                    bit_count = 0

        # Append the remaining bits, padding with zeros if necessary
        if bit_count > 0:
            row <<= (8 - bit_count)  # Fill remaining bits with zeros
            bitmap.append(row)

        # Update font data with the bitmap
        bitmap_data.append(bitmap)

    # find max width/height
    max_height = 0
    for glyph in list_glyph_data:
        max_height = max(glyph['box_h'], max_height)

    save_file(font_path, {
        "bitmap": bitmap_data,
        "max_height": max_height,
        "glyphs": list_glyph_data,
    })
    
def save_file(font_path, font_data):
    with open (font_path, 'w', encoding='ISO8859-1') as f:
        # Output header
        f.write(header_start)
        f.write(f"// Font           : {'font_name'}\n")
        f.write(f"// Point Size     : {'font_height'}\n")
        f.write(f"// Memory usage   : {'999'} bytes\n\n")

        # writing bitmap data
        f.write(f"uint8_t {'font_name'}_glyph_bitmap[] = ")
        f.write( "{\n")

        bitmap_index, glyph_index = (0,0)
        for glyph in font_data["glyphs"]:
            bitmap_data = font_data["bitmap"][glyph_index]

            f.write(f"    /* {chr(glyph_index+32)} */\n    ")
            if bitmap_data:
                f.write( ",".join([f"0x{byte:02X}" for byte in bitmap_data]))
            else:
                f.write('0x0')
            f.write( ",\n\n")

            glyph["bitmap_index"] = bitmap_index
            bitmap_index += len(bitmap_data)

            glyph_index += 1

        f.write("};\n\n")

        f.write(f"static const rg_font_glyph_dsc_t {'font_name'}_glyph_dsc[] = ")
        f.write("{\n")

        for glyph in font_data["glyphs"]:
            f.write("    {.bitmap_index = ")
            f.write(str(glyph["bitmap_index"]))

            f.write(", .adv_w = ")
            f.write(str(glyph["adv_w"]))

            f.write(", .box_w = ")
            f.write(str(glyph["box_w"]))

            f.write(", .box_h = ")
            f.write(str(glyph["box_h"]))

            f.write(", .ofs_x = ")
            f.write(str(glyph["ofs_x"]))

            f.write(", .ofs_y = ")
            f.write(str(glyph["ofs_y"]))

            f.write("},\n")

        f.write("};\n\n")


        f.write(f"const rg_font_t font_{'font_name'} = ")
        f.write("{\n")
        f.write(f"    .bitmap_data = {'font_name'}_glyph_bitmap,\n")
        f.write(f"    .glyph_dsc = {'font_name'}_glyph_dsc,\n")
        f.write(f"    .name = \"{'font_name'}\",\n")
        f.write(f"    .height = {font_data['max_height']}\n")
        f.write("};\n")

def extract_bytes():
    array_index = 0
    byte_list = []
    global list_glyph_data
    list_glyph_data = []
    with open(font_path, 'r', encoding='ISO8859-1') as file:
        inside_data_section = False
        for line in file:
            line = line.strip()
            # skip commented lines
            if '/*' in line or '//' in line:
                continue
            
            # Detect when the .data section starts
            if '[] = {' in line:
                array_index += 1
                inside_data_section = True
                continue

            # Detect when the font struct starts
            if 'const rg_font_t' in line:
                inside_data_section = True
                continue

            # Detect when the .data section ends
            if inside_data_section and '};' in line:
                inside_data_section = False
            # Skip lines outside the .data section

            if not inside_data_section:
                continue

            if array_index == 1: # bitmap pixels data
                # Extract bytes from the line
                parts = line.split(',')
                for part in parts:
                    part = part.strip()
                    if part.startswith('0x'):
                        try:
                            byte_list.append(int(part, 16))
                        except ValueError:
                            pass

            if array_index == 2: # glyph descriptor
                parts = line.split(',')
                for part in parts:
                    if "bitmap_index" in part:
                        bitmap_index = int(part.split('=')[1])
                    elif "adv_w" in part:
                        adv_w = int(part.split('=')[1])
                    elif "box_w" in part:
                        box_w = int(part.split('=')[1])
                    elif "box_h" in part:
                        box_h = int(part.split('=')[1])
                    elif "ofs_x" in part:
                        ofs_x = int(part.split('=')[1])
                    elif "ofs_y" in part:
                        ofs_y = int(part.split('=')[1][1])
                        glyph_data = {
                        "bitmap_index": bitmap_index,
                        "adv_w": adv_w,
                        "box_w": box_w,
                        "box_h": box_h,
                        "ofs_x": ofs_x,
                        "ofs_y": ofs_y
                        }
                        print(glyph_data)
                        list_glyph_data.append(glyph_data)

    renderCfont(byte_list)

def select_file():
    filetypes = (
        ('c font', '*.c'),
        ('All files', '*.*')
    )

    filename = filedialog.askopenfilename(
        title='Open a c Font',
        initialdir=os.getcwd(),
        filetypes=filetypes)

    global font_path
    font_path = filename

def motion(event):
    global x
    global y
    x = event.x
    y = event.y
    if x%pixel_size != 0:
        x-= x%pixel_size
    if y%pixel_size != 0:
        y-= y%pixel_size
    canvas.itemconfig(ov, fill="White")
    canvas.coords(ov, x, y, x+pixel_size, y+pixel_size)

def click(event):
    x_pixel = x//pixel_size
    y_pixel = y//pixel_size
    if rect_ids[y_pixel][x_pixel] == -1:
        rect_ids[y_pixel][x_pixel] = canvas.create_rectangle(x, y, x + pixel_size, y + pixel_size, fill="white")
    else:
        canvas.delete(rect_ids[y_pixel][x_pixel])
        rect_ids[y_pixel][x_pixel] = -1
        canvas.itemconfig(ov, fill="Black")  # Changes the fill color to black to "hide" it

def slide(event):
    global x
    global y
    global lastrect_xy
    x = event.x
    y = event.y
    if x%pixel_size != 0:
        x-= x%pixel_size
    if y%pixel_size != 0:
        y-= y%pixel_size
    canvas.coords(ov, x, y, x+pixel_size, y+pixel_size)

    x_pixel = x//pixel_size
    y_pixel = y//pixel_size
    if rect_ids[y_pixel][x_pixel] != lastrect_xy:
        if rect_ids[y_pixel][x_pixel] == -1:
            rect_ids[y_pixel][x_pixel] = canvas.create_rectangle(x, y, x + pixel_size, y + pixel_size, fill="white")
        else:
            canvas.delete(rect_ids[y_pixel][x_pixel])
            rect_ids[y_pixel][x_pixel] = -1
        lastrect_xy = (y_pixel,x_pixel)


window = Tk()
window.title("C font editor")

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

rect_ids = []  # This is gonna be used to store rectangles ids
for y in range(canva_height):
    line = [-1] * canva_width  # Create a new list for each row
    rect_ids.append(line)

# choose font button
choose_font_button = ttk.Button(frame, text='Choose C font', command=select_file)
choose_font_button.pack(side="left", padx=5)

# Variable to hold the state of the checkbox
bounding_box_bool = IntVar()  # 0 for unchecked, 1 for checked
checkbox = Checkbutton(frame, text="Bounding box", variable=bounding_box_bool)
checkbox.pack(side="left", padx=5)

b1 = Button(frame, text="Render", width=14, height=2, background="blue", foreground="white", command=extract_bytes)
b1.pack(side="left", padx=5)

b1 = Button(frame, text="Export", width=14, height=2, background="green", foreground="white", command=generate_font_data)
b1.pack(side="left", padx=5)

frame = Frame(window).pack(anchor="w", padx=2, pady=2)
canvas = Canvas(frame, width=canva_width*pixel_size, height=canva_height*pixel_size, bg="black")

# ov is the small pixel that 'stick' to the mouse
ov = canvas.create_rectangle(0,0,pixel_size,pixel_size,fill="white")

canvas.focus_set()
canvas.bind('<Motion>', motion)
canvas.bind("<Button 1>",click)
canvas.bind("<B1-Motion>",slide)
canvas.pack(side="left", padx=5)

window.mainloop()