from tkinter import Tk, Button, Frame, Canvas, filedialog, Checkbutton, IntVar, Label, StringVar, Entry, DISABLED, NORMAL
import os

font_size = 14
max_height = 0
char_code_edit = ord('R')

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

def renderCfont():
    canvas.delete("all")
    global bitmap_data

    global list_bbox
    list_bbox = []

    offset_x_1 = 1
    offset_y_1 = 1

    global list_glyph_data

    # we get the char list to render (we shift the indexes by 32 because 32 first chars aren't included)
    if list_char_render.get() != "":
        list_char_code_render = [ord(i)-32 for i in (list_char_render.get())]
        b2.config(state=DISABLED)
    else:
        list_char_code_render = [i for i in range(0, 255-32)]
        b2.config(state=NORMAL)

    for char_code in list_char_code_render:
        offset_y = list_glyph_data[char_code]['ofs_y']
        width = list_glyph_data[char_code]['box_w']
        height = list_glyph_data[char_code]['box_h']
        offset_x = list_glyph_data[char_code]['ofs_x']
        xDelta = list_glyph_data[char_code]['adv_w']

        offset_x_1 += offset_x

        if bounding_box_bool.get():
            canvas.create_rectangle((offset_x_1)*p_size, (offset_y_1+offset_y)*p_size, (width+offset_x_1)*p_size, (height+offset_y_1+offset_y)*p_size, width=1, outline="red",fill='') # bounding box

        bbox = {
            "x0": offset_x_1,
            "y0": offset_y_1+offset_y,
            "x1": width+offset_x_1,
            "y1": height+offset_y_1+offset_y,
        }

        byte_list = bitmap_data[char_code]
        bitmap_index = 0
        bit_index = 0
        byte = byte_list[bitmap_index]
        modulo_8 = (True if width*height%8 == 0 else False)

        for y in range(height):
            for x in range(width):    
                if byte & 0b10000000: # Pixel(x,y) = 1
                    canvas.create_rectangle((x+offset_x_1)*p_size, (y+offset_y_1+offset_y)*p_size, (x+offset_x_1)*p_size+p_size, (y+offset_y_1+offset_y)*p_size+p_size,fill="white")
                
                if bit_index == 7:
                    bit_index = 0
                    bitmap_index += 1
                    if not (modulo_8 and y == height-1):
                        byte = byte_list[bitmap_index]

                else:
                    byte = byte << 1 # we shift data[n] to get the next pixel on the most significant bit
                    bit_index += 1

        if offset_x_1+3*xDelta <= canva_width:
            offset_x_1 += xDelta
        else:
            offset_x_1 = 1
            offset_y_1 += font_size + 1

        list_bbox.append(bbox)


def render_single_char():
    global bitmap_data
    canvas_1.delete("all")

    # we also have to clear the matrix
    global rect_ids
    rect_ids = []
    for y in range(40):
        line = [-1] * 40
        rect_ids.append(line)

    global char_code_edit
    char_code_edit = ord(char_to_edit.get()) - 32
    global list_glyph_data

    # ov is the small pixel that stick to the mouse, we want to keep this one
    global ov
    ov = canvas_1.create_rectangle(0,0,p_size_c,p_size_c,fill="white")

    offset_y = list_glyph_data[char_code_edit]['ofs_y']
    width = list_glyph_data[char_code_edit]['box_w']
    height = list_glyph_data[char_code_edit]['box_h']
    offset_x = list_glyph_data[char_code_edit]['ofs_x']
    advance_width = list_glyph_data[char_code_edit]['adv_w']

    glyph_data_text = (
        'width : ' + str(width) + '\n' +
        'height : ' + str(height) + '\n' +
        'offset_x : ' + str(offset_x) + '\n' +
        'offset_y : ' + str(offset_y) + '\n' +
        'advance_width : ' + str(advance_width))
    output.config(text = glyph_data_text)

    global single_bbox
    single_bbox = (offset_x, offset_y, offset_x+width, offset_y+height)

    bit_index = 0

    if bounding_box_bool.get():
        canvas_1.create_rectangle((offset_x)*p_size_c, (offset_y)*p_size_c, (width+offset_x)*p_size_c, (height+offset_y)*p_size_c, width=1, outline="red",fill='') # bounding box

    byte_list = bitmap_data[char_code_edit]
    bitmap_index = 0
    byte = byte_list[bitmap_index]
    modulo_8 = (True if width*height%8 == 0 else False)
    for y in range(height):
        for x in range(width):
            if byte & 0b10000000: # Pixel(x,y) = 1
                rect_ids[y+offset_y][x+offset_x] = canvas_1.create_rectangle(
                    (x+offset_x)*p_size_c, 
                    (y+offset_y)*p_size_c, 
                    (x+offset_x)*p_size_c+p_size_c, 
                    (y+offset_y)*p_size_c+p_size_c,fill="white")

            if bit_index == 7:
                bit_index = 0
                bitmap_index += 1
                if not (modulo_8 and y == height-1):
                    byte = byte_list[bitmap_index]

            else:
                byte = byte << 1 # we shift data[n] to get the next pixel on the most significant bit
                bit_index += 1


