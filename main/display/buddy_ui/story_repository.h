#ifndef STORY_REPOSITORY_H
#define STORY_REPOSITORY_H

#include <string>
#include <vector>

struct StoryQuestion {
    std::string question;
    std::vector<std::string> options;
    int correct_index;
};

struct StoryItem {
    std::string id;
    std::string title;
    std::vector<std::string> pages;
    std::vector<StoryQuestion> questions;
};

class StoryRepository {
public:
    static StoryRepository& GetInstance();

    const std::vector<StoryItem>& GetAllStories() const { return stories_; }
    StoryItem GetRandomStory() const;
    StoryItem GetRandomStoryExcluding(const std::string& exclude_id) const;

private:
    StoryRepository();
    void InitializeStories();

    std::vector<StoryItem> stories_;
};

#endif // STORY_REPOSITORY_H
