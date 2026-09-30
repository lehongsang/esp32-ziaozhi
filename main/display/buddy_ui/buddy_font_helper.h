#pragma once

#include <lvgl.h>
#include "lvgl_theme.h"
#include <string>
#include <unordered_map>

LV_FONT_DECLARE(BUILTIN_TEXT_FONT);

/**
 * @brief Get the standard multilingual font for Buddy UI.
 */
inline const lv_font_t* GetBuddyFont() {
    auto light_theme = LvglThemeManager::GetInstance().GetTheme("light");
    if (light_theme && light_theme->text_font() && light_theme->text_font()->font()) {
        return light_theme->text_font()->font();
    }
    auto dark_theme = LvglThemeManager::GetInstance().GetTheme("dark");
    if (dark_theme && dark_theme->text_font() && dark_theme->text_font()->font()) {
        return dark_theme->text_font()->font();
    }
    return &BUILTIN_TEXT_FONT;
}

/**
 * @brief Normalizes Vietnamese characters in U+1EA0-U+1EF9 range to compatible base letters
 * ensuring no characters are skipped/dropped by the display font renderer.
 */
inline std::string SanitizeVietnamese(const std::string& str) {
    static const std::unordered_map<std::string, std::string> kMap = {
        {"Ạ", "A"}, {"ạ", "a"}, {"Ả", "A"}, {"ả", "a"}, {"Ấ", "A"}, {"ấ", "a"},
        {"Ầ", "A"}, {"ầ", "a"}, {"Ẩ", "A"}, {"ẩ", "a"}, {"Ẫ", "A"}, {"ẫ", "a"},
        {"Ậ", "A"}, {"ậ", "a"}, {"Ắ", "A"}, {"ắ", "a"}, {"Ằ", "A"}, {"ằ", "a"},
        {"Ẳ", "A"}, {"ẳ", "a"}, {"Ẵ", "A"}, {"ẵ", "a"}, {"Ặ", "A"}, {"ặ", "a"},
        {"Ẹ", "E"}, {"ẹ", "e"}, {"Ẻ", "E"}, {"ẻ", "e"}, {"Ẽ", "E"}, {"ẽ", "e"},
        {"Ế", "E"}, {"ế", "e"}, {"Ề", "E"}, {"ề", "e"}, {"Ể", "E"}, {"ể", "e"},
        {"Ễ", "E"}, {"ễ", "e"}, {"Ệ", "E"}, {"ệ", "e"}, {"Ỉ", "I"}, {"ỉ", "i"},
        {"Ị", "I"}, {"ị", "i"}, {"Ọ", "O"}, {"ọ", "o"}, {"Ỏ", "O"}, {"ỏ", "o"},
        {"Ố", "O"}, {"ố", "o"}, {"Ồ", "O"}, {"ồ", "o"}, {"Ổ", "O"}, {"ổ", "o"},
        {"Ỗ", "O"}, {"ỗ", "o"}, {"Ộ", "O"}, {"ộ", "o"}, {"Ớ", "O"}, {"ớ", "o"},
        {"Ờ", "O"}, {"ờ", "o"}, {"Ở", "O"}, {"ở", "o"}, {"Ỡ", "O"}, {"ỡ", "o"},
        {"Ợ", "O"}, {"ợ", "o"}, {"Ụ", "U"}, {"ụ", "u"}, {"Ủ", "U"}, {"ủ", "u"},
        {"Ứ", "U"}, {"ứ", "u"}, {"Ừ", "U"}, {"ừ", "u"}, {"Ử", "U"}, {"ử", "u"},
        {"Ữ", "U"}, {"ữ", "u"}, {"Ự", "U"}, {"ự", "u"}, {"Ỳ", "Y"}, {"ỳ", "y"},
        {"Ỵ", "Y"}, {"ỵ", "y"}, {"Ỷ", "Y"}, {"ỷ", "y"}, {"Ỹ", "Y"}, {"ỹ", "y"}
    };

    std::string result;
    result.reserve(str.size());

    for (size_t i = 0; i < str.size(); ) {
        unsigned char c = static_cast<unsigned char>(str[i]);
        if (c == 0xE1 && i + 2 < str.size()) {
            std::string sub = str.substr(i, 3);
            auto it = kMap.find(sub);
            if (it != kMap.end()) {
                result += it->second;
                i += 3;
                continue;
            }
        }
        result += str[i];
        i++;
    }
    return result;
}
