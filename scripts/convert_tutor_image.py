import os
import collections
import numpy as np
from PIL import Image, ImageFilter

def extract_cloud(cloud_src_path):
    img = Image.open(cloud_src_path).convert("RGB")
    arr = np.array(img, dtype=np.float32)
    H, W, _ = arr.shape

    # Blue pixels: outline & tail rings
    blue_mask = (arr[:,:,2] > arr[:,:,0] + 25) & (arr[:,:,2] > arr[:,:,1] + 10)

    # 1. Main cloud interior: flood fill from center (400, 500)
    main_inside = np.zeros((H, W), dtype=bool)
    q = collections.deque([(400, 500)])
    main_inside[400, 500] = True
    while q:
        y, x = q.popleft()
        for dy, dx in ((-1,0), (1,0), (0,-1), (0,1)):
            ny, nx = y + dy, x + dx
            if 0 <= ny < H and 0 <= nx < W and not main_inside[ny, nx] and not blue_mask[ny, nx]:
                main_inside[ny, nx] = True
                q.append((ny, nx))

    # 2. Tail circles interior (strictly bounded inside circle radii):
    tail_inside = np.zeros((H, W), dtype=bool)
    for cy, cx, r_max in [(715, 715, 45), (788, 770, 30)]:
        q = collections.deque([(cy, cx)])
        seen = set([(cy, cx)])
        if not blue_mask[cy, cx]:
            tail_inside[cy, cx] = True
        while q:
            y, x = q.popleft()
            for dy, dx in ((-1,0), (1,0), (0,-1), (0,1)):
                ny, nx = y + dy, x + dx
                if (ny, nx) not in seen and 0 <= ny < H and 0 <= nx < W:
                    seen.add((ny, nx))
                    if (ny - cy)**2 + (nx - cx)**2 <= r_max**2 and not blue_mask[ny, nx]:
                        tail_inside[ny, nx] = True
                        q.append((ny, nx))

    # Foreground is ONLY main_inside + tail_inside + blue_mask
    fg = main_inside | tail_inside | blue_mask

    alpha = np.zeros((H, W), dtype=np.uint8)
    alpha[fg] = 255

    # Pure white interior
    rgb_arr = arr.copy().astype(np.uint8)
    rgb_arr[main_inside | tail_inside] = [255, 255, 255]

    cloud_rgba = np.dstack((rgb_arr, alpha)).astype(np.uint8)
    cloud_img = Image.fromarray(cloud_rgba, 'RGBA')
    bbox = cloud_img.getbbox()
    return cloud_img.crop(bbox)

def process_and_convert_tutor_bg(robot_src_path, cloud_src_path, out_c_path):
    print(f"Loading Robot: {robot_src_path}...")
    robot_orig = Image.open(robot_src_path).convert("RGB")
    arr_r = np.array(robot_orig, dtype=np.float32)
    
    # 1. Background vertical gradient profile & cutout
    bg_profile = np.mean(arr_r[:, :40], axis=1)
    diff = np.sqrt(np.sum((arr_r - bg_profile[:, None, :])**2, axis=2))
    alpha_r = np.clip((diff - 12) / (35 - 12), 0, 1)
    mask_r = Image.fromarray((alpha_r * 255).astype(np.uint8)).filter(ImageFilter.MaxFilter(5)).filter(ImageFilter.GaussianBlur(1))

    robot_rgba = robot_orig.convert("RGBA")
    robot_rgba.putalpha(mask_r)
    cropped_robot = robot_rgba.crop((130, 50, 1000, 1000))

    # Scale robot (height ~ 168 px)
    target_rh = 168
    scale_r = target_rh / cropped_robot.height
    target_rw = int(cropped_robot.width * scale_r)
    scaled_robot = cropped_robot.resize((target_rw, target_rh), Image.Resampling.LANCZOS)
    
    # Robot shifted slightly up and positioned to align with cloud tail
    pos_rx = 168
    pos_ry = 42

    # 2. Process cloud thought bubble (spacious 184px width)
    print(f"Loading Cloud: {cloud_src_path}...")
    cropped_cloud = extract_cloud(cloud_src_path)
    cloud_w = 184
    scale_c = cloud_w / cropped_cloud.width
    cloud_h = int(cropped_cloud.height * scale_c)
    scaled_cloud = cropped_cloud.resize((cloud_w, cloud_h), Image.Resampling.LANCZOS)
    pos_cx = 2
    pos_cy = 24

    # 3. Canvas 320x240 with smooth gradient
    W_CANVAS, H_CANVAS = 320, 240
    top_c = np.array([163, 88, 155], dtype=np.float32)
    bot_c = np.array([198, 138, 206], dtype=np.float32)
    grad_arr = np.zeros((H_CANVAS, W_CANVAS, 3), dtype=np.uint8)
    for y in range(H_CANVAS):
        t = y / float(H_CANVAS - 1)
        c = (1 - t) * top_c + t * bot_c
        grad_arr[y, :] = c.astype(np.uint8)

    canvas = Image.fromarray(grad_arr).convert("RGBA")
    canvas.paste(scaled_robot, (pos_rx, pos_ry), scaled_robot)
    canvas.paste(scaled_cloud, (pos_cx, pos_cy), scaled_cloud)
    final_canvas = canvas.convert("RGB")

    # 4. Convert to RGB565 C array
    w, h = final_canvas.size
    pixels = final_canvas.load()

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
    var_name = "buddy_bg_tutor"

    os.makedirs(os.path.dirname(out_c_path), exist_ok=True)
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
    print(f"Generated {out_c_path}, size: {os.path.getsize(out_c_path)} bytes")

if __name__ == "__main__":
    src_robot = "C:/Users/Admin/.gemini/antigravity-ide/brain/027e9156-979d-4430-86d3-24150d547ac1/.user_uploaded/media_1788794244131.jpg"
    src_cloud = "C:/Users/Admin/.gemini/antigravity-ide/brain/027e9156-979d-4430-86d3-24150d547ac1/.user_uploaded/media_1788795582899.png"
    out_file = "main/display/buddy_ui/assets/buddy_bg_tutor.c"
    process_and_convert_tutor_bg(src_robot, src_cloud, out_file)
