#pragma once

#include <string>
#include <vector>

struct QuizQuestion {
    std::string id;
    std::string question;
    std::vector<std::string> options;
    int correct_index;
};

class QuizRepository {
public:
    static QuizRepository& GetInstance();

    const std::vector<QuizQuestion>& GetAllQuestions() const;
    std::vector<QuizQuestion> GetRandomSessionQuestions(size_t count = 10);

private:
    QuizRepository();
    std::vector<QuizQuestion> questions_;
    void InitializeDefaultBank();
};
