#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "windows.h"
#include "tools.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT
    
public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    
    QStringList serialNamePort; // 串口列表
    QSerialPort *serialPort; // 串口设备
    
    uint8_t keyArray[256], keyArrayAir[256]; // 按键记录
    uint8_t keyReport[8]; // 键盘报文
    
    short keyNum;
    uint8_t func = 0;//, oldKV = 0;
    
private slots:
    void keyHandle(uint8_t keyValue, bool ifPress); // 按键处理
    void keyPressEvent(QKeyEvent *event); // 按键按下
    void keyReleaseEvent(QKeyEvent *event); // 按键抬起
    
    void on_bt_comOpen_clicked(); // 串口打开
    void on_bt_comClose_clicked(); // 串口关闭
    void on_serialReceive(); // 串口接收数据
    
    void searchPort(); // 搜索可用串口
    
    void SendKeyReport(uint8_t *report); // 发送键盘报文
    
    
    
    void on_cB_KeepOnTop_stateChanged(int arg1); // 保持最前
    
    void on_cB_HoldOn_stateChanged(int arg1); // 按键保持
    
private:
    Ui::Widget *ui;
};

#endif // WIDGET_H










