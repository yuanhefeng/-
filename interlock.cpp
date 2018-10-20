#include "interlock.h"
#include "ui_interlock.h"

int InterLock::Chance = 0;
int InterLock::First = 0;
QList<int> InterLock::MessageList;


//本项目目前所使用的通信协议以及用到的各种状态设置依赖于上一个项目的通信协议
InterLock::InterLock(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InterLock)
{
    ui->setupUi(this);
    //隐藏程序主窗口
    this->hide();
    //新建QSystemTrayIcon对象，创建托盘项
    mSysTrayIcon = new QSystemTrayIcon(this);
    //新建托盘要显示的icon，设置托盘项图标
    QIcon icon = QIcon("./InterLock/app.ico");
    //将icon设到QSystemTrayIcon对象中，添加菜单项
    mSysTrayIcon->setIcon(icon);
    //当鼠标移动到托盘上的图标时，会显示此处设置的内容
    mSysTrayIcon->setToolTip(QObject::trUtf8("测试系统托盘图标"));
    //在系统托盘显示此对象
    mSysTrayIcon->show();
    udpSocket = new QUdpSocket(this);

    if(udpSocket->bind(QHostAddress("127.0.0.1"),4001))
    {
        qDebug()<<"XXXXXXX";
    }

  /*  X_Direction = 0xff;
    XF_Direction = 0xff;
    XD_Direction = 0xff;
    S_Direction = 0xff;
    SF_Direction = 0xff;*/

     beginSignalID = 0xff;
     endSignalID = 0xff;

    direction = 0xff;
    udpSocket_receive = new QUdpSocket(this);
    udpSocket_receive->bind(QHostAddress("127.0.0.1"),4002);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(TimerTicked()));
    connect(udpSocket_receive,SIGNAL(readyRead()),this,SLOT(readPendingDatagrams()));
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("interlocking");
    db.setUserName("root");
    db.setPassword("root");

    if (!db.open()) {
        qDebug()<<"数据库打开失败";
    }
    if(Chance == 0)
    {
        SectionDataCache();
        SignalDataCache();
        SwitchDataCache();
        //RuleDataCache();
        Chance ++;
    }
    timer->start(1000);
}

InterLock::~InterLock()
{
    delete ui;
}

//接收信号,假如修改点东西
int i=0;
void InterLock::readPendingDatagrams()
{
    while (udpSocket_receive->hasPendingDatagrams())
    {
        quint32 datagramSize = udpSocket_receive->pendingDatagramSize();
        QByteArray buf(datagramSize, 0);
        udpSocket_receive->readDatagram(buf.data(), buf.size());
        if(0x20==buf[7])
        {
            i++;
            if(i==1)
            {
                        QString sectionid;
                        QSqlQuery querySection("select * from sectioninfo");
                        while(querySection.next())
                        {
                            sectionid=querySection.value(0).toString();
                            SectionsDataMap.find(sectionid).value().LockStatus='1';
                        }
                    }
        }

        if(i>=1)
        {
//            if(0x20 == buf[7])//上电解锁
//            {
                //设置上电解锁
                qDebug() <<"ShangDian:"<<buf.toInt();
                ShangDian();
                if(0x30 == buf[7])//建立进路————————————————————————√
                {
                    InLine(buf[12],buf[17],1);
                }
                if(0x31 == buf[7])//取消进路————————————————————————√
                {
                    qDebug() <<"RemoveRoute:"<< buf.toHex();
                    RemoveRoute(buf[12],1);
                }
                if(0x22 == buf[7])//信号重开
                {
                    qDebug() <<"XinHaoCK:"<<buf.toInt();
                    XinHaoCK(buf[12]);
                }
                if(0x32 == buf[7])//引导进路————————————————————————√
                {
                    qDebug() <<"YinDaoJL:"<<buf.toInt();
                    InLine(buf[12],0,2);
                    //YinDaoJL(buf[12]);
                }
                if(0x33 == buf[7])//引导总锁————————————————————————√
                {
                    qDebug() <<"YinDaoZS:"<<buf.toInt();
                    YinDaoZS(buf[12]);
                }
                if(0x34 == buf[7])//总人解————————————————————————√
                {
                    qDebug() <<"RenGong:"<<buf.toInt();
                    RemoveRoute(buf[12],2);
                    //RenGong(buf[12]);
                }
                if(0x50 == buf[7])//道岔总定反，单解锁————————————————————————√
                {
                    qDebug() <<"ZongDingFanDanJieSuo:"<<buf.toInt();
                    ZongDingFanDanJieSuo(buf[12],buf[17]);
                }
                if(0x25 == buf[7])//封锁按钮————————————————————————√
                {
                    qDebug() <<"FengSuo:"<<buf.toInt();
                    FengSuo(buf[12],buf[17]);
                }

                if(0x54 == buf[7])//道岔失表
                {
                    qDebug() <<"ShiBiao:"<<buf.toInt();
                    SwitchLoss(buf[12]);
                }
                if(0x55 == buf[7])//道岔取消失表
                {
                    qDebug() <<"QuXiaoShiBiao:"<<buf.toInt();
                    SwitchNoLoss(buf[12]);
                }

                if(0x64 == buf[7])//分路不良
                {
                    qDebug() <<"FenLu:"<<buf.toInt();
                    FenLu(buf[12]);
                }
                if(0x63 == buf[7])//区解锁
                {
                    qDebug() <<"QuJS:"<<buf.toInt();
                    QuGJ(buf[12]);
                }
                /*if( == buf[])//调车进路
                {
                    qDebug() <<"DiaoChe:"<<buf.toInt();
                    DiaoChe();
                }*/
                if(0x24 == buf[7])//故障设置
                {
                    qDebug() <<"GuZhang:"<< buf.toInt();
                    GuZhang(buf[12],buf[17]);
                }
                if(0x35 == buf[7])//闭塞操作
                {
                    qDebug() <<"BiSe:"<<buf.toInt();
                    BiSe(buf[12]);
                }
                if(0x26==buf[7])//灯丝断丝
                {
                    DSDS(buf[12],buf[17]);
                }
                if(0x60 == buf[7])//占用
                {
                    qDebug() <<"ZhanYong:"<<buf.toInt();
                    ZhanYong(buf[12]);
                }
                if(0x40 == buf[7])//模拟行车
                {
                    qDebug() <<"UnlockState:"<<buf.toInt();
                    UnlockState(buf[12],buf[17]);
                }
                /*if( == buf[]) //信号机
                {
                    qDebug() <<"SetupSignal:"<< buf.toInt();
                    SetupSignal(buf[],buf[]);
                }
                if( == buf[11])
                {
                    qDebug() <<"IntervalEncapsalutation:"<< buf.toInt();
                    InterEncapsalutation(buf[],buf[],buf[],buf[],buf[]);
                }*/
            //}
        }
        else return;
    }
}

