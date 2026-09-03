#include "playwidget.h"

#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>

#include "coinbutton.h"
#include "formattime.h"
#include "iconbutton.h"
#include "leveldatabase.h"
#include "progressmanager.h"
#include "recordmanager.h"
#include "soundplayer.h"

namespace {
constexpr int kWindowWidth = 390;
constexpr int kWindowHeight = 570;

// 棋盘：60px 槽位 + 4px 间隙，4×4 居中；金币 56px 叠于槽位内。
constexpr int kSlotSize = 60;
constexpr int kCellPitch = 64;
constexpr int kBoardX0 = (kWindowWidth - (3 * kCellPitch + kSlotSize)) / 2; // 69
constexpr int kBoardY0 = 168;
constexpr int kCoinInset = 2;

constexpr int kNeighborDelayMs = 160;
constexpr int kFlipTotalMs = 230;   // 8 帧 × 26ms，留一点余量

constexpr int kBannerWidth = 300;
constexpr int kBannerHeight = 64;
constexpr int kBannerX = (kWindowWidth - kBannerWidth) / 2;
constexpr int kBannerY = 206;

// 金色胶囊按钮（游戏中与结算面板共用）。
const char *kGoldButtonStyle =
    "QPushButton { background-color: #F5B93B; color: #241703; border: none;"
    "  border-radius: 17px; padding: 0 14px; min-height: 32px;"
    "  font-family: 'Microsoft YaHei UI'; font-size: 11.5pt; font-weight: bold; }"
    "QPushButton:hover { background-color: #FFC94F; }"
    "QPushButton:pressed { background-color: #D89A2B; }";
} // namespace

PlayWidget::PlayWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(kWindowWidth, kWindowHeight);
    m_background.load(QStringLiteral(":/images/bg.png"));
    buildUi();

    m_clock.setInterval(1000);
    connect(&m_clock, &QTimer::timeout, this, [this] {
        ++m_seconds;
        updateStats();
    });

    // 快捷键：Esc 返回选关，F5 重开本关。
    auto *escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, this, [this] {
        SoundPlayer::play(SoundPlayer::Id::Back);
        emit backRequested();
    });
    auto *restartShortcut = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(restartShortcut, &QShortcut::activated, this, [this] {
        SoundPlayer::play(SoundPlayer::Id::Tap);
        restartLevel();
    });
}

