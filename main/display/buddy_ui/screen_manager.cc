#include "screen_manager.h"
#include "buddy_sync_service.h"
#include <esp_log.h>

#define TAG "BuddyScreenManager"

BuddyScreenManager::BuddyScreenManager() {}
BuddyScreenManager::~BuddyScreenManager() {}

BuddyScreenManager& BuddyScreenManager::GetInstance() {
    static BuddyScreenManager instance;
    return instance;
}

void BuddyScreenManager::Initialize(lv_obj_t* root_parent) {
    if (!root_parent) {
        root_parent = lv_screen_active();
    }

    // 1. Create Base Background Container
    lv_obj_t* bg = lv_obj_create(root_parent);
    lv_obj_remove_style_all(bg);
    lv_obj_set_size(bg, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(bg, lv_color_hex(0x1A1A24), 0);
    lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, 0);

    // 2. Create Horizontal Tileview Container for 5 Screens
    tileview_ = lv_tileview_create(bg);
    lv_obj_remove_style_all(tileview_);
    lv_obj_set_size(tileview_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(tileview_, LV_OPA_TRANSP, 0);

    // 3. Add 5 Main Screens
    // Screen 0: Home
    tiles_[0] = lv_tileview_add_tile(tileview_, 0, 0, (lv_dir_t)(LV_DIR_LEFT | LV_DIR_RIGHT));
    home_screen_.Create(tiles_[0]);

    // Screen 1: Today's Quest
    tiles_[1] = lv_tileview_add_tile(tileview_, 1, 0, (lv_dir_t)(LV_DIR_LEFT | LV_DIR_RIGHT));
    quest_screen_.Create(tiles_[1]);

    // Screen 2: AI Tutor
    tiles_[2] = lv_tileview_add_tile(tileview_, 2, 0, (lv_dir_t)(LV_DIR_LEFT | LV_DIR_RIGHT));
    tutor_screen_.Create(tiles_[2]);

    // Screen 3: Savings Dream Goal
    tiles_[3] = lv_tileview_add_tile(tileview_, 3, 0, (lv_dir_t)(LV_DIR_LEFT | LV_DIR_RIGHT));
    savings_screen_.Create(tiles_[3]);

    // Screen 4: Family Moment
    tiles_[4] = lv_tileview_add_tile(tileview_, 4, 0, (lv_dir_t)(LV_DIR_LEFT | LV_DIR_RIGHT));
    family_screen_.Create(tiles_[4]);

    // 4. Create Page Indicator Dots at Bottom
    CreatePageIndicators(bg);

    // 5. Invisible Top Pull-Down Touch Zone (Full Width 320x28px, Clean & Unobtrusive)
    top_pull_zone_ = lv_obj_create(bg);
    lv_obj_remove_style_all(top_pull_zone_);
    lv_obj_set_size(top_pull_zone_, 320, 28);
    lv_obj_align(top_pull_zone_, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(top_pull_zone_, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(top_pull_zone_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(top_pull_zone_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(top_pull_zone_, TopPullZoneCb, LV_EVENT_ALL, this);

    // 6. Create Settings Control Center Overlay (Sits on top layer)
    settings_screen_.Create(bg);

    // 7. Register Scroll, Gesture and Touch Tracking Events on Tileview and Tiles
    lv_obj_add_event_cb(tileview_, TileviewScrollCb, LV_EVENT_VALUE_CHANGED, this);
    lv_obj_add_event_cb(tileview_, GlobalGestureCb, LV_EVENT_GESTURE, this);
    lv_obj_add_event_cb(tileview_, ScreenTouchCb, LV_EVENT_PRESSED, this);
    lv_obj_add_event_cb(tileview_, ScreenTouchCb, LV_EVENT_PRESSING, this);
    lv_obj_add_event_cb(tileview_, ScreenTouchCb, LV_EVENT_RELEASED, this);

    for (int i = 0; i < static_cast<int>(BuddyScreenId::kScreenCount); ++i) {
        if (tiles_[i]) {
            lv_obj_add_flag(tiles_[i], LV_OBJ_FLAG_GESTURE_BUBBLE);
            lv_obj_add_event_cb(tiles_[i], GlobalGestureCb, LV_EVENT_GESTURE, this);
            lv_obj_add_event_cb(tiles_[i], ScreenTouchCb, LV_EVENT_PRESSED, this);
            lv_obj_add_event_cb(tiles_[i], ScreenTouchCb, LV_EVENT_PRESSING, this);
            lv_obj_add_event_cb(tiles_[i], ScreenTouchCb, LV_EVENT_RELEASED, this);
        }
    }

    UpdateIndicators(0);

    // 8. Initialize Backend Sync Service & Hooks
    BuddySyncService::GetInstance().Initialize(&quest_screen_, &savings_screen_, &family_screen_);
    family_screen_.OnLikeClicked([](bool liked) {
        if (liked) {
            BuddySyncService::GetInstance().NotifyFamilyLove();
        }
    });

    ESP_LOGI(TAG, "BuddyScreenManager successfully initialized with 5 screens + Control Center Dropdown");
}

void BuddyScreenManager::CreatePageIndicators(lv_obj_t* parent) {
    indicator_container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(indicator_container_);
    lv_obj_set_size(indicator_container_, 80, 16);
    lv_obj_align(indicator_container_, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_set_flex_flow(indicator_container_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(indicator_container_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(indicator_container_, 6, 0);

    for (int i = 0; i < static_cast<int>(BuddyScreenId::kScreenCount); ++i) {
        indicator_dots_[i] = lv_obj_create(indicator_container_);
        lv_obj_set_size(indicator_dots_[i], 6, 6);
        lv_obj_set_style_radius(indicator_dots_[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(indicator_dots_[i], lv_color_hex(0x666677), 0);
        lv_obj_set_style_border_width(indicator_dots_[i], 0, 0);
    }
}

void BuddyScreenManager::UpdateIndicators(int active_index) {
    if (!indicator_container_ || lv_obj_has_flag(indicator_container_, LV_OBJ_FLAG_HIDDEN)) {
        return;
    }
    for (int i = 0; i < static_cast<int>(BuddyScreenId::kScreenCount); ++i) {
        if (indicator_dots_[i]) {
            if (i == active_index) {
                lv_obj_set_size(indicator_dots_[i], 14, 6);
                lv_obj_set_style_bg_color(indicator_dots_[i], lv_color_hex(0xFFB703), 0);
            } else {
                lv_obj_set_size(indicator_dots_[i], 6, 6);
                lv_obj_set_style_bg_color(indicator_dots_[i], lv_color_hex(0x555566), 0);
            }
        }
    }
}

void BuddyScreenManager::SwitchTo(BuddyScreenId id, bool anim) {
    if (!tileview_) return;
    uint32_t idx = static_cast<uint32_t>(id);
    if (idx < static_cast<uint32_t>(BuddyScreenId::kScreenCount)) {
        lv_tileview_set_tile_by_index(tileview_, idx, 0, anim ? LV_ANIM_ON : LV_ANIM_OFF);
        current_screen_ = id;
        UpdateIndicators(idx);
        if (on_screen_changed_) {
            on_screen_changed_(id);
        }
    }
}

void BuddyScreenManager::TileviewScrollCb(lv_event_t* e) {
    auto* self = static_cast<BuddyScreenManager*>(lv_event_get_user_data(e));
    if (!self || !self->tileview_) return;

    lv_obj_t* active_tile = lv_tileview_get_tile_active(self->tileview_);
    for (int i = 0; i < static_cast<int>(BuddyScreenId::kScreenCount); ++i) {
        if (self->tiles_[i] == active_tile) {
            if (self->current_screen_ != static_cast<BuddyScreenId>(i)) {
                self->current_screen_ = static_cast<BuddyScreenId>(i);
                self->UpdateIndicators(i);
                if (self->on_screen_changed_) {
                    self->on_screen_changed_(self->current_screen_);
                }
            }
            break;
        }
    }
}

void BuddyScreenManager::OnScreenChanged(std::function<void(BuddyScreenId new_screen)> callback) {
    on_screen_changed_ = callback;
}

void BuddyScreenManager::SetIndicatorsVisible(bool visible) {
    if (!indicator_container_) return;
    if (visible) {
        lv_obj_clear_flag(indicator_container_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(indicator_container_, LV_OBJ_FLAG_HIDDEN);
    }
}

void BuddyScreenManager::SetTileviewScrollable(bool scrollable) {
    if (!tileview_) return;
    if (scrollable) {
        lv_obj_add_flag(tileview_, LV_OBJ_FLAG_SCROLLABLE);
    } else {
        lv_obj_remove_flag(tileview_, LV_OBJ_FLAG_SCROLLABLE);
    }
}

void BuddyScreenManager::TopPullZoneCb(lv_event_t* e) {
    auto* self = static_cast<BuddyScreenManager*>(lv_event_get_user_data(e));
    if (!self || self->settings_screen_.IsOpen()) return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t* indev = lv_indev_active();

    if (code == LV_EVENT_CLICKED) {
        ESP_LOGI(TAG, "Top pull zone clicked -> Open Control Center");
        self->settings_screen_.Open();
    } else if (code == LV_EVENT_PRESSED) {
        if (indev) {
            lv_indev_get_point(indev, &self->touch_start_point_);
            self->touch_tracking_ = true;
        }
    } else if (code == LV_EVENT_PRESSING || code == LV_EVENT_RELEASED) {
        if (self->touch_tracking_ && indev) {
            lv_point_t curr;
            lv_indev_get_point(indev, &curr);
            int32_t dy = curr.y - self->touch_start_point_.y;
            int32_t dx = abs(curr.x - self->touch_start_point_.x);
            if (dy >= 18 && dy > dx) {
                ESP_LOGI(TAG, "Swipe down on Top Zone detected (dy=%ld) -> Opening Control Center", (long)dy);
                self->touch_tracking_ = false;
                self->settings_screen_.Open();
            }
        }
        if (code == LV_EVENT_RELEASED) {
            self->touch_tracking_ = false;
        }
    }
}

void BuddyScreenManager::ScreenTouchCb(lv_event_t* e) {
    auto* self = static_cast<BuddyScreenManager*>(lv_event_get_user_data(e));
    if (!self || self->settings_screen_.IsOpen()) return;

    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t* indev = lv_indev_active();
    if (!indev) return;

    if (code == LV_EVENT_PRESSED) {
        lv_indev_get_point(indev, &self->touch_start_point_);
        // If touch starts within top 60px of display, enable swipe-down tracking
        if (self->touch_start_point_.y <= 60) {
            self->touch_tracking_ = true;
        }
    } else if (code == LV_EVENT_PRESSING || code == LV_EVENT_RELEASED) {
        if (self->touch_tracking_) {
            lv_point_t curr;
            lv_indev_get_point(indev, &curr);
            int32_t dy = curr.y - self->touch_start_point_.y;
            int32_t dx = abs(curr.x - self->touch_start_point_.x);
            if (dy >= 22 && dy > (dx * 3 / 4)) {
                ESP_LOGI(TAG, "Screen swipe down detected from y=%ld (dy=%ld) -> Opening Control Center", (long)self->touch_start_point_.y, (long)dy);
                self->touch_tracking_ = false;
                self->settings_screen_.Open();
            }
        }
        if (code == LV_EVENT_RELEASED) {
            self->touch_tracking_ = false;
        }
    }
}

void BuddyScreenManager::GlobalGestureCb(lv_event_t* e) {
    auto* self = static_cast<BuddyScreenManager*>(lv_event_get_user_data(e));
    if (!self || self->settings_screen_.IsOpen()) return;

    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if (dir == LV_DIR_BOTTOM) {
        ESP_LOGI(TAG, "Swipe down gesture detected -> Opening Control Center");
        self->settings_screen_.Open();
    }
}
