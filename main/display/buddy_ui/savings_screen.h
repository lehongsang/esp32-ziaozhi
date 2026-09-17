#ifndef SAVINGS_SCREEN_H
#define SAVINGS_SCREEN_H

#include <string>
#include <vector>
#include <cstdint>

#include <lvgl.h>

enum class GoalType {
    kNone = 0,
    kBike,         // 1. My New Bike
    kRobot,        // 2. Smart Robot
    kLego,         // 3. LEGO Castle
    kSkateboard,   // 4. Pro Skateboard
    kRollerSkates, // 5. Roller Skates
    kGameConsole,  // 6. Game Console
    kTeddyBear,    // 7. Giant Teddy Bear
    kBookshelf,    // 8. Science Bookshelf
    kTelescope,    // 9. Space Telescope
    kGuitar        // 10. Acoustic Guitar
};

struct GoalItemDef {
    GoalType type;
    std::string name;
    int32_t default_price;
    const lv_image_dsc_t* image_dsc;
};

class SavingsScreen {
public:
    SavingsScreen();
    ~SavingsScreen();

    void Create(lv_obj_t* parent);
    void SetGoal(GoalType type, const std::string& name, int32_t current_amount, int32_t target_amount, const std::string& currency = "d");
    void AddSavings(int32_t amount);
    void ResetGoal();

    lv_obj_t* GetContainer() const { return root_container_; }

    void ShowEmptyView();
    void ShowActiveGoalView();
    void ShowGoalSelectionView();
    void ShowPriceInputView(GoalType selected_type);

private:
    void LoadFromNVS();
    void SaveToNVS();
    bool IsGoalConfigured(GoalType type) const;
    void SetGoalConfigured(GoalType type, bool configured);
    int32_t GetSavedGoalPrice(GoalType type) const;
    void SetSavedGoalPrice(GoalType type, int32_t price);
    void RefreshGoalSelectionList();
    void UpdateActiveGoalUI(bool animate = true);
    void UpdatePriceInputDisplay();

    void BuildEmptyView();
    void BuildActiveGoalView();
    void BuildGoalSelectionView();
    void BuildPriceInputView();

    const GoalItemDef* FindGoalDef(GoalType type) const;

    static void OnCreateGoalClicked(lv_event_t* e);
    static void OnChangeGoalClicked(lv_event_t* e);
    static void OnEditPriceClicked(lv_event_t* e);
    static void OnBackToPreviousClicked(lv_event_t* e);
    static void OnGoalSelected(lv_event_t* e);

    static void OnKeypadDigitClicked(lv_event_t* e);
    static void OnQuickAddAmountClicked(lv_event_t* e);
    static void OnConfirmPriceClicked(lv_event_t* e);

    // Root and Sub-Views
    lv_obj_t* root_container_ = nullptr;
    lv_obj_t* view_empty_ = nullptr;
    lv_obj_t* view_active_goal_ = nullptr;
    lv_obj_t* view_select_goal_ = nullptr;
    lv_obj_t* view_input_price_ = nullptr;

    // View: Active Goal Elements
    lv_obj_t* goal_title_label_ = nullptr;
    lv_obj_t* goal_image_ = nullptr;
    lv_obj_t* amount_label_ = nullptr;
    lv_obj_t* bar_progress_ = nullptr;

    // View: Goal Selection Elements
    lv_obj_t* goal_list_container_ = nullptr;

    // View: Price Input Elements
    lv_obj_t* input_goal_name_label_ = nullptr;
    lv_obj_t* input_amount_display_label_ = nullptr;
    std::string input_buffer_ = "2000000";
    GoalType pending_goal_type_ = GoalType::kNone;

    // Current State
    bool has_active_goal_ = false;
    GoalType current_goal_type_ = GoalType::kNone;
    std::string current_goal_name_ = "";
    int32_t current_amount_ = 0;
    int32_t target_amount_ = 0;
    std::string currency_ = "d";

    // Enum-Based Catalog
    std::vector<GoalItemDef> goal_definitions_;
};

#endif // SAVINGS_SCREEN_H
