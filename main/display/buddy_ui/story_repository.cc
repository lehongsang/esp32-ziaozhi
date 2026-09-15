#include "story_repository.h"
#include <esp_random.h>
#include <algorithm>

StoryRepository& StoryRepository::GetInstance() {
    static StoryRepository instance;
    return instance;
}

StoryRepository::StoryRepository() {
    InitializeStories();
}

void StoryRepository::InitializeStories() {
    stories_ = {
        // Story 1
        {
            "s1",
            "The Little Red Apple",
            {
                "Piggy was walking in the green forest on a sunny morning.\nHe saw a shiny red apple hanging high on a big tree.",
                "Bunny hopped over and helped Piggy shake the tree branch.\nThe sweet apple fell down, and they shared it happily together!"
            },
            {
                {"What did Piggy see on the tree?", {"A yellow banana", "A red apple", "A blue bird", "A green leaf"}, 1},
                {"Who helped Piggy shake the tree?", {"Puppy", "Kitten", "Bunny", "Bear"}, 2},
                {"What did they do with the apple?", {"Threw it away", "Sold it", "Shared it happily", "Hid it"}, 2}
            }
        },
        // Story 2
        {
            "s2",
            "The Brave Little Puppy",
            {
                "Max is a small puppy with fluffy brown ears.\nOne evening, he heard a tiny kitten crying behind the wooden fence.",
                "Max ran fast, guided his friend Piggy to the spot,\nand together they brought the cute kitten home safely."
            },
            {
                {"What color are Max's fluffy ears?", {"White", "Brown", "Black", "Golden"}, 1},
                {"Who was crying behind the fence?", {"A puppy", "A bird", "A tiny kitten", "A duck"}, 2},
                {"What did Max and Piggy do?", {"Brought kitten home", "Ran away", "Went to sleep", "Ate lunch"}, 0}
            }
        },
        // Story 3
        {
            "s3",
            "The Flying Colorful Kite",
            {
                "On a windy afternoon, Leo and Piggy took a rainbow kite to the hill.\nThe cool wind blew gently across the grassy field.",
                "The kite soared high into the bright blue sky like a bird.\nAll the woodland friends cheered and clapped with joy!"
            },
            {
                {"Where did they fly the kite?", {"At the beach", "On the hill", "In the cave", "In the bedroom"}, 1},
                {"What color was the kite?", {"Rainbow", "Plain white", "Dark gray", "Brown"}, 0},
                {"How did the friends feel?", {"Sad", "Angry", "Cheered with joy", "Scared"}, 2}
            }
        },
        // Story 4
        {
            "s4",
            "The Magic Sunflower Seed",
            {
                "Emma found a small shiny seed in her garden.\nShe carefully planted it in soft soil and watered it every morning.",
                "In two weeks, a tall, bright golden sunflower bloomed.\nSweet yellow honeybees came by to say hello!"
            },
            {
                {"What did Emma find in her garden?", {"A gold coin", "A magic seed", "A toy car", "A stone"}, 1},
                {"What kind of flower bloomed?", {"A red rose", "A blue tulip", "A golden sunflower", "A daisy"}, 2},
                {"Who came to say hello?", {"Honeybees", "Butterflies", "Ants", "Frogs"}, 0}
            }
        },
        // Story 5
        {
            "s5",
            "Baking Sweet Cookies",
            {
                "Piggy put on a chef hat to bake warm butter cookies.\nHe mixed flour, sweet honey, and chocolate chips in a big bowl.",
                "The kitchen smelled wonderful as the oven timer rang.\nPiggy served hot crunchy cookies with a glass of fresh milk."
            },
            {
                {"What kind of cookies did Piggy bake?", {"Salty chips", "Butter cookies", "Spicy bread", "Sour cake"}, 1},
                {"What did Piggy wear on his head?", {"A baseball cap", "A crown", "A chef hat", "A helmet"}, 2},
                {"What drink did he serve with cookies?", {"Fresh milk", "Hot soup", "Orange soda", "Iced tea"}, 0}
            }
        },
        // Story 6
        {
            "s6",
            "The Singing Star in the Sky",
            {
                "Late at night, little Mia looked out her bedroom window.\nA tiny sparkling star twinkled and hummed a sweet bedtime song.",
                "Mia smiled, made a wish for peaceful dreams, and closed her eyes.\nThe silver moon watched over her all night long."
            },
            {
                {"When did Mia look out her window?", {"At noon", "In the morning", "Late at night", "At breakfast"}, 2},
                {"What did the tiny star do?", {"Fell down", "Hummed a song", "Disappeared", "Blew wind"}, 1},
                {"Who watched over Mia all night?", {"The warm sun", "The clouds", "The silver moon", "The birds"}, 2}
            }
        },
        // Story 7
        {
            "s7",
            "The Rainbow Fish Adventure",
            {
                "Finny the little fish swam in the sparkling blue pond.\nHis shiny scales sparkled in seven beautiful rainbow colors.",
                "Finny shared his glowing scales with shy baby fish,\nand soon the entire pond was glowing like a magical garden."
            },
            {
                {"Where did Finny swim?", {"In a pool", "In a blue pond", "In a bathtub", "In the desert"}, 1},
                {"How many colors did his scales have?", {"Two", "Three", "Seven colors", "One"}, 2},
                {"Why was the pond glowing?", {"Night lights", "Finny shared scales", "Fireworks", "Stars"}, 1}
            }
        },
        // Story 8
        {
            "s8",
            "The Sunny Forest Picnic",
            {
                "On Sunday, Piggy, Bunny, and Bear packed a picnic basket.\nThey brought fresh strawberries, sweet corn, and apple juice.",
                "They spread a checkered blanket under a shady oak tree\nand sang cheerful songs until the golden sunset arrived."
            },
            {
                {"What day did they have a picnic?", {"Monday", "Friday", "Sunday", "Tuesday"}, 2},
                {"Which fruits did they bring?", {"Strawberries & corn", "Lemons & onions", "Grapes & peppers", "Coconuts"}, 0},
                {"Where did they spread the blanket?", {"On a rock", "Under an oak tree", "On the road", "In the water"}, 1}
            }
        },
        // Story 9
        {
            "s9",
            "The Lost Wooden Toy",
            {
                "Teddy the bear lost his favorite wooden train in the tall grass.\nHe looked under bushes and around the garden bench.",
                "Piggy used his sharp eyes and found the toy train by the flowers.\nTeddy gave Piggy a big warm hug with a joyful smile!"
            },
            {
                {"What toy did Teddy lose?", {"A teddy bear", "A wooden train", "A rubber duck", "A plastic ball"}, 1},
                {"Where was the train found?", {"By the flowers", "In the river", "On the roof", "Inside a box"}, 0},
                {"How did Teddy thank Piggy?", {"Gave a toy", "Gave a warm hug", "Cooked dinner", "Ran away"}, 1}
            }
        },
        // Story 10
        {
            "s10",
            "The Little Rainy Day Boots",
            {
                "Raindrops fell gently from fluffy gray clouds.\nPiggy put on his bright yellow raincoat and shiny red boots.",
                "He jumped into shallow puddles with happy splashes!\nSoon the rain stopped, and a giant rainbow arched across the sky."
            },
            {
                {"What color was Piggy's raincoat?", {"Bright yellow", "Dark green", "Purple", "Pink"}, 0},
                {"What color were his boots?", {"Black", "Shiny red", "White", "Blue"}, 1},
                {"What appeared after the rain stopped?", {"Snow", "A thunderstorm", "A giant rainbow", "Fog"}, 2}
            }
        }
    };
}

StoryItem StoryRepository::GetRandomStory() const {
    if (stories_.empty()) {
        return {
            "default",
            "A Happy Day",
            {"Piggy and friends had a wonderful day playing together."},
            {{"Was Piggy happy?", {"Yes", "No", "Sad", "Angry"}, 0}}
        };
    }
    uint32_t idx = esp_random() % stories_.size();
    return stories_[idx];
}

StoryItem StoryRepository::GetRandomStoryExcluding(const std::string& exclude_id) const {
    if (stories_.empty()) {
        return GetRandomStory();
    }
    std::vector<size_t> valid_indices;
    for (size_t i = 0; i < stories_.size(); ++i) {
        if (stories_[i].id != exclude_id) {
            valid_indices.push_back(i);
        }
    }
    if (valid_indices.empty()) {
        return GetRandomStory();
    }
    uint32_t choice = esp_random() % valid_indices.size();
    return stories_[valid_indices[choice]];
}
