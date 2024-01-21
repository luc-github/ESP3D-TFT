import argparse
#Need: pip install pillow
from PIL import Image


# Analyze args of command line
parser = argparse.ArgumentParser(description='Convert ESP3D-TFT dump to PNG image.')
parser.add_argument('source', help='Path of source file')
parser.add_argument('--width', type=int, default=800, help='Screen width (default: 800)')
parser.add_argument('--height', type=int, default=480, help='Screen height (défault: 480)')
args = parser.parse_args()
destination = args.source + ".png"

# Read source file
with open(args.source, 'rb') as file:
    image_data = file.read()

# New image
image = Image.new('RGB', (args.width, args.height))

# Conversion image data to pixels
pixels = []
for i in range(0, len(image_data), 2):
    pixel_16bits = (image_data[i+1] << 8) | image_data[i]
    r = (pixel_16bits & 0xF800) >> 11
    g = (pixel_16bits & 0x07E0) >> 5
    b = pixel_16bits & 0x001F
    r = (r << 3) | (r >> 2)
    g = (g << 2) | (g >> 4)
    b = (b << 3) | (b >> 2)
    pixels.append(( r,g, b))

# Affect pixels to image
image.putdata(pixels)

#Save PNG
image.save(destination)
