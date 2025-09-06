#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "windows.h"
#include "tools.h"
#include "hidapi.h"

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
    
    hid_device *hidDev = NULL; // HID设备
    
    uint8_t *keyState = NULL, keyStateLen = 16;
    uint8_t keyArray[256], keyArrayAir[256], keyArrayAirOld[256]; // 按键记录
    uint8_t keyReport[8]; // 键盘报文
    
    short keyNum;
    uint8_t func = 0;//, oldKV = 0;
    
private:
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *showAction;
    QAction *quitAction;
    
protected:
    void closeEvent(QCloseEvent *event) override; // 关闭事件
    
private slots:
    void keyHandle(uint8_t keyValue, bool ifPress); // 按键处理
    void keyPressEvent(QKeyEvent *event); // 按键按下
    void keyReleaseEvent(QKeyEvent *event); // 按键抬起
    
    void on_bt_comOpen_clicked(); // 串口打开
    void on_bt_comClose_clicked(); // 串口关闭
    void on_serialReceive(); // 串口接收数据
    
    void searchPort(); // 搜索可用串口
    
    void on_bt_hidOpen_clicked(); // HID打开
    void on_bt_hidClose_clicked(); // HID关闭
    
    uint8_t searchHid(); // 搜索可用HID
    
    void SendKeyReport(uint8_t *report); // 发送键盘报文
    void SendKeyState(void); // 向系统发送按键状态
    
    void ComUnpack(uint8_t *buf, int len); // 串口解包
    
    void KeyUnpack(uint8_t *buf, int len); // 按键解包
    
    void on_cB_KeepOnTop_stateChanged(int arg1); // 保持最前
    
    void on_cB_HoldOn_stateChanged(int arg1); // 按键保持
    
private:
    Ui::Widget *ui;
};

#endif // WIDGET_H










