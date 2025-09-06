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
    trayIcon->setToolTip("AirKeybrd"); // 鼠标悬停图标时显示文本

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
    
    if(ui->cbox_baudRate->currentText() == "4800") baudRate = QSerialPort::Baud4800;
    else if(ui->cbox_baudRate->currentText() == "9600") baudRate = QSerialPort::Baud9600;
    else if(ui->cbox_baudRate->currentText() == "19200") baudRate = QSerialPort::Baud19200;
    else if(ui->cbox_baudRate->currentText() == "38400") baudRate = QSerialPort::Baud38400;
    else if(ui->cbox_baudRate->currentText() == "115200") baudRate = QSerialPort::Baud115200;
    
    serialPort->setPortName(ui->cbox_comName->currentText());
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(QSerialPort::Data8); // 默认8位数据
    serialPort->setStopBits(QSerialPort::OneStop); // 默认1位停止位
    serialPort->setParity(QSerialPort::NoParity); // 默认无校验
    
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

void Widget::on_bt_hidOpen_clicked() // HID打开
{
    on_bt_hidClose_clicked(); // HID关闭
    uint8_t res = searchHid(); // 搜索可用HID (并打开)
    if(res == CHID_OK) ui->bt_hidOpen->setStyleSheet("color: rgb(0,200,0)"); // 打开文本变绿色
    else{
        ui->bt_hidOpen->setStyleSheet("color: red");
        hidDev = NULL;
    }
}

void Widget::on_bt_hidClose_clicked() // HID关闭
{
    hid_close(hidDev); // 关闭 无返回值
    hidDev = NULL;
    ui->bt_hidOpen->setStyleSheet("color: black");
}

uint8_t Widget::searchHid() // 搜索可用HID
{
    std::vector<hid_device_info> matchVector; // 匹配设备列表
    if(hid_init() == -1) return CHID_ERR_INIT; // HidApi未成功初始化则退出
    hid_device_info *devInfos = hid_enumerate(ui->sb_vid->value(), ui->sb_pid->value()); // 按任意VID和PID枚举设备 得到链表
    hid_device_info *devInfo = NULL; // 用来存储选中的设备信息
    
    int findDevNum = 0; // 设备计数
    for(hid_device_info *iter = devInfos; iter; iter = iter->next){ // 在VID和PID符合的设备中进一步查找
        if(iter->usage_page >= ui->sb_usepageMin->value() 
        && iter->usage_page <= ui->sb_usepageMax->value()){ // 匹配UsagePage 实际主要是为了匹配端点
            findDevNum++; // 设备计数
            devInfo = iter; // 选定该设备信息
            matchVector.push_back(*iter); // 加入匹配列表
        }
    }

    if(findDevNum == 0) return CHID_NO_DEV;//未找到设备则退出
    if(findDevNum > 1 || 1){ // 若有多个设备 (暂时不管设备数量)
        bool ok = true;
        QStringList items; // 序列号字符串列表
        for(uint32_t i = 0; i < matchVector.size(); i++){
            QString serialStr = "";
            serialStr += QString::asprintf("%04X", matchVector[i].vendor_id) + " ";
            serialStr += QString::asprintf("%04X", matchVector[i].product_id) + " ";
            serialStr += QString::asprintf("%04X", matchVector[i].usage_page) + " ";
            serialStr += QString::fromWCharArray(matchVector[i].product_string) + " ";
            serialStr += QString::fromWCharArray(matchVector[i].serial_number);
            items.append(serialStr); // 填入
            // 序列号合法性检查
//            bool valid;
//            uint64_t sn = serialStr.toULongLong(&valid, 16); // 序列号转数字
//            if(serialStr.length() != 12 || !valid || sn > 0xFFFFFFFFFFFFULL) ok = false; // 记录有错误序列号
        }
        QString ifSnErr = "Device Info:";
        ifSnErr += "\nVID PID UPage Product SN";
//        if(!ok) ifSnErr += "\n(疑似有错误序列号 建议取消后重试)";
        QString item = QInputDialog::getItem(NULL, "Select Device", ifSnErr, items, 0, false, &ok);

        if(!ok) return CHID_MULTI_DEV; // 退出
        else{ // 选择了一个
            for(uint32_t i = 0; i < matchVector.size(); i++){ // 查找选中了哪个
                if(item == items[i]) devInfo = &matchVector[i]; // 选定该设备
            }
        }
    }
    
//    bool openRes = HID_API_Open(devInfo->path); // 打开设备
    hidDev = hid_open_path(devInfo->path); // 打开设备 返回句柄
    
    hid_free_enumeration(devInfos); // 释放链表空间
    
//    if(!openRes) return CHID_ERR_OPEN; // HID设备打开失败则退出
    if(!hidDev) return CHID_ERR_OPEN; // HID设备打开失败则退出
    return CHID_OK;
}

void Widget::SendKeyReport(uint8_t *report) // 发送键盘报文
{
    QByteArray txBuf = "";
//    txBuf.append(0x55); // 帧头
//    txBuf.append(0xE1); // 类型
//    for(int i = 0; i < 8; i++) txBuf.append(report[i]);
//    txBuf.append(0xAA); // 帧尾
    
    if(ui->cbox_mode->currentIndex() != 1) return; // 不输出直接返回
    
    if(0){ // COM
        if(serialPort->isOpen()) serialPort->write(txBuf);
    }
    else if(1){ // HID
        if(hidDev){
            uint8_t tmpBuf[1 + 1 + 8] = {9, 0x55, 0xE1};
            tmpBuf[3] = report[0]; // func键
            for(int i = 0; i < 6; i++) tmpBuf[i + 4] = report[i + 2]; // 拷贝按键
            int res = hid_write(hidDev, tmpBuf, sizeof(tmpBuf)); // 写入
//            return res; // 返回写入的字节数
        }
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























