//
// Created by PC on 24-12-11.
//

#ifndef LEVELMANAGER_H
#define LEVELMANAGER_H

#include <QObject>
#include <QSettings>


class LevelManager : public QObject {
    Q_OBJECT

public:
    explicit LevelManager(QObject *parent = nullptr);
    ~LevelManager();

    // 获取当前已解锁的最高关卡
    int getUnlockedLevel() const;

    // 解锁下一个关卡
    void unlockNextLevel(int currentLevel);

    // 检查指定关卡是否已解锁
    bool isLevelUnlocked(int level) const;

    // 重置所有关卡（可选）
    void resetLevels();

    // 总关卡数
    static const int totalLevels = 20;


private:
    QSettings *settings;
    const QString settingsKey = "unlockedLevel";
};


#endif //LEVELMANAGER_H
