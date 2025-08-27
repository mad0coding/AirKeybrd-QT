#include "widget.h"
#include "ui_widget.h"
#include <QMessageBox>
#include <QKeyEvent>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    
    
    trayIcon = new QSystemTrayIcon(this); // 托盘图标
    trayMenu = new QMenu(this); // 托盘菜单
    
    showAction = new QAction("Show", this); // 托盘菜单按钮
    quitAction = new QAction("Quit", this);
    
    trayMenu->addAction(showAction); // 按钮加入托盘菜单
    trayMenu->addSeparator(); // 加分隔线
    trayMenu->addAction(quitAction);
    
    qApp->setWindowIcon(QIcon(":/AirButton.ico")); // 测试代码!!!
//    trayIcon->setIcon(qApp->windowIcon());
    trayIcon->setIcon(QIcon(":/AirButton.ico")); // 必须加入图标
    trayIcon->setToolTip("Keyboard-COM"); // 鼠标悬停图标时显示文本

    trayIcon->setContextMenu(trayMenu); // 托盘菜单设置到托盘图标
    trayIcon->show();

    // 托盘菜单按钮信号槽
    connect(showAction, &QAction::triggered, this, &QWidget::showNormal);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    // 双击托盘图标也能恢复窗口
    connect(trayIcon, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason){
        if (reason == QSystemTrayIcon::DoubleClick) {
            this->showNormal();
            this->raise();
            this->activateWindow();
        }
    });
    
    
    serialPort = new QSerialPort(this); // 创建串口设备
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(on_serialReceive())); // 连接串口接收信号
    searchPort(); // 搜索可用串口
    
    keyState = new uint8_t[keyStateLen]; // 创建按键状态数组
    
    keyNum = 0;
    memset(keyArrayAirOld, 0, sizeof(keyArrayAirOld)); // 清空按键记录
    memset(keyArrayAir, 0, sizeof(keyArrayAir)); // 清空按键记录
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

void Widget::closeEvent(QCloseEvent *event)
{
    if(func & 0x01) return; // 按住Ctrl则直接关闭
    if(trayIcon->isVisible()){
        trayIcon->show();
        event->ignore(); // 阻止真正退出
        hide(); // 隐藏窗口
//        trayIcon->showMessage("提示", "程序已最小化到托盘");
    }
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
    
//    static int tmp = 0; // 测试代码!!!
//    if(!tmp) SendInputKey(kv_ctrl, true);
//    else SendInputKey(kv_ctrl, false);
//    tmp = !tmp;
}

void Widget::on_serialReceive() // 串口接收数据
{
//    QString buf;
//    buf = QString(serialPort->readAll()); // 暂不做任何处理
    QByteArray BAbuf = serialPort->readAll(); // 数据读入缓冲区
    int len = BAbuf.length(); // get length
    uint8_t *buf = (uint8_t*)BAbuf.data(); // 提取数据
    
    if(ui->cbox_mode->currentIndex() == 2){ // 输入模式
//        printf("Len:%d, Data:", len);
//        for(int i = 0; i < len; i++) printf("%02X,", buf[i]);
        ComUnpack(buf, len); // 串口解包
    }
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
    
    if(ui->cbox_mode->currentIndex() != 1) return; // 不输出直接返回
    
    if(1){ // COM
        if(serialPort->isOpen()) serialPort->write(txBuf);
    }
    else if(0){ // HID
        
    }
}

void Widget::SendKeyState(void) // 向系统发送按键状态
{
    if(ui->cbox_mode->currentIndex() != 2) return; // 不输出直接返回
    for(int i = 255; i >= 0; i--){ // 反向扫描以先处理功能键按下 测试代码
//        if(keyArrayAir[i] && !keyArrayAirOld[i]) SendInputKey(i, 1);
        if(keyArrayAir[i] != keyArrayAirOld[i]) SendInputKey(i, keyArrayAir[i]);
    }
}