def update_glyph_data():
    x0, y0, x1, y1 = single_bbox
    height = y1 - y0
    width = x1 - x0
    
    global char_code_edit
    global bitmap_data

    bitmap_data[char_code_edit] = []

    row = 0
    i = 0
    for y in range(height):
        for x in range(width):
            pixel = (1 if rect_ids[y + y0][x + x0] != -1 else 0)
            if i == 8:
                bitmap_data[char_code_edit].append(row)
                row = 0
                i = 0
            row = (row << 1) | pixel
            i += 1

    row = row << 8-i # to "fill" with zero the remaining empty bits
    bitmap_data[char_code_edit].append(row)


def save_font():
    global bitmap_data
    global list_glyph_data
    global max_height
    global font_path

    save_file(font_path, {
        "bitmap": bitmap_data,
        "max_height": max_height,
        "glyphs": list_glyph_data,
    })


def save_file(font_path, font_data):

    font_name = os.path.splitext(os.path.basename(font_path))[0]

    with open (font_path, 'w', encoding='ISO8859-1') as f:
        # Output header
        f.write(header_start)
        f.write(f"// Font           : {font_name}\n")
        f.write(f"// Point Size     : {'font_height'}\n")
        f.write(f"// Memory usage   : {'999'} bytes\n\n")

        # writing bitmap data
        f.write(f"uint8_t {font_name}_glyph_bitmap[] = ")
        f.write( "{\n")

        glyph_index = 0
        for glyph, bitmap_data in zip(font_data["glyphs"], font_data["bitmap"]):

            f.write(f"    /* {chr(glyph_index+32)} */\n    ")
            if bitmap_data:
                f.write( ",".join([f"0x{byte:02X}" for byte in bitmap_data]))
            else:
                f.write('0x0')
            f.write( ",\n\n")

            glyph_index += 1

        f.write("};\n\n")

        f.write(f"static const rg_font_glyph_dsc_t {font_name}_glyph_dsc[] = ")
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


        f.write(f"const rg_font_t font_{font_name} = ")
        f.write("{\n")
        f.write(f"    .bitmap_data = {font_name}_glyph_bitmap,\n")
        f.write(f"    .glyph_dsc = {font_name}_glyph_dsc,\n")
        f.write(f"    .name = \"{font_name[:-2]}\",\n")
        f.write(f"    .height = {font_data['max_height']}\n")
        f.write("};\n")


