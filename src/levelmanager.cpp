//
// Created by PC on 24-12-11.
//


#include "levelmanager.h"


LevelManager::LevelManager(QObject *parent) : QObject(parent) {
    // 初始化 QSettings
    // settings = new QSettings("SUST", "CoinFlip", this);
    settings = new QSettings("level.ini",QSettings::IniFormat);

    // 如果是首次运行，设置初始解锁关卡为 1
    if (!settings->contains(settingsKey)) {
        settings->setValue(settingsKey, 1);
    }
}

LevelManager::~LevelManager() {
    // QSettings 会自动删除，因为它有父对象
}

int LevelManager::getUnlockedLevel() const {
    return settings->value(settingsKey, 1).toInt();
}

void LevelManager::unlockNextLevel(int currentLevel) {
    int unlockedLevel = getUnlockedLevel();
    if (currentLevel >= unlockedLevel && currentLevel < totalLevels) {
        settings->setValue(settingsKey, currentLevel + 1);
    }
}

bool LevelManager::isLevelUnlocked(int level) const {
    return level <= getUnlockedLevel();
}

void LevelManager::resetLevels() {
    settings->setValue(settingsKey, 1);
}
