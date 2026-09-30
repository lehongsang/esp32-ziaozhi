from PIL import Image
import os
import struct

input_path = r"C:\Users\Admin\.gemini\antigravity-ide\brain\dad22dd9-5d74-41fe-a640-5aac2ad1a9b6\buddy_family_call_1790758091124.jpg"
output_c_path = r"c:\Users\Admin\Desktop\xiaozhi-esp32\xiaozhi-esp32\main\display\buddy_ui\assets\buddy_family_call.c"

# Resize to 140x110
w, h = 140, 110
img = Image.open(input_path).convert('RGB')
img = img.resize((w, h), Image.Resampling.LANCZOS)

byte_list = []
for y in range(h):
    for x in range(w):
        r, g, b = img.getpixel((x, y))
        # RGB565
        val = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
        # Little endian for LVGL RGB565
        byte_list.append(val & 0xFF)
        byte_list.append((val >> 8) & 0xFF)

print(f"Total bytes: {len(byte_list)}")

with open(output_c_path, 'w', encoding='utf-8') as f:
    f.write('#include "lvgl.h"\n\n')
    f.write('#ifndef LV_ATTRIBUTE_MEM_ALIGN\n#define LV_ATTRIBUTE_MEM_ALIGN\n#endif\n\n')
    f.write('const LV_ATTRIBUTE_MEM_ALIGN uint8_t buddy_family_call_map[] = {\n')
    
    for i in range(0, len(byte_list), 16):
        chunk = byte_list[i:i+16]
        hex_str = ", ".join(f"0x{b:02X}" for b in chunk)
        f.write(f"    {hex_str},\n")
        
    f.write('};\n\n')
    f.write('const lv_image_dsc_t buddy_family_call = {\n')
    f.write('    .header = {\n')
    f.write('        .magic = LV_IMAGE_HEADER_MAGIC,\n')
    f.write('        .cf = LV_COLOR_FORMAT_RGB565,\n')
    f.write('        .flags = 0,\n')
    f.write(f'        .w = {w},\n')
    f.write(f'        .h = {h},\n')
    f.write(f'        .stride = {w * 2},\n')
    f.write('        .reserved_2 = 0,\n')
    f.write('    },\n')
    f.write('    .data_size = sizeof(buddy_family_call_map),\n')
    f.write('    .data = buddy_family_call_map,\n')
    f.write('    .reserved = NULL,\n')
    f.write('};\n')

print("Generated buddy_family_call.c successfully!")
