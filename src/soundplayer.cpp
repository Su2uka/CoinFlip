#include "soundplayer.h"

#include <QCoreApplication>
#include <QSoundEffect>
#include <QUrl>

namespace {

struct SoundSpec
{
    const char *resource;
    qreal baseVolume;
};

const SoundSpec kSpecs[] = {
    {"qrc:/sounds/TapButtonSound.wav", 0.55},  // Tap
    {"qrc:/sounds/BackButtonSound.wav", 0.55}, // Back
    {"qrc:/sounds/ConFlipSound.wav", 0.85},    // Flip
    {"qrc:/sounds/LevelWinSound.wav", 1.0},    // Win
};

QSoundEffect *effectFor(int index)
{
    static QSoundEffect *kEffects[std::size(kSpecs)] = {};
    static bool kInitialized = false;
    if (!kInitialized) {
        for (int i = 0; i < int(std::size(kSpecs)); ++i) {
            auto *effect = new QSoundEffect(QCoreApplication::instance());
            effect->setSource(QUrl(QLatin1String(kSpecs[i].resource)));
            effect->setVolume(kSpecs[i].baseVolume);
            kEffects[i] = effect;
        }
        kInitialized = true;
    }
    return kEffects[index];
}

} // namespace

void SoundPlayer::play(Id id, qreal volume)
{
    const int index = int(id);
    if (index < 0 || index >= int(std::size(kSpecs)))
        return;
    auto *effect = effectFor(index);
    if (effect->status() == QSoundEffect::Error)
        return;
    effect->setVolume(kSpecs[index].baseVolume * volume);
    effect->play();
}
