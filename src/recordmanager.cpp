#include "recordmanager.h"

#include <QSettings>

namespace {

QString timeKey(int level)
{
    return QStringLiteral("record/%1/time").arg(level);
}

QString stepsKey(int level)
{
    return QStringLiteral("record/%1/steps").arg(level);
}

} // namespace

RecordManager &RecordManager::instance()
{
    static RecordManager manager;
    return manager;
}

LevelRecord RecordManager::record(int level) const
{
    QSettings settings;
    LevelRecord r;
    r.bestTime = settings.value(timeKey(level), -1).toInt();
    r.bestSteps = settings.value(stepsKey(level), -1).toInt();
    return r;
}

RecordManager::SubmitResult RecordManager::submit(int level, int seconds, int steps)
{
    QSettings settings;
    SubmitResult result;

    const int bestTime = settings.value(timeKey(level), -1).toInt();
    if (bestTime < 0 || seconds < bestTime) {
        settings.setValue(timeKey(level), seconds);
        result.timeImproved = true;
    }

    const int bestSteps = settings.value(stepsKey(level), -1).toInt();
    if (bestSteps < 0 || steps < bestSteps) {
        settings.setValue(stepsKey(level), steps);
        result.stepsImproved = true;
    }

    settings.sync();
    return result;
}
