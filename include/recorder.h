//
// Created by PC on 24-12-11.
//

#ifndef RECORDER_H
#define RECORDER_H

#include <QWidget>
#include <QTimer>
#include <QFile>
#include <map>

class Recorder : public QWidget {
    Q_OBJECT

public:
    Recorder();

    QString num2time(int number);

    void loadData();

    void saveData(QString path,std::map<int, int> &Map);


    void fileAction(QString path, std::map<int, int> &Map);

    QTimer *timer;

    int seconds = 0;
    int steps = 0;

    //存储关卡通关时间
    std::map<int, int> timeMap;
    //存储关卡通关步数
    std::map<int, int> stepsMap;

signals:
    void stepsUp();

public slots:
    // void onTimeout();
};


#endif //RECORDER_H
