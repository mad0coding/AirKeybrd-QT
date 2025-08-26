#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QKeyEvent>


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    
    serialPort = new QSerialPort(this); // 创建串口设备
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(on_serialReceive())); // 连接串口接收信号
    searchPort(); // 搜索可用串口
    
    keyNum = 0;
    memset(keyArray, 0, sizeof(keyArray)); // 清空按键记录
    memset(keyReport, 0, sizeof(keyReport)); // 清空键盘报文
    
    ui->lb_ctrl->setStyleSheet("color: grey");
    ui->lb_shift->setStyleSheet("color: grey");
    ui->lb_alt->setStyleSheet("color: grey");
    ui->lb_win->setStyleSheet("color: grey");
}

Widget::~Widget()
{
    delete ui;
}

void Widget::keyHandle(uint8_t keyValue, bool ifPress) // 按键处理
{
    key_to_report(keyReport, keyArray); // 按键数值转键盘报文
    keyReport[0] = func; // 这里使用受按键保持控制的func
    SendKeyReport(keyReport);
}

void Widget::keyPressEvent(QKeyEvent *event) // 按键按下
{
    if(!isActiveWindow()){ // 若当前为非活动窗口则可能发生切屏
        ui->lb_ctrl->setStyleSheet("color: grey");
        ui->lb_shift->setStyleSheet("color: grey");
        ui->lb_alt->setStyleSheet("color: grey");
        ui->lb_win->setStyleSheet("color: grey");
        
        func = 0;
        keyNum = 0;
        memset(keyArray, 0, sizeof(keyArray)); // 清空按键记录
        keyHandle(0, 0);
        return; // 舍弃键值直接返回
    }
    if(event->isAutoRepeat()) return;//若为自动重复触发或正在发送数据则返回
    int key1 = event->key();//读取第一种键值
    int key2 = event->nativeVirtualKey();//读取第二种键值
    
    uint8_t keyValue = key_to_USB(key1, key2); // 映射到USB键值
    
    if(keyValue >= 89 && keyValue <= 98+1 && ui->cB_MapNUM->isChecked()){ // NUM键映射
        if(keyValue <= 98) keyValue -= 59; // 映射到普通数字键
        else keyValue = 55; // 映射成主键盘的点.
    }
    
    keyArray[keyValue] = 1; // 记录按下
    
    if(keyValue >= 249 && keyValue <= 252) func |= 0x01 << (keyValue - 249);
    else{
        ui->lb_keyValue->setText("KeyValue:" + QString::number(keyValue));
        ui->lb_keyName->setText("KeyName:" + USB_to_str(keyValue, func & 0x02));
        keyNum++;
    }
    
    if(keyValue == 249) ui->lb_ctrl->setStyleSheet("color: black");
    else if(keyValue == 250) ui->lb_shift->setStyleSheet("color: black");
    else if(keyValue == 251) ui->lb_alt->setStyleSheet("color: black");
    else if(keyValue == 252) ui->lb_win->setStyleSheet("color: black");
    
    keyHandle(keyValue, 1);
}

void Widget::keyReleaseEvent(QKeyEvent *event) // 按键抬起
{
    if(event->isAutoRepeat()) return;//若为自动重复触发或正在发送数据则返回
    int key1 = event->key();//读取第一种键值
    int key2 = event->nativeVirtualKey();//读取第二种键值
    
    uint8_t keyValue = key_to_USB(key1, key2);//映射到USB键值
    
    if(keyValue >= 89 && keyValue <= 98+1 && ui->cB_MapNUM->isChecked()){ // NUM键映射
        if(keyValue <= 98) keyValue -= 59; // 映射到普通数字键
        else keyValue = 55; // 映射成主键盘的点.
    }
    
    keyArray[keyValue] = 0; // 记录抬起
    
    if(keyValue >= 249 && keyValue <= 252){
        if(!ui->cB_HoldOn->isChecked()) func &= ~(0x01 << (keyValue - 249));
    }
    else if(keyNum) keyNum--;
    
    if(keyNum == 0){
        ui->lb_keyValue->setText("KeyValue:None");
        ui->lb_keyName->setText("KeyName:None");
    }
    
    if(!ui->cB_HoldOn->isChecked()){
        if(keyValue == 249) ui->lb_ctrl->setStyleSheet("color: grey");
        else if(keyValue == 250) ui->lb_shift->setStyleSheet("color: grey");
        else if(keyValue == 251) ui->lb_alt->setStyleSheet("color: grey");
        else if(keyValue == 252) ui->lb_win->setStyleSheet("color: grey");
    }
    
    keyHandle(keyValue, 0);
}

