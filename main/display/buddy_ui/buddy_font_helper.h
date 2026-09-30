#pragma once

#include <lvgl.h>
#include <string>

LV_FONT_DECLARE(font_vietnamese_20_4);

/**
 * @brief Get the true Vietnamese Unicode font with full diacritics
 * (Ạ, ạ, Ả, ả, Ấ, ấ, Ầ, ầ, Ẩ, ẩ, Ẫ, ẫ, Ậ, ậ, Ắ, ắ, Ằ, ằ, Ẳ, ẳ, Ẵ, ẵ, Ặ, ặ,
 *  Ẹ, ẹ, Ẻ, ẻ, Ẽ, ẽ, Ế, ế, Ề, ề, Ể, ể, Ễ, ễ, Ệ, ệ,
 *  Ỉ, ỉ, Ị, ị,
 *  Ọ, ọ, Ỏ, ỏ, Ố, ố, Ồ, ồ, Ổ, ổ, Ỗ, ỗ, Ộ, ộ, Ớ, ớ, Ờ, ờ, Ở, ở, Ỡ, ỡ, Ợ, ợ,
 *  Ụ, ụ, Ủ, ủ, Ứ, ứng, Ừ, ừ, Ử, ử, Ữ, ữ, Ự, ự,
 *  Ỳ, ỳ, Ỵ, ỵ, Ỷ, ỷ, Ỹ, ỹ, đ, Đ).
 */
inline const lv_font_t* GetBuddyFont() {
    return &font_vietnamese_20_4;
}

/**
 * @brief Pass-through string function since font_vietnamese_20_4 now supports full Vietnamese Unicode natively.
 */
inline std::string SanitizeVietnamese(const std::string& str) {
    return str;
}
