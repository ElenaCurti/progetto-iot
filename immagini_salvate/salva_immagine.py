from PIL import Image, ImageDraw
import re
import sys , os

xres = 80
yres = 60
canvas_id = "canvas"

def init_canvas():
    canvas = Image.new("RGBA", (xres, yres), (204, 204, 204, 255))
    canvas.save("output_image.png")

def display(pixels):
    img = Image.new("RGBA", (xres, yres))
    draw = ImageDraw.Draw(img)

    ln = 0
    i = 0
    for y in range(yres):
        for x in range(xres):
            i = (y * xres + x) << 1
            pixel16 = (0xffff & pixels[i]) | ((0xffff & pixels[i + 1]) << 8)
            r = ((((pixel16 >> 11) & 0x1F) * 527) + 23) >> 6
            g = ((((pixel16 >> 5) & 0x3F) * 259) + 33) >> 6
            b = (((pixel16 & 0x1F) * 527) + 23) >> 6
            a = 255
            draw.point((x, y), fill=(r, g, b, a))
            ln += 4
    
    filename_without_extension = os.path.splitext(sys.argv[1])[0]
    new_filename = filename_without_extension + '.png'
    img.save(new_filename)
    os.remove(sys.argv[1])

def hex_string_to_uint8_array(hex_string):
    hex_string = re.sub(r'[^0-9a-fA-F]', '', hex_string)
    pairs = [hex_string[i:i+2] for i in range(0, len(hex_string), 2)]
    byte_array = [int(pair, 16) for pair in pairs]
    return byte_array

# Example usage:
init_canvas()

# Read pixel data from "foto.txt"
file_path = sys.argv[1] 
with open(file_path, 'r') as file:
    hex_data = file.read().strip()

# Convert hex string to uint8 array
array_interi = hex_string_to_uint8_array(hex_data)

# Display the image
display(array_interi)
