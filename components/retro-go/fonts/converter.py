from PIL import Image, ImageDraw, ImageFont
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

def generate_font_data(font_path, font_size):
    # Load the TTF font
    pil_font = ImageFont.truetype(font_path, font_size)

    # Initialize the font data structure
    font_data = []
    memory_usage = 0
    num_characters = 0

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
                row = (row << 1) | (1 if pixel > 1 else 0)
                i += 1

        row = row << 8-i # to "fill" with zero the remaining empty bits
        bitmap.append(row)

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

    # Output results
    return {
        "header": header,
        "glyphs": font_data,
        "memory_usage": memory_usage,
        "num_characters": num_characters,
    }

def save_file(file_path, font_data):
    with open (file_path, 'w', encoding='ISO8859-1') as f:
        # Output header
        f.write(f"// Memory usage: {font_data['memory_usage']} bytes\n")
        f.write(f"// Number of characters: {font_data['num_characters']}\n")

        f.write("#include \"../rg_gui.h\"\n\n")
        f.write(f"// Font         : {font_name}\n")
        f.write(f"// Point Size   : 12\n")
        f.write(f"// Memory usage : 1161 bytes\n")
        f.write(f"// # characters : 95\n\n")
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


# Example usage
font_name = "VeraBold"
font_path = os.path.abspath("VeraBold.ttf")  # Replace with your TTF font path
font_size = 11

font_data = generate_font_data(font_path, font_size)

max_width = 0
for glyph in font_data["glyphs"]:
    if glyph['width'] > max_width:
        max_width = glyph['width']
print(f"Max width : {max_width}")

save_file("OUTPUT_VeraBold.c", font_data)