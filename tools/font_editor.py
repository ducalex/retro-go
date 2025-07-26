from tkinter import Tk, Button, Frame, Canvas, filedialog, Checkbutton, IntVar, Label, StringVar, Entry, DISABLED, NORMAL
from font_converter import load_c_font, generate_c_font
import os

font_size = 14
char_code_edit = ord('R')
selected_glyph = 0

list_bbox = [] # ((cc, x0, y0, x1, y1), (cc, x0, y0, x1, y1), ...) used to find the correct glyph on the canva
list_glyph_data = dict() # contain font data for all glyphs

lastrect_xy = (0,0) # used for sliding function

def renderCfont():
    canvas.delete("all")
    global select_box
    select_box = canvas.create_rectangle(0,0,p_size,p_size, width=2, outline="blue")

    global list_bbox
    list_bbox = []

    offset_x_1 = 1
    offset_y_1 = 1

    max_height_local = 1

    global list_glyph_data

    # we get the char list to render
    if list_char_render.get() != "":
        list_char_code_render = [ord(i) for i in (list_char_render.get())]
    else:
        list_char_code_render = list(list_glyph_data.keys())

    for char_code in list_char_code_render:
        offset_y = list_glyph_data[char_code]['ofs_y']
        width = list_glyph_data[char_code]['box_w']
        height = list_glyph_data[char_code]['box_h']
        offset_x = list_glyph_data[char_code]['ofs_x']
        xDelta = list_glyph_data[char_code]['adv_w']

        max_height_local = max(max_height_local, offset_y + height)

        offset_x_1 += offset_x

        if bounding_box_bool.get():
            canvas.create_rectangle((offset_x_1)*p_size, (offset_y_1+offset_y)*p_size, (width+offset_x_1)*p_size, (height+offset_y_1+offset_y)*p_size, width=1, outline="red",fill='') # bounding box

        bbox = (
            char_code,
            offset_x_1,
            offset_y_1+offset_y,
            width+offset_x_1,
            height+offset_y_1+offset_y
        )

        byte_list = list_glyph_data[char_code]["bitmap"]
        if byte_list:
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
            offset_y_1 += max_height_local
            max_height_local = 1

        list_bbox.append(bbox)


def render_single_char():
    canvas_1.delete("all")

    # we also have to clear the matrix
    global rect_ids
    rect_ids = []
    for y in range(40):
        line = [-1] * 40
        rect_ids.append(line)

    global char_code_edit
    char_code_edit = ord(char_to_edit.get())
    global list_glyph_data

    offset_y = list_glyph_data[char_code_edit]['ofs_y']
    width = list_glyph_data[char_code_edit]['box_w']
    height = list_glyph_data[char_code_edit]['box_h']
    offset_x = list_glyph_data[char_code_edit]['ofs_x']
    advance_width = list_glyph_data[char_code_edit]['adv_w']

    max_size = max(width+offset_x, height+offset_y)
    canvas_width = (screen_width // 4)     
    canvas_height = (screen_height // 2) 
    
    global p_size_c
    p_size_c = min(canvas_width, canvas_height) // max_size - 1

    # ov is the small pixel that stick to the mouse, we want to keep this one
    global ov
    ov = canvas_1.create_rectangle(0,0,p_size_c,p_size_c,fill="white")

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

    byte_list = list_glyph_data[char_code_edit]["bitmap"]
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

    list_glyph_data[char_code_edit]["bitmap"] = []

    row = 0
    i = 0
    for y in range(height):
        for x in range(width):
            pixel = (1 if rect_ids[y + y0][x + x0] != -1 else 0)
            if i == 8:
                list_glyph_data[char_code_edit]["bitmap"].append(row)
                row = 0
                i = 0
            row = (row << 1) | pixel
            i += 1

    row = row << 8-i # to "fill" with zero the remaining empty bits
    list_glyph_data[char_code_edit]["bitmap"].append(row)

    save_font()
    renderCfont()


def save_font():
    global list_glyph_data
    global font_path
    font_name = os.path.splitext(os.path.basename(font_path))[0]
    with open(font_path, 'w', encoding='UTF-8') as f:
        f.write(generate_c_font(font_name, font_size, list_glyph_data.values()))


def extract_data():
    global list_glyph_data
    list_glyph_data.clear()

    font_name, font_size, font_data = load_c_font(font_path)
    for glyph in font_data:
        list_glyph_data[glyph["char_code"]] = glyph

    b1.config(state=NORMAL)
    b3.config(state=NORMAL)
    b4.config(state=NORMAL)
    renderCfont()


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
    x = event.x // p_size
    y = event.y // p_size

    global selected_glyph

    for bbox in list_bbox:
        cc, x0, y0, x1, y1 = bbox
        if x >= x0 and y >= y0 and x <= x1 and y <= y1:
            canvas.coords(select_box, x0*p_size-1, y0*p_size-1, x1*p_size+1, y1*p_size+1)
            selected_glyph = cc


def click(event):
    global char_to_edit
    global selected_glyph
    char_to_edit.set(chr(selected_glyph))
    render_single_char()


def motion_1(event):
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


def click_1(event):
    x_pixel = x//p_size_c
    y_pixel = y//p_size_c
    if rect_ids[y_pixel][x_pixel] == -1:
        rect_ids[y_pixel][x_pixel] = canvas_1.create_rectangle(x, y, x + p_size_c, y + p_size_c, fill="white")
    else:
        canvas_1.delete(rect_ids[y_pixel][x_pixel])
        rect_ids[y_pixel][x_pixel] = -1
        canvas_1.itemconfig(ov, fill="Black")  # Changes the fill color to black to "hide" it


def slide_1(event):
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

# TODO : make it dynamic
p_size = 6 # pixel size on the global renderer
p_size_c = 24 # pixel size on the single char renderer

# Get screen width and height
screen_width = window.winfo_screenwidth()
screen_height = window.winfo_screenheight()
# Set the window size to fill the entire screen
window.geometry(f"{screen_width}x{screen_height}")

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
Label(frame, text="String to render:").pack(side="left", padx=5)
list_char_render = StringVar(value=str(""))
Entry(frame, textvariable=list_char_render, width=50).pack(side="left", padx=5)

b1 = Button(frame, text="Render", width=12, height=2, background="blue", foreground="white", command=renderCfont)
b1.pack(side="left", padx=5)
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
b1.config(state=DISABLED)
b3.config(state=DISABLED)
b4.config(state=DISABLED)

canvas_1 = Canvas(frame_left, width=char_edit_windows_width*p_size_c, height=char_edit_windows_height*p_size_c, bg="black")
canvas_1.pack(side="left", padx=5)

# ov is the small pixel that 'stick' to the mouse
ov = canvas_1.create_rectangle(0,0,p_size_c,p_size_c,fill="white")

canvas_1.focus_set()
canvas_1.bind('<Motion>', motion_1)
canvas_1.bind("<Button 1>",click_1)
canvas_1.bind("<B1-Motion>",slide_1)
##### end of left side #####

##### right side #####
frame_right = Frame(frame_bottom)
frame_right.pack(side="right", padx=2, pady=2)
canvas = Canvas(frame_right, width=canva_width*p_size, height=canva_height*p_size, bg="black")
canvas.pack(anchor="n", side="left", padx=5)
canvas.focus_set()
canvas.bind('<Motion>', motion)
canvas.bind("<Button 1>",click)

select_box = canvas.create_rectangle(0,0,p_size,p_size, width=2, outline="blue")
##### end of right side #####
########## end of bottom ##########

window.mainloop()