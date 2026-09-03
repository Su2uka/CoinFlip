#include "playwidget.h"

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
constexpr int kCellPitch = 52;      // 50px 底座 + 2px 间隙
constexpr int kBoardX0 = 92;        // (390 - (3*52 + 50)) / 2
constexpr int kBoardY0 = 190;
constexpr int kNeighborDelayMs = 160;
constexpr int kFlipTotalMs = 260;   // 8 帧 × 28ms，留一点余量
constexpr int kBannerWidth = 241;
constexpr int kBannerHeight = 84;
constexpr int kBannerX = (kWindowWidth - kBannerWidth) / 2;
constexpr int kBannerY = 208;

const char *kPillButtonStyle =
    "QPushButton { background-color: #FFEDC2; color: #6B4416; border: none;"
    "  border-radius: 17px; padding: 0 14px; min-height: 32px;"
    "  font-family: 'Microsoft YaHei UI'; font-size: 11.5pt; font-weight: bold; }"
    "QPushButton:hover { background-color: #FFF6DA; }"
    "QPushButton:pressed { background-color: #EBD8A9; }"
    "QPushButton:disabled { color: #B49B74; }";
} // namespace

PlayWidget::PlayWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(kWindowWidth, kWindowHeight);
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
    // 顶部：关卡名 + 计时/步数面板。
    m_levelLabel = new QLabel(this);
    m_levelLabel->setGeometry(20, 12, 180, 44);
    QFont levelFont = m_levelLabel->font();
    levelFont.setFamilies({QString::fromUtf8("华文新魏"), QStringLiteral("KaiTi"),
                           QStringLiteral("Microsoft YaHei UI")});
    levelFont.setPointSize(19);
    levelFont.setBold(true);
    m_levelLabel->setFont(levelFont);
    m_levelLabel->setStyleSheet(QStringLiteral("color: #FFF7E6;"));
    auto *levelShadow = new QGraphicsDropShadowEffect(m_levelLabel);
    levelShadow->setBlurRadius(6);
    levelShadow->setOffset(0, 2);
    levelShadow->setColor(QColor(0, 0, 0, 150));
    m_levelLabel->setGraphicsEffect(levelShadow);

    auto *statPanel = new QWidget(this);
    statPanel->setObjectName(QStringLiteral("statPanel"));
    statPanel->setAttribute(Qt::WA_StyledBackground, true); // 让 QWidget 绘制样式表背景
    statPanel->setGeometry(kWindowWidth - 150 - 14, 12, 150, 54);
    statPanel->setStyleSheet(QStringLiteral(
        "#statPanel { background-color: rgba(255, 248, 230, 150); border-radius: 10px; }"
        "#statPanel QLabel { color: #4A2F14; font-family: 'Microsoft YaHei UI';"
        "  font-size: 11pt; font-weight: 600; }"));
    m_timeLabel = new QLabel(statPanel);
    m_stepsLabel = new QLabel(statPanel);
    auto *statLayout = new QVBoxLayout(statPanel);
    statLayout->setContentsMargins(12, 7, 12, 7);
    statLayout->setSpacing(1);
    statLayout->addWidget(m_timeLabel);
    statLayout->addWidget(m_stepsLabel);

    // 棋盘：底座 + 金币。
    for (int r = 0; r < LevelDatabase::kRows; ++r) {
        for (int c = 0; c < LevelDatabase::kCols; ++c) {
            auto *node = new QLabel(this);
            node->setPixmap(QPixmap(QStringLiteral(":/images/BoardNode(1).png")));
            node->setGeometry(kBoardX0 + c * kCellPitch, kBoardY0 + r * kCellPitch, 50, 50);
            node->setAttribute(Qt::WA_TransparentForMouseEvents);

            auto *coin = new CoinButton(r, c, false, this);
            coin->move(kBoardX0 + c * kCellPitch + 2, kBoardY0 + r * kCellPitch + 2);
            connect(coin, &QPushButton::clicked, this,
                    [this, coin] {
                        if (coin->tryActivate())
                            onCoinActivated(coin);
                    });
            m_coins[r][c] = coin;
        }
    }

    // 底部：重开 + 返回。
    auto *restartButton = new QPushButton(QStringLiteral("重玩"), this);
    restartButton->setStyleSheet(kPillButtonStyle);
    restartButton->setGeometry(16, kWindowHeight - 48, 88, 34);
    restartButton->setToolTip(QStringLiteral("重新开始本关（F5）"));
    restartButton->setCursor(Qt::PointingHandCursor);
    connect(restartButton, &QPushButton::clicked, this, [this] {
        SoundPlayer::play(SoundPlayer::Id::Tap);
        restartLevel();
    });

    auto *backButton = new IconButton(QStringLiteral(":/images/BackButton.png"),
                                      QStringLiteral(":/images/BackButtonSelected.png"), this);
    backButton->move(kWindowWidth - backButton->width() - 14, kWindowHeight - 44);
    connect(backButton, &QPushButton::clicked, this, [this] {
        SoundPlayer::play(SoundPlayer::Id::Back);
        emit backRequested();
    });

    // 通关横幅与结算面板（平时隐藏）。
    m_bannerLabel = new QLabel(this);
    m_bannerLabel->setPixmap(QPixmap(QStringLiteral(":/images/LevelCompletedDialogBg.png")));
    m_bannerLabel->setGeometry(kBannerX, -kBannerHeight - 8, kBannerWidth, kBannerHeight);
    m_bannerLabel->hide();

    m_resultPanel = new QWidget(this);
    m_resultPanel->setObjectName(QStringLiteral("winPanel"));
    m_resultPanel->setAttribute(Qt::WA_StyledBackground, true);
    m_resultPanel->setGeometry(35, 316, 320, 156);
    m_resultPanel->setStyleSheet(QStringLiteral(
        "#winPanel { background-color: rgba(46, 30, 16, 205); border-radius: 14px; }"
        "#winPanel QLabel { color: #FFF6E3; font-family: 'Microsoft YaHei UI';"
        "  font-size: 12pt; font-weight: 600; }"
        "#winPanel QPushButton { background-color: #FFEDC2; color: #6B4416; border: none;"
        "  border-radius: 17px; padding: 0 12px; min-height: 34px;"
        "  font-family: 'Microsoft YaHei UI'; font-size: 11.5pt; font-weight: bold; }"
        "#winPanel QPushButton:hover { background-color: #FFF6DA; }"
        "#winPanel QPushButton:pressed { background-color: #EBD8A9; }"));
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
    painter.drawPixmap(0, 0, QPixmap(QStringLiteral(":/images/PlayLevelSceneBg.png")));
}
