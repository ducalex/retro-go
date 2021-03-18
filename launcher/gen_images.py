import os, sys, glob, textwrap

# This scripts should be run to update images.h when you edit images

os.chdir(sys.path[0])

output  = """
typedef struct {
    size_t size;
    const uint8_t data[];
} binfile_t;

"""

for file in glob.glob("images/*.png"):
    with open(file, "rb") as f:
        data = f.read()
        size = len(data)
        struct_name = os.path.basename(file)[0:-4]
        hexdata = ""
        for c in data:
            hexdata += "0x%02X, " % c

        hexdata = "\n".join(textwrap.wrap(hexdata[0:-2], 100))

        output += "static const binfile_t %s = {" % struct_name
        output += ".size = %d, .data = {\n%s\n} " % (size, hexdata)
        output += "};\n\n"

with open("main/images.h", "w", newline='') as f:
    f.write(output)
