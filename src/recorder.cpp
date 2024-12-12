//
// Created by PC on 24-12-11.
//

// You may need to build the project (run Qt uic code generator) to get "ui_Recorder.h" resolved

#include "recorder.h"


Recorder::Recorder() {
    // 创建计时器
    timer = new QTimer(this); // 创建计时器对象，设置父对象为 this

    // 设定计时器时间间隔为1000ms (1秒)
    timer->setInterval(1000); // 设置时间间隔为 1 秒

    //加载数据
    loadData();
}


QString Recorder::num2time(int number) {
    // 计算小时和分钟
    int minutes = number / 60;
    int seconds = number % 60;

    // 格式化时间为 "mm:ss"
    return QString("%1:%2")
            .arg(minutes, 2, 10, QChar('0')) // 补零
            .arg(seconds, 2, 10, QChar('0')); // 补零
}

void Recorder::loadData() {
    QString timeData = "t.dat";
    QString stepsData = "s.dat";
    fileAction(timeData, this->timeMap);
    fileAction(stepsData, this->stepsMap);
}

void Recorder::saveData(QString path, std::map<int, int> &Map) {
    QFile file(path);
    std::map<int, int> *mapPtr = &Map;

    //写入
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file); // 创建数据流
        out.setVersion(QDataStream::Qt_5_15); // 设置数据流版本（保持兼容性）

        // 将 Map 的大小写入文件（方便读取时知道有多少数据）
        out << static_cast<quint32>(mapPtr->size());

        // 写入的每个键值对
        for (const auto &pair: *mapPtr) {
            out << pair.first << pair.second;
        }

        file.close(); // 关闭文件
        qDebug() << QString("数据已写入文件:%1").arg(path);
    }
}


void Recorder::fileAction(QString path, std::map<int, int> &Map) {
    QFile file(path);
    std::map<int, int> *mapPtr = &Map;
    if (file.exists()) {
        //文件存在
        qDebug() << "文件已存在";
        if (file.open(QIODevice::ReadOnly)) {
            QDataStream in(&file);
            in.setVersion(QDataStream::Qt_5_15);

            quint32 size;
            in >> size; // 先读取 map 的大小

            // this->timeMap.clear(); // 清空当前 map
            mapPtr->clear();
            for (quint32 i = 0; i < size; ++i) {
                int key, value;
                in >> key >> value; // 读取键值对
                // this->timeMap[key] = value;
                mapPtr->insert(std::make_pair(key, value));
            }

            // for (int i = 1; i <= 20; ++i) {
            //     qDebug() << QString("%1  %2").arg(i).arg(mapPtr->at(i));
            // }

            file.close();
            qDebug() << QString("%1数据已从文件加载。").arg(path);
        }
    } else {
        //文件不存在
        qDebug() << QString("%1文件不存在").arg(path);
        // 尝试创建文件
        if (file.open(QIODevice::WriteOnly)) {
            qDebug() << QString("文件创建成功:%1").arg(path);
            for (int i = 1; i <= 20; ++i) {
                mapPtr->insert(std::make_pair(i, 2147483647));
            }
            file.close(); // 关闭文件
            //写入
            if (file.open(QIODevice::WriteOnly)) {
                QDataStream out(&file); // 创建数据流
                out.setVersion(QDataStream::Qt_5_15); // 设置数据流版本（保持兼容性）

                // 将 Map 的大小写入文件（方便读取时知道有多少数据）
                out << static_cast<quint32>(mapPtr->size());

                // 写入 Map 的每个键值对
                for (const auto &pair: *mapPtr) {
                    out << pair.first << pair.second;
                }

                file.close(); // 关闭文件
                qDebug() << QString("数据已写入文件:%1").arg(path);
            }
        } else {
            qDebug() << "文件创建失败：";
        }
    }
}
