#ifndef MAINMENUWIDGET_H
#define MAINMENUWIDGET_H

#include <QWidget>

class IconButton;
class QLabel;

// 主菜单页：使用自带标题的 MenuSceneBg 背景 + 开始按钮。
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
    IconButton *m_startButton = nullptr;
    QLabel *m_versionLabel = nullptr;
};

#endif // MAINMENUWIDGET_H
