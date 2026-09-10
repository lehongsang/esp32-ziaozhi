#include "quiz_repository.h"

#include <algorithm>
#include <random>
#include <esp_random.h>

QuizRepository& QuizRepository::GetInstance() {
    static QuizRepository instance;
    return instance;
}

QuizRepository::QuizRepository() {
    InitializeDefaultBank();
}

const std::vector<QuizQuestion>& QuizRepository::GetAllQuestions() const {
    return questions_;
}

std::vector<QuizQuestion> QuizRepository::GetRandomSessionQuestions(size_t count) {
    if (questions_.empty()) {
        return {};
    }

    std::vector<QuizQuestion> pool = questions_;
    
    // Hardware RNG shuffle on ESP32
    std::mt19937 g(esp_random());
    std::shuffle(pool.begin(), pool.end(), g);

    if (count > pool.size()) {
        count = pool.size();
    }

    return std::vector<QuizQuestion>(pool.begin(), pool.begin() + count);
}

void QuizRepository::InitializeDefaultBank() {
    questions_ = {
        // Multiplication Table 2 - 9
        {"q01", "What is 8 × 6 ?", {"42", "48", "56", "40"}, 1},
        {"q02", "What is 7 × 7 ?", {"42", "54", "49", "48"}, 2},
        {"q03", "What is 9 × 4 ?", {"36", "32", "28", "40"}, 0},
        {"q04", "What is 6 × 7 ?", {"48", "42", "36", "45"}, 1},
        {"q05", "What is 5 × 9 ?", {"40", "50", "45", "55"}, 2},
        {"q06", "What is 8 × 8 ?", {"64", "56", "72", "60"}, 0},
        {"q07", "What is 9 × 9 ?", {"72", "81", "90", "79"}, 1},
        {"q08", "What is 4 × 8 ?", {"36", "28", "32", "30"}, 2},
        {"q09", "What is 7 × 8 ?", {"54", "64", "58", "56"}, 3},
        {"q10", "What is 6 × 9 ?", {"54", "63", "48", "56"}, 0},
        {"q11", "What is 3 × 9 ?", {"24", "27", "21", "30"}, 1},
        {"q12", "What is 4 × 7 ?", {"24", "26", "28", "32"}, 2},
        {"q13", "What is 8 × 5 ?", {"45", "35", "50", "40"}, 3},
        {"q14", "What is 7 × 5 ?", {"35", "30", "40", "25"}, 0},
        {"q15", "What is 6 × 6 ?", {"30", "36", "42", "32"}, 1},
        {"q16", "What is 9 × 8 ?", {"70", "82", "72", "76"}, 2},
        {"q17", "What is 7 × 9 ?", {"56", "64", "72", "63"}, 3},
        {"q18", "What is 4 × 6 ?", {"24", "28", "20", "30"}, 0},
        {"q19", "What is 8 × 3 ?", {"21", "24", "27", "18"}, 1},
        {"q20", "What is 9 × 3 ?", {"24", "21", "27", "30"}, 2},
        
        // Fast Addition & Subtraction
        {"q21", "What is 45 + 35 ?", {"70", "75", "85", "80"}, 3},
        {"q22", "What is 100 - 37 ?", {"63", "53", "73", "67"}, 0},
        {"q23", "What is 58 + 24 ?", {"72", "82", "84", "78"}, 1},
        {"q24", "What is 90 - 45 ?", {"35", "55", "45", "40"}, 2},
        {"q25", "What is 36 + 48 ?", {"74", "82", "94", "84"}, 3},
        {"q26", "What is 75 - 28 ?", {"47", "57", "43", "53"}, 0},
        {"q27", "What is 64 + 27 ?", {"81", "91", "87", "93"}, 1},
        {"q28", "What is 120 - 45 ?", {"65", "85", "75", "70"}, 2},
        {"q29", "What is 88 + 19 ?", {"97", "105", "109", "107"}, 3},
        {"q30", "What is 50 - 18 ?", {"32", "34", "28", "36"}, 0},
        
        // Fast Division
        {"q31", "What is 48 ÷ 6 ?", {"6", "8", "7", "9"}, 1},
        {"q32", "What is 56 ÷ 7 ?", {"6", "7", "8", "9"}, 2},
        {"q33", "What is 72 ÷ 8 ?", {"6", "8", "7", "9"}, 3},
        {"q34", "What is 81 ÷ 9 ?", {"9", "8", "7", "6"}, 0},
        {"q35", "What is 63 ÷ 7 ?", {"8", "9", "7", "6"}, 1},
        {"q36", "What is 42 ÷ 6 ?", {"6", "8", "7", "9"}, 2},
        {"q37", "What is 36 ÷ 4 ?", {"7", "8", "6", "9"}, 3},
        {"q38", "What is 54 ÷ 6 ?", {"9", "8", "7", "6"}, 0},
        {"q39", "What is 64 ÷ 8 ?", {"6", "8", "7", "9"}, 1},
        {"q40", "What is 35 ÷ 5 ?", {"6", "5", "7", "8"}, 2},
        
        // Geometry & Kid Logic
        {"q41", "A triangle has how many sides?", {"4", "5", "6", "3"}, 3},
        {"q42", "A square has how many corners?", {"4", "3", "5", "6"}, 0},
        {"q43", "How many minutes in 1 hour?", {"50", "60", "70", "100"}, 1},
        {"q44", "How many hours in 1 day?", {"12", "48", "24", "36"}, 2},
        {"q45", "How many days in 1 week?", {"5", "6", "8", "7"}, 3},
        {"q46", "Half of 50 is?", {"25", "20", "30", "15"}, 0},
        {"q47", "Double of 16 is?", {"28", "32", "34", "30"}, 1},
        {"q48", "What is 10 × 10 ?", {"90", "110", "100", "1000"}, 2},
        {"q49", "How many sides does a pentagon have?", {"4", "6", "7", "5"}, 3},
        {"q50", "What is 25 + 25 + 25 ?", {"75", "65", "85", "70"}, 0}
    };
}
