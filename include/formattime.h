#ifndef FORMATTIME_H
#define FORMATTIME_H

#include <QString>

// 将秒数格式化为 "mm:ss"。
inline QString formatSeconds(int seconds)
{
    return QStringLiteral("%1:%2")
        .arg(qMax(0, seconds) / 60, 2, 10, QChar('0'))
        .arg(qMax(0, seconds) % 60, 2, 10, QChar('0'));
}

#endif // FORMATTIME_H
