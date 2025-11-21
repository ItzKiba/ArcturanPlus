#include <Geode/Geode.hpp>
#include <algorithm>
#include <vector>

using namespace geode::prelude;

// doing everything in main HELL YEAH

const std::string LEVEL_NAME_PATTERN = "tarcturan";

const std::vector<int> SAVABLE_ITEM_IDS = {
    5003, 5015, 5020, 5028, 5030, 5037, 5040, 5051, 5052
};

bool arcturanCheck(PlayLayer* playLayer) {
    std::string name = playLayer->m_level->m_levelName;
    std::transform(name.begin(), name.end(), name.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return name.find(LEVEL_NAME_PATTERN) != std::string::npos;
}

#include <Geode/modify/PlayLayer.hpp>
class $modify(ArcturanPlayLayer, PlayLayer) {
    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        if (!arcturanCheck(this)) return;

        if (this->m_effectManager->countForItem(9999) == 1) {
            saveItemIDs();
            this->m_effectManager->updateCountForItem(9999, 0);
            this->updateCounters(9999, 0);
            Notification::create("Item IDs saved", NotificationIcon::Success, 0.5f)->show();
        } else if (this->m_effectManager->countForItem(9999) == 2) {
            loadItemIDs();
            this->m_effectManager->updateCountForItem(9999, 0);
            this->updateCounters(9999, 0);
            Notification::create("Item IDs loaded", NotificationIcon::Success, 0.5f)->show();
        } else if (this->m_effectManager->countForItem(9999) == 3) { // allows for checking if the mod is enabled
            this->m_effectManager->updateCountForItem(9999, 0);
            this->updateCounters(9999, 0);
        }

        if (this->m_effectManager->countForItem(9998) == 1) {
            loadPlatTime();
            this->m_effectManager->updateCountForItem(9998, 0);
            this->updateCounters(9998, 0);
            Notification::create("Loaded platformer time", NotificationIcon::Success, 0.5f)->show();
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
            int value = Mod::get()->getSavedValue<int>(fmt::format("arcturan_item_{}", id));
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
        if (arcturanCheck(this)) {
            savePlatTime();
            saveItemIDs();
        }
        PlayLayer::onQuit();
    }

    void levelComplete() {
        if (arcturanCheck(this)) {
            savePlatTime();
            saveItemIDs();
        }
        PlayLayer::levelComplete();
    }
};