void PlayWidget::buildUi()
{
    // 顶部：关卡名。
    m_levelLabel = new QLabel(this);
    m_levelLabel->setGeometry(20, 12, 180, 44);
    QFont levelFont(QStringLiteral("Microsoft YaHei UI"), 18, QFont::Bold);
    m_levelLabel->setFont(levelFont);
    m_levelLabel->setStyleSheet(QStringLiteral("color: #F4F7FF;"));
    auto *levelShadow = new QGraphicsDropShadowEffect(m_levelLabel);
    levelShadow->setBlurRadius(14);
    levelShadow->setOffset(0, 3);
    levelShadow->setColor(QColor(0, 0, 0, 170));
    m_levelLabel->setGraphicsEffect(levelShadow);

    // 计时/步数面板（深色玻璃）。
    auto *statPanel = new QWidget(this);
    statPanel->setObjectName(QStringLiteral("statPanel"));
    statPanel->setAttribute(Qt::WA_StyledBackground, true); // 让 QWidget 绘制样式表背景
    statPanel->setGeometry(kWindowWidth - 150 - 16, 14, 150, 56);
    statPanel->setStyleSheet(QStringLiteral(
        "#statPanel { background-color: rgba(13, 18, 40, 165);"
        "  border: 1px solid rgba(255, 255, 255, 38); border-radius: 12px; }"
        "#statPanel QLabel { color: #DEE6F5; font-family: 'Microsoft YaHei UI';"
        "  font-size: 10.5pt; font-weight: 600; }"));
    m_timeLabel = new QLabel(statPanel);
    m_stepsLabel = new QLabel(statPanel);
    auto *statLayout = new QVBoxLayout(statPanel);
    statLayout->setContentsMargins(14, 8, 14, 8);
    statLayout->setSpacing(1);
    statLayout->addWidget(m_timeLabel);
    statLayout->addWidget(m_stepsLabel);

    // 棋盘：槽位（深色玻璃凹槽）+ 金币。
    for (int r = 0; r < LevelDatabase::kRows; ++r) {
        for (int c = 0; c < LevelDatabase::kCols; ++c) {
            auto *node = new QLabel(this);
            node->setObjectName(QStringLiteral("boardSlot"));
            node->setAttribute(Qt::WA_StyledBackground, true);
            node->setGeometry(kBoardX0 + c * kCellPitch, kBoardY0 + r * kCellPitch,
                              kSlotSize, kSlotSize);
            node->setStyleSheet(QStringLiteral(
                "#boardSlot { background-color: rgba(8, 12, 26, 150);"
                "  border: 1px solid rgba(255, 255, 255, 26); border-radius: 14px; }"));
            node->setAttribute(Qt::WA_TransparentForMouseEvents);

            auto *coin = new CoinButton(r, c, false, this);
            coin->move(kBoardX0 + c * kCellPitch + kCoinInset,
                       kBoardY0 + r * kCellPitch + kCoinInset);
            connect(coin, &QPushButton::clicked, this,
                    [this, coin] {
                        if (coin->tryActivate())
                            onCoinActivated(coin);
                    });
            m_coins[r][c] = coin;
        }
    }

    // 底部：重玩 + 返回。
    auto *restartButton = new QPushButton(QStringLiteral("重玩"), this);
    restartButton->setStyleSheet(kGoldButtonStyle);
    restartButton->setGeometry(16, kWindowHeight - 50, 92, 34);
    restartButton->setToolTip(QStringLiteral("重新开始本关（F5）"));
    restartButton->setCursor(Qt::PointingHandCursor);
    connect(restartButton, &QPushButton::clicked, this, [this] {
        SoundPlayer::play(SoundPlayer::Id::Tap);
        restartLevel();
    });

    auto *backButton = new IconButton(QStringLiteral(":/images/btn_back.png"), QString(), this);
    backButton->move(kWindowWidth - backButton->width() - 14,
                     kWindowHeight - backButton->height() - 12);
    connect(backButton, &QPushButton::clicked, this, [this] {
        SoundPlayer::play(SoundPlayer::Id::Back);
        emit backRequested();
    });

    // 通关横幅与结算面板（平时隐藏）。
    m_bannerLabel = new QLabel(QStringLiteral("★ 通关成功 ★"), this);
    m_bannerLabel->setObjectName(QStringLiteral("winBanner"));
    m_bannerLabel->setAttribute(Qt::WA_StyledBackground, true);
    m_bannerLabel->setAlignment(Qt::AlignCenter);
    QFont bannerFont(QStringLiteral("Microsoft YaHei UI"), 15, QFont::Bold);
    bannerFont.setLetterSpacing(QFont::AbsoluteSpacing, 3.0);
    m_bannerLabel->setFont(bannerFont);
    m_bannerLabel->setStyleSheet(QStringLiteral(
        "#winBanner { background-color: rgba(13, 18, 40, 226);"
        "  border: 2px solid #F5B93B; border-radius: 18px; color: #FFD766; }"));
    m_bannerLabel->setGeometry(kBannerX, -kBannerHeight - 8, kBannerWidth, kBannerHeight);
    m_bannerLabel->hide();

    m_resultPanel = new QWidget(this);
    m_resultPanel->setObjectName(QStringLiteral("winPanel"));
    m_resultPanel->setAttribute(Qt::WA_StyledBackground, true);
    m_resultPanel->setGeometry(35, 310, 320, 172);
    m_resultPanel->setStyleSheet(QStringLiteral(
        "#winPanel { background-color: rgba(13, 18, 40, 238);"
        "  border: 1px solid rgba(255, 255, 255, 40); border-radius: 18px; }"
        "#winPanel QLabel { color: #EDF2FC; font-family: 'Microsoft YaHei UI';"
        "  font-size: 11.5pt; font-weight: 600; }"
        "#winPanel QPushButton { background-color: #F5B93B; color: #241703; border: none;"
        "  border-radius: 16px; padding: 0 10px; min-height: 32px;"
        "  font-family: 'Microsoft YaHei UI'; font-size: 11pt; font-weight: bold; }"
        "#winPanel QPushButton:hover { background-color: #FFC94F; }"
        "#winPanel QPushButton:pressed { background-color: #D89A2B; }"));
    m_resultLabel = new QLabel(m_resultPanel);
    m_resultLabel->setAlignment(Qt::AlignCenter);

    m_nextButton = new QPushButton(QStringLiteral("下一关"), m_resultPanel);
    m_replayButton = new QPushButton(QStringLiteral("重玩"), m_resultPanel);
    m_backButton = new QPushButton(QStringLiteral("返回选关"), m_resultPanel);
    for (QPushButton *button : {m_nextButton, m_replayButton, m_backButton})
        button->setCursor(Qt::PointingHandCursor);
    connect(m_nextButton, &QPushButton::clicked, this, [this] {
        SoundPlayer::play(SoundPlayer::Id::Tap);
        startLevel(m_level + 1);
    });
    connect(m_replayButton, &QPushButton::clicked, this, [this] {
        SoundPlayer::play(SoundPlayer::Id::Tap);
        restartLevel();
    });
    connect(m_backButton, &QPushButton::clicked, this, [this] {
        SoundPlayer::play(SoundPlayer::Id::Back);
        emit backRequested();
    });

    auto *resultLayout = new QVBoxLayout(m_resultPanel);
    resultLayout->setContentsMargins(16, 14, 16, 14);
    resultLayout->setSpacing(12);
    resultLayout->addWidget(m_resultLabel);
    auto *buttonRow = new QHBoxLayout;
    buttonRow->setSpacing(10);
    buttonRow->addWidget(m_nextButton);
    buttonRow->addWidget(m_replayButton);
    buttonRow->addWidget(m_backButton);
    resultLayout->addLayout(buttonRow);

    m_resultPanel->hide();
}

