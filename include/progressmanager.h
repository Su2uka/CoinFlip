#ifndef PROGRESSMANAGER_H
#define PROGRESSMANAGER_H

// 解锁进度管理：通过第 N 关后解锁第 N+1 关。
// 进度通过 QSettings 持久化（Windows 下保存在注册表，随应用组织/应用名定位）。
class ProgressManager
{
public:
    static ProgressManager &instance();

    // 关卡编号从 1 开始。
    bool isUnlocked(int level) const;
    int unlockedThrough() const;

    // 通关 level 后调用（解锁 level+1，如有）。
    void unlock(int level);

    // 清空进度，回到只解锁第 1 关。
    void reset();

private:
    ProgressManager();
    void persist();

    int m_unlocked = 1;
};

#endif // PROGRESSMANAGER_H
