#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <QPropertyAnimation>
#include <QPushButton>

// 贴图按钮：悬停轻微放大、按下缩小（或切换按下贴图），
// 可选的待机浮动动画，用于替代教程中 zoom1/zoom2 双动画的写法。
class IconButton : public QPushButton
{
    Q_OBJECT

public:
    // normalPath 正常贴图；pressedPath 可选，为空时按下用缩小代替。
    IconButton(const QString &normalPath, const QString &pressedPath = QString(),
               QWidget *parent = nullptr);

    // 关闭后不再响应点击、不再有悬停反馈（锁定关卡用，视觉另行弱化）。
    void setClickEnabled(bool on);
    bool clickEnabled() const { return m_clickable; }

    // 运行期切换正常态贴图（解锁/锁定外观切换用）。
    void setPixmap(const QPixmap &pm);

    // 待机上下浮动动画（用于主菜单的开始按钮）。
    void startFloating(int amplitudePx = 6, int periodMs = 1800);
    void stopFloating();

    // 立即回到静止状态（页面切换回来时调用，避免残留缩放状态）。
    void resetVisualState();

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void animateTo(const QRect &target, int ms);
    void applyHoverScale(bool hovered);
    void showPixmap(const QPixmap &pm);

    QPixmap m_normalPixmap;
    QPixmap m_pressedPixmap;
    bool m_clickable = true;
    bool m_hovered = false;
    QRect m_baseRect;
    QPropertyAnimation *m_geometryAnim = nullptr;
    QPropertyAnimation *m_floatingAnim = nullptr;

    static constexpr qreal kHoverScale = 1.06;
    static constexpr qreal kPressScale = 0.93;
};

#endif // ICONBUTTON_H
