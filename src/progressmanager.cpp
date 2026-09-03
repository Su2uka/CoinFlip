#include "progressmanager.h"

#include <QSettings>

#include "leveldatabase.h"

ProgressManager &ProgressManager::instance()
{
    static ProgressManager manager;
    return manager;
}

ProgressManager::ProgressManager()
{
    QSettings settings;
    m_unlocked = qBound(1,
                        settings.value(QStringLiteral("progress/unlocked"), 1).toInt(),
                        LevelDatabase::kCount);
}

bool ProgressManager::isUnlocked(int level) const
{
    return level >= 1 && level <= m_unlocked;
}

int ProgressManager::unlockedThrough() const
{
    return m_unlocked;
}

void ProgressManager::unlock(int level)
{
    if (level < 1 || level >= LevelDatabase::kCount)
        return;
    const int next = level + 1;
    if (next > m_unlocked) {
        m_unlocked = next;
        persist();
    }
}

void ProgressManager::reset()
{
    m_unlocked = 1;
    persist();
}

void ProgressManager::persist()
{
    QSettings settings;
    settings.setValue(QStringLiteral("progress/unlocked"), m_unlocked);
}
