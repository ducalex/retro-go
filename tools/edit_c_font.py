from tkinter import Tk, Label, Entry, StringVar, Button, Frame, Canvas, filedialog, ttk, Checkbutton, IntVar
import os

windows_x = 1280
windows_y = 720

pixel_size = 5
font_size = 14

canva_width = windows_x//pixel_size
canva_height = windows_y//pixel_size

def renderCfont(byte_list):
    canvas.delete("all")
    byte_index = 0
    offset_x_1 = 1
    offset_y_1 = 1
    while byte_index <= len(byte_list)-5:
        print(byte_list[byte_index])
        offset_y = byte_list[byte_index+1]
        width = byte_list[byte_index+2]
        height = byte_list[byte_index+3]

        bit_index = 0
        data_index = byte_index+6

        if bounding_box_bool.get():
            canvas.create_rectangle((offset_x_1)*pixel_size, (offset_y_1+offset_y)*pixel_size, (width+offset_x_1)*pixel_size, (height+offset_y_1+offset_y)*pixel_size, width=1, outline="blue",fill='') # bounding box

        for y in range(height):
            for x in range(width):
                if byte_list[data_index] & 0b10000000: # Pixel(x,y) = 1
                    canvas.create_rectangle((x+offset_x_1)*pixel_size, (y+offset_y_1+offset_y)*pixel_size, (x+offset_x_1)*pixel_size+pixel_size, (y+offset_y_1+offset_y)*pixel_size+pixel_size,fill="white")

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


def extract_bytes():
    byte_list = []
    with open(font_path, 'r', encoding='ISO8859-1') as file:
        inside_data_section = False
        for line in file:
            line = line.strip()
            # Detect when the .data section starts
            if '.data = {' in line:
                inside_data_section = True
                continue
            # Detect when the .data section ends
            if inside_data_section and '},' in line:
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


window = Tk()
frame = Frame(window)
frame.pack(padx=20, pady=10)

lab1 = Label(frame, text="C Font renderer")
lab1.grid(row=0, column=0, columnspan=3, padx=2, pady=2)

# choose font button
choose_font_button = ttk.Button(frame, text='Choose C font', command=select_file)
choose_font_button.grid(row=1, column=0, padx=2, pady=2)

# Variable to hold the state of the checkbox
bounding_box_bool = IntVar()  # 0 for unchecked, 1 for checked
checkbox = Checkbutton(frame, text="Bounding box", variable=bounding_box_bool)
checkbox.grid(row=1, column=1, padx=2, pady=2)

b1 = Button(frame, text="Render", width=14, height=2, background="blue", foreground="white", command=extract_bytes)
b1.grid(row=1, column=2, padx=2, pady=2)

canvas = Canvas(frame, width=canva_width*pixel_size, height=canva_height*pixel_size, bg="black")
canvas.focus_set()
canvas.grid(row=2, column=0, columnspan=3)

window.mainloop()