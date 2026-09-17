import os
from PIL import Image

def convert_to_rgb565_c_array(src_path, var_name, out_c_path, size=(110, 110), crop_ratio=0.85):
    print(f"Converting {src_path} -> {out_c_path} ({size[0]}x{size[1]})...")
    im = Image.open(src_path).convert("RGB")
    w_src, h_src = im.size
    
    if crop_ratio < 1.0:
        crop_w = int(w_src * crop_ratio)
        crop_h = int(h_src * crop_ratio)
        left = (w_src - crop_w) // 2
        top = (h_src - crop_h) // 2
        im = im.crop((left, top, left + crop_w, top + crop_h))
        
    img = im.resize(size, Image.Resampling.LANCZOS)
    
    w, h = img.size
    pixels = img.load()
    
    data = bytearray()
    for y in range(h):
        for x in range(w):
            r, g, b = pixels[x, y]
            r5 = (r >> 3) & 0x1F
            g6 = (g >> 2) & 0x3F
            b5 = (b >> 3) & 0x1F
            val = (r5 << 11) | (g6 << 5) | b5
            data.append(val & 0xFF)
            data.append((val >> 8) & 0xFF)
            
    stride = w * 2
    
    with open(out_c_path, "w", encoding="utf-8") as f:
        f.write('#include "lvgl.h"\n\n')
        f.write('#ifndef LV_ATTRIBUTE_MEM_ALIGN\n#define LV_ATTRIBUTE_MEM_ALIGN\n#endif\n\n')
        f.write(f'const LV_ATTRIBUTE_MEM_ALIGN uint8_t {var_name}_map[] = {{\n')
        
        chunk_size = 16
        for i in range(0, len(data), chunk_size):
            chunk = data[i:i+chunk_size]
            hex_str = ", ".join(f"0x{b:02X}" for b in chunk)
            f.write(f"    {hex_str},\n")
            
        f.write("};\n\n")
        f.write(f"const lv_image_dsc_t {var_name} = {{\n")
        f.write("    .header = {\n")
        f.write("        .magic = LV_IMAGE_HEADER_MAGIC,\n")
        f.write("        .cf = LV_COLOR_FORMAT_RGB565,\n")
        f.write("        .flags = 0,\n")
        f.write(f"        .w = {w},\n")
        f.write(f"        .h = {h},\n")
        f.write(f"        .stride = {stride},\n")
        f.write("        .reserved_2 = 0,\n")
        f.write("    },\n")
        f.write(f"    .data_size = sizeof({var_name}_map),\n")
        f.write(f"    .data = {var_name}_map,\n")
        f.write("    .reserved = NULL,\n")
        f.write("};\n")
    print(f"Done {out_c_path}, size: {os.path.getsize(out_c_path)} bytes")

if __name__ == "__main__":
    base_dir = r"C:\Users\Admin\.gemini\antigravity-ide\brain\0004555b-5589-4cff-96e7-e3a42271f588"
    out_dir = r"main\display\buddy_ui\assets"
    os.makedirs(out_dir, exist_ok=True)
    
    hungry_piggy_src = os.path.join(base_dir, "piggy_hungry_wait_1789615289775.jpg")
    happy_piggy_src = os.path.join(base_dir, "piggy_happy_fed_1789615471899.jpg")
    corn_src = os.path.join(base_dir, "food_corn_3d_1789615304701.jpg")
    potato_src = os.path.join(base_dir, "food_potato_3d_1789615322986.jpg")
    carrot_src = os.path.join(base_dir, "food_carrot_3d_1789615447988.jpg")
    
    # Piggy full/medium avatars enlarged to 130x130 for prominent central character display
    convert_to_rgb565_c_array(hungry_piggy_src, "buddy_piggy_hungry", os.path.join(out_dir, "buddy_piggy_hungry.c"), (130, 130), 0.88)
    convert_to_rgb565_c_array(happy_piggy_src, "buddy_piggy_happy", os.path.join(out_dir, "buddy_piggy_happy.c"), (130, 130), 0.88)
    
    # Food item icons (44x44)
    convert_to_rgb565_c_array(corn_src, "buddy_food_corn", os.path.join(out_dir, "buddy_food_corn.c"), (44, 44), 0.95)
    convert_to_rgb565_c_array(potato_src, "buddy_food_potato", os.path.join(out_dir, "buddy_food_potato.c"), (44, 44), 0.95)
    convert_to_rgb565_c_array(carrot_src, "buddy_food_carrot", os.path.join(out_dir, "buddy_food_carrot.c"), (44, 44), 0.95)
