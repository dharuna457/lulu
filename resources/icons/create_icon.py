from PIL import Image, ImageDraw

# Create a simple 512x512 icon
img = Image.new('RGBA', (512, 512), (61, 220, 132, 255))
draw = ImageDraw.Draw(img)

# Draw Android robot simplified
# Head
draw.ellipse([176, 140, 336, 240], fill='white')
# Body
draw.rectangle([176, 220, 336, 400], fill='white')
draw.ellipse([176, 380, 336, 420], fill='white')
# Eyes
draw.ellipse([206, 160, 226, 180], fill=(61, 220, 132))
draw.ellipse([286, 160, 306, 180], fill=(61, 220, 132))
# Arms
draw.rectangle([140, 230, 170, 350], fill='white')
draw.rectangle([342, 230, 372, 350], fill='white')
# Legs
draw.rectangle([200, 400, 240, 480], fill='white')
draw.rectangle([272, 400, 312, 480], fill='white')

img.save('linuxdroid.png')
print("Icon created successfully")
