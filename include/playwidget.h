#ifndef PLAYWIDGET_H
#define PLAYWIDGET_H

#include <QPixmap>
#include <QList>
#include <QPointer>
#include <QTimer>
#include <QWidget>

class CoinButton;
class QLabel;
class QPropertyAnimation;
class QPushButton;

// 游戏页：4×4 金币棋盘 + 计时/步数 + 通关结算。
class PlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlayWidget(QWidget *parent = nullptr);

    // 进入指定关卡（重置棋盘、计时与步数）。
    void startLevel(int level);

signals:
    void backRequested();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void buildUi();
    void restartLevel();
    void onCoinActivated(CoinButton *coin);
    void finishMove();
    void onWin();
    void showResultPanel();
    void updateStats();
    void setBoardInteractive(bool on);
    void playEntranceAnimation();
    void stopPendingAnimations();

    int m_level = 1;
    int m_session = 0; // 每次 startLevel/restartLevel 递增，用于丢弃过期的定时器回调
    QPixmap m_background;
    int m_board[4][4] = {};
    CoinButton *m_coins[4][4] = {};

    bool m_busy = false;     // 翻转动画进行中
    bool m_finished = false; // 本关已通关
    int m_seconds = 0;
    int m_steps = 0;
    QTimer m_clock;

    QLabel *m_levelLabel = nullptr;
    QLabel *m_timeLabel = nullptr;
    QLabel *m_stepsLabel = nullptr;

    // 通关覆盖层（横幅 + 结算面板）
    QLabel *m_bannerLabel = nullptr;
    QPropertyAnimation *m_bannerAnim = nullptr;
    QWidget *m_resultPanel = nullptr;
    QLabel *m_resultLabel = nullptr;
    QPushButton *m_nextButton = nullptr;
    QPushButton *m_replayButton = nullptr;
    QPushButton *m_backButton = nullptr;

    // 入场动画用 QPointer 跟踪：动画对象可能已自毁，避免悬空指针。
    QList<QPointer<QPropertyAnimation>> m_entranceAnims;
};

#endif // PLAYWIDGET_H
