#pragma once

#include <lvgl.h>
#include "lvgl_theme.h"

LV_FONT_DECLARE(BUILTIN_TEXT_FONT);

/**
 * @brief Get the standard multilingual font for Buddy UI.
 * Prioritizes the active LvglTheme text font (loaded from assets.bin with full Vietnamese/Unicode glyphs),
 * falling back to BUILTIN_TEXT_FONT (font_noto_sans_basic_20_4).
 */
inline const lv_font_t* GetBuddyFont() {
    return &BUILTIN_TEXT_FONT;
}