void PlayWidget::startLevel(int level)
{
    m_level = qBound(1, level, LevelDatabase::kCount);
    m_levelLabel->setText(QStringLiteral("第 %1 关").arg(m_level));
    restartLevel();
}

void PlayWidget::restartLevel()
{
    ++m_session;
    stopPendingAnimations();

    const LevelDatabase::Board &data = LevelDatabase::levels()[m_level];
    for (int r = 0; r < LevelDatabase::kRows; ++r) {
        for (int c = 0; c < LevelDatabase::kCols; ++c) {
            m_board[r][c] = data[r][c];
            m_coins[r][c]->setGoldFace(data[r][c] == 1);
        }
    }

    m_seconds = 0;
    m_steps = 0;
    updateStats();
    m_clock.start();

    m_finished = false;
    m_busy = false;
    setBoardInteractive(true);

    m_bannerLabel->hide();
    m_resultPanel->hide();

    playEntranceAnimation();
}

void PlayWidget::onCoinActivated(CoinButton *coin)
{
    if (m_busy || m_finished)
        return;
    m_busy = true;
    setBoardInteractive(false);

    ++m_steps;
    updateStats();
    SoundPlayer::play(SoundPlayer::Id::Flip);

    m_board[coin->row()][coin->col()] ^= 1;
    coin->flip();

    const int session = m_session;
    QTimer::singleShot(kNeighborDelayMs, this, [this, session, coin] {
        if (session != m_session)
            return;
        static constexpr int dr[4] = {1, -1, 0, 0};
        static constexpr int dc[4] = {0, 0, 1, -1};
        for (int a = 0; a < 4; ++a) {
            const int r = coin->row() + dr[a];
            const int c = coin->col() + dc[a];
            if (r >= 0 && r < LevelDatabase::kRows && c >= 0 && c < LevelDatabase::kCols) {
                m_board[r][c] ^= 1;
                m_coins[r][c]->flip();
            }
        }
    });
    QTimer::singleShot(kNeighborDelayMs + kFlipTotalMs, this, [this, session] {
        if (session != m_session)
            return;
        finishMove();
    });
}

void PlayWidget::finishMove()
{
    m_busy = false;

    LevelDatabase::Board board;
    for (int r = 0; r < LevelDatabase::kRows; ++r)
        for (int c = 0; c < LevelDatabase::kCols; ++c)
            board[r][c] = m_board[r][c];

    if (LevelDatabase::isSolved(board)) {
        onWin();
    } else {
        setBoardInteractive(true);
    }
}

