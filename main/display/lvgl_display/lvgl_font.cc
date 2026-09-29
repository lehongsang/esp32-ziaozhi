#include "lvgl_font.h"
#include <cbin_font.h>

LV_FONT_DECLARE(BUILTIN_TEXT_FONT);

LvglCBinFont::LvglCBinFont(void* data) {
    font_ = cbin_font_create(static_cast<uint8_t*>(data));
    if (font_ != nullptr) {
        font_->fallback = &BUILTIN_TEXT_FONT;
        font_->release_glyph = nullptr;
    }
}

LvglCBinFont::~LvglCBinFont() {
    if (font_ != nullptr) {
        cbin_font_delete(font_);
    }
}