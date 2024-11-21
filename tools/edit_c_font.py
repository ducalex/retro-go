from tkinter import Tk, Button, Frame, Canvas, filedialog, ttk, Checkbutton, IntVar
import os

windows_x = 1280
windows_y = 720

pixel_size = 6
font_size = 14

canva_width = windows_x//pixel_size
canva_height = windows_y//pixel_size

rect_ids = []  # This is gonna be used to store rectangles ids
for y in range(canva_height):
    line = [-1] * canva_width  # Create a new list for each row
    rect_ids.append(line)

list_bbox = [] # ((x0, y0, x1, y1), (x0, y0, x1, y1), ...) used to find the correct pixels on the canva
font_data = [] # contain font data for all glyphs

def edit_C_char(byte_list, list_char, list_pixel):
    byte_index = 0
    for char in list_char:
        while byte_list[byte_index] != char:
            width = byte_list[byte_index+2]
            height = byte_list[byte_index+3]
            bit_index = 0
            data_index = byte_index+6

            print(byte_list[byte_index])


def renderCfont(byte_list):
    canvas.delete("all")

    # we also have to clear the matrix
    global rect_ids
    rect_ids = []
    for y in range(canva_height):
        line = [-1] * canva_width
        rect_ids.append(line)

    global font_data
    font_data = []

    global list_bbox
    list_bbox = []

    # ov is the small pixel that stick to the mouse, we want to keep this one
    global ov
    ov = canvas.create_rectangle(0,0,pixel_size,pixel_size,fill="white")

    byte_index = 0
    offset_x_1 = 1
    offset_y_1 = 1

    while byte_index <= len(byte_list)-5:
        char_code = byte_list[byte_index]
        offset_y = byte_list[byte_index+1]
        width = byte_list[byte_index+2]
        height = byte_list[byte_index+3]
        xOffset = byte_list[byte_index+4]
        xDelta = byte_list[byte_index+5]

        bit_index = 0
        data_index = byte_index+6

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
                if byte_list[data_index] & 0b10000000: # Pixel(x,y) = 1
                    rect_ids[y+offset_y_1+offset_y][x+offset_x_1] = canvas.create_rectangle((x+offset_x_1)*pixel_size, (y+offset_y_1+offset_y)*pixel_size, (x+offset_x_1)*pixel_size+pixel_size, (y+offset_y_1+offset_y)*pixel_size+pixel_size,fill="white")

                if bit_index == 7:
                    bit_index = 0
                    data_index += 1

                else:
                    byte_list[data_index] = byte_list[data_index] << 1 # we shift data[n] to get the next pixel on the least significant bit
                    bit_index += 1

        if offset_x_1+2*width+6 <= canva_width:
            offset_x_1 += width + 2
        else:
            offset_x_1 = 1
            offset_y_1 += font_size + 1

        
        if (width*height)%8 != 0:
            byte_index += ((width*height)//8+7)

        else:
            byte_index += (((width*height)//8+6) if (width*height) != 0 else 6)

        glyph_data = {
            "char_code": char_code,
            "y_offset": offset_y,
            "width": width,
            "height": height,
            "x_offset": xOffset,
            "x_delta": xDelta,
            "data": [],
        }

        font_data.append(glyph_data)
        list_bbox.append(bbox)

def generate_font_data():
    # Initialize the font data structure

    for font, bbox in zip(font_data, list_bbox):
        width, height = font['width'], font['height']
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
        font['data'] = bitmap
        print(font['data'])
        
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

    save_file(font_path, {
        "header": header,
        "glyphs": font_data,
    })
    
def save_file(font_name, font_data):
    with open (font_name, 'w', encoding='ISO8859-1') as f:
        # Output header

        f.write("#include \"../rg_gui.h\"\n\n")
        f.write(f"// Font           : {'arial'}\n")
        f.write(f"// Point Size     : 12\n")
        f.write(f"// Treshold Value : {0}\n")
        f.write(f"// Memory usage   : {0} bytes\n")
        f.write(f"// # characters   : {0}\n\n")
        f.write(f"const rg_font_t font_VeraBold12 = ")
        f.write("{\n")
        f.write(f"    .name = \"{'test'}\",\n")
        f.write("    .type = 1,\n")
        f.write(f"    .width = {font_data['header']['char_width']},\n")
        f.write(f"    .height = {font_data['header']['char_height']+3},\n")
        f.write(f"    .chars = {0},\n")
        f.write("    .data = {\n")

        # output glyph data
        for glyph in font_data["glyphs"]:
            f.write(f"        // '{chr(glyph['char_code'])}'\n")
            f.write(f"        0x{glyph['char_code']:02X},0x{glyph['y_offset']:02X},0x{glyph['width']:02X},"
                    f"0x{glyph['height']:02X},0x{glyph['x_offset']:02X},0x{glyph['x_delta']:02X},\n        ")
            f.write(",".join([f"0x{byte:02X}" for byte in glyph["data"]]) + ",\n")

        f.write("\n    // Terminator\n")
        f.write("    0xFF,\n")
        f.write("  },\n")
        f.write("};\n")

def extract_bytes():
    byte_list = []
    with open(font_path, 'r', encoding='ISO8859-1') as file:
        inside_data_section = False
        for line in file:
            line = line.strip()
            # skip commented lines
            if '//' in line:
                continue
            # Detect when the .data section starts
            if '.data = {' in line:
                inside_data_section = True
                continue
            # Detect when the .data section ends
            if inside_data_section and '}' in line:
                break
            # Skip lines outside the .data section
            if not inside_data_section:
                continue
            # Extract bytes from the line
            parts = line.split(',')
            for part in parts:
                part = part.strip()
                if part.startswith('0x'):
                    try:
                        byte_list.append(int(part, 16))
                    except ValueError:
                        pass

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
    canvas.itemconfig(ov, fill="White")  # Changes the fill color to black to "hide" it
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
    x = event.x
    y = event.y
    if x%pixel_size != 0:
        x-= x%pixel_size
    if y%pixel_size != 0:
        y-= y%pixel_size
    canvas.coords(ov, x, y, x+pixel_size, y+pixel_size)

    x_pixel = x//pixel_size
    y_pixel = y//pixel_size
    if rect_ids[y_pixel][x_pixel] == -1:
        rect_ids[y_pixel][x_pixel] = canvas.create_rectangle(x, y, x + pixel_size, y + pixel_size, fill="white")
    else:
        canvas.delete(rect_ids[y_pixel][x_pixel])
        rect_ids[y_pixel][x_pixel] = -1

window = Tk()
window.title("C font editor")

frame = Frame(window)
frame.pack(anchor="center", padx=10, pady=2)

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