void PlayWidget::onWin()
{
    m_finished = true;
    m_clock.stop();
    setBoardInteractive(false);

    const RecordManager::SubmitResult submit =
        RecordManager::instance().submit(m_level, m_seconds, m_steps);
    ProgressManager::instance().unlock(m_level);

    QString text = QStringLiteral("第 %1 关完成！\n用时 %2 · 步数 %3")
                       .arg(m_level)
                       .arg(formatSeconds(m_seconds))
                       .arg(m_steps);
    if (submit.timeImproved && submit.stepsImproved)
        text += QStringLiteral("\n🎉 用时与步数双双刷新纪录！");
    else if (submit.timeImproved)
        text += QStringLiteral("\n🎉 刷新最快用时纪录！");
    else if (submit.stepsImproved)
        text += QStringLiteral("\n🎉 刷新最少步数纪录！");
    m_resultLabel->setText(text);
    m_nextButton->setVisible(m_level < LevelDatabase::kCount);

    const int session = m_session;
    QTimer::singleShot(300, this, [this, session] {
        if (session != m_session)
            return;
        SoundPlayer::play(SoundPlayer::Id::Win);
        m_bannerLabel->show();
        m_bannerLabel->raise();
        m_bannerAnim = new QPropertyAnimation(m_bannerLabel, "geometry", this);
        m_bannerAnim->setDuration(850);
        m_bannerAnim->setEasingCurve(QEasingCurve::OutBounce);
        m_bannerAnim->setStartValue(
            QRect(kBannerX, -kBannerHeight - 8, kBannerWidth, kBannerHeight));
        m_bannerAnim->setEndValue(QRect(kBannerX, kBannerY, kBannerWidth, kBannerHeight));
        connect(m_bannerAnim, &QPropertyAnimation::finished, this, [this, session] {
            if (session != m_session)
                return;
            m_bannerAnim = nullptr;
            showResultPanel();
        });
        m_bannerAnim->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void PlayWidget::showResultPanel()
{
    auto *effect = new QGraphicsOpacityEffect(m_resultPanel);
    effect->setOpacity(0.0);
    m_resultPanel->setGraphicsEffect(effect);
    m_resultPanel->show();
    m_resultPanel->raise();

    auto *anim = new QPropertyAnimation(effect, "opacity", effect);
    anim->setDuration(260);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &QPropertyAnimation::finished, this, [this] {
        m_resultPanel->setGraphicsEffect(nullptr);
    }, Qt::QueuedConnection);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void PlayWidget::updateStats()
{
    m_timeLabel->setText(QStringLiteral("时间  %1").arg(formatSeconds(m_seconds)));
    m_stepsLabel->setText(QStringLiteral("步数  %1").arg(m_steps));
}

void PlayWidget::setBoardInteractive(bool on)
{
    for (int r = 0; r < LevelDatabase::kRows; ++r)
        for (int c = 0; c < LevelDatabase::kCols; ++c)
            m_coins[r][c]->setInteractive(on);
}

void PlayWidget::playEntranceAnimation()
{
    const int session = m_session;
    setBoardInteractive(false);
    m_busy = true;

    int totalMs = 0;
    for (int r = 0; r < LevelDatabase::kRows; ++r) {
        for (int c = 0; c < LevelDatabase::kCols; ++c) {
            CoinButton *coin = m_coins[r][c];
            const int delay = (r * LevelDatabase::kCols + c) * 22;

            auto *effect = new QGraphicsOpacityEffect(coin);
            effect->setOpacity(0.0);
            coin->setGraphicsEffect(effect);

            auto *anim = new QPropertyAnimation(effect, "opacity", effect);
            anim->setDuration(150);
            anim->setStartValue(0.0);
            anim->setEndValue(1.0);
            anim->setEasingCurve(QEasingCurve::OutQuad);
            connect(anim, &QPropertyAnimation::finished, coin, [this, coin, session] {
                if (session == m_session)
                    coin->setGraphicsEffect(nullptr);
            }, Qt::QueuedConnection);
            m_entranceAnims.append(anim);

            QTimer::singleShot(delay, this, [this, anim, session] {
                if (session != m_session)
                    return;
                anim->start(QAbstractAnimation::DeleteWhenStopped);
            });
            totalMs = qMax(totalMs, delay + 150);
        }
    }

    QTimer::singleShot(totalMs + 40, this, [this, session] {
        if (session != m_session)
            return;
        m_busy = false;
        setBoardInteractive(true);
    });
}

void PlayWidget::stopPendingAnimations()
{
    for (const QPointer<QPropertyAnimation> &anim : m_entranceAnims) {
        if (!anim.isNull())
            anim->stop();
    }
    m_entranceAnims.clear();
    for (int r = 0; r < LevelDatabase::kRows; ++r)
        for (int c = 0; c < LevelDatabase::kCols; ++c)
            m_coins[r][c]->setGraphicsEffect(nullptr);

    if (m_bannerAnim) {
        m_bannerAnim->stop();
        m_bannerAnim->deleteLater();
        m_bannerAnim = nullptr;
    }
    m_resultPanel->setGraphicsEffect(nullptr);
}

void PlayWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawPixmap(rect(), m_background, QRectF(m_background.rect()));
}