void Widget::ComUnpack(uint8_t *buf, int len) // 串口解包
{
    enum UnpackState{
        US_WAIT_H0 = 0,
        US_WAIT_H1,
        US_WAIT_TYPE,
        US_WAIT_LEN,
        US_WAIT_DATA,
        US_WAIT_SUM,
        US_WAIT_END,
    };
    static UnpackState unpackState = US_WAIT_H0;
    static uint16_t dataLen = 0, dataIdx = 0, tmpBufLen = 0;
    static uint8_t pkgType = 0, dataSum = 0;
    static uint8_t *tmpBuf = NULL;
    while(len--){
        uint8_t b = *buf++;
        switch(unpackState){
        case US_WAIT_H0:
            if(b == 0x55) unpackState = US_WAIT_H1;
            break;
        case US_WAIT_H1:
            if(b == 0xAA) unpackState = US_WAIT_TYPE;
            else unpackState = US_WAIT_H0;
            break;
        case US_WAIT_TYPE:
            pkgType = b;
            if(b == 1) unpackState = US_WAIT_LEN; // type 1
            else unpackState = US_WAIT_H0; // invalid type
            break;
        case US_WAIT_LEN:
            unpackState = US_WAIT_DATA;
            if(pkgType == 1) dataLen = (b + 7) / 8;
            dataIdx = 0;
            dataSum = 0;
            if(dataLen > tmpBufLen){ // no enough space
                tmpBufLen = dataLen;
                delete tmpBuf;
                tmpBuf = new uint8_t[tmpBufLen];
                memset(tmpBuf, 0, tmpBufLen);
            }
            break;
        case US_WAIT_DATA:
            tmpBuf[dataIdx++] = b;
            dataSum += b;
            if(dataIdx >= dataLen) unpackState = US_WAIT_SUM;
            break;
        case US_WAIT_SUM:
            if(b == dataSum) unpackState = US_WAIT_END; // sum correct
            else unpackState = US_WAIT_H0;
            break;
        case US_WAIT_END:
            if(b == 0x5A){ // all correct
//                printf("Len:%d, Data:", dataLen);
//                for(int i = 0; i < dataLen; i++) printf("%02X,", tmpBuf[i]);
                KeyUnpack(tmpBuf, dataLen); // 按键解包
            }
            unpackState = US_WAIT_H0;
            break;
        default:
            unpackState = US_WAIT_H0;
            break;
        }
    }
}

void Widget::KeyUnpack(uint8_t *buf, int len) // 按键解包
{
    for(int i = 0; i < len * 8; i++){
        keyState[i] = !!(buf[i / 8] & (1 << (i % 8)));
    }
    memcpy(keyArrayAirOld, keyArrayAir, sizeof(keyArrayAir));
    memset(keyArrayAir, 0, sizeof(keyArrayAir));
    
//#define KDM(k)      (k)
#define KDM(k)      (9 - (k))
    if(keyState[KDM(0)]){
        if(keyState[KDM(1)]) keyArrayAir[kv_F1 - '1' + '2'] = 1;
        if(keyState[KDM(3)]) keyArrayAir[kv_left] = 1;
        if(keyState[KDM(5)]) keyArrayAir[kv_right] = 1;
        if(keyState[KDM(7)]) keyArrayAir[kv_ctrl] = keyArrayAir[kv_A - 'A' + 'A'] = 1;
        if(keyState[KDM(9)]) keyArrayAir[kv_ctrl] = keyArrayAir[kv_A - 'A' + 'F'] = 1;
        
        if(keyState[KDM(2)]) keyArrayAir[kv_space] = 1;
        if(keyState[KDM(4)]) keyArrayAir[kv_shift] = keyArrayAir[kv_win] = keyArrayAir[kv_A - 'A' + 'S'] = 1;
        if(keyState[KDM(6)]) keyArrayAir[kv_esc] = 1;
        if(keyState[KDM(8)]) keyArrayAir[kv_return] = 1;
    }
    else{
        if(keyState[KDM(1)]) keyArrayAir[kv_ctrl] = 1;
        if(keyState[KDM(3)]) keyArrayAir[kv_alt] = keyArrayAir[kv_tab] = 1;
        if(keyState[KDM(5)]) keyArrayAir[kv_ctrl] = keyArrayAir[kv_A - 'A' + 'S'] = 1;
        if(keyState[KDM(7)]) keyArrayAir[kv_delete] = 1;
        if(keyState[KDM(9)]) keyArrayAir[kv_backspace] = 1;
        
        if(keyState[KDM(2)]) keyArrayAir[kv_ctrl] = keyArrayAir[kv_A - 'A' + 'Z'] = 1;
        if(keyState[KDM(4)]) keyArrayAir[kv_ctrl] = keyArrayAir[kv_A - 'A' + 'X'] = 1;
        if(keyState[KDM(6)]) keyArrayAir[kv_ctrl] = keyArrayAir[kv_A - 'A' + 'C'] = 1;
        if(keyState[KDM(8)]) keyArrayAir[kv_ctrl] = keyArrayAir[kv_A - 'A' + 'V'] = 1;
    }
    
    
#undef KDM
    
    SendKeyState(); // 向系统发送按键状态
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























