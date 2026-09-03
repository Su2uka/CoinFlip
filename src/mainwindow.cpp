#include "mainwindow.h"

#include <QGraphicsOpacityEffect>
#include <QIcon>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "chooselevelwidget.h"
#include "mainmenuwidget.h"
#include "playwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("翻金币 · Mystic CoinFlip"));
    setWindowIcon(QIcon(QStringLiteral(":/images/Coin0001.png")));
    setFixedSize(390, 570);
    // 页面淡入淡出过渡时的底色（用 palette 而非全局样式表，避免被子控件继承）。
    QPalette windowPalette = palette();
    windowPalette.setColor(QPalette::Window, QColor(0x0C, 0x10, 0x22));
    setPalette(windowPalette);
    setAutoFillBackground(true);

    m_stack = new QStackedWidget(this);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_stack);

    m_menu = new MainMenuWidget;
    m_select = new ChooseLevelWidget;
    m_play = new PlayWidget;
    m_stack->addWidget(m_menu);    // 0 主菜单
    m_stack->addWidget(m_select);  // 1 选关
    m_stack->addWidget(m_play);    // 2 游戏

    connect(m_menu, &MainMenuWidget::startRequested, this, [this] { switchTo(1); });
    connect(m_select, &ChooseLevelWidget::backRequested, this, [this] { switchTo(0); });
    connect(m_select, &ChooseLevelWidget::levelSelected, this, [this](int level) {
        m_play->startLevel(level);
        switchTo(2);
    });
    connect(m_play, &PlayWidget::backRequested, this, [this] { switchTo(1); });
}

void MainWindow::switchTo(int index)
{
    if (m_transitioning || index == m_stack->currentIndex())
        return;
    m_transitioning = true;

    QWidget *oldPage = m_stack->currentWidget();
    QWidget *newPage = m_stack->widget(index);

    // 先淡出旧页，再淡入新页，避免生硬切换。
    auto *outEffect = new QGraphicsOpacityEffect(oldPage);
    outEffect->setOpacity(1.0);
    oldPage->setGraphicsEffect(outEffect);
    auto *outAnim = new QPropertyAnimation(outEffect, "opacity", outEffect);
    outAnim->setDuration(150);
    outAnim->setStartValue(1.0);
    outAnim->setEndValue(0.0);
    outAnim->setEasingCurve(QEasingCurve::InQuad);
    connect(outAnim, &QPropertyAnimation::finished, this,
            [this, oldPage, newPage]() {
        m_stack->setCurrentWidget(newPage);

        auto *inEffect = new QGraphicsOpacityEffect(newPage);
        inEffect->setOpacity(0.0);
        newPage->setGraphicsEffect(inEffect);
        auto *inAnim = new QPropertyAnimation(inEffect, "opacity", inEffect);
        inAnim->setDuration(170);
        inAnim->setStartValue(0.0);
        inAnim->setEndValue(1.0);
        inAnim->setEasingCurve(QEasingCurve::OutQuad);
        connect(inAnim, &QPropertyAnimation::finished, this,
                [this, oldPage, newPage]() {
            newPage->setGraphicsEffect(nullptr);
            oldPage->setGraphicsEffect(nullptr);
            m_transitioning = false;
        }, Qt::QueuedConnection);
        inAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }, Qt::QueuedConnection);
    outAnim->start(QAbstractAnimation::DeleteWhenStopped);
}
