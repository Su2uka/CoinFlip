#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H

// 通关成绩管理：记录每关的最佳用时与最少步数（QSettings 持久化）。
struct LevelRecord
{
    int bestTime = -1;   // 秒；-1 表示暂无记录
    int bestSteps = -1;  // -1 表示暂无记录

    bool hasAny() const { return bestTime >= 0 || bestSteps >= 0; }
};

class RecordManager
{
public:
    struct SubmitResult
    {
        bool timeImproved = false;
        bool stepsImproved = false;
        bool any() const { return timeImproved || stepsImproved; }
    };

    static RecordManager &instance();

    LevelRecord record(int level) const;

    // 提交一次通关成绩，返回是否刷新了纪录（并自动保存）。
    SubmitResult submit(int level, int seconds, int steps);

private:
    RecordManager() = default;
};

#endif // RECORDMANAGER_H
