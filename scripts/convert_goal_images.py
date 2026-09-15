import os
from PIL import Image

def convert_to_rgb565_c_array(src_path, var_name, out_c_path, size=(76, 76)):
    print(f"Converting {src_path} -> {out_c_path} ({size[0]}x{size[1]})...")
    img = Image.open(src_path).convert("RGB")
    img = img.resize(size, Image.Resampling.LANCZOS)
    
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
    base_dir = r"C:\Users\Admin\.gemini\antigravity-ide\brain\f5c6a1da-39eb-44ff-b89b-3091cff43511"
    out_dir = r"main\display\buddy_ui\assets"
    
    piggy_src = os.path.join(base_dir, "goal_dream_piggy_art_1789096312151.jpg")
    bike_src = os.path.join(base_dir, "goal_bike_art_1789094795722.jpg")
    robot_src = os.path.join(base_dir, "goal_robot_art_1789094813720.jpg")
    lego_src = os.path.join(base_dir, "goal_lego_art_1789094893591.jpg")
    
    convert_to_rgb565_c_array(piggy_src, "goal_art_piggy", os.path.join(out_dir, "goal_art_piggy.c"), (76, 76))
    convert_to_rgb565_c_array(bike_src, "goal_art_bike", os.path.join(out_dir, "goal_art_bike.c"), (76, 76))
    convert_to_rgb565_c_array(robot_src, "goal_art_robot", os.path.join(out_dir, "goal_art_robot.c"), (76, 76))
    convert_to_rgb565_c_array(lego_src, "goal_art_lego", os.path.join(out_dir, "goal_art_lego.c"), (76, 76))