void Widget::on_bt_comOpen_clicked() // 串口打开
{
    serialPort->close();
    
    QSerialPort::BaudRate baudRate; // 波特率
    QSerialPort::DataBits dataBits; // 数据位
    QSerialPort::StopBits stopBits; // 停止位
    QSerialPort::Parity checkBits; // 校验位
    
    if(ui->cbox_baudRate->currentText() == "4800") baudRate = QSerialPort::Baud4800;
    else if(ui->cbox_baudRate->currentText() == "9600") baudRate = QSerialPort::Baud9600;
    else if(ui->cbox_baudRate->currentText() == "19200") baudRate = QSerialPort::Baud19200;
    else if(ui->cbox_baudRate->currentText() == "38400") baudRate = QSerialPort::Baud38400;
    else if(ui->cbox_baudRate->currentText() == "115200") baudRate = QSerialPort::Baud115200;
    
    dataBits = QSerialPort::Data8; // 默认8位数据
    stopBits = QSerialPort::OneStop; // 默认1位停止位
    checkBits = QSerialPort::NoParity; // 默认无校验
    
    serialPort->setPortName(ui->cbox_comName->currentText());
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(dataBits);
    serialPort->setStopBits(stopBits);
    serialPort->setParity(checkBits);
    
    if(serialPort->open(QIODevice::ReadWrite) == true){
        ui->bt_comOpen->setStyleSheet("color: rgb(0,200,0)"); // 打开文本变绿色
        //QMessageBox::information(this,"提示","成功");
    }
    else{
        //QMessageBox::critical(this,"提示","失败");
        ui->bt_comOpen->setStyleSheet("color: red");
        searchPort();//搜索可用串口
    }
}

void Widget::on_bt_comClose_clicked() // 串口关闭
{
    serialPort->close();
    ui->bt_comOpen->setStyleSheet("color: black");
    searchPort();
}

void Widget::on_serialReceive() // 串口接收数据
{
    QString buf;
    buf = QString(serialPort->readAll()); // 暂不做任何处理
}

void Widget::searchPort() // 搜索可用串口
{
    serialNamePort.clear(); // 清空列表
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){ // 搜索可用串口
        serialNamePort << info.portName(); // 填入列表
    }
    ui->cbox_comName->clear(); // 清空控件
    ui->cbox_comName->addItems(serialNamePort); // 列表填入控件
}

void Widget::SendKeyReport(uint8_t *report) // 发送键盘报文
{
    QByteArray txBuf = "";
    txBuf.append(0xFF); // 帧头
    for(int i = 0; i < 8; i++) txBuf.append(report[i]);
    txBuf.append(0xFE); // 帧尾
    
    if(!ui->cB_Output->isChecked()) return; // 不输出直接返回
    
    if(1){ // COM
        if(serialPort->isOpen()) serialPort->write(txBuf);
    }
    else if(0){ // HID
        
    }
}

void Widget::on_cB_KeepOnTop_stateChanged(int arg1) // 保持最前
{
    Qt::WindowFlags flags = windowFlags();
    
    if (arg1) flags |= Qt::WindowStaysOnTopHint;
    else flags &= ~Qt::WindowStaysOnTopHint;
    
    searchPort();

    hide(); // 先隐藏
    setWindowFlags(flags);
    show(); // 再显示
    // 这会造成窗口闪一下 但暂未有更简单的办法
}

void Widget::on_cB_HoldOn_stateChanged(int arg1) // 按键保持
{
    if(!arg1){
        ui->lb_ctrl->setStyleSheet("color: grey");
        ui->lb_shift->setStyleSheet("color: grey");
        ui->lb_alt->setStyleSheet("color: grey");
        ui->lb_win->setStyleSheet("color: grey");
        
        func = 0;
        keyNum = 0;
        memset(keyArray, 0, sizeof(keyArray)); // 清空按键记录
        keyHandle(0, 0);
    }
}























