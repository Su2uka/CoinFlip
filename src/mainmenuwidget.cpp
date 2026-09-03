#include "mainmenuwidget.h"

#include <QLabel>
#include <QPainter>

#include "iconbutton.h"
#include "soundplayer.h"

namespace {
constexpr int kWindowWidth = 390;
constexpr int kWindowHeight = 570;
}

MainMenuWidget::MainMenuWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(kWindowWidth, kWindowHeight);

    // 开始按钮（带待机浮动动画）。
    m_startButton = new IconButton(QStringLiteral(":/images/MenuSceneStartButton.png"),
                                   QString(), this);
    m_startButton->move((kWindowWidth - m_startButton->width()) / 2, 336);
    m_startButton->startFloating();
    connect(m_startButton, &QPushButton::clicked, this, [this] {
        SoundPlayer::play(SoundPlayer::Id::Tap);
        emit startRequested();
    });

    // 版本号。
    m_versionLabel = new QLabel(QStringLiteral("v" APP_VERSION), this);
    m_versionLabel->move(12, kWindowHeight - 28);
    m_versionLabel->setStyleSheet(QStringLiteral(
        "color: rgba(255, 247, 230, 190); font-size: 9pt; font-family: 'Microsoft YaHei UI';"));
}

void MainMenuWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    // MenuSceneBg 自带 "MYSTIC COINFLIP" 标题，直接铺满即可。
    painter.drawPixmap(0, 0, QPixmap(QStringLiteral(":/images/MenuSceneBg.png")));
}
