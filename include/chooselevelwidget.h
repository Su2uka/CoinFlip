#ifndef CHOOSELEVELWIDGET_H
#define CHOOSELEVELWIDGET_H

#include <QWidget>

class IconButton;
class QLabel;

// 选关页：4×5 关卡按钮网格，锁定的关卡半透明置灰，悬停显示最佳成绩。
class ChooseLevelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChooseLevelWidget(QWidget *parent = nullptr);

signals:
    void backRequested();
    void levelSelected(int level);

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void buildUi();
    void refresh();

    IconButton *m_levelButtons[20] = {};
    QLabel *m_numberLabels[20] = {};
    QLabel *m_unlockedCountLabel = nullptr;
    bool m_navigating = false;
};

#endif // CHOOSELEVELWIDGET_H
