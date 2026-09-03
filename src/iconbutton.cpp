#include "iconbutton.h"

#include <QLabel>
#include <QMouseEvent>
#include <QResizeEvent>

IconButton::IconButton(const QString &normalPath, const QString &pressedPath, QWidget *parent)
    : QPushButton(parent)
{
    setCursor(Qt::PointingHandCursor);
    setFlat(true);
    setFocusPolicy(Qt::NoFocus); // 鼠标驱动的界面，避免焦点高亮干扰贴图按钮的视觉
    setStyleSheet(QStringLiteral(
        "QPushButton { background-color: transparent; border: none; padding: 0px; }"
        "QPushButton:pressed { background-color: transparent; }"));

    m_normalPixmap.load(normalPath);
    if (!pressedPath.isEmpty())
        m_pressedPixmap.load(pressedPath);

    setFixedSize(m_normalPixmap.size());
    showPixmap(m_normalPixmap);
}

void IconButton::setPixmap(const QPixmap &pm)
{
    m_normalPixmap = pm;
    showPixmap(pm);
}

void IconButton::setClickEnabled(bool on)
{
    m_clickable = on;
    setCursor(on ? Qt::PointingHandCursor : Qt::ArrowCursor);
    if (!on)
        resetVisualState();
}

void IconButton::startFloating(int amplitudePx, int periodMs)
{
    stopFloating();
    m_baseRect = geometry();

    m_floatingAnim = new QPropertyAnimation(this, "pos", this);
    m_floatingAnim->setLoopCount(-1);
    m_floatingAnim->setDuration(periodMs);
    m_floatingAnim->setEasingCurve(QEasingCurve::InOutSine);
    m_floatingAnim->setStartValue(m_baseRect.topLeft());
    m_floatingAnim->setKeyValueAt(0.5, m_baseRect.topLeft() + QPoint(0, -amplitudePx));
    m_floatingAnim->setEndValue(m_baseRect.topLeft());
    m_floatingAnim->start();
}

void IconButton::stopFloating()
{
    if (m_floatingAnim) {
        m_floatingAnim->stop();
        m_floatingAnim->deleteLater();
        m_floatingAnim = nullptr;
    }
}

void IconButton::resetVisualState()
{
    m_hovered = false;
    if (m_geometryAnim) {
        m_geometryAnim->stop();
        m_geometryAnim->deleteLater();
        m_geometryAnim = nullptr;
    }
    if (m_baseRect.isValid())
        setGeometry(m_baseRect);
    showPixmap(m_normalPixmap);
}

void IconButton::enterEvent(QEnterEvent *event)
{
    QPushButton::enterEvent(event);
    if (!m_clickable)
        return;
    m_hovered = true;
    if (m_floatingAnim) {
        // 浮动动画与悬停动画冲突，悬停期间暂停浮动。
        m_floatingAnim->pause();
        m_baseRect = geometry();
    }
    raise();
    applyHoverScale(true);
}

void IconButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    if (!m_clickable)
        return;
    m_hovered = false;
    applyHoverScale(false);
    if (m_floatingAnim)
        m_floatingAnim->resume();
}

void IconButton::mousePressEvent(QMouseEvent *event)
{
    if (!m_clickable) {
        event->ignore();
        return;
    }
    if (!m_pressedPixmap.isNull()) {
        showPixmap(m_pressedPixmap);
    } else if (m_baseRect.isValid()) {
        const QSize shrunk = m_baseRect.size() * kPressScale;
        animateTo(QRect(m_baseRect.center() - QPoint(shrunk.width() / 2, shrunk.height() / 2),
                        shrunk), 80);
    }
    QPushButton::mousePressEvent(event);
}

void IconButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_clickable) {
        event->ignore();
        return;
    }
    showPixmap(m_normalPixmap);
    if (m_hovered && m_baseRect.isValid()) {
        setGeometry(m_baseRect);
        applyHoverScale(true);
    } else if (m_baseRect.isValid()) {
        animateTo(m_baseRect, 100);
    }
    QPushButton::mouseReleaseEvent(event);
}

void IconButton::resizeEvent(QResizeEvent *event)
{
    QPushButton::resizeEvent(event);
    // 覆盖在按钮上的子 QLabel（例如关卡编号）随按钮尺寸变化。
    for (QLabel *label : findChildren<QLabel *>())
        label->resize(size());
}

void IconButton::animateTo(const QRect &target, int ms)
{
    if (!m_geometryAnim) {
        m_geometryAnim = new QPropertyAnimation(this, "geometry", this);
        m_geometryAnim->setEasingCurve(QEasingCurve::OutCubic);
    }
    m_geometryAnim->stop();
    m_geometryAnim->setDuration(ms);
    m_geometryAnim->setStartValue(geometry());
    m_geometryAnim->setEndValue(target);
    m_geometryAnim->start();
}

void IconButton::applyHoverScale(bool hovered)
{
    if (!m_baseRect.isValid()) {
        if (!m_floatingAnim)
            m_baseRect = geometry();
        if (!m_baseRect.isValid())
            return;
    }
    if (hovered) {
        const QSize grown = m_baseRect.size() * kHoverScale;
        animateTo(QRect(m_baseRect.center() - QPoint(grown.width() / 2, grown.height() / 2),
                        grown), 120);
    } else {
        animateTo(m_baseRect, 120);
    }
}

void IconButton::showPixmap(const QPixmap &pm)
{
    setIcon(pm);
    setIconSize(pm.size());
}
