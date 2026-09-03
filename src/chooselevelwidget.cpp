#include "chooselevelwidget.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QPainter>
#include <QTimer>

#include "iconbutton.h"
#include "formattime.h"
#include "leveldatabase.h"
#include "progressmanager.h"
#include "recordmanager.h"
#include "soundplayer.h"

namespace {
constexpr int kWindowWidth = 390;
constexpr int kWindowHeight = 570;
constexpr int kLevelCount = LevelDatabase::kCount;

// 与原教程一致的网格布局参数，但适配 390 宽的窗口。
constexpr int kGridX0 = 58;
constexpr int kGridY0 = 110;
constexpr int kCellPitchX = 72;
constexpr int kCellPitchY = 74;

// 解锁关卡用不透明徽章贴图，锁定关卡保留半透明玻璃球素材。
QPixmap solidLevelIcon()
{
    static const QPixmap pm(QStringLiteral(":/images/LevelIconSolid.png"));
    return pm;
}

QPixmap glassLevelIcon()
{
    static const QPixmap pm(QStringLiteral(":/images/LevelIcon.png"));
    return pm;
}
} // namespace

ChooseLevelWidget::ChooseLevelWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(kWindowWidth, kWindowHeight);
    buildUi();
}

void ChooseLevelWidget::buildUi()
{
    // 标题。
    auto *titleLabel = new QLabel(QStringLiteral("选 择 关 卡"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setGeometry(0, 22, kWindowWidth, 48);
    QFont titleFont = titleLabel->font();
    titleFont.setFamilies({QString::fromUtf8("华文新魏"), QStringLiteral("KaiTi"),
                           QStringLiteral("Microsoft YaHei UI")});
    titleFont.setPointSize(21);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet(QStringLiteral("color: #FFF7E6;"));
    auto *titleShadow = new QGraphicsDropShadowEffect(titleLabel);
    titleShadow->setBlurRadius(6);
    titleShadow->setOffset(0, 2);
    titleShadow->setColor(QColor(0, 0, 0, 150));
    titleLabel->setGraphicsEffect(titleShadow);

    // 解锁进度提示。
    m_unlockedCountLabel = new QLabel(this);
    m_unlockedCountLabel->setAlignment(Qt::AlignCenter);
    m_unlockedCountLabel->setGeometry(0, 68, kWindowWidth, 24);
    m_unlockedCountLabel->setStyleSheet(QStringLiteral(
        "color: rgba(255, 247, 230, 175); font-size: 9.5pt; font-family: 'Microsoft YaHei UI';"));

    // 关卡按钮网格。
    for (int i = 0; i < kLevelCount; ++i) {
        auto *button = new IconButton(QStringLiteral(":/images/LevelIconSolid.png"),
                                      QString(), this);
        button->move(kGridX0 + (i % 4) * kCellPitchX, kGridY0 + (i / 4) * kCellPitchY);

        // 编号标签作为按钮的子控件，随悬停放大一起移动。
        auto *numberLabel = new QLabel(QString::number(i + 1), button);
        numberLabel->setGeometry(0, 0, button->width(), button->height());
        numberLabel->setAlignment(Qt::AlignCenter);
        QFont numberFont = numberLabel->font();
        numberFont.setPointSize(13);
        numberFont.setBold(true);
        numberLabel->setFont(numberFont);
        numberLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        auto *numberShadow = new QGraphicsDropShadowEffect(numberLabel);
        numberShadow->setBlurRadius(3);
        numberShadow->setOffset(0, 1);
        numberShadow->setColor(QColor(40, 24, 8, 170));
        numberLabel->setGraphicsEffect(numberShadow);

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
    auto *backButton = new IconButton(QStringLiteral(":/images/BackButton.png"),
                                      QStringLiteral(":/images/BackButtonSelected.png"), this);
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
        button->setPixmap(unlocked ? solidLevelIcon() : glassLevelIcon());
        numberLabel->setStyleSheet(unlocked
            ? QStringLiteral("color: #FFF7E6;")
            : QStringLiteral("color: rgba(215, 205, 195, 0.8);"));

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
            // 锁定关卡：半透明弱化。
            auto *dimmer = new QGraphicsOpacityEffect(button);
            dimmer->setOpacity(0.38);
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
    painter.drawPixmap(0, 0, QPixmap(QStringLiteral(":/images/OtherSceneBg.png")));
}
