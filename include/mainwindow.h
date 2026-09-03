#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

class ChooseLevelWidget;
class MainMenuWidget;
class QStackedWidget;
class PlayWidget;

// 单窗口外壳：主菜单 / 选关 / 游戏三个页面共享一个窗口，切换时带淡入淡出过渡。
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void switchTo(int index);

    QStackedWidget *m_stack = nullptr;
    MainMenuWidget *m_menu = nullptr;
    ChooseLevelWidget *m_select = nullptr;
    PlayWidget *m_play = nullptr;
    bool m_transitioning = false;
};

#endif // MAINWINDOW_H
