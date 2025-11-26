#include <Geode/Geode.hpp>
#include <algorithm>
#include <vector>
#include <Geode/utils/cocos.hpp>

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

using namespace geode::prelude;

// doing everything in main heck yeah

const std::string LEVEL_NAME_PATTERN = "tarcturan";

// 52 - Has Save File (The mod should check for a file and set this id accordingly)
// 5003- Player's Held Item ID (0-60)
// 5015 - Player's Level (0-30) 
// 5020 - Player's Equipped Ability ID (0 - 15)
// 5028 - Potion Level (0 - 10)
// 5037 - Current XP amount (0 - 99999)
// 5040 - Player money (0 - 99999)
// 5051 - The Dungeon the player is up to (1-6)
// 1901-1905 - Has viewed unlock animation for dungeon 2-6
// 5052 - Player's Class (1 = Melee or 0 = Magic)
// 5053 - Player's Armor Level
// 2000 - Armor rune 1
// 2001 - Armor rune 2
// 2002 - Armor rune 3
// 2050 to 2062 - Player's unlocked runes ()
// 9700-9730 - Unlocked Legendary Drops
// 5201 - 5260 - Unlocked Weapons
// 5301 - 5311 - Unlocked Abilities

const std::vector<int> SAVABLE_ITEM_IDS = {
    52, 
    
    5003, 5015, 5020, 5028, 5037, 5040, 5051,
    1901, 1902, 1903, 1904, 1905,
    5052, 5053,
    2000, 2001, 2002,
    2050, 2051, 2052, 2053, 2054, 2055, 2056, 2057, 2058, 2059, 2060, 2061, 2062, 
    
    9700, 9701, 9702, 9703, 9704, 9705, 9706, 9707, 9708, 9709, 9710,
    9710, 9711, 9712, 9713, 9714, 9715, 9716, 9717, 9718, 9719, 9720,
    9720, 9721, 9722, 9723, 9724, 9725, 9726, 9727, 9728, 9729, 9730,

    5201, 5202, 5203, 5204, 5205, 5206, 5207, 5208, 5209, 5210,
    5211, 5212, 5213, 5214, 5215, 5216, 5217, 5218, 5219, 5220,
    5221, 5222, 5223, 5224, 5225, 5226, 5227, 5228, 5229, 5230,
    5231, 5232, 5233, 5234, 5235, 5236, 5237, 5238, 5239, 5240,
    5241, 5242, 5243, 5244, 5245, 5246, 5247, 5248, 5249, 5250,
    5251, 5252, 5253, 5254, 5255, 5256, 5257, 5258, 5259, 5260,

    5301, 5302, 5303, 5304, 5305, 5306, 5307, 5308, 5309, 5310, 5311
};

geode::Notification* createCustomNotif(std::string const& text) {
    auto spr = CCSprite::createWithSpriteFrameName("logo.png"_spr);
    auto notif = Notification::create(text, spr, 2.0f);
    return notif;
}

bool arcturanCheck(std::string levelName) {
    std::transform(levelName.begin(), levelName.end(), levelName.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return levelName.find(LEVEL_NAME_PATTERN) != std::string::npos;
}

class $modify(ArcturanPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // load ID 52 immediately
        int value = Mod::get()->getSavedValue<int>(fmt::format("arcturan_item_{}", 52), 0);
        if (value) {
            this->m_effectManager->updateCountForItem(52, value);
            this->updateCounters(52, value);
        }

        return true;
    }
    
    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (!arcturanCheck(this->m_level->m_levelName)) return;

        if (this->m_isPracticeMode) return;

        if (this->m_effectManager->countForItem(9999) == 1) {
            saveItemIDs();
            this->m_effectManager->updateCountForItem(9999, 0);
            this->updateCounters(9999, 0);
            auto notif = createCustomNotif("Autosave completed");
            notif->show();
        } else if (this->m_effectManager->countForItem(9999) == 2) {
            loadItemIDs();
            this->m_effectManager->updateCountForItem(9999, 0);
            this->updateCounters(9999, 0);
            auto notif = createCustomNotif("Loaded save data");
            notif->show();
        } else if (this->m_effectManager->countForItem(9999) == 3) { // allows for checking if the mod is enabled
            this->m_effectManager->updateCountForItem(9999, 0);
            this->updateCounters(9999, 0);
        }

        if (this->m_effectManager->countForItem(9998) == 1) {
            loadPlatTime();
            this->m_effectManager->updateCountForItem(9998, 0);
            this->updateCounters(9998, 0);
        }

        return;
    }

    void saveItemIDs() {
        for (int id : SAVABLE_ITEM_IDS) {
            Mod::get()->setSavedValue<int>(fmt::format("arcturan_item_{}", id), this->m_effectManager->countForItem(id));
        }
    }

    void loadItemIDs() {
        for (int id : SAVABLE_ITEM_IDS) {
            int value = Mod::get()->getSavedValue<int>(fmt::format("arcturan_item_{}", id), 0);
            if (value) {
                this->m_effectManager->updateCountForItem(id, value);
                this->updateCounters(id, value);
            }
        }
    }

    void savePlatTime() {
        Mod::get()->setSavedValue<double>(fmt::format("arcturan_plat_time"), this->m_timePlayed);
    }

    void loadPlatTime() {
        double value = Mod::get()->getSavedValue<double>(fmt::format("arcturan_plat_time"));
        if (value) {
            this->m_timePlayed = value;
        }
    }

    void onQuit() {
        if (arcturanCheck(this->m_level->m_levelName) && !this->m_isPracticeMode) {
            savePlatTime();
            saveItemIDs();
        }
        PlayLayer::onQuit();
    }

    void levelComplete() {
        if (arcturanCheck(this->m_level->m_levelName) && !this->m_isPracticeMode) {
            savePlatTime();
            saveItemIDs();
        }
        PlayLayer::levelComplete();
    }
};

class $modify(ArcturanPauseLayer, PauseLayer) {
    void customSetup() {

        PauseLayer::customSetup();

        auto pl = PlayLayer::get();
        if (!pl) return;

        if (!arcturanCheck(pl->m_level->m_levelName)) return;

        auto practiceBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(this->getChildByIDRecursive("practice-button"));
        if (!practiceBtn) return;

        practiceBtn->setEnabled(false);
        practiceBtn->setColor({50, 50, 50});
    }
};

