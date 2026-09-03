#include "chooselevelwidget.h"

#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QPainter>
#include <QTimer>

#include "formattime.h"
#include "iconbutton.h"
#include "leveldatabase.h"
#include "progressmanager.h"
#include "recordmanager.h"
#include "soundplayer.h"

namespace {
constexpr int kWindowWidth = 390;
constexpr int kWindowHeight = 570;
constexpr int kLevelCount = LevelDatabase::kCount;

// 徽章网格：58px 徽章，4 列 5 行居中。
constexpr int kBadgeSize = 58;
constexpr int kGridX0 = 55;
constexpr int kGridY0 = 118;
constexpr int kCellPitchX = 74;
constexpr int kCellPitchY = 76;
} // namespace

ChooseLevelWidget::ChooseLevelWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(kWindowWidth, kWindowHeight);
    m_background.load(QStringLiteral(":/images/bg.png"));
    buildUi();
}

void ChooseLevelWidget::buildUi()
{
    // 标题。
    auto *titleLabel = new QLabel(QStringLiteral("选择关卡"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setGeometry(0, 26, kWindowWidth, 44);
    QFont titleFont(QStringLiteral("Microsoft YaHei UI"), 20, QFont::Bold);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, 6.0);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet(QStringLiteral("color: #F4F7FF;"));
    auto *titleShadow = new QGraphicsDropShadowEffect(titleLabel);
    titleShadow->setBlurRadius(16);
    titleShadow->setOffset(0, 3);
    titleShadow->setColor(QColor(0, 0, 0, 170));
    titleLabel->setGraphicsEffect(titleShadow);

    // 解锁进度提示。
    m_unlockedCountLabel = new QLabel(this);
    m_unlockedCountLabel->setAlignment(Qt::AlignCenter);
    m_unlockedCountLabel->setGeometry(0, 70, kWindowWidth, 24);
    m_unlockedCountLabel->setStyleSheet(QStringLiteral(
        "color: rgba(203, 213, 235, 165); font-size: 10pt;"
        " font-family: 'Microsoft YaHei UI';"));

    // 关卡徽章网格。
    QFont numberFont(QStringLiteral("Segoe UI"), 14, QFont::Bold);
    for (int i = 0; i < kLevelCount; ++i) {
        auto *button = new IconButton(QStringLiteral(":/images/badge_level.png"), QString(), this);
        button->move(kGridX0 + (i % 4) * kCellPitchX, kGridY0 + (i / 4) * kCellPitchY);

        // 编号标签作为按钮的子控件，随悬停放大一起移动。
        auto *numberLabel = new QLabel(QString::number(i + 1), button);
        numberLabel->setGeometry(0, 0, button->width(), button->height() - 6);
        numberLabel->setAlignment(Qt::AlignCenter);
        numberLabel->setFont(numberFont);
        numberLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

        const int level = i + 1;
        connect(button, &QPushButton::clicked, this, [this, level] {
            if (m_navigating || !ProgressManager::instance().isUnlocked(level))
                return;
            m_navigating = true;
            SoundPlayer::play(SoundPlayer::Id::Tap);
            QTimer::singleShot(180, this, [this, level] {
                m_navigating = false;
                emit levelSelected(level);
            });
        });

        m_levelButtons[i] = button;
        m_numberLabels[i] = numberLabel;
    }

    // 返回按钮。
    auto *backButton = new IconButton(QStringLiteral(":/images/btn_back.png"), QString(), this);
    backButton->move(kWindowWidth - backButton->width() - 14,
                     kWindowHeight - backButton->height() - 12);
    connect(backButton, &QPushButton::clicked, this, [this] {
        SoundPlayer::play(SoundPlayer::Id::Back);
        emit backRequested();
    });

    refresh();
}

void ChooseLevelWidget::refresh()
{
    const ProgressManager &progress = ProgressManager::instance();
    m_unlockedCountLabel->setText(QStringLiteral("已解锁 %1 / %2 关")
                                      .arg(progress.unlockedThrough())
                                      .arg(kLevelCount));

    for (int i = 0; i < kLevelCount; ++i) {
        const int level = i + 1;
        IconButton *button = m_levelButtons[i];
        QLabel *numberLabel = m_numberLabels[i];
        const bool unlocked = progress.isUnlocked(level);

        button->setClickEnabled(unlocked);
        numberLabel->setStyleSheet(unlocked
            ? QStringLiteral("color: #F5C04C;")
            : QStringLiteral("color: rgba(203, 213, 235, 90);"));

        if (unlocked) {
            button->setGraphicsEffect(nullptr);
            const LevelRecord record = RecordManager::instance().record(level);
            QString tip = QStringLiteral("第 %1 关").arg(level);
            if (record.hasAny()) {
                if (record.bestTime >= 0)
                    tip += QStringLiteral("\n最佳用时 %1").arg(formatSeconds(record.bestTime));
                if (record.bestSteps >= 0)
                    tip += QStringLiteral("\n最少步数 %1").arg(record.bestSteps);
            } else {
                tip += QStringLiteral("\n暂无通关记录");
            }
            button->setToolTip(tip);
        } else {
            // 锁定关卡：整体弱化。
            auto *dimmer = new QGraphicsOpacityEffect(button);
            dimmer->setOpacity(0.35);
            button->setGraphicsEffect(dimmer);
            button->setToolTip(QStringLiteral("通关第 %1 关后解锁").arg(level - 1));
        }
    }
}

void ChooseLevelWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    m_navigating = false;
    for (IconButton *button : m_levelButtons)
        button->resetVisualState();
    refresh();
}

void ChooseLevelWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawPixmap(rect(), m_background, QRectF(m_background.rect()));
}
