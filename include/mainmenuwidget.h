#ifndef MAINMENUWIDGET_H
#define MAINMENUWIDGET_H

#include <QPixmap>
#include <QWidget>

class IconButton;
class QLabel;

// 主菜单页：深空靛蓝背景 + 文字标题 + 金色开始按钮。
class MainMenuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainMenuWidget(QWidget *parent = nullptr);

signals:
    void startRequested();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap m_background;
    IconButton *m_startButton = nullptr;
};

#endif // MAINMENUWIDGET_H
