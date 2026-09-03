#include "coinbutton.h"

#include <QMouseEvent>
#include <QPropertyAnimation>

namespace {
constexpr int kFrameCount = 8;
constexpr int kFrameMs = 26;
constexpr int kCoinSize = 56;
} // namespace

CoinButton::CoinButton(int row, int col, bool goldFace, QWidget *parent)
    : QPushButton(parent), m_row(row), m_col(col), m_gold(goldFace)
{
    setCursor(Qt::PointingHandCursor);
    setFlat(true);
    setFocusPolicy(Qt::NoFocus);
    setStyleSheet(QStringLiteral(
        "QPushButton { background-color: transparent; border: none; padding: 0px; }"
        "QPushButton:pressed { background-color: transparent; }"));

    for (int i = 0; i < kFrameCount; ++i)
        m_frames[i].load(QStringLiteral(":/images/coin_%1.png").arg(i + 1));

    setFixedSize(kCoinSize, kCoinSize);
    setFrame(m_gold ? 1 : kFrameCount);

    m_frameTimer.setInterval(kFrameMs);
    connect(&m_frameTimer, &QTimer::timeout, this, [this] {
        m_frame += m_step;
        if (m_frame < 1 || m_frame > kFrameCount) {
            // 走完整圈，停在目标面上。
            m_frame = qBound(1, m_frame, kFrameCount);
            m_frameTimer.stop();
            m_flipping = false;
        }
        setFrame(m_frame);
    });
}

void CoinButton::flip()
{
    if (m_flipping)
        return;
    m_gold = !m_gold;
    m_step = m_gold ? -1 : 1;
    m_flipping = true;
    m_frameTimer.start();
}

void CoinButton::setGoldFace(bool gold)
{
    m_frameTimer.stop();
    m_flipping = false;
    m_gold = gold;
    setFrame(m_gold ? 1 : kFrameCount);
}

void CoinButton::playPop()
{
    if (!m_baseRect.isValid())
        m_baseRect = geometry();
    if (m_baseRect.isNull())
        return;

    auto *anim = new QPropertyAnimation(this, "geometry", this);
    anim->setDuration(220);
    anim->setEasingCurve(QEasingCurve::OutQuad);
    const QSize grown = m_baseRect.size() * 1.14;
    const QRect peak(m_baseRect.center() - QPoint(grown.width() / 2, grown.height() / 2), grown);
    anim->setKeyValueAt(0.0, m_baseRect);
    anim->setKeyValueAt(0.5, peak);
    anim->setKeyValueAt(1.0, m_baseRect);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

bool CoinButton::tryActivate()
{
    if (!m_interactive || m_flipping)
        return false;
    playPop();
    return true;
}

void CoinButton::setFrame(int frame)
{
    m_frame = qBound(1, frame, kFrameCount);
    setIcon(m_frames[m_frame - 1]);
    // 各帧素材尺寸略有差异，统一缩放到按钮尺寸避免动画抖动。
    setIconSize(size());
}
