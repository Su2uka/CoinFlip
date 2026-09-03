#ifndef COINBUTTON_H
#define COINBUTTON_H

#include <QPixmap>
#include <QPushButton>
#include <QTimer>

// 金币按钮：8 帧翻转动画 + 点击回弹效果。
// 金币正面为第 1 帧，反面为第 8 帧，中间帧模拟翻转过程。
class CoinButton : public QPushButton
{
    Q_OBJECT

public:
    CoinButton(int row, int col, bool goldFace, QWidget *parent = nullptr);

    int row() const { return m_row; }
    int col() const { return m_col; }
    bool isGold() const { return m_gold; }
    bool isFlipping() const { return m_flipping; }

    // 玩家是否可以点击（动画期间 / 已通关时由棋盘统一控制）。
    void setInteractive(bool on) { m_interactive = on; }

    // 若当前可交互则播放回弹并返回 true（翻转逻辑由棋盘处理）。
    bool tryActivate();

    // 翻转到另一面（带帧动画，状态立即翻转）。
    void flip();

    // 立即显示某一面的静帧（重置棋盘用）。
    void setGoldFace(bool gold);

    // 点击时的回弹反馈。
    void playPop();

protected:
private:
    void setFrame(int frame);

    int m_row;
    int m_col;
    bool m_gold;
    bool m_flipping = false;
    bool m_interactive = true;
    int m_frame = 1; // 1..8
    int m_step = 0;
    QTimer m_frameTimer;
    QPixmap m_frames[8]; // 下标 0..7 对应 Coin0001..Coin0008
    QRect m_baseRect;
};

#endif // COINBUTTON_H
