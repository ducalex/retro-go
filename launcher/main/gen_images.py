import os, sys, glob, textwrap

# This scripts should be run to update images.c when you edit images

os.chdir(sys.path[0])

output = '#include "gui.h"\n\n'
refs = ""

for file in sorted(glob.glob("images/*.png")):
    with open(file, "rb") as f:
        data = f.read()
        size = len(data)
        struct_name = os.path.basename(file)[0:-4]
        hexdata = ""
        for c in data:
            hexdata += "\\x%02X" % c

        output += 'static const binfile_t %s = {"%s", %d, {\n"%s"\n}\n};\n\n' % (
            struct_name,
            os.path.basename(file),
            len(data),
            '"\n"'.join(textwrap.wrap(hexdata, 100)),
        )
        refs += "&%s,\n" % struct_name

output += "\nconst binfile_t *builtin_images[] = {%s\n0\n};\n" % refs

with open("images.c", "w", newline="") as f:
    f.write(output)