//【01操作·进路设置+引导进路】
void InterLock::InLine(byte beginSignalID,byte endSignalID,int type)
{
    QString beginSignalName = QString();//起点信号机名
    QString endSignalName = QString();//终点信号机名字
    QString SectionNames;//所有区段名字的连接字符串
    QString SwitchNames;//所有道岔名字的连接字符串
    QString lineruleid;//进路路线ID
    QString First;//选取的路线序号（有共同起点终点信号机的路线）
    QString sql;//sql语句
    QString Type;//进路类型：侧线/正线/通路/调车
    QList<QString> RuleList;//需要向所有进路集合增加的本次进路（包括起点信号机、经过地所有区段、道岔、进路类型）

    //起点信号机ID取得起点信号机名字
    QString selectbeginsignalid = QString("select * from signalinfo WHERE signalinfo.SignalID = %1").arg(beginSignalID);
    QSqlQuery querybeginsignalid(selectbeginsignalid);
    while(querybeginsignalid.next())
    {
        beginSignalName = querybeginsignalid.value(3).toString();
    }

    //获取该条进路需要经过的所有道岔和区段
    if(type ==1){     //正常进路
        //终点信号机ID取得终点信号机名字
        QString selectendsignalid = QString("select * from signalinfo WHERE signalinfo.SignalID = %1").arg(endSignalID);
        QSqlQuery queryendsignalid(selectendsignalid);
        while(queryendsignalid.next())
        {
            endSignalName = queryendsignalid.value(3).toString();
        }
        //进路中所有的道岔
        sql = QString("select * from interlockinginfo WHERE BeginSignal = \"%1\" AND EndSignal = \"%2\"").arg(beginSignalName).arg(endSignalName);
    }
    else if(type ==2){   //引导进路
        QString LineName = beginSignalName + "引导";
        //进路中所有的道岔
        sql = QString("select * from interlockinginfo WHERE BeginSignal = \"%1\" AND RouteName = \"%2\"").arg(beginSignalName).arg(LineName);
    }
    QSqlQuery queryinterlocking(sql);

    //进路条件判断
    while(queryinterlocking.next())
    {
        //道岔条件判断
        SwitchNames = queryinterlocking.value(7).toString();
        //拆分道岔名字和定反位
        QByteArray num = switchesStrextract(SwitchNames);
        int switchname;
        for(int k=0;k<num.count();k+=2){
            switchname = num[k];//进路的区段名字
            int switchid;//对应的区段ID
            QString selectswitch = QString("SELECT * FROM switch WHERE SwitchName = %1").arg(switchname);
            QSqlQuery queryswitch(selectswitch);
            while(queryswitch.next())
            {
                switchid = queryswitch.value(0).toInt();
                //----------------【判断条件7】道岔被锁闭，且定反位与进路需求不一致----------------
                //如果进路要求道岔定位，但是道岔是反位，且道岔被锁闭，则进路无效
                if((num[k+1]+0) == 0 && SwitchDataMap.find(switchid).value().switchPos == 0x01 && SwitchDataMap.find(switchid).value().switchLock == 0x01){
                    MessageList.append(2);
                    MessageList.append(switchid);
                    MessageList.append(130);
                    return;
                }
                //如果进路要求道岔反位，但是道岔是定位，且道岔被锁闭，则进路无效
                if((num[k+1]+0) == 2 && SwitchDataMap.find(switchid).value().switchPos == 0x00 && SwitchDataMap.find(switchid).value().switchLock == 0x01){
                    MessageList.append(2);
                    MessageList.append(switchid);
                    MessageList.append(130);
                    return;
                }
                //----------------【判断条件8】道岔失表----------------：
                if(SwitchDataMap.find(switchid).value().SwitchLoss == 0x01){
                    MessageList.append(2);
                    MessageList.append(switchid);
                    MessageList.append(131);
                    return;
                }
                //----------------【判断条件9】道岔封锁----------------：
                if(SwitchDataMap.find(switchid).value().blockStatus == 0x01){
                    MessageList.append(2);
                    MessageList.append(switchid);
                    MessageList.append(132);
                    return;
                }
            }
        }

        //区段条件判断
        SectionNames = queryinterlocking.value(5).toString();
        //拆分区段名字
        QStringList sectionsplit = SectionNames.split(",");
        foreach (QString sectionname, sectionsplit) {
            QString sectionid;
            QString selectsection = QString("SELECT * FROM sectioninfo WHERE SectionName = \"%1\"").arg(sectionname);
            QSqlQuery querysection(selectsection);
            while(querysection.next())
            {
                sectionid = querysection.value(0).toString();
                //----------------【判断条件1】区段锁闭----------------
                if(SectionsDataMap.find(sectionid).value().LockStatus == 0x01){
                    MessageList.append(1);
                    MessageList.append(sectionid.toInt());
                    MessageList.append(101);
                    return;
                }
                //----------------【判断条件2】区段封锁----------------：
                if(SectionsDataMap.find(sectionid).value().blockStatus == 0x01){
                    MessageList.append(1);
                    MessageList.append(sectionid.toInt());
                    MessageList.append(102);
                    return;
                }
                //----------------【判断条件3】区段分路不良----------------：
                if(SectionsDataMap.find(sectionid).value().PoorStatus == 0x01){
                    MessageList.append(1);
                    MessageList.append(sectionid.toInt());
                    MessageList.append(103);
                    return;
                }
                //----------------【判断条件4】区段占用----------------：
                if(SectionsDataMap.find(sectionid).value().sectionStatus == 0x01){
                    MessageList.append(1);
                    MessageList.append(sectionid.toInt());
                    MessageList.append(104);
                    return;
                }
                //----------------【判断条件5】区段白光带故障----------------：
                if(SectionsDataMap.find(sectionid).value().SectionWhiteObstacle == 0x01){
                    MessageList.append(1);
                    MessageList.append(sectionid.toInt());
                    MessageList.append(105);
                    return;
                }
                //----------------【判断条件6】区段红光带故障----------------：
                if(SectionsDataMap.find(sectionid).value().SectionRedObstacle == 0x01){
                    MessageList.append(1);
                    MessageList.append(sectionid.toInt());
                    MessageList.append(106);
                    return;
                }
            }
        }
    }


    QSqlQuery queryinterlockingB(sql);
    //所有条件满足，为进路地区段增加锁闭，为进路地道岔改变定反位并锁闭
    while(queryinterlockingB.next())
    {
        First = queryinterlockingB.value(9).toString();
        //条件判断,暂时设置First = 1
        lineruleid = queryinterlockingB.value(0).toString();
        SectionNames = queryinterlockingB.value(5).toString();
        SwitchNames = queryinterlockingB.value(7).toString();
        Type = queryinterlockingB.value(8).toString();
        if(SectionNames.count() == 0 || SwitchNames.count() == 0)
        {
            return ;
        }

        //拆分道岔名字和定反位
        QByteArray num = switchesStrextract(SwitchNames);
        int switchname;
        for(int k=0;k<num.count();k+=2){
            switchname = num[k];//进路的区段名字
            int switchid;//对应的区段ID
            QString selectswitch = QString("SELECT * FROM switch WHERE SwitchName = %1").arg(switchname);
            QSqlQuery queryswitch(selectswitch);
            while(queryswitch.next())
            {
                switchid = queryswitch.value(0).toInt();
                if(SwitchDataMap.find(switchid).value().switchPos == (num[k+1]+0)){//如果要求该道岔是定位
                    if(SwitchDataMap.find(switchid).value().switchStates != 0x01){
                        SwitchDataMap.find(switchid).value().switchStates = 0x01;//启用该道岔的（定/反）
                        SwitchDataMap.find(switchid - 1).value().switchStates = 0x00;//关闭该道岔的（反/定）
                        MessageList.append(2);
                        MessageList.append(switchid);
                        MessageList.append(61);
                    }
                    if(type == 1){//对象正常锁闭
                        SwitchDataMap.find(switchid).value().switchLock = 0x01;
                        MessageList.append(2);
                        MessageList.append(switchid);
                        MessageList.append(62);
                    }else if(type == 2){//对象引导锁闭
                        SwitchDataMap.find(switchid).value().switchLock = 0x03;
                        MessageList.append(2);
                        MessageList.append(switchid);
                        MessageList.append(62);
                    }
                }else if(SwitchDataMap.find(switchid).value().switchPos == (num[k+1]-2)){//如果要求该道岔是反位
                    if(SwitchDataMap.find(switchid).value().switchStates != 0x01){
                        SwitchDataMap.find(switchid).value().switchStates = 0x01;//启用该道岔的（定/反）
                        SwitchDataMap.find(switchid + 1).value().switchStates = 0x00;//关闭该道岔的（反/定）
                        MessageList.append(2);
                        MessageList.append(switchid);
                        MessageList.append(60);
                    }
                    if(type == 1){//对象正常锁闭
                        SwitchDataMap.find(switchid).value().switchLock = 0x01;
                        MessageList.append(2);
                        MessageList.append(switchid);
                        MessageList.append(62);
                    }else if(type == 2){//对象引导锁闭
                        SwitchDataMap.find(switchid).value().switchLock = 0x03;
                        MessageList.append(2);
                        MessageList.append(switchid);
                        MessageList.append(62);
                    }
                }
            }
        }

        //拆分区段名字
        QStringList sectionsplit = SectionNames.split(",");
        foreach (QString sectionname, sectionsplit) {
            QString sectionid;
            QString selectsection = QString("SELECT * FROM sectioninfo WHERE SectionName = \"%1\"").arg(sectionname);
            QSqlQuery querysection(selectsection);
            while(querysection.next())
            {
                sectionid = querysection.value(0).toString();
                if(type == 1){
                    SectionsDataMap.find(sectionid).value().LockStatus = 0x01;   //对象正常锁闭
                    MessageList.append(1);
                    MessageList.append(sectionid.toInt());
                    MessageList.append(6);
                }else if(type == 2){
                    SectionsDataMap.find(sectionid).value().LockStatus = 0x03;   //对象引导锁闭
                    MessageList.append(1);
                    MessageList.append(sectionid.toInt());
                    MessageList.append(6);
                }
            }
        }
        if(type==0x01)//正常进路信号机状态
        {
        if(Type=='1')//正线进站信号灯状态
        {
           if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
           {
               SignalsDataMap.find(beginSignalID).value().signalStatus=0x04;
           }
           else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
           {
               SignalsDataMap.find(beginSignalID).value().signalStatus=0x01;
           }
           else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
           {
               SignalsDataMap.find(beginSignalID).value().signalStatus=0x04;
           }
           else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
           {
               SignalsDataMap.find(beginSignalID).value().signalStatus=0x01;
           }
           else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
           {
               SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
           }
           else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
           {
               SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
           }
           else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
           {
               SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
           }
           else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
           {
               SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
           }
           else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
           {
               SignalsDataMap.find(beginSignalID).value().signalStatus=0x0a;
           }
           else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
           {
               SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
           }
           else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
           {
               SignalsDataMap.find(beginSignalID).value().signalStatus=0x0a;
           }
           else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
           {
               SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
           }
           else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
           {
               SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
           }
           else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
           {
               SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
           }
           else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
           {
               SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
           }
           else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
           {
               SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
           }
        }
        if(Type=='2')//侧线进站信号灯状态
        {
            if(SignalsDataMap.find(beginSignalID).value().signalStatus==0x02)
            {
                continue;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x02;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x01;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x02;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x01;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0b;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0b;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
        }
        if(Type=='3')//列车出站进路信号灯状态
        {
            if(SignalsDataMap.find(beginSignalID).value().signalStatus==0x06)
            {
                continue;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x06;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x01;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x06;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x01;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0e;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0e;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
        }
        if(Type=='4')//调车进路信号灯状态
        {
            if(SignalsDataMap.find(beginSignalID).value().signalStatus==0x08)
            {
                continue;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x08;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x07;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x08;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x07;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0c;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0c;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0c;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0c;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0d;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0c;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0d;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0c;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0c;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0c;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0c;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0c;
            }
        }
        if(Type=='5')//出站兼调车信号灯状态
        {
            if(SignalsDataMap.find(beginSignalID).value().signalStatus==0x08)
            {
                continue;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x08;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x01;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x08;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x01;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0d;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x0d;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x00)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x09;
            }
        }
        }
        else if(type==0x02)//引导进路信号灯状态
        {
            SignalsDataMap.find(beginSignalID).value().signalStatus=0x0f;//红白
        }

        RuleList.append(Type);
        RuleList.append(SectionNames);
        RuleList.append(SwitchNames);
        RuleMap[beginSignalName] = RuleList;
    }
}

//【02操作·总取消+总人解】
void InterLock::RemoveRoute(byte beginSignalID,int type)
{
    QString sectionid;
    QString beginSignalName;
    bool sectionremove = false;//是否取消进路成功（判断总取消和引导进路成功的条件）
    bool switchremove = false;
    //查询起始信号机ID对应名称
    QString selectBeginSignalStr = QString("select * from signalinfo WHERE signalinfo.SignalID = %1").arg(beginSignalID);
    QSqlQuery queryBeginSignal(selectBeginSignalStr);
    while(queryBeginSignal.next())
    {
        beginSignalName = queryBeginSignal.value(3).toString();
    }

    //取消进路条件判断
    //区段判断
    QMap<QString,QList<QString>>::iterator it = RuleMap.find(beginSignalName);
    while(it != RuleMap.end()&&it.key() == beginSignalName)
    {
        QString str = it.value()[1];
        QStringList strSplit = str.split(",");
        for(int i=0; i<strSplit.length(); i++)
        {
            QString selectsection = QString("SELECT * FROM sectioninfo WHERE SectionName = \"%1\"").arg(strSplit[i]);
            QSqlQuery querysection(selectsection);
            while(querysection.next())
            {
                sectionid = querysection.value(0).toString();
            }
            //----------------【判断条件1】区段封锁----------------
            if(SectionsDataMap.find(sectionid).value().blockStatus == 0x01){
                return;
            }
            //----------------【判断条件2】区段分路不良----------------
            if(SectionsDataMap.find(sectionid).value().PoorStatus == 0x01){
                return;
            }
            //----------------【判断条件3】区段占用----------------
            if(SectionsDataMap.find(sectionid).value().sectionStatus == 0x01){
                return;
            }
            //----------------【判断条件4】区段白光带故障----------------
            if(SectionsDataMap.find(sectionid).value().SectionWhiteObstacle == 0x01){
                return;
            }
            //----------------【判断条件5】区段红光带故障----------------
            if(SectionsDataMap.find(sectionid).value().SectionRedObstacle == 0x01){
                return;
            }
        }
        it++;
    }
    //道岔判断
    QMap<QString,QList<QString>>::iterator switchIt = RuleMap.find(beginSignalName);
    while(switchIt != RuleMap.end()&&switchIt.key() == beginSignalName)
    {
        QString SwitchNames = switchIt.value()[2];
        QByteArray num = switchesStrextract(SwitchNames);
        for(int k=0;k<num.count();k+=2){
            int switchname = num[k];
            int switchid;//对应的区段ID
            QString selectswitch = QString("SELECT * FROM switch WHERE SwitchName = %1").arg(switchname);
            QSqlQuery queryswitch(selectswitch);
            while(queryswitch.next())
            {
                switchid = queryswitch.value(0).toInt();
                //----------------【判断条件6】道岔失表----------------
                if(SwitchDataMap.find(switchid).value().SwitchLoss == 0x01){
                    return;
                }
                //----------------【判断条件7】道岔封锁----------------
                if(SwitchDataMap.find(switchid).value().blockStatus == 0x01){
                    return;
                }

            }
        }
        switchIt++;
    }

    //取消区段锁定
    QMap<QString,QList<QString>>::iterator itstr = RuleMap.find(beginSignalName);
    while(itstr != RuleMap.end()&&itstr.key() == beginSignalName)
    {
        QString str = itstr.value()[1];
        QStringList strSplit = str.split(",");
        for(int i=0; i<strSplit.length(); i++)
        {
            QString selectsection = QString("SELECT * FROM sectioninfo WHERE SectionName = \"%1\"").arg(strSplit[i]);
            QSqlQuery querysection(selectsection);
            while(querysection.next())
            {
                sectionid = querysection.value(0).toString();
            }
            if(type == 1)
            { //总取消
                if(SectionsDataMap.find(sectionid).value().LockStatus == 0x01)
                {
                    SectionsDataMap.find(sectionid).value().LockStatus = 0x02;
                    sectionremove = true;
                }
            }else if(type == 2)
            { //总人解
                if(SectionsDataMap.find(sectionid).value().LockStatus == 0x01 || SectionsDataMap.find(sectionid).value().LockStatus == 0x03)
                {
                    SectionsDataMap.find(sectionid).value().LockStatus = 0x02;
                    sectionremove = true;
                }
            }
        }
        itstr++;
    }

    //取消道岔锁定
    QMap<QString,QList<QString>>::iterator swIt = RuleMap.find(beginSignalName);
    while(swIt != RuleMap.end()&&swIt.key() == beginSignalName)
    {
        QString SwitchNames = swIt.value()[2];
        int i = 0;
        QMap<int,SwitchData>::iterator switchit;
        QByteArray num = switchesStrextract(SwitchNames);
        for(int k=0;k<num.count();k+=2){
            int switchname = num[k];
            for(switchit = SwitchDataMap.begin();switchit != SwitchDataMap.end();++switchit)
            {
                if(num[k+1].operator == (0x01)){//反位
                    if(switchit.value().switchName == switchname && switchit.value().switchPos == 1){
                        if(type == 1)
                        { //总取消
                            if(switchit.value().switchLock == 0x01){
                                switchit.value().switchLock = 0x02;
                                switchremove = true;
                            }
                        }
                        else if(type == 2)
                        { //总人解
                            if(switchit.value().switchLock == 0x01 || switchit.value().switchLock == 0x03){
                                switchit.value().switchLock = 0x02;
                                switchremove = true;
                            }
                        }
                    }
                }else if(num[k+1].operator == (0x02)){ //定位
                    if(switchit.value().switchName == switchname && switchit.value().switchPos == 0){
                        if(type == 1)
                        { //总取消
                            if(switchit.value().switchLock == 0x01){
                                switchit.value().switchLock = 0x02;
                                switchremove = true;
                            }
                        }
                        else if(type == 2)
                        { //总人解
                            if(switchit.value().switchLock == 0x01 || switchit.value().switchLock == 0x03){
                                switchit.value().switchLock = 0x02;
                                switchremove = true;
                            }
                        }
                    }
                }
                i++;
            }
        }
        swIt++;
    }
    if(sectionremove == true && switchremove == true){
        RuleMap.remove(beginSignalName);
        SignalsDataMap.find(beginSignalID).value().signalStatus = Red;
    }

}

//【03操作·引导总锁】
void InterLock::YinDaoZS(byte Direction)
{
    //01为锁闭状态，02为未锁闭
    byte direction = Direction;
    SwitchData switchData;
    QMap<int,SwitchData>::iterator itSwitch;

    if(0x01 == direction)//X引导总锁
    {
        for(itSwitch = SwitchDataMap.begin();itSwitch != SwitchDataMap.end();++itSwitch)
        {
            if(itSwitch.value().switchName % 2 != 0)//为奇数
            {
                itSwitch.value().switchLock = 0x01;
            }
        }
    }
    if(0x02 == direction)//S引导总锁
    {
        for(itSwitch = SwitchDataMap.begin();itSwitch != SwitchDataMap.end();++itSwitch)
        {
            if(itSwitch.value().switchName % 2 == 0)//为偶数
            {
                itSwitch.value().switchLock = 0x01;
            }
        }
    }
}

//【04操作·封锁按钮（封锁/取消封锁）】
void InterLock::FengSuo(byte Id,byte Type)
{
    QString thisid = QString::number(Id, 10);
    int Switchid;
    if(0x01 == Type)//封锁信号机
    {
        if(SignalsDataMap.find(Id).value().blockStatus != 0x01){
            SignalsDataMap.find(Id).value().blockStatus = 0x01;
        }else{
           SignalsDataMap.find(Id).value().blockStatus = 0x02;
        }

    }
    if(0x03 == Type)//封锁轨道
    {
        qDebug()<<SectionsDataMap.find(thisid).value().blockStatus;
        if(SectionsDataMap.find(thisid).value().blockStatus != 0x01)
        {
            SectionsDataMap.find(thisid).value().blockStatus = 0x01;
        }
        else
        {
            SectionsDataMap.find(thisid).value().blockStatus = 0x02;
        }
    }
    if(0x02 == Type)//封锁道岔
    {
        QString selectswitchid = QString("SELECT * from switch where SwitchName IN (select SwitchName from switch WHERE SwitchName = %1)").arg(Id);
        QSqlQuery querySwitchid(selectswitchid);
        while(querySwitchid.next())
        {
            Switchid = querySwitchid.value(0).toInt();
            if(SwitchDataMap.find(Switchid).value().blockStatus != 0x01)
            {
                SwitchDataMap.find(Switchid).value().blockStatus = 0x01;
            }
            else
            {
                SwitchDataMap.find(Switchid).value().blockStatus = 0x02;
            }
        }
    }
}

//【05操作·区故解】
void InterLock::QuGJ(byte Snum)
{
    QString sectionname;

    //根据区段的nameid查找到区段名字
    QString selectsid = QString("select * from section WHERE section.sectionnameid = %1").arg(Snum);
    QSqlQuery queryid(selectsid);
    while(queryid.next())
    {
        sectionname = queryid.value(1).toString();
    }

    QMap<QString,QList<QString>> ::iterator it;
    int i = 0;
    for(it = RuleMap.begin();it != RuleMap.end();++it)
    {
        //如果该条进路的区段字符串拼接中包含需要区故解的区段
        if(it.value()[1].contains(sectionname)){
            QStringList NameList = it.value()[1].split(",");
            int len = NameList.length();
            for(int i=0; i<len; i++)
            {
                QMap<QString,SectionData>::iterator itsec = SectionsDataMap.find(NameList[i]);
                //如果进路中发现该区段白光带故障,或锁闭状态
                if(itsec.value().SectionWhiteObstacle == 0x01 || itsec.value().LockStatus == 0x01)
                {
                    SectionsDataMap.find(NameList[i]).value().SectionWhiteObstacle = 0x02;//取消该区段的白光带故障
                    SectionsDataMap.find(NameList[i]).value().LockStatus = 0x02;//取消该区段的锁闭状态
                    //第一个区段故障
                    if(i == 0)
                    {
                        SectionsDataMap.find(NameList[i+1]).value().SectionWhiteObstacle = 0x01;//且下一位区段变成白光带故障
                    }
                    //第二个区段故障
                    if(i == 1){
                        SectionsDataMap.find(NameList[i+1]).value().SectionWhiteObstacle = 0x01;//且下一位区段变成白光带故障
                        SectionsDataMap.find(NameList[i-1]).value().SectionWhiteObstacle = 0x02;//取消该区段的白光带故障
                        SectionsDataMap.find(NameList[i-1]).value().LockStatus = 0x02;//取消该区段的锁闭状态
                    }
                    //如果是最后一段区段故障
                    else if(i == len-1)
                    {
                        for(int j=i-1;j>=0;j--)
                        {
                            QMap<QString,SectionData>::iterator itsec1 = SectionsDataMap.find(NameList[j]);
                            if(itsec1.value().LockStatus == 0x02 && itsec1.value().SectionWhiteObstacle == 0x02){//如果区故解区段之前的区段已经被区故解（即无锁闭且无白光带故障）
                                continue;
                            }else{
                               itsec1.value().SectionWhiteObstacle = 0x01;//且前面的区段全部变成白光带故障
                            }
                        }
                    }
                    //如果是倒数第二段区段故障
                    else if(i == len-2)
                    {
                        for(int j=i-1;j>=0;j++)
                        {
                            SectionsDataMap.find(NameList[i+1]).value().SectionWhiteObstacle = 0x02;//取消该区段的白光带故障
                            SectionsDataMap.find(NameList[i+1]).value().LockStatus = 0x02;//取消该区段的锁闭状态
                            QMap<QString,SectionData>::iterator itsec1 = SectionsDataMap.find(NameList[j]);
                            if(itsec1.value().LockStatus == 0x02 && itsec1.value().SectionWhiteObstacle == 0x02){//如果区故解区段之前的区段已经被区故解（即无锁闭且无白光带故障）
                                continue;
                            }else{
                                itsec1.value().SectionWhiteObstacle = 0x01;//且前面的区段全部变成白光带故障
                            }
                        }
                    }
                    //如果是进路中的中间区段
                    else if(i>0 && i<len-1)
                    {
                        SectionsDataMap.find(NameList[i+1]).value().SectionWhiteObstacle = 0x01;//且下一位区段变成白光带故障
                        for(int j=i-1;j>=0;j--)
                        {
                            QMap<QString,SectionData>::iterator itsec1 = SectionsDataMap.find(NameList[j]);
                            if(itsec1.value().LockStatus == 0x02 && itsec1.value().SectionWhiteObstacle == 0x02){//如果区故解区段之前的区段已经被区故解（即无锁闭且无白光带故障）
                                continue;
                            }else{
                                itsec1.value().SectionWhiteObstacle = 0x01;//且前面的区段全部变成白光带故障
                            }
                        }
                    }
                }
            }
        }
        i++;
    }
}

//【06操作·信号重开】
void InterLock::XinHaoCK(byte SignalID)
{
    QString beginSignalName;
    QString selectname =QString("select *from signalinfo WHERE signalinfo.SignalID=%1").arg(SignalID);
    QSqlQuery sqlname(selectname);
    while(sqlname.next())
    {
        beginSignalName=sqlname.value(3).toString();
        qDebug()<<"beginSignalName:"<<beginSignalName;
    }
    if(RuleMap.keys().contains(beginSignalName))
    {
        for(int i=0;i<RuleMap[beginSignalName].length();i++)
        {
            QString type=RuleMap[beginSignalName][0];
            QString sectionname=RuleMap[beginSignalName][1];
            QStringList SectionName=sectionname.split(",");
            for(int a=0; a<SectionName.length();a++)
            {
                qDebug()<<SectionName[a];
                QString sectionid;
                QString selectid= QString("select *from sectioninfo WHERE sectioninfo.SectionName = \"%1\"").arg(SectionName[a]);
                QSqlQuery sqlid(selectid);
                while(sqlid.next())
                {
                    sectionid=sqlid.value(0).toString();
                    qDebug()<<sectionid;
                    if(SectionsDataMap.find(sectionid).value().sectionStatus==0x01)
                    {
                        if(type=='4')
                        {
                            SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                            SignalsDataMap.find(SignalID).value().signalLockStatus=0x01;
                        }
                        else
                        {
                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalLockStatus=0x01;
                        }
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().signalLockStatus==0x02)
                    {
                        continue;
                    }
                    else if(SignalsDataMap.find(SignalID).value().signalLockStatus==0x01&&SectionsDataMap.find(sectionid).value().sectionStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().signalLockStatus==0x01&&SectionsDataMap.find(sectionid).value().sectionStatus==0x02)//如果信号机锁闭并且无故障
                        {if(type=='1')
                        {
                            SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                            SignalsDataMap.find(SignalID).value().signalLockStatus=0x02;
                        }
                        if(type=='2')
                        {
                            SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                            SignalsDataMap.find(SignalID).value().signalLockStatus=0x02;
                        }
                        if(type=='3')
                        {
                            SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                            SignalsDataMap.find(SignalID).value().signalLockStatus=0x02;
                        }
                        if(type=='4')
                        {
                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                            SignalsDataMap.find(SignalID).value().signalLockStatus=0x02;
                        }
                        if(type=='5')
                        {
                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                            SignalsDataMap.find(SignalID).value().signalLockStatus=0x02;
                        }
                    }
                }
            }
        }
    }
    else
    {
        return;
    }
}

//【07操作·道岔总定反,单解锁】
void InterLock::ZongDingFanDanJieSuo(byte Snum,byte Sstatus)
{
    int SwitchID;
    int SwitchNameID;
    QString selectSwitchID = QString("select *from switch WHERE switch.SwitchNameID =%1").arg(Snum);
    QSqlQuery sqlSwitchID(selectSwitchID);
    while(sqlSwitchID.next())
    {
        SwitchID=sqlSwitchID.value(0).toInt();
        SwitchNameID =sqlSwitchID.value(5).toInt();
        //道岔单锁
        if(Sstatus == 0x03)
        {
            if(0x01 == SwitchDataMap.find(SwitchID).value().switchLock)
            {
                return;
            }
            else
                SwitchDataMap.find(SwitchID).value().switchLock = 0x01;
        }
        //道岔解锁
        if(Sstatus == 0x04)
        {
            if(0x02 == SwitchDataMap.find(SwitchID).value().switchLock)
            {
                return;
            }
            else
                SwitchDataMap.find(SwitchID).value().switchLock = 0x02;
        }
        //道岔定位
        if(Sstatus == 0x01)
        {
            if(0x00 ==SwitchDataMap.find(SwitchID).value().switchPos)//如果是定位就启用
            {
                SwitchDataMap.find(SwitchID).value().switchStates = 0x01;
            }
            else if(0x01 == SwitchDataMap.find(SwitchID).value().switchPos)//如果是反位就不启用
            {
                SwitchDataMap.find(SwitchID).value().switchStates = 0x00;
            }
        }
        //道岔反位
        if(Sstatus == 0x02)
        {
            if(0x01 == SwitchDataMap.find(SwitchID).value().switchPos)//如果是反位就启用
            {
                SwitchDataMap.find(SwitchID).value().switchStates = 0x01;
            }

            else if(0x00 == SwitchDataMap.find(SwitchID).value().switchPos)//如果是定位不启用
            {
                SwitchDataMap.find(SwitchID).value().switchStates = 0x00;
            }
        }
    }
}

//【08操作·灯丝断丝】
void InterLock::DSDS(byte SignalID, byte Status)
{
    QString beginSignalName;
    QString selectSignalName = QString("select *from signalinfo WHERE signalinfo.SignalID=%1").arg(SignalID);
    QSqlQuery sqlSignalName(selectSignalName);
    while(sqlSignalName.next())
    {
        beginSignalName=sqlSignalName.value(3).toString();
    }

    if(RuleMap.keys().contains(beginSignalName))//如果该信号机所在位置有进路
    {
        for (int i=0;i<RuleMap[beginSignalName].length();i++)
        {
            QString type = RuleMap[beginSignalName][0];
            qDebug()<<type;
            if(type == '1')//正线进站进路
            {
                if(Status==0x01)
                {
                    if(SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0a;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0a;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                }
                if(Status==0x02)
                {
                    if(SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                }
                if(Status==0x03)//清除故障
                {
                    if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                }
                if(Status==0x04)
                {
                    if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0a;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                }
                if(Status==0x05)
                {
                    if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                }
                if(Status==0x06)
                {
                    if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0a;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0a;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                }

            }
            else if(type == '2')//侧线进站进路
            {
                if(Status==0x01)//红蓝主灯丝断
                {
                    if(SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0b;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0b;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                }
                if(Status==0x02)
                {
                    if(SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                }
                if(Status==0x03)//清除故障
                {
                    if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                }
                if(Status==0x04)
                {
                    if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0b;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                }
                if(Status==0x05)
                {
                    if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                }
                if(Status==0x06)
                {
                    if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0b;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0b;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                }
            }
            else if(type =='3')//出站列车进路
            {
                if(Status==0x01)//红蓝主灯丝断
                {
                    if(SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0e;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0e;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0e;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                }
                if(Status==0x02)
                {
                    if(SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                }
                if(Status==0x03)//清除故障
                {
                    if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                }
                if(Status==0x04)
                {
                    if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0e;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                }
                if(Status==0x05)
                {
                    if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                }
                if(Status==0x06)
                {
                    if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0e;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0e;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                    }
                }
            }
            else if(type =='4')//调车进路
            {
                if(Status==0x01)//红蓝主灯丝断
                {
                    if(SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0d;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0d;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                }
                if(Status==0x02)
                {
                    if(SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                    }
                }
                if(Status==0x03)//清除故障
                {
                    if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                    }
                }
                if(Status==0x04)
                {
                    if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0d;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                }
                if(Status==0x05)
                {
                    if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                }
                if(Status==0x06)
                {
                    if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                    {
                        return;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0d;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0d;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                    else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                    {
                        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                        SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                    }
                }

            }
            else if(type =='5')//出站调车进路
                {
                    if(Status==0x01)//红蓝主灯丝断
                    {
                        if(SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                        {
                            return;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x0d;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x0d;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                    }
                    if(Status==0x02)
                    {
                        if(SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            return;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                    }
                    if(Status==0x03)//清除故障
                    {
                        if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            return;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                    }
                    if(Status==0x04)
                    {
                        if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                        {
                            return;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x0d;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                    }
                    if(Status==0x05)
                    {
                        if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            return;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                    }
                    if(Status==0x06)
                    {
                        if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            return;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x0d;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                    }
            }
        }
    }
    else
    {
        if(SignalsDataMap.find(SignalID).value().signalType==0x01)//进站信号机
        {
            if(Status==0x01)
            {
                if(SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                {
                    return;
                }
                else
                {
                    SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                    SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                }
            }
            if(Status==0x02)
            {
                if(SignalsDataMap.find(SignalID).value().DSStatus==0x02)
                {
                    return;
                }
                else
                {
                    SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                    SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                }
            }
            if(Status==0x03)
            {
                if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                {
                    return;
                }
                else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                {
                    SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                    SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                }
                else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                {
                    SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                    SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                }
                else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                {
                     SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                      SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                      SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                }
            }
            if(Status==0x04)
            {
                if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                {
                    return;
                }
                else
                {
                    SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                }
            }
            if(Status==0x05)
            {
                if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                {
                    return;
                }
                else
                {
                    SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x01;
                }
            }
            if(Status==0x06)
            {
                if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                {
                    return;
                }
                else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                {
                    SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                }
                else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                {
                    SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                }
                else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                {
                     SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                      SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                }
            }
        }
        if(SignalsDataMap.find(SignalID).value().signalType==0x02)//出站信号机
        {
            if(Status==0x01)
            {
                if(SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                {
                    return;
                }
                else
                {
                    SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                    SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                }
            }
            if(Status==0x02)
            {
                if(SignalsDataMap.find(SignalID).value().DSStatus==0x02)
                {
                    return;
                }
                else
                {
                    SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                    SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                }
            }
            if(Status==0x03)
            {
                if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                {
                    return;
                }
                else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                {
                    SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                    SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                }
                else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                {
                    SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                    SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                }
                else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                {
                     SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                      SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                      SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                }
            }
            if(Status==0x04)
            {
                if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                {
                    return;
                }
                else
                {
                    SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                }
            }
            if(Status==0x05)
            {
                if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                {
                    return;
                }
                else
                {
                    SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x01;
                }
            }
            if(Status==0x06)
            {
                if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                {
                    return;
                }
                else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                {
                    SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                }
                else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                {
                    SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                }
                else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                {
                     SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                      SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                }
            }
        }
        if(SignalsDataMap.find(SignalID).value().signalType==0x03)//调车信号机
        {
            if(Status==0x01)
            {
                if(SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                {
                    return;
                }
                else
                {
                    SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                    SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                }
            }
            if(Status==0x02)
            {
                if(SignalsDataMap.find(SignalID).value().DSStatus==0x02)
                {
                    return;
                }
                else
                {
                    SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                    SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                }
            }
            if(Status==0x03)
            {
                if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                {
                    return;
                }
                else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                {
                    SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                    SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                }
                else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                {
                    SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                    SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                }
                else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                {
                     SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                      SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                      SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                }
            }
            if(Status==0x04)
            {
                if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                {
                    return;
                }
                else
                {
                    SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                }
            }
            if(Status==0x05)
            {
                if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                {
                    return;
                }
                else
                {
                    SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x01;
                }
            }
            if(Status==0x06)
            {
                if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                {
                    return;
                }
                else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                {
                    SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                }
                else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                {
                    SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                }
                else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                {
                     SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                      SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                }
            }
        }
    }

}

//【09操作·灯丝复原】（待测试）
void InterLock::DSFY(byte Status)
{
    int SignalID;
    QString selectsignalid =QString("select *from signalinfo WHERE signalinfo.signalregion=%1").arg(Status);
    QSqlQuery sqlsignalid(selectsignalid);
    while(sqlsignalid.next())
    {
        SignalID=sqlsignalid.value(0).toInt();
        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
        QString BeginSignalName;
        QString selectsignalname = QString("select *from signalinfo WHERE signalinfo.SiganlName=%1").arg(SignalID);
        QSqlQuery sqlName(selectsignalname);
        while(sqlName.next())
        {
            BeginSignalName=sqlName.value(3).toString();
        }
        if(RuleMap.keys().contains(BeginSignalName))
        {
            for (int i=0;i<RuleMap[BeginSignalName].length();i++)
            {
                QString type =RuleMap[BeginSignalName][0];//进路类型
                if(type=='1')
                {
                    SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                }
                else if(type=='2')
                {
                    SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                }
                else if(type=='3')
                {
                    SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                }
                else if(type=='4')
                {
                    SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                }
                else if(type=='5')
                {
                    SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                }
            }
        }
        else
        {
            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
        }
    }

}

//【10操作·区段占用】
void InterLock::ZhanYong(byte sectionID)//nameid
{
    QString SectionName;
    QString id = QString::number(sectionID, 10);
    if(SectionsDataMap.find(id).value().sectionStatus==0x01)
    {
        SectionsDataMap.find(id).value().sectionStatus=0x02;
    }
    else SectionsDataMap.find(id).value().sectionStatus=0x01;

    QList<int> forswithc;
    QString selectSeName =QString("select *from section WHERE section.sectionnameid=%1").arg(id);
    QSqlQuery sqlName(selectSeName);
    while(sqlName.next())
    {
        SectionName=sqlName.value(1).toString();
        int sectionforswtich = sqlName.value(4).toInt();
        if(sectionforswtich != NULL){
            forswithc.append(sectionforswtich);
        }
    }
    for(int i=0; i<forswithc.length();i++){
        if(SwitchDataMap.find(forswithc[i]).value().switchStates == 0x01){
            if(SwitchDataMap.find(forswithc[i]).value().switchOccupy == 0x01){
                SwitchDataMap.find(forswithc[i]).value().switchOccupy = 0x02;
            }else{
                SwitchDataMap.find(forswithc[i]).value().switchOccupy = 0x01;
            }
        }
    }

    QMap<QString,QList<QString>>::iterator ruledata;
     //int i=0;
    for(ruledata = RuleMap.begin();ruledata != RuleMap.end();++ruledata)
    {
        if(ruledata.value()[1].contains(SectionName))
        {
            QString signalname = ruledata.value()[3];
            int signalid;
            QString signalsql = QString("select * from signalinfo WHERE SingalName = \"%1\"").arg(signalname);
            QSqlQuery querySignalid(signalsql);
            while(querySignalid.next())
            {
                signalid = querySignalid.value(0).toInt();
            }
            if(SectionsDataMap.find(id).value().sectionStatus==0x01)
            {
                SignalsDataMap.find(signalid).value().signalLockStatus = 0x01;
                SignalsDataMap.find(signalid).value().signalStatus = 0x01;
            }
            else if(SectionsDataMap.find(id).value().sectionStatus==0x02)
            {
                SignalsDataMap.find(signalid).value().signalLockStatus = 0x01;
            }
        }
        //i++
    }
}

//【11操作道岔失表】
void InterLock::SwitchLoss(byte SwitchName){
    int switchid;
    QString selectSwitchID = QString("select *from switch WHERE SwitchName =%1").arg(SwitchName);
    QSqlQuery sqlSwitchID(selectSwitchID);
    while(sqlSwitchID.next())
    {
        switchid = sqlSwitchID.value(0).toInt();
        SwitchDataMap.find(switchid).value().SwitchLoss = 0x01;
    }
}

//【12操作·道岔取消失表】
void InterLock::SwitchNoLoss(byte SwitchName){
    int switchid;
    QString selectSwitchID = QString("select *from switch WHERE SwitchName =%1").arg(SwitchName);
    QSqlQuery sqlSwitchID(selectSwitchID);
    while(sqlSwitchID.next())
    {
        switchid = sqlSwitchID.value(0).toInt();
        SwitchDataMap.find(switchid).value().SwitchLoss = 0x02;
    }
}



//道岔字符串信息提取
QByteArray InterLock::switchesStrextract(QString switchesStr)
{
    QByteArray switchesInfo;
    QStringList switchesSplit = switchesStr.split(",");
    for(int i=0; i<switchesSplit.length(); i++)
    {
        QByteArray switchesData;
        QRegExp rxMethod1(tr("^[\\(](\\d+)/(\\d+)[\\)]$"));//匹配(1/3)类型
        if(rxMethod1.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod1.cap(1).toInt();
            switchesData[1] = 0x01;
            switchesData[2] = rxMethod1.cap(2).toInt();
            switchesData[3] = 0x01;
        }
        QRegExp rxMethod2(tr("^(\\d+)/(\\d+)$"));//匹配1/3类型
        if(rxMethod2.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod2.cap(1).toInt();
            switchesData[1] = 0x02;
            switchesData[2] = rxMethod2.cap(2).toInt();
            switchesData[3] = 0x02;
        }
        QRegExp rxMethod3(tr("^[\\(](\\d+)[\\)]$"));//匹配(5)类型
        if(rxMethod3.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod3.cap(1).toInt();
            switchesData[1] = 0x01;
        }
        QRegExp rxMethod4(tr("^(\\d+)$"));//匹配5类型
        if(rxMethod4.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod4.cap(1).toInt();
            switchesData[1] = 0x02;
        }
        QRegExp rxMethod5(tr("^[\\[](\\d+)/(\\d+)[\\]]$"));//匹配[1/3]类型 防护到定位
        if(rxMethod5.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod5.cap(1).toInt();
            switchesData[1] = 0x02;
            switchesData[2] = rxMethod5.cap(2).toInt();
            switchesData[3] = 0x02;
        }
        QRegExp rxMethod6(tr("^[\\[][\\(](\\d+)/(\\d+)[\\)][\\]]$"));//匹配[(1/3)]类型 防护到方位
        if(rxMethod6.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod6.cap(1).toInt();
            switchesData[1] = 0x01;
            switchesData[2] = rxMethod6.cap(2).toInt();
            switchesData[3] = 0x01;
        }
        QRegExp rxMethod7(tr("^[\\[](\\d+)[\\]]$"));//匹配[5]类型 防护到定位
        if(rxMethod7.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod7.cap(1).toInt();
            switchesData[1] = 0x02;
        }
        QRegExp rxMethod8(tr("^[\\[][\\(](\\d+)[\\)][\\]]$"));//匹配[(5)]类型 防护到反位
        if(rxMethod8.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod8.cap(1).toInt();
            switchesData[1] = 0x01;
        }
        QRegExp rxMethod9(tr("^[\\{](\\d+)/(\\d+)[\\}]$"));//匹配{1/3}类型 带动到定位
        if(rxMethod9.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod9.cap(1).toInt();
            switchesData[1] = 0x02;
            switchesData[2] = rxMethod9.cap(2).toInt();
            switchesData[3] = 0x02;
        }
        QRegExp rxMethod10(tr("^[\\{][\\(](\\d+)/(\\d+)[\\)][\\}]$"));//匹配{(1/3)}类型 带动到反位
        if(rxMethod10.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod10.cap(1).toInt();
            switchesData[1] = 0x01;
            switchesData[2] = rxMethod10.cap(2).toInt();
            switchesData[3] = 0x01;
        }
        QRegExp rxMethod11(tr("^[\\{](\\d+)[\\}]$"));//匹配{5}类型
        if(rxMethod11.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod11.cap(1).toInt();
            switchesData[1] = 0x02;
        }
        QRegExp rxMethod12(tr("^[\\{][\\(](\\d+)[\\)][\\}]$"));//匹配{(5)}类型
        if(rxMethod12.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod12.cap(1).toInt();
            switchesData[1] = 0x01;
        }
        switchesInfo.append(switchesData);
    }
    return switchesInfo;
}

void InterLock::TimerTicked()
{
    //InterEncapsalutation(beginSignalID,endSignalID);
    //InterEncapsalutation(X_Direction,XF_Direction,XD_Direction,S_Direction,SF_Direction);
    QByteArray data = SectionEncapsalutation();
    udpSocket->writeDatagram(data,QHostAddress("127.0.0.1"),4401);
    QByteArray data1 = SignalEncapsalutation();
    udpSocket->writeDatagram(data1,QHostAddress("127.0.0.1"),4402);
    QByteArray data2 = SwitchEncapsalutation();
    udpSocket->writeDatagram(data2,QHostAddress("127.0.0.1"),4403);
   /* QByteArray data3 = RuleEncapsalutation();
    udpSocket->writeDatagram(data3,QHostAddress("127.0.0.1"),4404);*/
}

//设置信号机
void InterLock::SetupSignal(byte SignalID,byte signalStatus)
{
    //0灰色禁用，1位单红日常，2为双黄，3为双黄闪，4为单黄，5为绿黄，6为单绿,7为单蓝,8为白
    SignalsDataMap.find(SignalID).value().signalStatus = signalStatus;
}

//全站区段信息缓存
void InterLock::SectionDataCache()
{
    SectionData sectioData;
    QSqlQuery querySection("select * from sectioninfo");
    while (querySection.next())
    {
        sectioData.sectionId = querySection.value(0).toInt();
        sectioData.sectionName = querySection.value(1).toInt();
        sectioData.LockStatus = 0x01;
        sectioData.sectionStatus = 0x02;
        sectioData.blockStatus = 0x02;
        sectioData.First1 = 0x01;
        sectioData.SectionRedObstacle = 0x00;
        sectioData.SectionWhiteObstacle = 0x00;
        sectioData.PoorStatus =0x02;

        SectionsDataMap[querySection.value(0).toString()] = sectioData;
    }
}

//全站区段信息封装
QByteArray InterLock::SectionEncapsalutation()
{
    QByteArray frameHead("\x10\x00\x00\x00\x00\x10\x10\x01\x4a\x98\x07\x00",12);
    QByteArray frameEnd("\x59\x4F\x44\x4F",4);
    QByteArray SectionDataInfo;
    QByteArray SectionInitData;
    SectionData SectioData;int i = 0;
    QMap<QString,SectionData>::iterator it;
    SectionInitData.resize(54*sizeof(SectioData));
    byte sectionNr = SectionsDataMap.count();
    frameHead[1] = (sectionNr*9)+5;
    frameHead[11] = sectionNr;
    SectionDataInfo.append(frameHead);
    SectionDataInfo.append(First);
    if(First != 0){
       First = 0;
    }
    for(it = SectionsDataMap.begin();it != SectionsDataMap.end();++it)
    {
        memcpy(SectionInitData.data()+i*sizeof(it.value()),&it.value(),sizeof(it.value()));
        i++;
    }
    SectionDataInfo.append(SectionInitData);

    //发送消息提示框信息
    if(MessageList.count() != 0){

        QByteArray MessageSend;
        for(int j = 0;j < MessageList.count(); j++)
        {
            MessageSend.append(MessageList[j]);
        }

        SectionDataInfo.append(MessageSend);
        //清空发送消息提示框信息
        MessageList.clear();
    }
    SectionDataInfo.append(frameEnd);
    return SectionDataInfo;
}

//全站信号机信息缓存
void InterLock::SignalDataCache()
{
    SignalData signalData;
    QSqlQuery querySignal("select * from SignalInfo");
    while (querySignal.next())
    {
        signalData.signalId = querySignal.value(0).toInt();
        signalData.signalStatus = querySignal.value(6).toInt();
        signalData.blockStatus = querySignal.value(4).toInt();
        signalData.DSStatus =querySignal.value(5).toInt();
        signalData.RedAllDSStatus = querySignal.value(7).toInt();
        signalData.GreenDSStatus = querySignal.value(8).toInt();
        signalData.GreenAllDSStatus = querySignal.value(9).toInt();
        signalData.signalType =querySignal.value(1).toInt();
        signalData.signalLockStatus = 0x02;

        SignalsDataMap[querySignal.value(0).toInt()] = signalData;
    }
}

//全站信号机信息封装
QByteArray InterLock::SignalEncapsalutation()
{
    QByteArray signalInitData;
    QByteArray signalDataInfo;
    SignalData signalData;
    int i = 0;
    QMap<int,SignalData>::iterator it;
    signalDataInfo.resize(47*sizeof(signalData));
    QByteArray frameHead("\x10\x00\x00\x00\x00\x11\x10\x01\x4a\x98\x07\x00",12);
    QByteArray frameEnd("\x59\x4F\x44\x4F",4);
    byte signalNr = SignalsDataMap.count();
    frameHead[1] = (signalNr*9)+5;
    frameHead[11] = signalNr;
    signalInitData.append(frameHead);
    for(it = SignalsDataMap.begin();it != SignalsDataMap.end();++it)
    {
        memcpy(signalDataInfo.data()+i*sizeof(it.value()),&it.value(),sizeof(it.value()));
        i++;
    }
    signalInitData.append(signalDataInfo);
    signalInitData.append(frameEnd);
    return signalInitData;
}

//全站道岔信息缓存
void InterLock::SwitchDataCache()
{
    SwitchData switchData;
    QSqlQuery querySwitch("select * from Switch");
    while(querySwitch.next())
    {
        switchData.switchID            = querySwitch.value(0).toInt();
        switchData.switchName          = querySwitch.value(1).toInt();
        switchData.switchPos           = querySwitch.value(2).toInt();
        switchData.switchStates        = querySwitch.value(3).toInt();
        switchData.switchLock          = querySwitch.value(4).toInt();
        switchData.switchNameID        = querySwitch.value(5).toInt();
        switchData.blockStatus         = querySwitch.value(6).toInt();
        switchData.switchOccupy        =0x02;
        SwitchDataMap[querySwitch.value(0).toInt()] = switchData;
    }
}

//全站道岔信息封装
QByteArray InterLock::SwitchEncapsalutation()
{
    QByteArray switchInitData;
    QByteArray switchDataInfo;
    SwitchData switchData;int i = 0;
    QMap<int,SwitchData>::iterator it;
    switchDataInfo.resize(52*sizeof(switchData));
    QByteArray frameHead("\x10\x00\x00\x00\x00\x12\x10\x01\x4a\x98\x07\x00",12);
    QByteArray frameEnd("\x59\x4F\x44\x4F",4);
    byte switchNr = SwitchDataMap.count();
    frameHead[1] = (switchNr*8)+5;
    frameHead[11] = switchNr;
    switchInitData.append(frameHead);
    for(it = SwitchDataMap.begin();it != SwitchDataMap.end();++it)
    {
        memcpy(switchDataInfo.data()+i*sizeof(it.value()),&it.value(),sizeof(it.value()));
        i++;
    }
    switchInitData.append(switchDataInfo);
    switchInitData.append(frameEnd);
    return switchInitData;
}

//进路规则信息缓存
void InterLock::RuleDataCache()
{
    LineRuleData LineruleData;
    QSqlQuery queryRule("select * from linerule");
    while(queryRule.next())
    {
        LineruleData.lineruleID           = queryRule.value(0).toInt();
        LineruleData.beginSignalName      = queryRule.value(1).toInt();
        LineruleData.endSignalName        = queryRule.value(2).toInt();
        LineruleData.Type                 = queryRule.value(3).toInt();
        LineruleData.First                = queryRule.value(4).toInt();
        RuleDataMap[queryRule.value(0).toString()] = LineruleData;
    }
}

//进路规则信息封装
QByteArray InterLock::RuleEncapsalutation()
{
    QByteArray ruleInitData;
    QByteArray ruleDataInfo;
    LineRuleData LineruleData;int i = 0;
    QMap<QString,LineRuleData>::iterator it;
    ruleDataInfo.resize(58*sizeof(LineruleData));
    QByteArray frameHead("\x10\x00\x00\x00\x00\x12\x10\x01\x4a\x98\x07\x00",12);
    QByteArray frameEnd("\x59\x4F\x44\x4F",4);
    byte ruleNr = RuleDataMap.count();
    frameHead[1] = (ruleNr*5)+5;
    frameHead[11] = ruleNr;
    ruleInitData.append(frameHead);
    for(it = RuleDataMap.begin();it != RuleDataMap.end();++it)
    {
        memcpy(ruleDataInfo.data()+i*sizeof(it.value()),&it.value(),sizeof(it.value()));
        i++;
    }
    ruleInitData.append(ruleDataInfo);
    ruleInitData.append(frameEnd);
    return ruleInitData;
}

//三点检查逐条解锁
void InterLock::UnlockState(byte beginSignalID,byte endSignalID)
{
    QString beginSignalName = QString();
    QString endSignalName = QString();
    QString sectionsStr;
    QString switchesStr;int i;
    QString selectBeginSignalStr = QString("select * from signalinfo WHERE signalinfo.SignalID = %1").arg(beginSignalID);
    QSqlQuery queryBeginSignal(selectBeginSignalStr);
    while(queryBeginSignal.next())
    {
        beginSignalName = queryBeginSignal.value(3).toString();
    }
    QString selectEndSignalStr = QString("select * from signalinfo WHERE signalinfo.SignalID = %1").arg(endSignalID);
    QSqlQuery queryEndSignal(selectEndSignalStr);
    while(queryEndSignal.next())
    {
        endSignalName = queryEndSignal.value(3).toString();
    }
    QString selectInterlockRouteStr = QString("select * from interlocking WHERE interlocking.BeginSignal = \"%1\" AND interlocking.EndSignal = \"%2\"").arg(beginSignalName).arg(endSignalName);
    QSqlQuery queryInterlockRoute(selectInterlockRouteStr);
    while(queryInterlockRoute.next())
    {
        sectionsStr = queryInterlockRoute.value(5).toString();
        qDebug()<<"str"<<sectionsStr;
        switchesStr = queryInterlockRoute.value(7).toString();
        qDebug()<<"str"<<switchesStr;
    }
    if(sectionsStr.isEmpty() || switchesStr.isEmpty())
    {
        return;
    }
    QStringList sectionsSplit = sectionsStr.split(",");
    for(i = 0; i < 2*sectionsSplit.length()-1; i++)
    {
        int n=(i+1)/2;
        if(i == 0)
        {
            SectionsDataMap.find(sectionsSplit[0]).value().sectionStatus = 0x01;
        }
        if(i != 0)
        {
            SectionsDataMap.find(sectionsSplit[n]).value().sectionStatus = 0x01;
            if(i % 2 == 0)
            {
                SectionsDataMap.find(sectionsSplit[n-1]).value().sectionStatus = 0x02;
            }
            if(i % 2 != 0)
            {
                SectionsDataMap.find(sectionsSplit[n-1]).value().sectionStatus = 0x01;
            }
        }
    }
    /*for(i=0;i<sectionsSplit.length();i++)
    {
        QMap<QString,SectionData>::iterator it = SectionsDataMap.find(sectionsSplit[i]);
        //QMap<int,SignalData>::iterator itSignal = SignalsDataMap.find(beginSignalName);
        if(0x01 == it.value().sectionStatus)//红光带故障
        {
            SectionsDataMap.find(sectionsSplit[i-1]).value().sectionStatus = ;//解锁故障
            SignalsDataMap.find(beginSignalName).value().signalStatus = 0x01;
        }
    }*/
}

//上电解锁
void InterLock::ShangDian()
{
    SwitchData switchData;
    QMap<int,SwitchData>::iterator itSwitch;
    QSqlQuery selectswitch("select * from switch");
    while(selectswitch.next())
    {
        for(itSwitch = SwitchDataMap.begin();itSwitch != SwitchDataMap.end();++itSwitch)
        {
            switchData.switchLock = 0x02;//上电后解锁，状态为空闲
        }
    }
}

//引导进路
void InterLock::YinDaoJL(byte Direction)
{
    direction = Direction;
    QMap<QString,SectionData>::iterator it;
    QMap<QString,QString>::iterator itstr;
    for(itstr = LockRouteMap.begin();itstr != LockRouteMap.end();++itstr)
    {
        //XF引导按钮
        if(0x02 == direction)
        {
            if(SwitchDataMap.find(1).value().switchStates == 0x00 && SwitchDataMap.find(19).value().switchStates == 0x00)//单黄闪,道岔定位为0，反位为1
            {
                if(SwitchDataMap.find(27).value().switchStates == 0x00)
                {
                    SectionsDataMap.find("IIAG").value().LockStatus = 0x01;//白光带,不消失，锁闭状态
                    SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("1/19WG").value().LockStatus = 0x01;
                    SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("IIG").value().LockStatus = 0x01;
                    SignalsDataMap.find(1).value().signalStatus = 0x06;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(1).value().signalStatus = 0x01;
                    }
                }
                if(SwitchDataMap.find(27).value().switchStates == 0x01 && SwitchDataMap.find(29).value().switchStates == 0x00)
                {
                    SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                    SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("1/19WG").value().LockStatus = 0x01;
                    SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("27/29WG").value().LockStatus = 0x01;
                    SectionsDataMap.find("29DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("4G").value().LockStatus = 0x01;
                    SignalsDataMap.find(1).value().signalStatus = 0x03;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(1).value().signalStatus = 0x01;
                    }
                }
            }
            if(SwitchDataMap.find(1).value().switchStates == 0x01 && SwitchDataMap.find(3).value().switchStates == 0x01)
            {
                if(SwitchDataMap.find(9).value().switchStates == 0x00 && SwitchDataMap.find(15).value().switchStates == 0x00 && SwitchDataMap.find(17).value().switchStates == 0x00)
                {
                    if(SwitchDataMap.find(23).value().switchStates == 0x00)
                    {
                        SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                        SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("IG").value().LockStatus = 0x01;
                        SignalsDataMap.find(1).value().signalStatus = 0x03;

                        if(it.value().sectionStatus == 0x01)
                        {
                            SignalsDataMap.find(1).value().signalStatus = 0x01;
                        }
                    }
                    if(SwitchDataMap.find(23).value().switchStates == 0x01 && SwitchDataMap.find(25).value().switchStates == 0x01)
                    {
                        SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                        SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3G").value().LockStatus = 0x01;
                        SignalsDataMap.find(1).value().signalStatus = 0x03;

                        if(it.value().sectionStatus == 0x01)
                        {
                            SignalsDataMap.find(1).value().signalStatus = 0x01;
                        }
                    }
                }
                if(SwitchDataMap.find(9).value().switchStates == 0x01 && SwitchDataMap.find(11).value().switchStates == 0x01)
                {
                    if(SwitchDataMap.find(21).value().switchStates == 0x00 && SwitchDataMap.find(25).value().switchStates == 0x00)
                    {
                        SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                        SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3G").value().LockStatus = 0x01;
                        SignalsDataMap.find(1).value().signalStatus = 0x03;

                        if(it.value().sectionStatus == 0x01)
                        {
                            SignalsDataMap.find(1).value().signalStatus = 0x01;
                        }
                    }
                    if(SwitchDataMap.find(21).value().switchStates == 0x01)
                    {
                        SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                        SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("5G").value().LockStatus = 0x01;
                        SignalsDataMap.find(1).value().signalStatus = 0x03;

                        if(it.value().sectionStatus == 0x01)
                        {
                            SignalsDataMap.find(1).value().signalStatus = 0x01;
                        }
                    }
                }
            }
        }
        //X引导按钮
        if(0x01 == direction)
        {
            if(SwitchDataMap.find(5).value().switchStates == 0x01 && SwitchDataMap.find(7).value().switchStates == 0x01 && SwitchDataMap.find(13).value().switchStates == 0x00 && SwitchDataMap.find(11).value().switchStates == 0x00)
            {
                if(SwitchDataMap.find(25).value().switchStates == 0x00)
                {
                    SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                    SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("3G").value().LockStatus = 0x01;
                    SignalsDataMap.find(2).value().signalStatus = 0x03;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(2).value().signalStatus = 0x01;
                    }
                }
                if(SwitchDataMap.find(21).value().switchStates == 0x01)
                {
                    SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                    SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("5G").value().LockStatus = 0x01;
                    SignalsDataMap.find(2).value().signalStatus = 0x03;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(2).value().signalStatus = 0x01;
                    }
                }
            }
            if(SwitchDataMap.find(5).value().switchStates == 0x00 && SwitchDataMap.find(3).value().switchStates == 0x00 && SwitchDataMap.find(9).value().switchStates == 0x01 && SwitchDataMap.find(11).value().switchStates == 0x01)
            {
                if(SwitchDataMap.find(25).value().switchStates == 0x00)
                {
                    SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                    SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("3G").value().LockStatus = 0x01;
                    SignalsDataMap.find(2).value().signalStatus = 0x03;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(2).value().signalStatus = 0x01;
                    }
                }
                if(SwitchDataMap.find(21).value().switchStates == 0x01)
                {
                    SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                    SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("5G").value().LockStatus = 0x01;
                    SignalsDataMap.find(2).value().signalStatus = 0x03;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(2).value().signalStatus = 0x01;
                    }
                }
            }
            if(SwitchDataMap.find(5).value().switchStates == 0x00 && SwitchDataMap.find(3).value().switchStates == 0x00 && SwitchDataMap.find(9).value().switchStates == 0x00)
            {
                if(SwitchDataMap.find(17).value().switchStates == 0x00)
                {
                    if(SwitchDataMap.find(23).value().switchStates == 0x00)
                    {
                        SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                        SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("IG").value().LockStatus = 0x01;
                        SignalsDataMap.find(2).value().signalStatus = 0x06;

                        if(it.value().sectionStatus == 0x01)//红光带占用
                        {
                            SignalsDataMap.find(2).value().signalStatus = 0x06;
                        }
                    }
                    if(SwitchDataMap.find(23).value().switchStates == 0x01 && SwitchDataMap.find(25).value().switchStates == 0x01)
                    {
                        SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                        SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3G").value().LockStatus = 0x01;
                        SignalsDataMap.find(2).value().signalStatus = 0x03;

                        if(it.value().sectionStatus == 0x01)//红光带占用
                        {
                            SignalsDataMap.find(2).value().signalStatus = 0x01;
                        }
                    }
                }
                if(SwitchDataMap.find(17).value().switchStates == 0x01 && SwitchDataMap.find(19).value().switchStates == 0x01)
                {
                    if(SwitchDataMap.find(27).value().switchStates == 0x00)
                    {
                        SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                        SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("IIG").value().LockStatus = 0x01;
                        SignalsDataMap.find(2).value().signalStatus = 0x03;

                        if(it.value().sectionStatus == 0x01)
                        {
                            SignalsDataMap.find(2).value().signalStatus = 0x01;
                        }
                    }
                    if(SwitchDataMap.find(27).value().switchStates == 0x01 && SwitchDataMap.find(29).value().switchStates == 0x00)
                    {
                        SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                        SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("27/29WG").value().LockStatus = 0x01;
                        SectionsDataMap.find("29DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("4G").value().LockStatus = 0x01;
                        SignalsDataMap.find(2).value().signalStatus = 0x03;

                        if(it.value().sectionStatus == 0x01)
                        {
                            SignalsDataMap.find(2).value().signalStatus = 0x01;
                        }
                    }
                }
            }
        }
        //XD引导按钮
        if(0x03 == direction)
        {
            if(SwitchDataMap.find(7).value().switchStates == 0x00)
            {
                if(SwitchDataMap.find(11).value().switchStates == 0x00 && SwitchDataMap.find(13).value().switchStates == 0x00)
                {
                    if(SwitchDataMap.find(21).value().switchStates == 0x00 && SwitchDataMap.find(25).value().switchStates == 0x00)
                    {
                        SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3G").value().LockStatus = 0x01;
                        SignalsDataMap.find(13).value().signalStatus = 0x06;

                        if(it.value().sectionStatus == 0x01)
                        {
                            SignalsDataMap.find(13).value().signalStatus = 0x01;
                        }
                    }
                    if(SwitchDataMap.find(21).value().switchStates == 0x01)
                    {
                        SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("5G").value().LockStatus = 0x01;
                        SignalsDataMap.find(13).value().signalStatus = 0x03;

                        if(it.value().sectionStatus == 0x01)
                        {
                            SignalsDataMap.find(13).value().signalStatus = 0x01;
                        }
                    }
                }
                if(SwitchDataMap.find(13).value().switchStates == 0x01 && SwitchDataMap.find(15).value().switchStates == 0x01)
                {
                    if(SwitchDataMap.find(17).value().switchStates == 0x00 && SwitchDataMap.find(23).value().switchStates == 0x00)
                    {
                        SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("IG").value().LockStatus = 0x01;
                        SignalsDataMap.find(13).value().signalStatus = 0x03;

                        if(it.value().sectionStatus == 0x01)
                        {
                            SignalsDataMap.find(13).value().signalStatus = 0x01;
                        }
                    }
                    if(SwitchDataMap.find(17).value().switchStates == 0x01 && SwitchDataMap.find(19).value().switchStates == 0x01)
                    {
                        if(SwitchDataMap.find(27).value().switchStates == 0x00)
                        {
                            SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("IIG").value().LockStatus = 0x01;
                            SignalsDataMap.find(13).value().signalStatus = 0x03;

                            if(it.value().sectionStatus == 0x01)
                            {
                                SignalsDataMap.find(13).value().signalStatus = 0x01;
                            }
                        }
                        if(SwitchDataMap.find(27).value().switchStates == 0x01 && SwitchDataMap.find(29).value().switchStates == 0x00)
                        {
                            SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("27/29WG").value().LockStatus = 0x01;
                            SectionsDataMap.find("29DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("4G").value().LockStatus = 0x01;
                            SignalsDataMap.find(13).value().signalStatus = 0x03;

                            if(it.value().sectionStatus == 0x01)
                            {
                                SignalsDataMap.find(13).value().signalStatus = 0x01;
                            }
                        }
                    }
                }
            }
        }
        //S引导按钮
        if(0x04 == direction)
        {
            if(SwitchDataMap.find(8).value().switchStates == 0x00 && SwitchDataMap.find(10).value().switchStates == 0x00)
            {
                if(SwitchDataMap.find(14).value().switchStates == 0x00)
                {
                    SectionsDataMap.find("IIBG").value().LockStatus = 0x01;
                    SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("14DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("IIG").value().LockStatus = 0x01;
                    SignalsDataMap.find(4).value().signalStatus = 0x06;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(4).value().signalStatus = 0x01;
                    }
                }
                if(SwitchDataMap.find(14).value().switchStates == 0x01 && SwitchDataMap.find(29).value().switchStates == 0x00)
                {
                    SectionsDataMap.find("IIBG").value().LockStatus = 0x01;
                    SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("14DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("4G").value().LockStatus = 0x01;
                    SectionsDataMap.find("29DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("27/29WG").value().LockStatus = 0x01;
                    SignalsDataMap.find(4).value().signalStatus = 0x03;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(4).value().signalStatus = 0x01;
                    }
                }
            }
            if(SwitchDataMap.find(10).value().switchStates == 0x01 && SwitchDataMap.find(12).value().switchStates == 0x01)
            {
                if(SwitchDataMap.find(16).value().switchStates == 0x00)
                {
                    SectionsDataMap.find("IIBG").value().LockStatus = 0x01;
                    SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("IG").value().LockStatus = 0x01;
                    SignalsDataMap.find(4).value().signalStatus = 0x03;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(4).value().signalStatus = 0x01;
                    }
                }
                if(SwitchDataMap.find(16).value().switchStates == 0x01)
                {
                    if(SwitchDataMap.find(18).value().switchStates == 0x00)
                    {
                        SectionsDataMap.find("IIBG").value().LockStatus = 0x01;
                        SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("18DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3G").value().LockStatus = 0x01;
                        SignalsDataMap.find(4).value().signalStatus = 0x03;

                        if(it.value().sectionStatus == 0x01)
                        {
                            SignalsDataMap.find(4).value().signalStatus = 0x01;
                        }
                    }
                    if(SwitchDataMap.find(18).value().switchStates == 0x01 && SwitchDataMap.find(20).value().switchStates == 0x01)
                    {
                        if(SwitchDataMap.find(22).value().switchStates == 0x00)
                        {
                            SectionsDataMap.find("IIBG").value().LockStatus = 0x01;
                            SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("18DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("20DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("22DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("5G").value().LockStatus = 0x01;
                            SignalsDataMap.find(4).value().signalStatus = 0x03;

                            if(it.value().sectionStatus == 0x01)
                            {
                                SignalsDataMap.find(4).value().signalStatus = 0x01;
                            }
                        }
                    }
                }
            }
        }
        //SF引导按钮
        if(0x05 == direction)
        {
            if(SwitchDataMap.find(4).value().switchStates == 0x00)
            {
                if(SwitchDataMap.find(6).value().switchStates == 0x00 && SwitchDataMap.find(12).value().switchStates == 0x00)
                {
                    if(SwitchDataMap.find(16).value().switchStates == 0x00)
                    {
                        SectionsDataMap.find("4DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("IG").value().LockStatus = 0x01;
                        SignalsDataMap.find(3).value().signalStatus = 0x06;

                        if(it.value().sectionStatus == 0x01)
                        {
                            SignalsDataMap.find(3).value().signalStatus = 0x01;
                        }
                    }
                    if(SwitchDataMap.find(16).value().switchStates == 0x01)
                    {
                        if(SwitchDataMap.find(18).value().switchStates == 0x00)
                        {
                            SectionsDataMap.find("4DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("18DG").value().LockStatus = 0x01;
                            SectionsDataMap.find("3G").value().LockStatus = 0x01;
                            SignalsDataMap.find(3).value().signalStatus = 0x03;

                            if(it.value().sectionStatus == 0x01)
                            {
                                SignalsDataMap.find(3).value().signalStatus = 0x01;
                            }
                        }
                        if(SwitchDataMap.find(18).value().switchStates == 0x01 && SwitchDataMap.find(20).value().switchStates == 0x01)
                        {
                            if(SwitchDataMap.find(22).value().switchStates == 0x00)
                            {
                                SectionsDataMap.find("4DG").value().LockStatus = 0x01;
                                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                                SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                                SectionsDataMap.find("18DG").value().LockStatus = 0x01;
                                SectionsDataMap.find("20DG").value().LockStatus = 0x01;
                                SectionsDataMap.find("22DG").value().LockStatus = 0x01;
                                SectionsDataMap.find("5G").value().LockStatus = 0x01;
                                SignalsDataMap.find(3).value().signalStatus = 0x03;

                                if(it.value().sectionStatus == 0x01)
                                {
                                    SignalsDataMap.find(3).value().signalStatus = 0x01;
                                }
                            }
                        }
                    }
                }
                if(SwitchDataMap.find(6).value().switchStates == 0x01 && SwitchDataMap.find(8).value().switchStates == 0x01)
                {
                    if(SwitchDataMap.find(14).value().switchStates == 0x00)
                    {
                        SectionsDataMap.find("4DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("14DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("IIG").value().LockStatus = 0x01;
                        SignalsDataMap.find(3).value().signalStatus = 0x03;

                        if(it.value().sectionStatus == 0x01)
                        {
                            SignalsDataMap.find(3).value().signalStatus = 0x01;
                        }
                    }
                    if(SwitchDataMap.find(14).value().switchStates == 0x01 && SwitchDataMap.find(29).value().switchStates == 0x00)
                    {
                        SectionsDataMap.find("4DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("14DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("4G").value().LockStatus = 0x01;
                        SectionsDataMap.find("29DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("27/29WG").value().LockStatus = 0x01;
                        SignalsDataMap.find(3).value().signalStatus = 0x03;

                        if(it.value().sectionStatus == 0x01)
                        {
                            SignalsDataMap.find(3).value().signalStatus = 0x01;
                        }
                    }
                }
            }
        }
    }
}

//总人解
void InterLock::RenGong(byte beginSignalID)
{
   /* byte beginID = beginSignalID;
    RemoveRoute(beginID);*/
    QString beginSignalName;
    byte Type;
    //查询起始信号机ID对应名称
    QString selectBeginSignalStr = QString("select * from signalinfo WHERE signalinfo.SignalID = %1").arg(beginSignalID);
    QSqlQuery queryBeginSignal(selectBeginSignalStr);
    while(queryBeginSignal.next())
    {
        beginSignalName = queryBeginSignal.value(3).toString();
    }

    QString selectBeginSignal = QString("select * from linerule WHERE linerule.StarSignalName = %1").arg(beginSignalName);
    QSqlQuery queryBeginSignal1(selectBeginSignal);
    while(queryBeginSignal1.next())
    {
        Type = queryBeginSignal1.value(3).toInt();
    }
    if(RuleDataMap.find(beginSignalName).value().Type == 0 || RuleDataMap.find(beginSignalName).value().Type == 1 || RuleDataMap.find(beginSignalName).value().Type == 2)
    {
        QMap<QString,QString>::iterator itstr = LockRouteMap.find(beginSignalName);
        while(itstr != LockRouteMap.end()&&itstr.key() == beginSignalName)
        {
            QString str = itstr.value();
            QStringList strSplit = str.split(",");
            for(int i=0; i<strSplit.length(); i++)
            {
                QMap<QString,SectionData>::iterator it = SectionsDataMap.find(strSplit[i]); //找到特定的“键-值”对
                it.value().LockStatus = 0x10;
            }
            itstr++;
        }
        LockRouteMap.remove(beginSignalName);
        SignalsDataMap.find(beginSignalID).value().signalStatus = Red;
    }
}

//分路不良
void InterLock::FenLu(int Snum)
{

}

//调车进路
void InterLock::DiaoChe()
{
    QMap<QString,QString>::iterator itstr;
    for(itstr = LockRouteMap.begin();itstr != LockRouteMap.end();++itstr)
    {
        if("D1,D9" == itstr.key())
        {
            SectionsDataMap.find("1DG").value().LockStatus = 0x01;
            SectionsDataMap.find("3DG").value().LockStatus = 0x01;
            SignalsDataMap.find(101).value().signalStatus = 0x08;//白
        }
        if("D1,D15" == itstr.key())
        {
            SectionsDataMap.find("1DG").value().LockStatus = 0x01;
            SectionsDataMap.find("1/19WG").value().LockStatus = 0x01;
            SignalsDataMap.find(101).value().signalStatus = 0x08;
        }
        if("D3,D9" == itstr.key())
        {
            SectionsDataMap.find("3DG").value().LockStatus = 0x01;
            SectionsDataMap.find("5DG").value().LockStatus = 0x01;
            SignalsDataMap.find(103).value().signalStatus = 0x08;
        }
        if("D3,D11" == itstr.key())
        {
            SectionsDataMap.find("5DG").value().LockStatus = 0x01;
            SectionsDataMap.find("7DG").value().LockStatus = 0x01;
            SignalsDataMap.find(103).value().signalStatus = 0x08;
        }
        if("D5,D1" == itstr.key())
        {
            SectionsDataMap.find("1DG").value().LockStatus = 0x01;
            SignalsDataMap.find(105).value().signalStatus = 0x08;
        }
        if("D7,D3" == itstr.key())
        {
            SectionsDataMap.find("3DG").value().LockStatus = 0x01;
            SectionsDataMap.find("5DG").value().LockStatus = 0x01;
            SignalsDataMap.find(107).value().signalStatus = 0x08;
        }
        if("D7,D1" == itstr.key())
        {
            SectionsDataMap.find("3DG").value().LockStatus = 0x01;
            SectionsDataMap.find("1DG").value().LockStatus = 0x01;
            SignalsDataMap.find(107).value().signalStatus = 0x08;
        }
        if("D9,S5" == itstr.key())
        {
            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
            SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
            SectionsDataMap.find("21DG").value().LockStatus = 0x01;
            SignalsDataMap.find(109).value().signalStatus = 0x08;
        }
        if("D9,D13" == itstr.key())
        {
            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
            SignalsDataMap.find(109).value().signalStatus = 0x08;
        }
        if("D11,D13" == itstr.key())
        {
            SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
            SignalsDataMap.find(111).value().signalStatus = 0x08;
        }
        if("D11,S5" == itstr.key())
        {
            SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
            SectionsDataMap.find("21DG").value().LockStatus = 0x01;
            SignalsDataMap.find(111).value().signalStatus = 0x08;
        }
        if("D11,S3" == itstr.key())
        {
            SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
            SectionsDataMap.find("21DG").value().LockStatus = 0x01;
            SectionsDataMap.find("25DG").value().LockStatus = 0x01;
            SignalsDataMap.find(111).value().signalStatus = 0x08;
        }
        if("D13,S3" == itstr.key())
        {
            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
            SectionsDataMap.find("25DG").value().LockStatus = 0x01;
            SignalsDataMap.find(113).value().signalStatus = 0x08;
        }
        if("D13,SI" == itstr.key())
        {
            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
            SignalsDataMap.find(113).value().signalStatus = 0x08;
        }
        if("D13,SII" == itstr.key())
        {
            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
            SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
            SignalsDataMap.find(113).value().signalStatus = 0x08;
        }
        if("D13,S4" == itstr.key())
        {
            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
            SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
            SignalsDataMap.find(113).value().signalStatus = 0x08;
        }
        if("D15,SII" == itstr.key())
        {
            SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
            SignalsDataMap.find(115).value().signalStatus = 0x08;
        }
        if("D15,S4" == itstr.key())
        {
            SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
            SignalsDataMap.find(115).value().signalStatus = 0x08;
        }
        if("D17,D19" == itstr.key())
        {
            SectionsDataMap.find("29DG").value().LockStatus = 0x01;
            SignalsDataMap.find(117).value().signalStatus = 0x08;
        }
        if("D17,D21" == itstr.key())
        {
            SectionsDataMap.find("29DG").value().LockStatus = 0x01;
            SignalsDataMap.find(117).value().signalStatus = 0x08;
        }
        if("D19,S4" == itstr.key())
        {
            SectionsDataMap.find("29DG").value().LockStatus = 0x01;
            SectionsDataMap.find("27/29WG").value().LockStatus= 0x01;
            SignalsDataMap.find(119).value().signalStatus = 0x08;
        }
        if("D21,S4" == itstr.key())
        {
            SectionsDataMap.find("29DG").value().LockStatus = 0x01;
            SectionsDataMap.find("27/29WG").value().LockStatus = 0x01;
            SignalsDataMap.find(121).value().signalStatus = 0x08;
        }
        if("S5,D7" == itstr.key())
        {
            SectionsDataMap.find("21DG").value().LockStatus = 0x01;
            SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
            SignalsDataMap.find(14).value().signalStatus = 0x08;
        }
        if("S5,XD" == itstr.key())
        {
            SectionsDataMap.find("21DG").value().LockStatus = 0x01;
            SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
            SectionsDataMap.find("7DG").value().LockStatus = 0x01;
            SignalsDataMap.find(14).value().signalStatus = 0x08;
        }
        if("S5,D3" == itstr.key())
        {
            SectionsDataMap.find("21DG").value().LockStatus = 0x01;
            SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
            SectionsDataMap.find("7DG").value().LockStatus = 0x01;
            SectionsDataMap.find("5DG").value().LockStatus = 0x01;
            SignalsDataMap.find(14).value().signalStatus = 0x08;
        }
        if("S3,D7" == itstr.key())
        {
            SectionsDataMap.find("25DG").value().LockStatus = 0x01;
            SectionsDataMap.find("11-23DG").value().LockStatus = 0x01;
            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
            SignalsDataMap.find(10).value().signalStatus = 0x08;
        }
        if("S3,D3" == itstr.key())
        {
            SectionsDataMap.find("25DG").value().LockStatus = 0x01;
            SectionsDataMap.find("21DG").value().LockStatus = 0x01;
            SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
            SectionsDataMap.find("7DG").value().LockStatus = 0x01;
            SectionsDataMap.find("5DG").value().LockStatus = 0x01;
            SignalsDataMap.find(10).value().signalStatus = 0x08;
        }
        if("S3,XD" == itstr.key())
        {
            SectionsDataMap.find("25DG").value().LockStatus = 0x01;
            SectionsDataMap.find("21DG").value().LockStatus= 0x01;
            SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
            SectionsDataMap.find("7DG").value().LockStatus = 0x01;
            SignalsDataMap.find(10).value().signalStatus = 0x08;
        }
        if("SI,D7" == itstr.key())
        {
            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
            SignalsDataMap.find(9).value().signalStatus = 0x08;
        }
        if("SI,D3" == itstr.key())
        {
            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
            SectionsDataMap.find("3DG").value().LockStatus = 0x01;
            SectionsDataMap.find("5DG").value().LockStatus = 0x01;
            SignalsDataMap.find(9).value().signalStatus = 0x08;
        }
        if("SI,XD" == itstr.key())
        {
            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
            SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
            SectionsDataMap.find("7DG").value().LockStatus = 0x01;
            SignalsDataMap.find(9).value().signalStatus = 0x08;
        }
        if("SII,D7" == itstr.key())
        {
            SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
            SignalsDataMap.find(11).value().signalStatus = 0x08;
        }
        if("SII,D5" == itstr.key())
        {
            SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
            SectionsDataMap.find("1/19WG").value().LockStatus = 0x01;
            SignalsDataMap.find(11).value().signalStatus = 0x08;
        }
        if("SII,D3" == itstr.key())
        {
            SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
            SectionsDataMap.find("3DG").value().LockStatus = 0x01;
            SectionsDataMap.find("5DG").value().LockStatus = 0x01;
            SignalsDataMap.find(11).value().signalStatus = 0x08;
        }
        if("SII,XD" == itstr.key())
        {
            SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
            SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
            SectionsDataMap.find("7DG").value().LockStatus = 0x01;
            SignalsDataMap.find(11).value().signalStatus = 0x08;
        }
        if("S4,D3" == itstr.key())
        {
            SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
            SectionsDataMap.find("3DG").value().LockStatus = 0x01;
            SectionsDataMap.find("5DG").value().LockStatus = 0x01;
            SignalsDataMap.find(12).value().signalStatus = 0x08;
        }
        if("S4,D5" == itstr.key())
        {
            SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
            SectionsDataMap.find("1/19WG").value().LockStatus = 0x01;
            SignalsDataMap.find(12).value().signalStatus = 0x08;
        }
        if("S4,D7" == itstr.key())
        {
            SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
            SignalsDataMap.find(12).value().signalStatus = 0x08;
        }
        if("S4,XD" == itstr.key())
        {
            SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
            SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
            SectionsDataMap.find("7DG").value().LockStatus  = 0x01;
            SignalsDataMap.find(12).value().signalStatus = 0x08;
        }
        if("D2,D8" == itstr.key())
        {
            SectionsDataMap.find("2DG").value().LockStatus = 0x01;
            SectionsDataMap.find("4DG").value().LockStatus = 0x01;
            SignalsDataMap.find(102).value().signalStatus = 0x08;
        }
        if("D2,D14" == itstr.key())
        {
            SectionsDataMap.find("2DG").value().LockStatus = 0x01;
            SectionsDataMap.find("2/20WG").value().LockStatus = 0x01;
            SignalsDataMap.find(102).value().signalStatus = 0x08;
        }
        if("D4,D12" == itstr.key())
        {
            SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
            SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
            SignalsDataMap.find(104).value().signalStatus = 0x08;
        }
        if("D4,D19" == itstr.key())
        {
            SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
            SectionsDataMap.find("14DG").value().LockStatus= 0x01;
            SectionsDataMap.find("4G").value().LockStatus = 0x01;
            SignalsDataMap.find(104).value().signalStatus = 0x08;
        }
        if("D4,XII" == itstr.key())
        {
            SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
            SectionsDataMap.find("14DG").value().LockStatus = 0x01;
            SignalsDataMap.find(104).value().signalStatus = 0x08;
        }
        if("D6,D2" == itstr.key())
        {
            SectionsDataMap.find("2DG").value().LockStatus = 0x01;
            SignalsDataMap.find(106).value().signalStatus = 0x08;
        }
        if("D8,D12" == itstr.key())
        {
            SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
            SignalsDataMap.find(108).value().signalStatus = 0x08;
        }
        if("D8,XII" == itstr.key())
        {
            SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
            SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
            SectionsDataMap.find("14DG").value().LockStatus = 0x01;
            SignalsDataMap.find(108).value().signalStatus = 0x08;
        }
        if("D8,D19" == itstr.key())
        {
            SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
            SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
            SectionsDataMap.find("14DG").value().LockStatus = 0x01;
            SectionsDataMap.find("4G").value().LockStatus = 0x01;
            SignalsDataMap.find(108).value().signalStatus = 0x08;
        }
        if("D10,D4" == itstr.key())
        {
            SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
            SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
            SignalsDataMap.find(110).value().signalStatus = 0x08;
        }
        if("D10,SF" == itstr.key())
        {
            SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
            SectionsDataMap.find("4DG").value().LockStatus = 0x01;
            SignalsDataMap.find(110).value().signalStatus = 0x08;
        }
        if("D10,D2" == itstr.key())
        {
            SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
            SectionsDataMap.find("4DG").value().LockStatus = 0x01;
            SectionsDataMap.find("2DG").value().LockStatus = 0x01;
            SignalsDataMap.find(110).value().signalStatus = 0x08;
        }
        if("D12,D16" == itstr.key())
        {
            SectionsDataMap.find("16DG").value().LockStatus = 0x01;
            SectionsDataMap.find("18DG").value().LockStatus = 0x01;
            SectionsDataMap.find("20DG").value().LockStatus = 0x01;
            SignalsDataMap.find(112).value().signalStatus = 0x08;
        }
        if("D12,X3" == itstr.key())
        {
            SectionsDataMap.find("16DG").value().LockStatus = 0x01;
            SectionsDataMap.find("18DG").value().LockStatus = 0x01;
            SignalsDataMap.find(112).value().signalStatus = 0x08;
        }
        if("D12,XI" == itstr.key())
        {
            SectionsDataMap.find("16DG").value().LockStatus = 0x01;
            SignalsDataMap.find(112).value().signalStatus = 0x08;
        }
        if("D14,D16" == itstr.key())
        {
            SectionsDataMap.find("20DG").value().LockStatus = 0x01;
            SignalsDataMap.find(114).value().signalStatus = 0x08;
        }
        if("D16,X5" == itstr.key())
        {
            SectionsDataMap.find("22DG").value().LockStatus = 0x01;
            SignalsDataMap.find(116).value().signalStatus = 0x08;
        }
        if("D16,D18" == itstr.key())
        {
            SectionsDataMap.find("22DG").value().LockStatus = 0x01;
            SignalsDataMap.find(116).value().signalStatus = 0x08;
        }
        if("D18,D6" == itstr.key())
        {
            SectionsDataMap.find("22DG").value().LockStatus = 0x01;
            SectionsDataMap.find("20DG").value().LockStatus = 0x01;
            SectionsDataMap.find("2/20WG").value().LockStatus = 0x01;
            SignalsDataMap.find(118).value().signalStatus = 0x08;
        }
        if("D18,D10" == itstr.key())
        {
            SectionsDataMap.find("22DG").value().LockStatus = 0x01;
            SectionsDataMap.find("20DG").value().LockStatus = 0x01;
            SectionsDataMap.find("18DG").value().LockStatus = 0x01;
            SectionsDataMap.find("16DG").value().LockStatus = 0x01;
            SignalsDataMap.find(118).value().signalStatus = 0x08;
        }
        if("X5,D6" == itstr.key())
        {
            SectionsDataMap.find("22DG").value().LockStatus = 0x01;
            SectionsDataMap.find("20DG").value().LockStatus= 0x01;
            SectionsDataMap.find("2/20WG").value().LockStatus = 0x01;
            SignalsDataMap.find(15).value().signalStatus = 0x08;
        }
        if("X5,D10" == itstr.key())
        {
            SectionsDataMap.find("22DG").value().LockStatus = 0x01;
            SectionsDataMap.find("20DG").value().LockStatus = 0x01;
            SectionsDataMap.find("18DG").value().LockStatus = 0x01;
            SectionsDataMap.find("16DG").value().LockStatus = 0x01;
            SignalsDataMap.find(15).value().signalStatus = 0x08;
        }
        if("X3,D10" == itstr.key())
        {
            SectionsDataMap.find("18DG").value().LockStatus = 0x01;
            SectionsDataMap.find("16DG").value().LockStatus = 0x01;
            SignalsDataMap.find(6).value().signalStatus = 0x08;
        }
        if("XI,D10" == itstr.key())
        {
            SectionsDataMap.find("16DG").value().LockStatus = 0x01;
            SignalsDataMap.find(8).value().signalStatus = 0x08;
        }
        if("XII,D4" == itstr.key())
        {
            SectionsDataMap.find("14DG").value().LockStatus = 0x01;
            SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
            SignalsDataMap.find(5).value().signalStatus = 0x08;
        }
        if("XII,SF" == itstr.key())
        {
            SectionsDataMap.find("14DG").value().LockStatus = 0x01;
            SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
            SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
            SectionsDataMap.find("4DG").value().LockStatus = 0x01;
            SignalsDataMap.find(5).value().signalStatus = 0x08;
        }
        if("XII,D2" == itstr.key())
        {
            SectionsDataMap.find("14DG").value().LockStatus = 0x01;
            SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
            SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
            SectionsDataMap.find("4DG").value().LockStatus = 0x01;
            SectionsDataMap.find("2DG").value().LockStatus = 0x01;
            SignalsDataMap.find(5).value().signalStatus = 0x08;
        }
        if("X4,D4" == itstr.key())
        {
            SectionsDataMap.find("14DG").value().LockStatus = 0x01;
            SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
            SignalsDataMap.find(7).value().signalStatus = 0x08;
        }
        if("X4,SF" == itstr.key())
        {
            SectionsDataMap.find("14DG").value().LockStatus = 0x01;
            SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
            SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
            SectionsDataMap.find("4DG").value().LockStatus = 0x01;
            SignalsDataMap.find(7).value().signalStatus = 0x08;
        }
        if("X4,D2" == itstr.key())
        {
            SectionsDataMap.find("14DG").value().LockStatus = 0x01;
            SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
            SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
            SectionsDataMap.find("4DG").value().LockStatus = 0x01;
            SectionsDataMap.find("2DG").value().LockStatus = 0x01;
            SignalsDataMap.find(7).value().signalStatus = 0x08;
        }
       // RuleDataMap1.find().value().Type = 3;
    }
}

//故障设置
void InterLock::GuZhang(byte Nameid,byte Sstatus)
{
    byte statue = Sstatus;
    QString secname;
    QString sectionforswitch;
    QString sectionforswitch2;
    byte switchid;
    QString selectnameid = QString("select * from section WHERE section.sectionnameid = %1").arg(Nameid);
    QSqlQuery querynameid(selectnameid);
    while(querynameid.next())
    {
        secname = querynameid.value(1).toString();
        sectionforswitch = querynameid.value(6).toString();
        sectionforswitch2 = querynameid.value(7).toString();
    }
//    if(statue == 0x01)//红光带故障
//    {
//        if(SectionsDataMap.find(secname).value().sectionForSwitch == NULL)
//        {
//            SectionsDataMap.find(secname).value().sectionStatus = 0x03;//红光带故障
//        }
//        else
//        {
//            if(SectionsDataMap.find(secname).value().sectionForSwitch2 == NULL)
//            {
//                QString selectswitchid = QString("select * from switch WHERE switch.SwitchID = %1").arg(sectionforswitch);
//                QSqlQuery queryswitchid(selectswitchid);
//                while(queryswitchid.next())
//                {
//                    switchid == queryswitchid.value(0).toString();
//                }
//                if(SwitchDataMap.find(switchid).value().switchStates == 0x01)
//                {
//                    SectionsDataMap.find(secname).value().sectionStatus = 0x03;
//                }
//            }
//            else if(SectionsDataMap.find(secname).value().sectionForSwitch2 != NULL)
//            {
//                QString selectswitchid = QString("select * from switch WHERE switch.SwitchID = %1").arg(sectionforswitch2);
//                QSqlQuery queryswitchid(selectswitchid);
//                while(queryswitchid.next())
//                {
//                    switchid == queryswitchid.value(0).toString();
//                }
//                if(SwitchDataMap.find(switchid).value().switchStates == 0x01)
//                {
//                    SectionsDataMap.find(secname).value().sectionStatus = 0x03;
//                }
//            }
//        }
//    }
//    if(statue == 0x02)//白光带故障
//    {
//        if(SectionsDataMap.find(secname).value().sectionForSwitch == NULL)
//        {
//            SectionsDataMap.find(secname).value().LockStatus = 0x04;//白光带故障
//        }
//        else
//        {
//            if(SectionsDataMap.find(secname).value().sectionForSwitch2 == NULL)
//            {
//                QString selectswitchid = QString("select * from switch WHERE switch.SwitchID = %1").arg(sectionforswitch);
//                QSqlQuery queryswitchid(selectswitchid);
//                while(queryswitchid.next())
//                {
//                    switchid == queryswitchid.value(0).toString();
//                }
//                if(SwitchDataMap.find(switchid).value().switchStates == 0x01)
//                {
//                    SectionsDataMap.find(secname).value().LockStatus = 0x04;
//                }
//            }
//            else if(SectionsDataMap.find(secname).value().sectionForSwitch2 != NULL)
//            {
//                QString selectswitchid = QString("select * from switch WHERE switch.SwitchID = %1").arg(sectionforswitch2);
//                QSqlQuery queryswitchid(selectswitchid);
//                while(queryswitchid.next())
//                {
//                    switchid == queryswitchid.value(0).toString();
//                }
//                if(SwitchDataMap.find(switchid).value().switchStates == 0x01)
//                {
//                    SectionsDataMap.find(secname).value().LockStatus = 0x04;
//                }
//            }
//        }
//    }
}

//闭塞操作
void InterLock::BiSe(byte Direction)
{
    byte direction = Direction;
    if(0x01 == direction)//X
    {}
    if(0x02 == direction)//XF
    {}
    if(0x03 == direction)//XD
    {}
    if(0x04 == direction)//S
    {}
    if(0x05 == direction)//SF
    {}
}


//设置进路
void InterLock::SetupRoute(byte beginSignalID,byte endSignalID)
{
    QString beginSignalName = QString();
    QString endSignalName = QString();
    QString sectionsStr;//建立进路经过的区段
    QString switchesStr;//建立进路经过的道岔
    //查询起始信号机ID对应名称
    QString selectBeginSignalStr = QString("select * from signalinfo WHERE signalinfo.SignalID = %1").arg(beginSignalID);
    QSqlQuery queryBeginSignal(selectBeginSignalStr);
    while(queryBeginSignal.next())
    {
        beginSignalName = queryBeginSignal.value(3).toString();
    }
    //查询结束信号机ID对应名称
    QString selectEndSignalStr = QString("select * from signalinfo WHERE signalinfo.SignalID = %1").arg(endSignalID);
    QSqlQuery queryEndSignal(selectEndSignalStr);
    while(queryEndSignal.next())
    {
        endSignalName = queryEndSignal.value(3).toString();
    }
    //获取建立进路信息
    QString selectInterlockRouteStr = QString("select * from interlocking WHERE interlocking.BeginSignal = \"%1\" AND interlocking.EndSignal = \"%2\"").arg(beginSignalName).arg(endSignalName);
    QSqlQuery queryInterlockRoute(selectInterlockRouteStr);
    while(queryInterlockRoute.next())
    {
        sectionsStr = queryInterlockRoute.value(5).toString();
        qDebug()<<"str"<<sectionsStr;
        switchesStr = queryInterlockRoute.value(7).toString();
        qDebug()<<"str"<<switchesStr;
    }
    if(sectionsStr.isEmpty() || switchesStr.isEmpty())
    {
        //QByteArray nullInfo("\xFF",1);进路不存在!
        //udpSocket_receive->writeDatagram(nullInfo,QHostAddress("127.0.0.1"),4402);
        return;
    }
    //检查建立进路中区段是否占用及锁闭
    QStringList sectionsSplit = sectionsStr.split(",");
    for(int i=0; i<sectionsSplit.length(); i++)
    {
        QMap<QString,SectionData>::iterator it = SectionsDataMap.find(sectionsSplit[i]); //找到特定的“键-值”对
        if(0x01 == it.value().sectionStatus || 0x01 == it.value().LockStatus)
        {
            qDebug()<<it.value().sectionStatus<<it.value().LockStatus;
            //QByteArray nullInfo("\xFF",1);区段占用或锁闭
            //udpSocket_receive->writeDatagram(nullInfo,QHostAddress("127.0.0.1"),4402);
            return;
        }
    }
    //锁闭区段
    QMap<QString,SectionData>::iterator it;
    for(int i=0; i<sectionsSplit.length(); i++)
    {
        it = SectionsDataMap.find(sectionsSplit[i]); //找到特定的“键-值”对
        it.value().LockStatus = 0x01;
    }

    LockRouteMap.insert(beginSignalName,sectionsStr);
    QByteArray switchesInfo("\x10\x00\x00\x00\x00\x12\x10\x01\x4a\x98\x07\x03\x10\x00\x10\x00\x10\x00\x59\x4F\x44\x4F",22);
    QByteArray num = switchesStrextract(switchesStr);

    udpSocket->writeDatagram(switchesInfo,QHostAddress("127.0.0.1"),4402);
//    QString signalStr = QString("%1,%2").arg(beginSignalName).arg(endSignalName);
//    LockRouteDirectionMap.insert(signalStr,sectionsStr);
//    InterEncapsulation(XF_Direction,X_Direction,SF_Direction,S_Direction);
}


//进路信息缓存，暂定01为锁定,02为无状态
void InterLock::InterEncapsalutation(byte beginSignalID,byte endSignalID)
{
    QString beginSignalName = QString();
    QString endSignalName = QString();

    QString SectionNames;
    QString SwitchNames;

    QList<SectionData> Sections;
    QList<SwitchData> Switchs;
    LineRuleData ruledata;
    SectionData sdata;
    QString lineruleid;
    QString First;
    int switchname;

    //起点信号机ID取得起点信号机名字
    QString selectbeginsignalid = QString("select * from signalinfo WHERE signalinfo.SignalID = %1").arg(beginSignalID);
    QSqlQuery querybeginsignalid(selectbeginsignalid);
    while(querybeginsignalid.next())
    {
        beginSignalName = querybeginsignalid.value(3).toString();
    }
    //终点信号机ID取得终点信号机名字
    QString selectendsignalid = QString("select * from signalinfo WHERE signalinfo.SignalID = %1").arg(endSignalID);
    QSqlQuery queryendsignalid(selectendsignalid);
    while(queryendsignalid.next())
    {
        endSignalName = queryendsignalid.value(3).toString();
    }

    //进路中所有的道岔
    QString selectinterlock = QString("select * from interlockinginfo WHERE BeginSignal = \"%1\" AND EndSignal = \"%2\"").arg(beginSignalName).arg(endSignalName);
    QSqlQuery queryinterlocking(selectinterlock);

    while(queryinterlocking.next())
    {
        First = queryinterlocking.value(9).toString();
        //条件判断,暂时设置First = 1
        lineruleid = queryinterlocking.value(0).toString();
        SectionNames = queryinterlocking.value(5).toString();
        SwitchNames = queryinterlocking.value(7).toString();
        if(SectionNames.count() == 0 || SwitchNames.count() == 0)
        {
            return ;
        }
        //拆分区段名字
        QStringList sectionsplit = SectionNames.split(",");
        foreach (QString sectionname, sectionsplit) {
            QString sectionid;
            QString selectsection = QString("SELECT * FROM sectioninfo WHERE SectionName = \"%1\"").arg(sectionname);
            QSqlQuery querysection(selectsection);
            while(querysection.next())
            {
                sectionid = querysection.value(0).toString();
                SectionsDataMap.find(sectionid).value().LockStatus = 0x01;   //对象锁闭状态
                sdata = SectionsDataMap.find(sectionid).value(); //从缓存中找到以拆分出的区段为名的对象
                //Sections.append(sdata);                            //加入进路区段集合中
            }
        }
        LockRouteMap[beginSignalName] = SectionNames;

        int i = 0;
        QMap<int,SwitchData>::iterator switchit;
        QByteArray num = switchesStrextract(SwitchNames);
        for(int k=0;k<num.count();k+=2){
            switchname = num[k];
            for(switchit = SwitchDataMap.begin();switchit != SwitchDataMap.end();++switchit)
            {
                if(num[k+1].operator == (0x01)){//反位
                    if(switchit.value().switchName == switchname && switchit.value().switchPos == 1){
                        switchit.value().switchLock = 0x01;
                        //Switchs.append(switchit.value());
                    }
                }else if(num[k+1].operator == (0x02)){ //定位
                    if(switchit.value().switchName == switchname && switchit.value().switchPos == 0){
                        switchit.value().switchLock = 0x01;
                        //Switchs.append(switchit.value());
                    }
                }
                i++;
            }
        }
        LockSwitchMap[beginSignalName] = SwitchNames;
        //为当前进路赋值：起点终点信号机名字、规则序号、规则ID、规则内所有的区段、规则内所有的道岔
//        ruledata.beginSignalName = beginSignalName;
//        ruledata.endSignalName = endSignalName;
//        ruledata.First = First.toInt();
//        ruledata.lineruleID = lineruleid.toInt();
//        ruledata.sections = Sections;
//        ruledata.switchs = Switchs;
//        NewRuleDataMap[ruledata.lineruleID] = ruledata;
    }

//    QString selectlinerule = QString("select * from linerule WHERE StarSignalName = \"%1\" AND EndSignalName = \"%2\"").arg(beginSignalName).arg(endSignalName);
//    QSqlQuery querylinerule(selectlinerule);
//    int len = querylinerule.size();
//    while(querylinerule.next())
//    {
//        lineruleid = querylinerule.value(0).toString();
//        First = querylinerule.value(4).toString();
//        QString lineforsection = QString("select * FROM lineruleforsection ls INNER JOIN section s ON ls.SectionID = s.SectionID WHERE ls.LineruleID = %1").arg(lineruleid);
//        QSqlQuery querylineforsection(lineforsection);
//        while(querylineforsection.next())
//        {
//            sdata.sectionId = querylineforsection.value(2).toInt();
//            sdata.sectionName = querylineforsection.value(4).toString();
//            sdata.sectionWee = querylineforsection.value(5).toString();
//            sdata.LockStatus = querylineforsection.value(6).toString();
//            sdata.LockStatus = querylineforsection.value(6).toString();
//        }
//        QSqlQuery querylinerule(selectlinerule);
//        //RuleDataMap[lineruleid] =

//    }
//    for(var i = 0 ;i<len;i++){
//        lineruleid = querylinerule.value(0).toInt();
//        First = querylinerule.value(4).toInt();
//    }
//    if(len == 1)
//    {
//        QString selectsectionfirst = QString("select * from interlockinginfo WHERE interlockinginfo.First1 = %1").arg(First);
//        QSqlQuery querysectionfirst(selectsectionfirst);
//        while(querysectionfirst.next())
//        {
//            First1 = querysectionfirst.value(9).toString();
//        }
//        if(First == First1)
//        {
//            if(Sections.isEmpty() || Switchs.isEmpty())
//            {
//                return ;
//            }
//            else
//            {
//                for(int i=0;i<sectionsplit.length();i++)
//                {
//                    QMap<QString,SectionData>::iterator it = SectionsDataMap.find(sectionsplit[i]);
//                    if(0x01 == it.value().LockStatus || 0x01 == it.value().sectionStatus || 0x01 == it.value().blockStatus)
//                    {
//                        return ;
//                    }
//                    else
//                    {
//                        SectionsDataMap.find(sectionsplit[i]).value().LockStatus = 0x01;
//                        if()
//                        {
//                            SignalsDataMap.find(beginSignalID).value().signalStatus = 0x03;//黄
//                        }
//                        else
//                        {
//                            SignalsDataMap.find(beginSignalID).value().signalStatus = 0x04;//双黄闪
//                        }
//                    }
//                }
//            }
//        }
//    }
//    if(len == 2)
//    {
//        QString selectsectionfirst = QString("select * from interlockinginfo WHERE interlockinginfo.First1 = %1").arg(First);
//        QSqlQuery querysectionfirst(selectsectionfirst);
//        while(querysectionfirst.next())
//        {
//            First1 = querysectionfirst.value(9).toString();
//        }
//        if(First == First1)
//        {
//            if(First1 == 1)
//            {
//                if(Sections.isEmpty() || Switchs.isEmpty())
//                {
//                    First1 = 2;
//                }
//                else
//                {
//                    for(int i=0;i<sectionsplit.length();i++)
//                    {
//                        QMap<QString,SectionData>::iterator it = SectionsDataMap.find(sectionsplit[i]);
//                        if(0x01 == it.value().LockStatus || 0x01 == it.value().sectionStatus || 0x01 == it.value().blockStatus)
//                        {
//                            First1 = 2;
//                        }
//                        else
//                        {
//                            SectionsDataMap.find(sectionsplit[i]).value().LockStatus = 0x01;
//                            if(Switchs)
//                            {
//                                SignalsDataMap.find(beginSignalID).value().signalStatus = 0x03;
//                            }
//                            else
//                            {
//                                SignalsDataMap.find(beginSignalID).value().signalStatus = 0x04;
//                            }
//                        }
//                    }
//                }
//            }
//            else if(First1 == 2)
//            {
//                if(Sections.isEmpty() || Switchs.isEmpty())
//                {
//                    return ;
//                }
//                else
//                {
//                    for(int i=0;i<sectionsplit.length();i++)
//                    {
//                        QMap<QString,SectionData>::iterator it = SectionsDataMap.find(sectionsplit[i]);
//                        if(0x01 == it.value().LockStatus || 0x01 == it.value().sectionStatus || 0x01 == it.value().blockStatus)
//                        {
//                            return ;
//                        }
//                        else
//                        {
//                            SectionsDataMap.find(sectionsplit[i]).value().LockStatus = 0x01;
//                            if()
//                            {
//                                SignalsDataMap.find(beginSignalID).value().signalStatus = 0x03;
//                            }
//                            else
//                            {
//                                SignalsDataMap.find(beginSignalID).value().signalStatus = 0x04;
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }
//    if(len == 3)
//    {
//        QString selectsectionfirst = QString("select *from interlockinginfo WHERE interlockinginfo.First1 = %1").arg(First);
//        QSqlQuery querysectionfirst(selectsectionfirst);
//        while(querysectionfirst.next())
//        {
//            First1 = querysectionfirst.value(9).toString();
//        }
//        if(First == First1)
//        {
//            if(First1 == 1)
//            {
//                if(Sections.isEmpty() || Switchs.isEmpty())
//                {
//                    First1 = 2;
//                }
//                else
//                {
//                    for(int i=0;i<sectionsplit.length();i++)
//                    {
//                        QMap<QString,SectionData>::iterator it = SectionsDataMap.find(sectionsplit[i]);
//                        if(0x01 == it.value().LockStatus || it.value().sectionStatus || 0x01 == it.value().blockStatus)
//                        {
//                            First1 = 2;
//                        }
//                        else
//                        {
//                            SectionsDataMap.find(sectionsplit[i]).value().LockStatus = 0x01;
//                            if()
//                            {
//                                SignalsDataMap.find(beginSignalID).value().signalStatus = 0x03;
//                            }
//                            else
//                            {
//                                SignalsDataMap.find(beginSignalID).value().signalStatus = 0x04;
//                            }
//                        }
//                    }
//                }
//            }
//            if(First1 == 2)
//            {
//                if(Sections.isEmpty() || Switchs.isEmpty())
//                {
//                    First1 = 3;
//                }
//                else
//                {
//                    for(int i=0;i<sectionsplit.length();i++)
//                    {
//                        QMap<QString,SectionData>::iterator it = SectionsDataMap.find(sectionsplit[i]);
//                        if(0x01 == it.value().LockStatus || 0x01 == it.value().sectionStatus || it.value().blockStatus)
//                        {
//                            First1 = 3;
//                        }
//                        else
//                        {
//                            SectionsDataMap.find(sectionsplit[i]).value().LockStatus = 0x01;
//                            if()
//                            {
//                                SignalsDataMap.find(beginSignalID).value().signalStatus = 0x03;
//                            }
//                            else
//                            {
//                                SignalsDataMap.find(beginSignalID).value().signalStatus = 0x04;
//                            }
//                        }
//                    }
//                }
//            }
//            if(First1 == 3)
//            {
//                if(Sections.isEmpty() || Switchs.isEmpty())
//                {
//                    return ;
//                }
//                else
//                {
//                    for(int i=0;i<sectionsplit.length();i++)
//                    {
//                        QMap<QString,SectionData>::iterator it = SectionsDataMap.find(sectionsplit[i]);
//                        if(0x01 == it.value().LockStatus || 0x01 == it.value().sectionStatus || 0x01 == it.value().blockStatus)
//                        {
//                            return ;
//                        }
//                        else
//                        {
//                            SectionsDataMap.find(sectionsplit[i]).value().LockStatus = 0x01;
//                            if()
//                            {
//                                SignalsDataMap.find(beginSignalID).value().signalStatus = 0x03;
//                            }
//                            else
//                            {
//                                SignalsDataMap.find(beginSignalID).value().signalStatus = 0x04;
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }


   /* if(0x10 == XDirection)//1线正向方向解析
    {
        itSignal = SignalsDataMap.find(2);
        itSignal.value().signalStatus = 0x01;

        for(itstr = LockRouteMap.begin();itstr != LockRouteMap.end();++itstr)
        {
            //if(beginSignalName = "X" && endSingalName = "X5")
            if("X,X5" == itstr.key())
            {
                itSignal = SignalsDataMap.find(2);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                SectionsDataMap.find("5DG").value().LockStatus= 0x01;
                SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                SectionsDataMap.find("5G").value().LockStatus = 0x01;
                SignalsDataMap.find(2).value().signalStatus = 0x01;

                if(SectionsDataMap.find("3DG").value().sectionStatus == 0x01 || SectionsDataMap.find("9-15DG").value().sectionStatus == 0x01)//判断道岔是否空闲来确定设置那条进路
                {
                    SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                    SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("5G").value().LockStatus = 0x01;
                    SignalsDataMap.find(2).value().signalStatus = 0x01;

                    if(it.value().sectionStatus == 0x01)//红光带故障或者白光带故障,分路不良
                    {
                        SignalsDataMap.find(2).value().signalStatus = 0x01;
                    }
                }
                else if(SectionsDataMap.find("7DG").value().sectionStatus == 0x01)
                {
                    SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                    SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("5G").value().LockStatus = 0x01;
                    SignalsDataMap.find(2).value().signalStatus = 0x01;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(2).value().signalStatus = 0x01;
                    }
                }
            }
            if("X,X3" == itstr.key())
            {
                itSignal = SignalsDataMap.find(2);
                itSignal.value().signalStatus = 0x03;

                if(SectionsDataMap.find("7DG").value().sectionStatus == 0x01)
                {
                    if(SectionsDataMap.find("17-23DG").value().sectionStatus == 0x01)
                    {
                        SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                        SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3G").value().LockStatus = 0x01;
                        SignalsDataMap.find(2).value().signalStatus = 0x01;

                        if(it.value().sectionStatus == 0x01)
                        {
                            SignalsDataMap.find(2).value().signalStatus = 0x01;
                        }
                    }
                    else if(SectionsDataMap.find("11-13DG").value().sectionStatus == 0x01)
                    {
                        SectionsDataMap.find("IAG").value().LockStatus= 0x01;
                        SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                        SectionsDataMap.find("3G").value().LockStatus = 0x01;
                        SignalsDataMap.find(2).value().signalStatus = 0x01;

                        if(it.value().sectionStatus == 0x01)
                        {
                            SignalsDataMap.find(2).value().signalStatus = 0x01;
                        }
                    }
                }
                else
                {
                    SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                    SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("3G").value().LockStatus = 0x01;
                    SignalsDataMap.find(2).value().signalStatus = 0x01;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(2).value().signalStatus = 0x01;
                    }
                }
            }
            if("X,XI" == itstr.key())
            {
                itSignal = SignalsDataMap.find(2);
                itSignal.value().signalStatus = 0x06;

                SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IG").value().LockStatus = 0x01;
                SignalsDataMap.find(2).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(2).value().signalStatus = 0x01;
                }
            }
            if("X,XII" == itstr.key())
            {
                itSignal = SignalsDataMap.find(2);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IIG").value().LockStatus = 0x01;
                SignalsDataMap.find(2).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(2).value().signalStatus = 0x01;
                }
            }
            if("X,X4" == itstr.key())
            {
                itSignal = SignalsDataMap.find(2);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                SectionsDataMap.find("27/29WG").value().LockStatus = 0x01;
                SectionsDataMap.find("29DG").value().LockStatus = 0x01;
                SectionsDataMap.find("4G").value().LockStatus = 0x01;
                SignalsDataMap.find(2).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(2).value().signalStatus = 0x01;
                }
            }
        }
    }
    if(0x01 == XDirection)//1线反向方向解析
    {
        for(itstr = LockRouteMap.begin();itstr != LockRouteMap.end();++itstr)
        {
//            itSignal = SignalsDataMap.find("");
//            itSignal.value().signalStatus = ;

            if("S5,X" == itstr.key())
            {
                itSignal = SignalsDataMap.find(14);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                SignalsDataMap.find(14).value().signalStatus = 0x01;

                if(SectionsDataMap.find("7DG").value().sectionStatus == 0x01)
                {
                    SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                    SignalsDataMap.find(14).value().signalStatus = 0x01;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(14).value().signalStatus = 0x01;
                    }
                }
                else if(SectionsDataMap.find("9-15DG").value().sectionStatus == 0x01 || SectionsDataMap.find("3DG").value().sectionStatus == 0x01)
                {
                    SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                    SignalsDataMap.find(14).value().signalStatus = 0x01;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(14).value().signalStatus = 0x01;
                    }
                }
            }
            if("S3,X" == itstr.key())
            {
                itSignal = SignalsDataMap.find(10);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                SignalsDataMap.find(10).value().signalStatus = 0x01;

                if(SectionsDataMap.find("21DG").value().sectionStatus == 0x01 || SectionsDataMap.find("11-13DG").value().sectionStatus == 0x01 || SectionsDataMap.find("7DG").value().sectionStatus == 0x01)
                {
                    SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                    SignalsDataMap.find(10).value().signalStatus = 0x01;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(10).value().signalStatus = 0x01;
                    }
                }
                else if(SectionsDataMap.find("17-23DG").value().sectionStatus == 0x01 || SectionsDataMap.find("7DG").value().sectionStatus == 0x01)
                {
                    SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                    SignalsDataMap.find(10).value().signalStatus = 0x01;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(10).value().signalStatus = 0x01;
                    }
                }
                else if(SectionsDataMap.find("17-23DG").value().sectionStatus == 0x01 || SectionsDataMap.find("9-15DG").value().sectionStatus == 0x01 || SectionsDataMap.find("3DG").value().sectionStatus == 0x01)
                {
                    SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                    SignalsDataMap.find(10).value().signalStatus = 0x01;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(10).value().signalStatus = 0x01;
                    }
                }
            }
            if("SI,X" == itstr.key())
            {
                itSignal = SignalsDataMap.find(9);
                itSignal.value().signalStatus = 0x06;

                SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                SignalsDataMap.find(9).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(9).value().signalStatus = 0x01;
                }
            }
            if("SII,X" == itstr.key())
            {
                itSignal = SignalsDataMap.find(11);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                SignalsDataMap.find(11).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(11).value().signalStatus = 0x01;
                }
            }
            if("S4,X" == itstr.key())
            {
                itSignal = SignalsDataMap.find(12);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                SectionsDataMap.find("5DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IAG").value().LockStatus = 0x01;
                SignalsDataMap.find(12).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(12).value().signalStatus = 0x01;
                }
            }
        }    
}
    if(0x10 == XFDirection)//2线正向方向解析
    {
        for(itstr = LockRouteMap.begin();itstr != LockRouteMap.end();++itstr)
        {
//            itSignal = SignalsDataMap.find("");
//            itSignal.value().signalStatus = ;

            if("S4,XF" == itstr.key())
            {
                itSignal = SignalsDataMap.find(12);
                itSignal.value().signalStatus = Double_yellow_flicker;

                SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                SectionsDataMap.find("1/19WG").value().LockStatus = 0x01;
                SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                SignalsDataMap.find(12).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(12).value().signalStatus =0x01;
                }
            }
            if("SII,XF" == itstr.key())
            {
                itSignal = SignalsDataMap.find(11);
                itSignal.value().signalStatus = 0x06;

                SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                SectionsDataMap.find("1/19WG").value().LockStatus = 0x01;
                SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                SignalsDataMap.find(11).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(11).value().signalStatus = 0x01;
                }
            }
            if("SI,XF" == itstr.key())
            {
                itSignal = SignalsDataMap.find(9);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                SignalsDataMap.find(9).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(9).value().signalStatus = 0x01;
                }
            }
            if("S3,XF" == itstr.key())
            {
                itSignal = SignalsDataMap.find(10);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                SignalsDataMap.find(10).value().signalStatus = 0x01;

                if(SectionsDataMap.find("21DG").value().sectionStatus == 0x01 || SectionsDataMap.find("11-13DG").value().sectionStatus == 0x01)
                {
                    SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                    SignalsDataMap.find(10).value().signalStatus = 0x01;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(10).value().signalStatus = 0x01;
                    }
                }
                else if(SectionsDataMap.find("17-23DG").value().sectionStatus == 0x01)
                {
                    SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                    SignalsDataMap.find(10).value().signalStatus = 0x01;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(10).value().signalStatus = 0x01;
                    }
                }
            }
            if("S5,XF" == itstr.key())
            {
                itSignal = SignalsDataMap.find(14);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                SignalsDataMap.find(14).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(14).value().signalStatus = 0x01;
                }
            }
        }
    }
    if(0x01 == XFDirection)//2线反向方向解析
    {
        for(itstr = LockRouteMap.begin();itstr != LockRouteMap.end();++itstr)
        {
            itSignal = SignalsDataMap.find(1);
            itSignal.value().signalStatus = 0x01;

            if("XF,X4" == itstr.key())
            {
                itSignal = SignalsDataMap.find(1);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                SectionsDataMap.find("1/19WG").value().LockStatus = 0x01;
                SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                SectionsDataMap.find("27/29WG").value().LockStatus = 0x01;
                SectionsDataMap.find("29DG").value().LockStatus = 0x01;
                SectionsDataMap.find("4G").value().LockStatus = 0x01;
                SignalsDataMap.find(1).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(1).value().signalStatus = 0x01;
                }
            }
            if("XF,XII" == itstr.key())
            {
                itSignal = SignalsDataMap.find(1);
                itSignal.value().signalStatus = 0x07;

                SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                SectionsDataMap.find("1/19WG").value().LockStatus = 0x01;
                SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IIG").value().LockStatus = 0x01;
                SignalsDataMap.find(1).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(1).value().signalStatus = 0x01;
                }
            }
            if("XF,XI" == itstr.key())
            {
                itSignal = SignalsDataMap.find(1);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IG").value().LockStatus = 0x01;
                SignalsDataMap.find(1).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(1).value().signalStatus =0x01;
                }
            }
            if("XF,X3" == itstr.key())
            {
                itSignal = SignalsDataMap.find(1);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3G").value().LockStatus = 0x01;
                SignalsDataMap.find(1).value().signalStatus = 0x01;

                if(SectionsDataMap.find("11-13DG").value().sectionStatus == 0x01 || SectionsDataMap.find("21DG").value().sectionStatus == 0x01)
                {
                    SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                    SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("3G").value().LockStatus = 0x01;
                    SignalsDataMap.find(1).value().signalStatus = 0x01;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(1).value().signalStatus = 0x01;
                    }
                }
                if(SectionsDataMap.find("17-23DG").value().sectionStatus == 0x01)
                {
                    SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                    SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                    SectionsDataMap.find("3G").value().LockStatus = 0x01;
                    SignalsDataMap.find(1).value().signalStatus = 0x01;

                    if(it.value().sectionStatus == 0x01)
                    {
                        SignalsDataMap.find(1).value().signalStatus =0x01;
                    }
                }
            }
            if("XF,X5" == itstr.key())
            {
                itSignal = SignalsDataMap.find(1);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
                SectionsDataMap.find("1DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3G").value().LockStatus = 0x01;
                SignalsDataMap.find(1).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(1).value().signalStatus = 0x01;
                }
            }
        }
    }
    if(0x10 == SDirection)//3线正向方向解析
    {
        for(itstr = LockRouteMap.begin();itstr != LockRouteMap.end();++itstr)
        {
            itSignal = SignalsDataMap.find(4);
            itSignal.value().signalStatus = 0x01;

            if("S,S4" == itstr.key())
            {
                itSignal = SignalsDataMap.find(4);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("IIBG").value().LockStatus = 0x01;
                SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                SectionsDataMap.find("14DG").value().LockStatus = 0x01;
                SectionsDataMap.find("4G").value().LockStatus = 0x01;
                SectionsDataMap.find("29DG").value().LockStatus = 0x01;
                SectionsDataMap.find("27/29WG").value().LockStatus = 0x01;
                SignalsDataMap.find(4).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(4).value().signalStatus = 0x01;
                }
            }
            if("S,SII" == itstr.key())
            {
                itSignal = SignalsDataMap.find(4);
                itSignal.value().signalStatus = 0x07;

                SectionsDataMap.find("IIBG").value().LockStatus = 0x01;
                SectionsDataMap.find("8-10BG").value().LockStatus = 0x01;
                SectionsDataMap.find("14DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IIG").value().LockStatus = 0x01;
                SignalsDataMap.find(4).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(4).value().signalStatus = 0x01;
                }
            }
            if("S,SI" == itstr.key())
            {
                itSignal = SignalsDataMap.find(4);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("IIBG").value().LockStatus = 0x01;
                SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IG").value().LockStatus = 0x01;
                SignalsDataMap.find(4).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(4).value().signalStatus = 0x01;
                }
            }
            if("S,S3" == itstr.key())
            {
                itSignal = SignalsDataMap.find(4);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("IIBG").value().LockStatus = 0x01;
                SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                SectionsDataMap.find("18DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3G").value().LockStatus = 0x01;
                SignalsDataMap.find(4).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(4).value().signalStatus =0x01;
                }
            }
            if("S,S5" == itstr.key())
            {
                itSignal = SignalsDataMap.find(4);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("IIBG").value().LockStatus = 0x01;
                SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                SectionsDataMap.find("18DG").value().LockStatus = 0x01;
                SectionsDataMap.find("20DG").value().LockStatus = 0x01;
                SectionsDataMap.find("22DG").value().LockStatus = 0x01;
                SignalsDataMap.find(4).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(4).value().signalStatus = 0x01;
                }
            }
        }
    }
    if(0x01 == SDirection)//3线反向方向解析
    {
        for(itstr = LockRouteMap.begin();itstr != LockRouteMap.end();++itstr)
        {
//            itSignal = SignalsDataMap.find("");
//            itSignal.value().signalStatus = Red;

            if("X4,S" == itstr.key())
            {
                itSignal = SignalsDataMap.find(7);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("14DG").value().LockStatus = 0x01;
                SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IIBG").value().LockStatus = 0x01;
                SignalsDataMap.find(7).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(7).value().signalStatus = 0x01;
                }
            }
            if("XII,S" == itstr.key())
            {
                itSignal = SignalsDataMap.find(5);
                itSignal.value().signalStatus = 0x07;

                SectionsDataMap.find("14DG").value().LockStatus = 0x01;
                SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IIBG").value().LockStatus = 0x01;
                SignalsDataMap.find(5).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(5).value().signalStatus = 0x01;
                }
            }
            if("XI,S" == itstr.key())
            {
                itSignal = SignalsDataMap.find(8);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IIBG").value().LockStatus = 0x01;
                SignalsDataMap.find(8).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(8).value().signalStatus = 0x01;
                }
            }
            if("X3,S" == itstr.key())
            {
                itSignal = SignalsDataMap.find(6);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("18DG").value().LockStatus = 0x01;
                SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IIBG").value().LockStatus = 0x01;
                SignalsDataMap.find(6).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(6).value().signalStatus = 0x01;
                }
            }
            if("X5,S" == itstr.key())
            {
                itSignal = SignalsDataMap.find(15);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("22DG").value().LockStatus = 0x01;
                SectionsDataMap.find("18DG").value().LockStatus = 0x01;
                SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IIBG").value().LockStatus = 0x01;
                SignalsDataMap.find(15).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(15).value().signalStatus = 0x01;
                }
            }
        }
    }
    if(0x10 == SFDirection)//4线正向方向解析
    {
        for(itstr = LockRouteMap.begin();itstr != LockRouteMap.end();++itstr)
        {
//            itSignal = SignalsDataMap.find("");
//            itSignal.value().signalStatus = Red;

            if("X5,SF" == itstr.key())
            {
                itSignal = SignalsDataMap.find(15);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("22DG").value().LockStatus = 0x01;
                SectionsDataMap.find("18DG").value().LockStatus = 0x01;
                SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                SectionsDataMap.find("4DG").value().LockStatus = 0x01;
                SignalsDataMap.find(15).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(15).value().signalStatus = 0x01;
                }
            }
            if("X3,SF" == itstr.key())
            {
                itSignal = SignalsDataMap.find(6);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("18DG").value().LockStatus = 0x01;
                SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                SectionsDataMap.find("4DG").value().LockStatus = 0x01;
                SignalsDataMap.find(6).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(6).value().signalStatus = 0x01;
                }
            }
            if("XI,SF" == itstr.key())
            {
                itSignal = SignalsDataMap.find(8);
                itSignal.value().signalStatus = 0x07;

                SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                SectionsDataMap.find("4DG").value().LockStatus = 0x01;
                SignalsDataMap.find(8).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(8).value().signalStatus = 0x01;
                }
            }
            if("XII,SF" == itstr.key())
            {
                itSignal = SignalsDataMap.find(5);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("14DG").value().LockStatus = 0x01;
                SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                SectionsDataMap.find("4DG").value().LockStatus = 0x01;
                SignalsDataMap.find(5).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                     SignalsDataMap.find(5).value().signalStatus = 0x01;
                }
            }
            if("X4,SF" == itstr.key())
            {
                itSignal = SignalsDataMap.find(7);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("14DG").value().LockStatus = 0x01;
                SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                SectionsDataMap.find("4DG").value().LockStatus = 0x01;
                SignalsDataMap.find(7).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(7).value().signalStatus = 0x01;
                }
            }
        }
    }
    if(0x01 == SFDirection)//4线反向方向解析
    {
        for(itstr = LockRouteMap.begin();itstr != LockRouteMap.end();++itstr)
        {
            itSignal = SignalsDataMap.find(3);
            itSignal.value().signalStatus = 0x01;

            if("SF,S5" == itstr.key())
            {
                itSignal = SignalsDataMap.find(3);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("4DG").value().LockStatus = 0x01;
                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                SectionsDataMap.find("18DG").value().LockStatus = 0x01;
                SectionsDataMap.find("20DG").value().LockStatus = 0x01;
                SectionsDataMap.find("22DG").value().LockStatus = 0x01;
                SectionsDataMap.find("5G").value().LockStatus = 0x01;
                SignalsDataMap.find(3).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(3).value().signalStatus = 0x01;
                }
            }
            if("SF,S3" == itstr.key())
            {
                itSignal = SignalsDataMap.find(3);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("4DG").value().LockStatus = 0x01;
                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                SectionsDataMap.find("18DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3G").value().LockStatus = 0x01;
                 SignalsDataMap.find(3).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                     SignalsDataMap.find(3).value().signalStatus = 0x01;
                }
            }
            if("SF,SI" == itstr.key())
            {
                itSignal = SignalsDataMap.find(3);
                itSignal.value().signalStatus = 0x07;

                SectionsDataMap.find("4DG").value().LockStatus = 0x01;
                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                SectionsDataMap.find("16DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IG").value().LockStatus = 0x01;
                SignalsDataMap.find(3).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(3).value().signalStatus = 0x01;
                }
            }
            if("SF,SII" == itstr.key())
            {
                itSignal = SignalsDataMap.find(3);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("4DG").value().LockStatus = 0x01;
                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                SectionsDataMap.find("14DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IIG").value().LockStatus = 0x01;
                SignalsDataMap.find(3).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(3).value().signalStatus = 0x01;
                }
            }
            if("SF,S4" == itstr.key())
            {
                itSignal = SignalsDataMap.find(3);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("4DG").value().LockStatus = 0x01;
                SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
                SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
                SectionsDataMap.find("14DG").value().LockStatus = 0x01;
                SectionsDataMap.find("4G").value().LockStatus = 0x01;
                SectionsDataMap.find("29DG").value().LockStatus = 0x01;
                SectionsDataMap.find("27/29WG").value().LockStatus = 0x01;
                SignalsDataMap.find(3).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(3).value().signalStatus = 0x01;
                }
            }
        }
    }
    if(0x10 == XDDirection)//5线正向方向解析
    {
        for(itstr = LockRouteMap.begin();itstr != LockRouteMap.end();++itstr)
        {
            itSignal = SignalsDataMap.find(13);
            itSignal.value().signalStatus = 0x01;

            if("XD,X5" == itstr.key())
            {
                itSignal = SignalsDataMap.find(13);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                SectionsDataMap.find("5G").value().LockStatus = 0x01;
                SignalsDataMap.find(13).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(13).value().signalStatus = 0x01;
                }
            }
            if("XD,X3" == itstr.key())
            {
                itSignal = SignalsDataMap.find(13);
                itSignal.value().signalStatus = 0x07;

                SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                SectionsDataMap.find("3G").value().LockStatus = 0x01;
                SignalsDataMap.find(13).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(13).value().signalStatus = 0x01;
                }
            }
            if("XD,XI" == itstr.key())
            {
                itSignal = SignalsDataMap.find(13);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IG").value().LockStatus = 0x01;
                SignalsDataMap.find(13).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(13).value().signalStatus = 0x01;
                }
            }
            if("XD,XII" == itstr.key())
            {
                itSignal = SignalsDataMap.find(13);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                SectionsDataMap.find("IIG").value().LockStatus = 0x01;
                SignalsDataMap.find(13).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(13).value().signalStatus = 0x01;
                }
            }
            if("XD,X4" == itstr.key())
            {
                itSignal = SignalsDataMap.find(13);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                SectionsDataMap.find("27/29WG").value().LockStatus = 0x01;
                SectionsDataMap.find("29DG").value().LockStatus = 0x01;
                SectionsDataMap.find("4G").value().LockStatus = 0x01;
                SignalsDataMap.find(13).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(13).value().signalStatus = 0x01;
                }
            }
        }
    }
    if(0x01 == XDDirection)//5线反向方向解析
    {
        for(itstr = LockRouteMap.begin();itstr != LockRouteMap.end();++itstr)
        {
//            itSignal = SignalsDataMap.find("");
//            itSignal.value().signalStatus = Red;

            if("S5,XD" == itstr.key())
            {
                itSignal = SignalsDataMap.find(14);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                SignalsDataMap.find(14).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(14).value().signalStatus = 0x01;
                }
            }
            if("S3,XD" == itstr.key())
            {
                itSignal = SignalsDataMap.find(10);
                itSignal.value().signalStatus = 0x07;

                SectionsDataMap.find("25DG").value().LockStatus = 0x01;
                SectionsDataMap.find("21DG").value().LockStatus = 0x01;
                SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                SignalsDataMap.find(10).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(10).value().signalStatus = 0x01;
                }
            }
            if("SI,XD" == itstr.key())
            {
                itSignal = SignalsDataMap.find(9);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                SignalsDataMap.find(9).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(9).value().signalStatus = 0x01;
                }
            }
            if("SII,XD" == itstr.key())
            {
                itSignal = SignalsDataMap.find(11);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                SignalsDataMap.find(11).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(11).value().signalStatus = 0x01;
                }
            }
            if("S4,XD" == itstr.key())
            {
                itSignal = SignalsDataMap.find(12);
                itSignal.value().signalStatus = 0x03;

                SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
                SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
                SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
                SectionsDataMap.find("11-13DG").value().LockStatus = 0x01;
                SectionsDataMap.find("7DG").value().LockStatus = 0x01;
                SignalsDataMap.find(12).value().signalStatus = 0x01;

                if(it.value().sectionStatus == 0x01)
                {
                    SignalsDataMap.find(12).value().signalStatus = 0x01;
                }
            }
        }
    }
    if(0x10 == XDirection && 0x10 == SFDirection)//X到SF的正向通路
    {
        if((LockRouteMap.end() != LockRouteMap.find("X,XI")) && (LockRouteMap.end() != LockRouteMap.find("XI,SF")))
        {
            itSignal = SignalsDataMap.find(2);
            itSignal.value().signalStatus = 0x06;
            itSignal = SignalsDataMap.find(8);
            itSignal.value().signalStatus = 0x06;

            SectionsDataMap.find("IAG").value().LockStatus = 0x01;
            SectionsDataMap.find("5DG").value().LockStatus = 0x01;
            SectionsDataMap.find("3DG").value().LockStatus = 0x01;
            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
            SectionsDataMap.find("IG").value().LockStatus = 0x01;
            SectionsDataMap.find("16DG").value().LockStatus = 0x01;
            SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
            SectionsDataMap.find("4DG").value().sectionStatus = 0x01;

            if(it.value().sectionStatus == 0x01)
            {
                SignalsDataMap.find(2).value().signalStatus = 0x01;
            }
        }
    }
    if(0x01 == XDirection && 0x01 == SFDirection)//X到SF的反向通路
    {
        if((LockRouteMap.end() != LockRouteMap.find("SF,SI")) && (LockRouteMap.end() != LockRouteMap.find("SI,X")))
        {
            itSignal = SignalsDataMap.find(3);
            itSignal.value().signalStatus = 0x06;
            itSignal = SignalsDataMap.find(9);
            itSignal.value().signalStatus = 0x06;

            SectionsDataMap.find("4DG").value().LockStatus = 0x01;
            SectionsDataMap.find("6-12DG").value().LockStatus = 0x01;
            SectionsDataMap.find("16DG").value().LockStatus = 0x01;
            SectionsDataMap.find("IG").value().LockStatus = 0x01;
            SectionsDataMap.find("17-23DG").value().LockStatus = 0x01;
            SectionsDataMap.find("9-15DG").value().LockStatus = 0x01;
            SectionsDataMap.find("3DG").value().LockStatus = 0x01;
            SectionsDataMap.find("5DG").value().LockStatus = 0x01;
            SectionsDataMap.find("IAG").value().LockStatus = 0x01;

            if(it.value().sectionStatus == 0x01)
            {
                SignalsDataMap.find(3).value().signalStatus = 0x01;
            }
        }
    }
    if(0x10 == SDirection && 0x10 == XFDirection)//S到XF的正向通路
    {
        if((LockRouteMap.end() != LockRouteMap.find("S,SII")) && (LockRouteMap.end() != LockRouteMap.find("SII,XF")))
        {
            itSignal = SignalsDataMap.find(4);
            itSignal.value().signalStatus = 0x06;
            itSignal = SignalsDataMap.find(11);
            itSignal.value().signalStatus = 0x06;

            SectionsDataMap.find("IIBG").value().LockStatus = 0x01;
            SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
            SectionsDataMap.find("14DG").value().LockStatus = 0x01;
            SectionsDataMap.find("IIG").value().LockStatus = 0x01;
            SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
            SectionsDataMap.find("1/19WG").value().LockStatus = 0x01;
            SectionsDataMap.find("1DG").value().LockStatus = 0x01;
            SectionsDataMap.find("IIAG").value().LockStatus = 0x01;

            if(it.value().sectionStatus == 0x01)
            {
                SignalsDataMap.find(4).value().signalStatus = 0x01;
            }
        }
    }
    if(0x01 == SDirection && 0x01 == XFDirection)//S到XF的反向通路
    {
        if((LockRouteMap.end() != LockRouteMap.find("XF,XII")) && (LockRouteMap.end() != LockRouteMap.find("XII,S")))
        {
            itSignal = SignalsDataMap.find(1);
            itSignal.value().signalStatus = 0x06;
            itSignal = SignalsDataMap.find(5);
            itSignal.value().signalStatus = 0x06;

            SectionsDataMap.find("IIAG").value().LockStatus = 0x01;
            SectionsDataMap.find("1DG").value().LockStatus = 0x01;
            SectionsDataMap.find("1/19WG").value().LockStatus = 0x01;
            SectionsDataMap.find("19-27DG").value().LockStatus = 0x01;
            SectionsDataMap.find("IIG").value().LockStatus = 0x01;
            SectionsDataMap.find("14DG").value().LockStatus = 0x01;
            SectionsDataMap.find("8-10DG").value().LockStatus = 0x01;
            SectionsDataMap.find("IIBG").value().LockStatus = 0x01;

            if(it.value().sectionStatus == 0x01)
            {
                SignalsDataMap.find(1).value().signalStatus = 0x01;
            }
        }
    }*/
}

void InterLock::on_pushButton_clicked()
{
    SetupRoute(1,8);
}

void InterLock::on_pushButton_2_clicked()
{
    RemoveRoute(1,1);
}
