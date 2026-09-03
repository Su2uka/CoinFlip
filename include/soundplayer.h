#ifndef SOUNDPLAYER_H
#define SOUNDPLAYER_H

#include <QObject>

// 全局音效播放器：懒加载 QSoundEffect，避免每个界面重复创建。
class SoundPlayer : public QObject
{
    Q_OBJECT

public:
    enum class Id
    {
        Tap,   // 通用按钮
        Back,  // 返回
        Flip,  // 翻转金币
        Win    // 通关
    };

    static void play(Id id, qreal volume = 1.0);

private:
    SoundPlayer() = default;
};

#endif // SOUNDPLAYER_H
