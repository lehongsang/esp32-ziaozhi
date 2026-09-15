#ifndef BUDDY_SCREEN_MANAGER_H
#define BUDDY_SCREEN_MANAGER_H

#include <vector>
#include <memory>
#include <functional>
#include <cstdint>

#include <lvgl.h>

#include "screen_types.h"
#include "buddy_home_screen.h"
#include "today_quest_screen.h"
#include "ai_tutor_screen.h"
#include "savings_screen.h"
#include "family_moment_screen.h"
#include "settings_screen.h"

class BuddyScreenManager {
public:
    static BuddyScreenManager& GetInstance();

    void Initialize(lv_obj_t* root_parent = nullptr);
    void SwitchTo(BuddyScreenId id, bool anim = true);
    BuddyScreenId GetCurrentScreen() const { return current_screen_; }

    BuddyHomeScreen& GetHomeScreen() { return home_screen_; }
    TodayQuestScreen& GetQuestScreen() { return quest_screen_; }
    AiTutorScreen& GetTutorScreen() { return tutor_screen_; }
    SavingsScreen& GetSavingsScreen() { return savings_screen_; }
    FamilyMomentScreen& GetFamilyScreen() { return family_screen_; }
    SettingsScreen& GetSettingsScreen() { return settings_screen_; }

    void OnScreenChanged(std::function<void(BuddyScreenId new_screen)> callback);

    void SetIndicatorsVisible(bool visible);
    void SetTileviewScrollable(bool scrollable);

private:
    BuddyScreenManager();
    ~BuddyScreenManager();

    lv_obj_t* tileview_ = nullptr;
    lv_obj_t* tiles_[static_cast<int>(BuddyScreenId::kScreenCount)] = {nullptr};
    lv_obj_t* indicator_container_ = nullptr;
    lv_obj_t* indicator_dots_[static_cast<int>(BuddyScreenId::kScreenCount)] = {nullptr};

    BuddyScreenId current_screen_ = BuddyScreenId::kScreenHome;
    std::function<void(BuddyScreenId)> on_screen_changed_;

    BuddyHomeScreen home_screen_;
    TodayQuestScreen quest_screen_;
    AiTutorScreen tutor_screen_;
    SavingsScreen savings_screen_;
    FamilyMomentScreen family_screen_;
    SettingsScreen settings_screen_;

    void CreatePageIndicators(lv_obj_t* parent);
    void UpdateIndicators(int active_index);
    static void TileviewScrollCb(lv_event_t* e);
};

#endif // BUDDY_SCREEN_MANAGER_H
