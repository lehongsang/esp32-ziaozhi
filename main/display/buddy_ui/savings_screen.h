#ifndef SAVINGS_SCREEN_H
#define SAVINGS_SCREEN_H

#include <string>
#include <cstdint>

#include <lvgl.h>

class SavingsScreen {
public:
    SavingsScreen();
    ~SavingsScreen();

    void Create(lv_obj_t* parent);
    void SetGoal(const std::string& name, int current_amount, int target_amount);

    lv_obj_t* GetContainer() const { return container_; }

private:
    lv_obj_t* container_ = nullptr;
    lv_obj_t* goal_title_ = nullptr;
    lv_obj_t* goal_card_ = nullptr;
    lv_obj_t* goal_icon_ = nullptr;
    lv_obj_t* percent_label_ = nullptr;
    lv_obj_t* amount_label_ = nullptr;
    lv_obj_t* bar_progress_ = nullptr;
};

#endif // SAVINGS_SCREEN_H
