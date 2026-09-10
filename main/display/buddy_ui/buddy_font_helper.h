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
    auto* light_theme = LvglThemeManager::GetInstance().GetTheme("light");
    if (light_theme && light_theme->text_font() && light_theme->text_font()->font()) {
        return light_theme->text_font()->font();
    }
    auto* dark_theme = LvglThemeManager::GetInstance().GetTheme("dark");
    if (dark_theme && dark_theme->text_font() && dark_theme->text_font()->font()) {
        return dark_theme->text_font()->font();
    }
    return &BUILTIN_TEXT_FONT;
}