def extract_data():
    array_index = 0
    global bitmap_data
    bitmap_data = []
    byte_list = []
    global list_glyph_data
    list_glyph_data = []
    with open(font_path, 'r', encoding='ISO8859-1') as file:

        # TODO: get the header

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
                        string = ""
                        for letter in part.split('=')[1]:
                            if letter == '}':
                                break
                            string += letter
                        ofs_y = int(string)

                        glyph_data = {
                        "bitmap_index": bitmap_index,
                        "adv_w": adv_w,
                        "box_w": box_w,
                        "box_h": box_h,
                        "ofs_x": ofs_x,
                        "ofs_y": ofs_y
                        }
                        list_glyph_data.append(glyph_data)

    # convert the byte list to an organised list for each glyph
    bitmap_data = []
    for glyph in list_glyph_data:
        width = glyph['box_w']
        height = glyph['box_h']
        bitmap_index = glyph['bitmap_index']

        if width*height != 0:
            byte = []
            for i in range((width*height)//8 + (1 if width*height%8 != 0 else 0)):
                byte.append(byte_list[bitmap_index + i])
        else:
            byte = [0]

        bitmap_data.append(byte)

    # find max height
    global max_height
    max_height = 0
    for glyph in list_glyph_data:
        max_height = max(glyph['box_h'] + glyph['ofs_y'], max_height)

    b3.config(state=NORMAL)
    b4.config(state=NORMAL)


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
    extract_data()


def motion(event):
    global x
    global y
    x = event.x
    y = event.y
    if x%p_size_c != 0:
        x-= x%p_size_c
    if y%p_size_c != 0:
        y-= y%p_size_c
    canvas_1.itemconfig(ov, fill="White")
    canvas_1.coords(ov, x, y, x+p_size_c, y+p_size_c)


def click(event):
    x_pixel = x//p_size_c
    y_pixel = y//p_size_c
    if rect_ids[y_pixel][x_pixel] == -1:
        rect_ids[y_pixel][x_pixel] = canvas_1.create_rectangle(x, y, x + p_size_c, y + p_size_c, fill="white")
    else:
        canvas_1.delete(rect_ids[y_pixel][x_pixel])
        rect_ids[y_pixel][x_pixel] = -1
        canvas_1.itemconfig(ov, fill="Black")  # Changes the fill color to black to "hide" it


def slide(event):
    global x
    global y
    global lastrect_xy
    x = event.x
    y = event.y
    if x%p_size_c != 0:
        x-= x%p_size_c
    if y%p_size_c != 0:
        y-= y%p_size_c
    canvas_1.coords(ov, x, y, x+p_size_c, y+p_size_c)

    x_pixel = x//p_size_c
    y_pixel = y//p_size_c
    if rect_ids[y_pixel][x_pixel] != lastrect_xy:
        if rect_ids[y_pixel][x_pixel] == -1:
            rect_ids[y_pixel][x_pixel] = canvas_1.create_rectangle(x, y, x + p_size_c, y + p_size_c, fill="white")
        else:
            canvas_1.delete(rect_ids[y_pixel][x_pixel])
            rect_ids[y_pixel][x_pixel] = -1
        lastrect_xy = (y_pixel,x_pixel)


window = Tk()
window.title("C font editor")

# Get screen width and height
screen_width = window.winfo_screenwidth()
screen_height = window.winfo_screenheight()
# Set the window size to fill the entire screen
window.geometry(f"{screen_width}x{screen_height}")

# TODO : make it dynamic
p_size = 8 # pixel size on the global renderer
p_size_c = 24 # pixel size on the single char renderer

char_edit_windows_width = (screen_width // 4)//p_size_c
char_edit_windows_height = (screen_height // 2)//p_size_c

canva_width = (screen_width-screen_width // 4)//p_size
canva_height = screen_height//p_size-16

frame = Frame(window)
frame.pack(anchor="center", padx=10, pady=2)

rect_ids = []  # This is gonna be used to store rectangles ids
for y in range(40):
    line = [-1] * 40  # Create a new list for each row
    rect_ids.append(line)

########## top ##########
# choose font button
choose_font_button = Button(frame, text='Choose C font', width=16, height=2, background="blue", foreground="white", command=select_file)
choose_font_button.pack(side="left", padx=5)

# Variable to hold the state of the checkbox
bounding_box_bool = IntVar()  # 0 for unchecked, 1 for checked
checkbox = Checkbutton(frame, text="Bounding box", variable=bounding_box_bool)
checkbox.pack(side="left", padx=5)

# Label and Entry for String to render
Label(frame, text="String to render").pack(side="left", padx=5)
list_char_render = StringVar(value=str(""))
Entry(frame, textvariable=list_char_render, width=50).pack(side="left", padx=5)

b1 = Button(frame, text="Render", width=12, height=2, background="blue", foreground="white", command=renderCfont)
b1.pack(side="left", padx=5)

b2 = Button(frame, text="Export", width=12, height=2, background="green", foreground="white", command=save_font)
b2.pack(side="left", padx=5)
########## end of top ##########

########## bottom ##########
frame_bottom = Frame(window)
frame_bottom.pack(anchor="s", padx=2, pady=2)

##### left side #####
frame_left = Frame(frame_bottom)
frame_left.pack(anchor="center", side="left", padx=2, pady=2)

# Label and Entry for Chars to render
Label(frame_left, text="Character to edit").pack(side="top", pady=2)
char_to_edit = StringVar(value=str("R"))
Entry(frame_left, textvariable=char_to_edit, width=4).pack(side="top", pady=2)

b3 = Button(frame_left, text="render", width=12, height=2, background="blue", foreground="white", command=render_single_char)
b3.pack(side="top", padx=5)

# display the glyph data
Label(frame_left, text="Glyph data :").pack(side="top", pady=2)
output = Label(frame_left, height = 5, width = 25, bg = "light cyan")
output.pack(side="top", padx=5)

b4 = Button(frame_left, text="save", width=12, height=2, background="green", foreground="white", command=update_glyph_data)
b4.pack(side="top", padx=5)

# disable buttons until a font is loaded
b3.config(state=DISABLED)
b4.config(state=DISABLED)

canvas_1 = Canvas(frame_left, width=char_edit_windows_width*p_size_c, height=char_edit_windows_height*p_size_c, bg="black")
canvas_1.pack(side="left", padx=5)

# ov is the small pixel that 'stick' to the mouse
ov = canvas_1.create_rectangle(0,0,p_size_c,p_size_c,fill="white")

canvas_1.focus_set()
canvas_1.bind('<Motion>', motion)
canvas_1.bind("<Button 1>",click)
canvas_1.bind("<B1-Motion>",slide)
##### end of left side #####

##### right side #####
frame_right = Frame(frame_bottom)
frame_right.pack(side="right", padx=2, pady=2)
canvas = Canvas(frame_right, width=canva_width*p_size, height=canva_height*p_size, bg="black")
canvas.pack(anchor="n", side="left", padx=5)
##### end of right side #####
########## end of bottom ##########

window.mainloop()