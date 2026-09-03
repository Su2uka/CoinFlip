#include "mainmenuwidget.h"

#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QPainter>

#include "iconbutton.h"
#include "soundplayer.h"

namespace {
constexpr int kWindowWidth = 390;
constexpr int kWindowHeight = 570;
} // namespace

MainMenuWidget::MainMenuWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(kWindowWidth, kWindowHeight);
    m_background.load(QStringLiteral(":/images/bg.png"));

    // 标题组：英文小字排印 + 中文主标题。
    auto *kickerLabel = new QLabel(QStringLiteral("MYSTIC COINFLIP"), this);
    kickerLabel->setAlignment(Qt::AlignCenter);
    kickerLabel->setGeometry(0, 96, kWindowWidth, 26);
    QFont kickerFont(QStringLiteral("Segoe UI"), 10, QFont::DemiBold);
    kickerFont.setLetterSpacing(QFont::AbsoluteSpacing, 7.0);
    kickerLabel->setFont(kickerFont);
    kickerLabel->setStyleSheet(QStringLiteral("color: #F5C04C;"));

    auto *titleLabel = new QLabel(QStringLiteral("翻 金 币"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setGeometry(0, 122, kWindowWidth, 84);
    QFont titleFont(QStringLiteral("Microsoft YaHei UI"), 33, QFont::Black);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet(QStringLiteral("color: #F4F7FF;"));
    auto *titleShadow = new QGraphicsDropShadowEffect(titleLabel);
    titleShadow->setBlurRadius(22);
    titleShadow->setOffset(0, 4);
    titleShadow->setColor(QColor(0, 0, 0, 190));
    titleLabel->setGraphicsEffect(titleShadow);

    auto *taglineLabel = new QLabel(QStringLiteral("点亮所有金币，解开 20 关谜题"), this);
    taglineLabel->setAlignment(Qt::AlignCenter);
    taglineLabel->setGeometry(0, 210, kWindowWidth, 26);
    QFont taglineFont(QStringLiteral("Microsoft YaHei UI"), 10);
    taglineLabel->setFont(taglineFont);
    taglineLabel->setStyleSheet(QStringLiteral("color: rgba(203, 213, 235, 170);"));

    // 开始按钮（带待机浮动动画）。
    m_startButton = new IconButton(QStringLiteral(":/images/btn_start.png"), QString(), this);
    m_startButton->move((kWindowWidth - m_startButton->width()) / 2, 336);
    m_startButton->startFloating();
    connect(m_startButton, &QPushButton::clicked, this, [this] {
        SoundPlayer::play(SoundPlayer::Id::Tap);
        emit startRequested();
    });

    // 版本号。
    QString version = QStringLiteral(APP_VERSION);
    if (!version.startsWith(QLatin1Char('v')))
        version.prepend(QLatin1Char('v'));
    auto *versionLabel = new QLabel(version, this);
    versionLabel->move(12, kWindowHeight - 28);
    versionLabel->setStyleSheet(QStringLiteral(
        "color: rgba(203, 213, 235, 130); font-size: 9pt; font-family: 'Segoe UI';"));
}

void MainMenuWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawPixmap(rect(), m_background, QRectF(m_background.rect()));
}
