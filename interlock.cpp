#include "interlock.h"
#include "ui_interlock.h"
#include "QTime"
#include "QApplication"

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
        qDebug()<<"XXXXXX2";
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
        TestStationSwitch();
        Chance ++;
    }
    timer->start(1000);
}

InterLock::~InterLock()
{
    delete ui;
}

int i=0;
int fq=0;
int fq1=0;
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
                if(0x42 == buf[7])//信号重开
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
                    //qDebug() <<"FenLu:"<<buf.toInt();
                    //FenLu(buf[12]);
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
                    //qDebug() <<"GuZhang:"<< buf.toInt();
                    //GuZhang(buf[12],buf[17]);
                }
                if(0x35 == buf[7])//闭塞操作
                {
                    //qDebug() <<"BiSe:"<<buf.toInt();
                    //BiSe(buf[12]);
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
                if(0x40 == buf[7])//模拟行车————————————————————————√
                {
                    qDebug() <<"UnlockState:"<<buf.toInt();
                    //UnlockState(buf[12]);
                    TestUnlockState(buf[12]);
                }
                if(0x23==buf[7])//红白光带故障————————————————————————√
                {
                    qDebug()<<"HBGZ"<<buf.toInt();
                    HBGZ(buf[12],buf[17]);
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

//【00操作·上电解锁】√
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
    QList<QString> RuleList1;
    QList<int> OneSwitch = {21,27,29,14,22,16};
    QMap<int,SwitchData> test = SwitchDataMap;

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
        sql = QString("select * from interlockinginfo WHERE BeginSignal = \"%1\" AND EndSignal = \"%2\" and `First` = 1").arg(beginSignalName).arg(endSignalName);
    }
    else if(type ==2){   //引导进路
        //QString LineName = beginSignalName + "引导";
        //进路中所有的道岔
        sql = QString("select * from interlockinginfo WHERE BeginSignal = \"%1\"").arg(beginSignalName);
        QSqlQuery queryYD(sql);
        //引导进路定反位判断
        while(queryYD.next())
        {
            SwitchNames = queryYD.value(7).toString();
            QString EndSignalName = queryYD.value(4).toString();
            QByteArray YDnum = switchesStrextract(SwitchNames);
            QString switchnameYD;
            for(int k=0;k<YDnum.count();k+=2){
                int switchname = YDnum[k];//进路途径地道岔名字,1反位，2定位
                switchnameYD = QString::number(switchname, 10);
                QList<int> switchidlist = SelectSwitchIdForName(switchnameYD);
                int OneK = YDnum[k+1]-1;
                int TwoK = YDnum[k+1];
                if(SwitchDataMap.find(switchidlist[0]).value().switchStates != OneK || TwoK != 0x02){
                    if(SwitchDataMap.find(switchidlist[1]).value().switchStates != TwoK || TwoK != 0x01){
                        break;
                    }
                }
                if(k==YDnum.count()-2){
                   endSignalName = EndSignalName;
                }
            }
        }
        if(endSignalName == NULL || endSignalName == ""){
            return;
        }else{
            sql = QString("select * from interlockinginfo WHERE BeginSignal = \"%1\" AND EndSignal = \"%2\" and `First` = 1").arg(beginSignalName).arg(endSignalName);
        }
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
                if((num[k+1]+0) == 2 && SwitchDataMap.find(switchid).value().switchPos == 0x01 && SwitchDataMap.find(switchid).value().switchStates == 0x01 && SwitchDataMap.find(switchid).value().switchLock == 0x01){
                    MessageListAdd(2,switchid,130);
                    return;
                }
                //如果进路要求道岔反位，但是道岔是定位，且道岔被锁闭，则进路无效
                if((num[k+1]+0) == 1 && SwitchDataMap.find(switchid).value().switchPos == 0x00 && SwitchDataMap.find(switchid).value().switchStates == 0x01 && SwitchDataMap.find(switchid).value().switchLock == 0x01){
                    MessageListAdd(2,switchid,130);
                    return;
                }
                //----------------【判断条件8】道岔失表----------------：
                if(SwitchDataMap.find(switchid).value().SwitchLoss == 0x01){
                    MessageListAdd(2,switchid,131);
                    return;
                }
                //----------------【判断条件9】道岔封锁----------------：
                if(SwitchDataMap.find(switchid).value().blockStatus == 0x01){
                    MessageListAdd(2,switchid,132);
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
                    MessageListAdd(1,sectionid.toInt(),101);
                    return;
                }
                //----------------【判断条件2】区段封锁----------------：
                if(SectionsDataMap.find(sectionid).value().blockStatus == 0x01){
                    MessageListAdd(1,sectionid.toInt(),102);
                    return;
                }
                //----------------【判断条件3】区段分路不良----------------：
                if(SectionsDataMap.find(sectionid).value().PoorStatus == 0x01){
                    MessageListAdd(1,sectionid.toInt(),103);
                    return;
                }
                //----------------【判断条件4】区段占用----------------：
                if(SectionsDataMap.find(sectionid).value().sectionStatus == 0x01){
                    MessageListAdd(1,sectionid.toInt(),104);
                    return;
                }
                //----------------【判断条件5】区段白光带故障----------------：
                if(SectionsDataMap.find(sectionid).value().SectionWhiteObstacle == 0x01){
                    MessageListAdd(1,sectionid.toInt(),105);
                    return;
                }
                //----------------【判断条件6】区段红光带故障----------------：
                if(SectionsDataMap.find(sectionid).value().SectionRedObstacle == 0x01){
                    MessageListAdd(1,sectionid.toInt(),106);
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
        Type = queryinterlockingB.value(9).toString();
        if(SectionNames.count() == 0 || SwitchNames.count() == 0)
        {
            return ;
        }

        //拆分道岔名字和定反位
        //声明一个锁闭道岔的ID拼接地字符串（模拟行车功能用）
        QString LockSwitchs;
        QByteArray num = switchesStrextract(SwitchNames);
        int switchname;
        int switchnametest;
        for(int k=0;k<num.count();k+=2){
            switchname = num[k];//进路的区段名字
            switchnametest = num[k+2];
            int switchid;//对应的区段ID
            QString selectswitch = QString("SELECT * FROM switch WHERE SwitchName = %1").arg(switchname);
            QSqlQuery queryswitch(selectswitch);
            while(queryswitch.next())
            {
                switchid = queryswitch.value(0).toInt();
                QString switchidString = QString::number(switchid, 10);//【模拟行车用】拼接地锁闭道岔地ID
                if(SwitchDataMap.find(switchid).value().switchPos == (num[k+1]+0)){//如果要求该道岔是定位
                    if(SwitchDataMap.find(switchid).value().switchStates != 0x01){
                        SwitchDataMap.find(switchid).value().switchStates = 0x01;//启用该道岔的（定/反）
                        LockSwitchs += switchidString + ",";
                        SwitchDataMap.find(switchid - 1).value().switchStates = 0x00;//关闭该道岔的（反/定）
                        MessageListAdd(2,switchid,61);
                    }
                    if(type == 1){//对象正常锁闭
                        SwitchDataMap.find(switchid).value().switchRoute = 0x01;
                        LockSwitchs += switchidString + ",";
                        MessageListAdd(2,switchid,62);
                    }else if(type == 2){//对象引导锁闭
                        SwitchDataMap.find(switchid).value().switchRoute = 0x03;
                        LockSwitchs += switchidString + ",";
                        MessageListAdd(2,switchid,62);
                    }
                }else if(SwitchDataMap.find(switchid).value().switchPos == (num[k+1]-2)){//如果要求该道岔是反位
                    if(SwitchDataMap.find(switchid).value().switchStates != 0x01){
                        SwitchDataMap.find(switchid).value().switchStates = 0x01;//启用该道岔的（定/反）
                        LockSwitchs += switchidString + ",";
                        SwitchDataMap.find(switchid + 1).value().switchStates = 0x00;//关闭该道岔的（反/定）
                        MessageListAdd(2,switchid,60);
                    }
                    if(type == 1){//对象正常锁闭
                        SwitchDataMap.find(switchid).value().switchRoute = 0x01;
                        LockSwitchs += switchidString + ",";
                        MessageListAdd(2,switchid,62);
                    }else if(type == 2){//对象引导锁闭
                        SwitchDataMap.find(switchid).value().switchRoute = 0x03;
                        LockSwitchs += switchidString + ",";
                        MessageListAdd(2,switchid,62);
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
                    UpdateSection(sectionid,"LockStatus",0x01);
                }else if(type == 2){
                    UpdateSection(sectionid,"LockStatus",0x03);
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

        RuleList1.append(SectionNames);
        RuleList.append(Type);
        RuleList.append(SectionNames);
        RuleList.append(SwitchNames);
        LockSwitchs.chop(1);
        RuleList.append(LockSwitchs);
        RuleMap[beginSignalName] = RuleList;
        RuleMap1[beginSignalName]=RuleList1;
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
                    UpdateSection(sectionid,"LockStatus",0x02);
                    sectionremove = true;
                }
            }else if(type == 2)
            { //总人解
                if(SectionsDataMap.find(sectionid).value().LockStatus == 0x01 || SectionsDataMap.find(sectionid).value().LockStatus == 0x03)
                {
                    UpdateSection(sectionid,"LockStatus",0x02);
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
                            if(switchit.value().switchRoute == 0x01){
                                switchit.value().switchRoute = 0x02;
                                switchremove = true;
                            }
                        }
                        else if(type == 2)
                        { //总人解
                            if(switchit.value().switchRoute == 0x01 || switchit.value().switchRoute == 0x03){
                                switchit.value().switchRoute = 0x02;
                                switchremove = true;
                            }
                        }
                    }
                }else if(num[k+1].operator == (0x02)){ //定位
                    if(switchit.value().switchName == switchname && switchit.value().switchPos == 0){
                        if(type == 1)
                        { //总取消
                            if(switchit.value().switchRoute == 0x01){
                                switchit.value().switchRoute = 0x02;
                                switchremove = true;
                            }
                        }
                        else if(type == 2)
                        { //总人解
                            if(switchit.value().switchRoute == 0x01 || switchit.value().switchRoute == 0x03){
                                switchit.value().switchRoute = 0x02;
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

        if(SignalsDataMap.find(beginSignalID).value().signalType==0x03)
        {
            if(SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01||SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus=0x07;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus= 0x0c;
            }
            else {SignalsDataMap.find(beginSignalID).value().signalStatus= 0x07;
            }
            SignalsDataMap.find(beginSignalID).value().signalLockStatus=0x02;//信号机开放
        }
        else
        {
            if(SignalsDataMap.find(beginSignalID).value().RedAllDSStatus==0x01||SignalsDataMap.find(beginSignalID).value().GreenAllDSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus = 0x01;
            }
            else if(SignalsDataMap.find(beginSignalID).value().DSStatus==0x01)
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus = 0x09;
            }
            else
            {
                SignalsDataMap.find(beginSignalID).value().signalStatus = 0x01;
            }

            SignalsDataMap.find(beginSignalID).value().signalLockStatus=0x02;//信号机机开放
        }
       RuleMap.remove(beginSignalName);
    }
}

//【03操作·引导总锁】 √
void InterLock::YinDaoZS(byte Direction)
{
    //01为锁闭状态，02为未锁闭
    byte direction = Direction;
    SwitchData switchData;
//    QString SwitchID;
//    QString SwitchID1;
    QMap<int,SwitchData>::iterator itSwitch;
//    QMap<QString,QList<QString>> ::iterator it;
//    for(it=RuleMap.begin();it!=RuleMap.end();it++)
//    {
//        SwitchID=it.value()[3];
//    }

    if(0x01 == direction)//X引导总锁
    {
        for(itSwitch = SwitchDataMap.begin();itSwitch != SwitchDataMap.end();++itSwitch)
        {
            int SwitchName =itSwitch.value().switchName;
//            QString selectID =QString("select *from switch WHERE switch.SwitchName=%1").arg(SwitchName);
//            QSqlQuery sqlID(selectID);
//            while(sqlID.next())
//            {
//                SwitchID1=sqlID.value(0).toString();
//            }
//            if(SwitchID.contains(SwitchID1))
//            {
//                continue;
//            }
               if(fq%2==0)
                {
                    if(itSwitch.value().switchName % 2 != 0)//为奇数
                    {
                        itSwitch.value().switchLock = 0x01;
                        MessageListAdd(2,SwitchName,68);
                    }
                }
                else if(fq%2!=0)
                {
                    if(itSwitch.value().switchName % 2 != 0)//为奇数
                    {
                        itSwitch.value().switchLock = 0x02;
                        MessageListAdd(2,SwitchName,70);
                    }
                }
            }
        fq++;
    }
    if(0x02 == direction)//S引导总锁
    {
        for(itSwitch = SwitchDataMap.begin();itSwitch != SwitchDataMap.end();++itSwitch)
        {
            int SwitchName =itSwitch.value().switchName;
                if(fq1%2==0)
                {
                    if(itSwitch.value().switchName % 2 == 0)//为偶数
                    {
                        itSwitch.value().switchLock = 0x01;
                        MessageListAdd(2,SwitchName,68);
                    }
                }
                else if(fq1%2!=0)
                {
                    if(itSwitch.value().switchName % 2 == 0)//为偶数
                    {
                        itSwitch.value().switchLock = 0x02;
                        MessageListAdd(2,SwitchName,70);
                    }
                }
            }
        fq1++;
    }
}

//【04操作·封锁按钮（封锁/取消封锁）】√  在有进路的情况下不能封锁道岔和区段
void InterLock::FengSuo(byte Id,byte Type)
{
    int sectionid;
    QString SWITCHID;
    QString thisid = QString::number(Id, 10);
    QString sectionname;
    int Switchid;
    int switchnameid;
    int lswitchid;

    if(0x01 == Type)//封锁信号机
    {
        if(SignalsDataMap.find(Id).value().blockStatus != 0x01){
            SignalsDataMap.find(Id).value().blockStatus = 0x01;
        }else{
           SignalsDataMap.find(Id).value().blockStatus = 0x02;
        }

    }
//    QString selectsignalname = QString("select *from section WHERE section.Sectionnameid=%1").arg(Id);
//    QSqlQuery sqlsignalname(selectsignalname);
//    while(sqlsignalname.next())
//    {
//        signalname=sqlsignalname.value(1).toString();
//    }

    if(0x03 == Type)//封锁轨道
    {
        qDebug()<<SectionsDataMap.find(thisid).value().blockStatus;
        QString selectsectionname =QString("SELECT *from section WHERE section.Sectionnameid=%1").arg(Id);
        QSqlQuery sqlname(selectsectionname);
        while(sqlname.next())
        {
            sectionid=sqlname.value(0).toInt();
            sectionname=sqlname.value(1).toString();
        }
        QMap<QString,QList<QString>>::iterator it;
        for(it =RuleMap.begin();it !=RuleMap.end();it++)
        {
            if(it.value()[1].contains(sectionname))
            {
                MessageListAdd(1,sectionid,138);
                return;
            }
        }
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
        int frq=0;
        QString selectswitchid = QString("SELECT * from switch where SwitchName IN (select SwitchName from switch WHERE SwitchName = %1)").arg(Id);
        QSqlQuery querySwitchid(selectswitchid);
        while(querySwitchid.next())
        {
            Switchid = querySwitchid.value(0).toInt();
            SWITCHID = querySwitchid.value(0).toString();
            switchnameid = querySwitchid.value(5).toInt();
            QMap<QString,QList<QString>>::iterator it;
            for(it =RuleMap.begin();it !=RuleMap.end();it++)
            {
                if(it.value()[3].contains(SWITCHID))
                {
                    MessageListAdd(2,Switchid,137);
                    return;
                }
            }
            frq++;
            if(frq==1)
            {
                QString selectSW =QString("select *from switch where switch.SwitchNameID=%1").arg(switchnameid);
                QSqlQuery sqlSW(selectSW);
                while(sqlSW.next())
                {
                    lswitchid = sqlSW.value(0).toInt();
                    if(SwitchDataMap.find(lswitchid).value().blockStatus != 0x01)
                    {
                        SwitchDataMap.find(lswitchid).value().blockStatus = 0x01;
                    }
                    else
                    {
                        SwitchDataMap.find(lswitchid).value().blockStatus = 0x02;
                    }
                }
            }
        }
    }
}

//【05操作·区故解】√
void InterLock::QuGJ(byte Snum)
{
    QMap<QString,QList<QString>> test = RuleMap1;
    QString sectionname;//声明①——需要区故解区段名字
    QString SectionId;//声明②——区故解区段ID
    QString SectionIdAdd;//声明③——区故解下一个区段ID
    QString SectionIdReduce;//声明④——区故解上一个区段ID
    int i;//声明⑥——区故解区段在进路锁闭区段中地检索号。
    int zi;
    int SectionIdCount;//声明⑨——区故解区段分轨数量
    int SectionIdAddCount;//声明⑩——区故解下一个区段分轨数量
    int SectionIdReduceCount;//声明十一——区故解上一个区段分轨数量
    QString beginsignalname;
    QString beginsignalname1;
    int signalid;
    QString sectioncache;
    QString NameList2;
    QStringList sectioncache1;
    QString sectioncache2;
    QStringList sectionvalue;
    QStringList NameList1;
//    QList<QString>heihei1;
    //根据区段的nameid查找到区段名字
    QString selectsid = QString("select * from section WHERE section.sectionnameid = %1").arg(Snum);
    QSqlQuery queryid(selectsid);
    while(queryid.next())
    {
        sectionname = queryid.value(1).toString();
    }

    //实现进路区段全被区故解时的进路缓存清除
    QMap<QString,QList<QString>> ::iterator it1;
    for(it1 = RuleMap1.begin();it1 != RuleMap1.end();++it1)
    {
        beginsignalname1 = it1.key();
        if(it1.value()[0].contains(sectionname))
        {
            NameList1 = it1.value()[0].split(",");
            int len=NameList1.length();
            zi=NameList1.indexOf(sectionname);
            if(len>3)
            {
                if(zi==0)
                {
                   NameList1.replace(zi,NULL);
                }
                if(zi==1)
                {
                    NameList1.replace(zi,NULL);
                    NameList1.replace(zi-1,NULL);
                }
                if(zi== len-1)
                {
                    NameList1.replace(zi,NULL);
                }
                if(zi ==len-2)
                {
                    NameList1.replace(zi,NULL);
                    NameList1.replace(zi+1,NULL);
                }
                if(zi>0&&zi<len-1)
                {
                    NameList1.replace(zi,NULL);
                }
            }
            else if(len==3)
            {
                if(zi==0)
                {
                     NameList1.replace(zi,NULL);
                }
                if(zi==1)
                {
                     NameList1.replace(zi,NULL);
                     NameList1.replace(zi+1,NULL);
                }
                if(zi==2)
                {
                     NameList1.replace(zi,NULL);
                }
            }
            else if(len==2||len==1)
            {
                 NameList1.replace(zi,NULL);
            }
            for(int zu=0;zu<NameList1.length();zu++)
            {
                NameList2=NameList1[zu];
                sectioncache1.append(NameList2);
                sectioncache = sectioncache1.join(",");
                sectioncache2 = sectioncache1.join("");

            }
            sectionvalue.append(sectioncache);
            RuleMap1[beginsignalname1]= sectionvalue;
        }
    }
   //循环所有进路
    QMap<QString,QList<QString>> ::iterator it;
    for(it = RuleMap.begin();it != RuleMap.end();++it)
    {
        //如果该条进路的区段字符串拼接中包含需要区故解的区段
        if(it.value()[1].contains(sectionname)){
            beginsignalname = it.key();
            QString selectsignalid = QString("SELECT *from signalinfo WHERE signalinfo.SingalName=\"%1\"").arg(beginsignalname);
            QSqlQuery sqlsignalid(selectsignalid);
            while(sqlsignalid.next())
            {//区故解该进路信号灯变红
                signalid=sqlsignalid.value(0).toInt();
                if(RuleMap.find(beginsignalname).value()[0]=='4')
                {
                    SignalsDataMap.find(signalid).value().signalStatus=0x07;
                }
                else SignalsDataMap.find(signalid).value().signalStatus=0x01;
            }
            QByteArray SwitchNameAndStatus = switchesStrextract(it.value()[2]);//声明⑦——需要区故解地进路中地所有道岔名字+定反位
            QStringList NameList = it.value()[1].split(",");//声明⑧——需要区故解地进路中地所有区段名字
            i = NameList.indexOf(sectionname);//区故解区段在进路锁闭区段中地检索号。
            int len = NameList.length();//进路锁闭区段地数量。
            SectionId = SelectIdForName(NameList[i]);
            SectionIdCount = SelectCountForName(NameList[i]);
            if(i<=NameList.length()-2){//如果区故解地不是最后一个区段
                SectionIdAdd = SelectIdForName(NameList[i+1]);
                SectionIdAddCount = SelectCountForName(NameList[i+1]);
            }
            if(i>0){//如果区故解地不是第一个区段
                SectionIdReduce = SelectIdForName(NameList[i-1]);
                SectionIdReduceCount = SelectCountForName(NameList[i-1]);
            }
            QMap<QString,SectionData>::iterator itsec = SectionsDataMap.find(SectionId);
            //如果进路中发现该区段白光带故障,或锁闭状态
            if(itsec.value().SectionWhiteObstacle == 0x01 || itsec.value().LockStatus == 0x01)
            {
                SwitchWhite(SectionIdCount,sectionname,SwitchNameAndStatus,0x02);//给道岔地白光带故障赋值
                SectionsDataMap.find(SectionId).value().SectionWhiteObstacle = 0x02;//取消该区段的白光带故障
                SectionsDataMap.find(SectionId).value().LockStatus = 0x02;//取消该区段的锁闭状态
                if(len>3)
                {
                    //第一个区段故障
                    if(i == 0)
                    {
                        if(SectionsDataMap.find(SectionIdAdd).value().SectionWhiteObstacle != 0x02 || SectionsDataMap.find(SectionIdAdd).value().LockStatus != 0x02){
                            SectionsDataMap.find(SectionIdAdd).value().SectionWhiteObstacle = 0x01;//且下一位区段变成白光带故障
                            SwitchWhite(SectionIdAddCount,NameList[i+1],SwitchNameAndStatus,0x01);//给道岔地白光带故障赋值
                        }
                    }
                    //第二个区段故障
                    else if(i == 1){
                        if(SectionsDataMap.find(SectionIdAdd).value().SectionWhiteObstacle != 0x02 || SectionsDataMap.find(SectionIdAdd).value().LockStatus != 0x02){
                            SectionsDataMap.find(SectionIdAdd).value().SectionWhiteObstacle = 0x01;//且下一位区段变成白光带故障
                            SwitchWhite(SectionIdAddCount,NameList[i+1],SwitchNameAndStatus,0x01);//给道岔地白光带故障赋值
                        }
                        SectionsDataMap.find(SectionIdReduce).value().SectionWhiteObstacle = 0x02;//连带取消第一个区段地白光带故障
                        SectionsDataMap.find(SectionIdReduce).value().LockStatus = 0x02;//连带取消第一个区段地锁闭状态
                        SwitchWhite(SectionIdReduceCount,NameList[i-1],SwitchNameAndStatus,0x02);//给道岔地白光带故障赋值
                    }
                    //如果是最后一段区段故障
                    else if(i == len-1)
                    {
                        for(int j=i-1;j>=0;j--)
                        {
                            QString SectionIdJ = SelectIdForName(NameList[j]);
                            QMap<QString,SectionData>::iterator itsec1 = SectionsDataMap.find(SectionIdJ);
                            if(itsec1.value().LockStatus == 0x02 && itsec1.value().SectionWhiteObstacle == 0x02){//如果区故解区段之前的区段已经被区故解（即无锁闭且无白光带故障）
                                continue;
                            }else{
                                itsec1.value().SectionWhiteObstacle = 0x01;//且前面的区段全部变成白光带故障
                                int SectionIdCountJ = SelectCountForName(NameList[j]);
                                SwitchWhite(SectionIdCountJ,NameList[j],SwitchNameAndStatus,0x01);//给道岔地白光带故障赋值
                            }
                        }
                    }
                    //如果是倒数第二段区段故障
                    else if(len>2 && i == len-2)
                    {
                        for(int j=i-1;j>=0;j--)
                        {
                            SectionsDataMap.find(SectionIdAdd).value().SectionWhiteObstacle = 0x02;//取消该区段的白光带故障
                            SectionsDataMap.find(SectionIdAdd).value().LockStatus = 0x02;//取消该区段的锁闭状态
                            SwitchWhite(SectionIdAddCount,NameList[i+1],SwitchNameAndStatus,0x02);//给道岔地白光带故障赋值
                            QString SectionIdJ = SelectIdForName(NameList[j]);
                            QMap<QString,SectionData>::iterator itsec1 = SectionsDataMap.find(SectionIdJ);
                            if(itsec1.value().LockStatus == 0x02 && itsec1.value().SectionWhiteObstacle == 0x02){//如果区故解区段之前的区段已经被区故解（即无锁闭且无白光带故障）
                                continue;
                            }else{
                                itsec1.value().SectionWhiteObstacle = 0x01;//且前面的区段全部变成白光带故障
                                int SectionIdCountJ = SelectCountForName(NameList[j]);
                                SwitchWhite(SectionIdCountJ,NameList[j],SwitchNameAndStatus,0x01);//给道岔地白光带故障赋值
                            }
                        }
                    }
                    //如果是进路中的中间区段
                    else if(i>0 && i<len-1)
                    {
                        if(SectionsDataMap.find(SectionIdAdd).value().SectionWhiteObstacle != 0x02 || SectionsDataMap.find(SectionIdAdd).value().LockStatus != 0x02){
                            SectionsDataMap.find(SectionIdAdd).value().SectionWhiteObstacle = 0x01;//且下一位区段变成白光带故障
                            SwitchWhite(SectionIdAddCount,NameList[i+1],SwitchNameAndStatus,0x01);//给道岔地白光带故障赋值
                        }
                        for(int j=i-1;j>=0;j--)
                        {
                            QString SectionIdJ = SelectIdForName(NameList[j]);
                            QMap<QString,SectionData>::iterator itsec1 = SectionsDataMap.find(SectionIdJ);
                            if(itsec1.value().LockStatus == 0x02 && itsec1.value().SectionWhiteObstacle == 0x02){//如果区故解区段之前的区段已经被区故解（即无锁闭且无白光带故障）
                                continue;
                            }else{
                                itsec1.value().SectionWhiteObstacle = 0x01;//且前面的区段全部变成白光带故障
                                int SectionIdCountJ = SelectCountForName(NameList[j]);
                                SwitchWhite(SectionIdCountJ,NameList[j],SwitchNameAndStatus,0x01);//给道岔地白光带故障赋值
                            }
                        }
                    }

                }
                else if(len==3)
                {
                    if(i==0)
                    {
                        if(SectionsDataMap.find(SectionIdAdd).value().SectionWhiteObstacle != 0x02 || SectionsDataMap.find(SectionIdAdd).value().LockStatus != 0x02)
                        {
                            SectionsDataMap.find(SectionIdAdd).value().SectionWhiteObstacle = 0x01;//下一位区段变成白光带故障
                            SwitchWhite(SectionIdAddCount,NameList[i+1],SwitchNameAndStatus,0x01);//给道岔地白光带故障赋值
                        }
                    }
                    if(i==1)
                    {
                        if(SectionsDataMap.find(SectionIdAdd).value().SectionWhiteObstacle != 0x02 || SectionsDataMap.find(SectionIdAdd).value().LockStatus != 0x02)
                        {
                            SectionsDataMap.find(SectionIdAdd).value().SectionWhiteObstacle = 0x01;//下一位区段变成白光带故障
                            SwitchWhite(SectionIdAddCount,NameList[i+1],SwitchNameAndStatus,0x01);//给道岔地白光带故障赋值
                        }
                        SectionsDataMap.find(SectionIdAdd).value().SectionWhiteObstacle = 0x02;
                        SwitchWhite(SectionIdAddCount,NameList[i+1],SwitchNameAndStatus,0x02);
                    }
                    if(i==2)
                    {
                        for(int j=i-1;j>=0;j--)
                        {
                            QString SectionIdJ = SelectIdForName(NameList[j]);
                            QMap<QString,SectionData>::iterator itsec1 = SectionsDataMap.find(SectionIdJ);
                            if(itsec1.value().LockStatus == 0x02 && itsec1.value().SectionWhiteObstacle == 0x02)
                            {//如果区故解区段之前的区段已经被区故解（即无锁闭且无白光带故障）
                                continue;
                            }
                            else
                            {
                                itsec1.value().SectionWhiteObstacle = 0x01;//且前面的区段全部变成白光带故障
                                int SectionIdCountJ = SelectCountForName(NameList[j]);
                                SwitchWhite(SectionIdCountJ,NameList[j],SwitchNameAndStatus,0x01);//给道岔地白光带故障赋值
                            }
                        }
                    }
                }
                else if(len==2)
                {
                    if(i==0)
                    {
                        if(SectionsDataMap.find(SectionIdAdd).value().SectionWhiteObstacle != 0x02 || SectionsDataMap.find(SectionIdAdd).value().LockStatus != 0x02)
                        {
                            SectionsDataMap.find(SectionIdAdd).value().SectionWhiteObstacle = 0x01;//下一位区段变成白光带故障
                            SwitchWhite(SectionIdAddCount,NameList[i+1],SwitchNameAndStatus,0x01);//给道岔地白光带故障赋值
                        }
                    }
                    if(i==1)
                    {
                        if(SectionsDataMap.find(SectionIdReduce).value().SectionWhiteObstacle != 0x02 || SectionsDataMap.find(SectionIdReduce).value().LockStatus != 0x02)
                        {
                            SectionsDataMap.find(SectionIdReduce).value().SectionWhiteObstacle = 0x01;//上一位区段变成白光带故障
                            SwitchWhite(SectionIdReduceCount,NameList[i-1],SwitchNameAndStatus,0x01);//给道岔地白光带故障赋值
                        }
                    }
                }
            }

        }

    }
    if(sectioncache2.isEmpty())
    {
        RuleMap.remove(beginsignalname1);
    }
}

//【06操作·信号重开】 √
void InterLock::XinHaoCK(byte SignalID)
{
    QString beginSignalName;
    QString endid;
    QString selectname =QString("select *from signalinfo WHERE signalinfo.SignalID=%1").arg(SignalID);
    QSqlQuery sqlname(selectname);
    while(sqlname.next())
    {
        beginSignalName=sqlname.value(3).toString();
        qDebug()<<"beginSignalName:"<<beginSignalName;
    }
    QMap<QString,QList<QString>>::iterator its;
    for(its=RuleMap.begin();its!=RuleMap.end();its++)
    {
        if(RuleMap.keys().contains(beginSignalName))
        {
            for(int i=0;i<RuleMap[beginSignalName].length();i++)
            {
                QString type=RuleMap[beginSignalName][0];
                QString sectionname=RuleMap[beginSignalName][1];
                QStringList SectionName=sectionname.split(",");
                QString sectionend =SectionName[SectionName.length()-1];
                QString selectendid = QString("select *from sectioninfo WHERE sectioninfo.SectionName = \"%1\"").arg(sectionend);
                QSqlQuery sqlendid(selectendid);
                while(sqlendid.next())
                {
                    endid=sqlendid.value(0).toString();
                }
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
                        if(SectionsDataMap.find(sectionid).value().sectionStatus==0x01||SectionsDataMap.find(sectionid).value().SectionRedObstacle==0x01||SectionsDataMap.find(sectionid).value().SectionWhiteObstacle==0x01||SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01||SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            MessageListAdd(3,SignalID,137);
                            return;
                        }
                        else if(endid==sectionid)//检查全部区段都没有故障占用
                            {
                            if(SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                            {
                                if(type=='1')
                                {
                                    SignalsDataMap.find(SignalID).value().signalLockStatus=0x02;
                                    SignalsDataMap.find(SignalID).value().signalStatus=0x0a;
                                }
                                else if(type=='2')
                                {
                                    SignalsDataMap.find(SignalID).value().signalLockStatus=0x02;
                                    SignalsDataMap.find(SignalID).value().signalStatus=0x0b;
                                }
                                else if(type=='3')
                                {
                                    SignalsDataMap.find(SignalID).value().signalLockStatus=0x02;
                                    SignalsDataMap.find(SignalID).value().signalStatus=0x0e;
                                }
                                else if(type=='4')
                                {
                                    SignalsDataMap.find(SignalID).value().signalLockStatus=0x02;
                                    SignalsDataMap.find(SignalID).value().signalStatus=0x0d;
                                }
                                else if(type=='5')
                                {
                                    SignalsDataMap.find(SignalID).value().signalLockStatus=0x02;
                                    SignalsDataMap.find(SignalID).value().signalStatus=0x0d;
                                }
                                if(i==0)
                                {
                                    MessageListAdd(3,SignalID,38);
                                }

                            }
                            else {
                                if(type=='1')
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
                                if(i==0)
                                {
                                    MessageListAdd(3,SignalID,38);
                                }
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
}

//【07操作·道岔总定反,单解锁】 √
void InterLock::ZongDingFanDanJieSuo(byte Snum,byte Sstatus)
{
   int SwitchID;
    QString switchid;
//    int SwitchNameID;
//    int SwitchName;
    int at=0;
    QString STID;//ID字符添加
//    int switchid;
    QString selectSwitchID = QString("select *from switch WHERE switch.SwitchNameID =%1").arg(Snum);
    QSqlQuery sqlSwitchID(selectSwitchID);
    while(sqlSwitchID.next())
    {
        SwitchID=sqlSwitchID.value(0).toInt();
        switchid=sqlSwitchID.value(0).toString();

        if(Sstatus == 0x03)//道岔单锁
                {
                    if(0x01 == SwitchDataMap.find(SwitchID).value().switchLock)
                    {
                        return;
                    }
                    else
                        {
                        SwitchDataMap.find(SwitchID).value().switchLock = 0x01;
                        if(at%2==1)
                        {
                            MessageListAdd(2,SwitchID,62);
                        }

                    }
                    continue;
                }
                //道岔解锁
                if(Sstatus == 0x04)
                {
                    if(0x02 == SwitchDataMap.find(SwitchID).value().switchLock)
                    {
                        return;
                    }
                    else
                    {
                        SwitchDataMap.find(SwitchID).value().switchLock = 0x02;
                        if(at%2==1)
                        {
                            MessageListAdd(2,SwitchID,67);
                        }
                    }
                    continue;

                }

                if(SwitchDataMap.find(SwitchID).value().SwitchLoss==0x01||SwitchDataMap.find(SwitchID).value().switchLock==0x01)//在道岔失表的或锁闭情况
                {
                    if(at==1)
                    {
                        MessageListAdd(2,SwitchID,136);
                    }
                    if(at==3)
                    {
                        MessageListAdd(2,SwitchID,136);
                    }
                    continue;
                }
        STID.append(switchid);
        STID.append(",");
        //如果道岔有故障或占用
        if(SwitchDataMap.find(SwitchID).value().switchred==0x01||SwitchDataMap.find(SwitchID).value().switchOccupy==0x01||SwitchDataMap.find(SwitchID).value().switchwhite==0x01)
        {
            MessageListAdd(2,SwitchID,136);
            STID.clear();
            return;
        }
    }
    QStringList STIDlist =STID.split(",");
    for(int q=0;STIDlist.length();q++)
    {
        QString u=STIDlist[q];
        int w =u.toInt();
        if(w==NULL)//防止最后一位为空
        {
            return;
        }
        //道岔定位
         if(Sstatus == 0x01)
        {
            if(0x00 ==SwitchDataMap.find(w).value().switchPos)//如果是定位就启用
            {

                if(SwitchDataMap.find(w).value().switchStates == 0x01)
                {
                    return;
                }
                else {
                    SwitchDataMap.find(w).value().switchStates = 0x01;
                    MessageListAdd(2,w,60);
                }

            }
            else if(0x01 == SwitchDataMap.find(w).value().switchPos)//如果是反位就不启用
            {
                if(SwitchDataMap.find(w).value().switchStates == 0x00)
                {
                    return;
                }
                else SwitchDataMap.find(w).value().switchStates = 0x00;
            }
        }
        //道岔反位
        if(Sstatus == 0x02)
        {
              if(0x01 == SwitchDataMap.find(w).value().switchPos)//如果是反位就启用
            {
                if(SwitchDataMap.find(w).value().switchStates == 0x01)
                {
                    return;
                }
                else {SwitchDataMap.find(w).value().switchStates = 0x01;
                MessageListAdd(2,w,61);}
            }

            else if(0x00 == SwitchDataMap.find(w).value().switchPos)//如果是定位不启用
            {
                if(SwitchDataMap.find(w).value().switchStates == 0x00)
                {
                    return;
                }
                else SwitchDataMap.find(w).value().switchStates = 0x00;
            }
        }
    }
}

//【08操作·灯丝断丝】 √
void InterLock::DSDS(byte SignalID, byte Status)
{
    QString sectionid;
    QString sectionendid;
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
            QString sectionNameList =RuleMap[beginSignalName][1];
            QStringList sectionName = sectionNameList.split(",");
            QString sectionend =sectionName[sectionName.length()-1];
            QString selectendid =QString("select *from sectioninfo WHERE sectioninfo.SectionName=\"%1\"").arg(sectionend);
            QSqlQuery sqlendid(selectendid);
            while(sqlendid.next())
            {
                sectionendid=sqlendid.value(0).toString();
            }
            for(int in=0;in<sectionName.length();in++)
            {
            QString selectsectionid =QString("select *from sectioninfo WHERE sectioninfo.SectionName=\"%1\"").arg(sectionName[in]);
            QSqlQuery sqlsectionid(selectsectionid);
            while(sqlsectionid.next())
            {
                sectionid=sqlsectionid.value(0).toString();
                //如果进路有故障占用，信号机灯丝断无颜色变化，信号灯锁闭的时候，信号机灯丝断丝无颜色变化
                if(SectionsDataMap.find(sectionid).value().sectionStatus==0x01||SectionsDataMap.find(sectionid).value().SectionRedObstacle==0x01||SectionsDataMap.find(sectionid).value().SectionWhiteObstacle==0x01||SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01||SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01||SignalsDataMap.find(SignalID).value().signalLockStatus==0x01)
                {
                    if(Status==0x01)
                    {
                        if(SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                        {
                            return;
                        }
                        else
                        {
                            MessageListAdd(3,SignalID,31);
                            SignalsDataMap.find(SignalID).value().DSStatus=0x01;
                            return;
                        }

                    }
                    if(Status==0x02)
                    {
                        if(SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            return;
                        }
                        else {
                            MessageListAdd(3,SignalID,33);
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x01;
                            return;
                        }
                    }
                    if(Status==0x03)
                    {
                        if(SignalsDataMap.find(SignalID).value().DSStatus==0x02&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x02)
                        {
                            return;
                        }
                        else
                        {
                            MessageListAdd(3,SignalID,35);
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
                            return;
                        }

                    }
                    if(Status==0x04)
                    {
                        if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                        {
                            return;
                        }
                        else {
                            MessageListAdd(3,SignalID,32);
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x01;
                            return;
                        }
                    }
                    if(Status==0x05)
                    {
                        if(SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            return;
                        }
                        else {
                            MessageListAdd(3,SignalID,34);
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x01;
                            return;
                        }
                    }
                    if(Status==0x06)
                    {
                        if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x02&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x02)
                        {
                            return;
                        }
                        else
                        {
                            MessageListAdd(3,SignalID,36);
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalLockStatus=0x02;
                            return;
                        }

                    }
                }
//                 否则如果区段没有故障且信号机开放的时候
            else if(sectionendid==sectionid&&SignalsDataMap.find(SignalID).value().signalLockStatus==0x02)
            {
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
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,31);
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
                        if(i==0)
                        {
//                            SignalsDataMap.find(SignalID).value().signalLockStatus=0x01;
                            MessageListAdd(3,SignalID,33);
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
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus = 0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                           SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,35);
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
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,32);
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
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x01;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        if(i==0)//只执行一次
                        {
//                            SignalsDataMap.find(SignalID).value().signalLockStatus=0x01;
                            MessageListAdd(3,SignalID,34);
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
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x0a;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x04;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x0a;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,36);
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
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,31);
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
                        if(i==0)
                        {
                            SignalsDataMap.find(SignalID).value().signalLockStatus=0x01;
                            MessageListAdd(3,SignalID,33);
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
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,35);
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
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,32);
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
                        if(i==0)
                        {
                            SignalsDataMap.find(SignalID).value().signalLockStatus=0x01;
                            MessageListAdd(3,SignalID,34);
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
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x0b;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x02;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x0b;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,36);
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
                        if(i==0)//信息提示框只显示一次
                        {
                            MessageListAdd(3,SignalID,31);
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
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,33);
                            SignalsDataMap.find(SignalID).value().signalLockStatus=0x01;
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
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,35);
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
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,32);
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
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,34);
                            SignalsDataMap.find(SignalID).value().signalLockStatus=0x01;
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
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x0e;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x06;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x0e;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x09;
                        }
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,36);
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
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,31);
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
                        if(i==0)
                        {
                            SignalsDataMap.find(SignalID).value().signalLockStatus=0x01;
                            MessageListAdd(3,SignalID,33);
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
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                        }
                        else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().DSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                        }
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,35);
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
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,32);
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
                        if(i==0)
                        {
                            SignalsDataMap.find(SignalID).value().signalLockStatus=0x01;
                            MessageListAdd(3,SignalID,34);
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
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().signalStatus=0x0d;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x00&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x08;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x0d;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x00&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x00&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x00)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                        }
                        else if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01&&SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01&&SignalsDataMap.find(SignalID).value().DSStatus==0x01&&SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01)
                        {
                            SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
                            SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
//                            SignalsDataMap.find(SignalID).value().signalStatus=0x0c;
                        }
                        if(i==0)
                        {
                            MessageListAdd(3,SignalID,36);
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
                            if(i==0)
                            {
                                MessageListAdd(3,SignalID,31);
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
                            if(i==0)
                            {
                                SignalsDataMap.find(SignalID).value().signalLockStatus=0x01;
                                MessageListAdd(3,SignalID,33);
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
                            if(i==0)
                            {
                                MessageListAdd(3,SignalID,35);
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
                            if(i==0)
                            {
                                MessageListAdd(3,SignalID,32);
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
                            if(i==0)
                            {
                                SignalsDataMap.find(SignalID).value().signalLockStatus=0x01;
                                MessageListAdd(3,SignalID,34);
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
                            if(i==0)
                            {
                                MessageListAdd(3,SignalID,36);
                            }
                        }
                }

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
                    MessageListAdd(3,SignalID,31);
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
                    MessageListAdd(3,SignalID,33);
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
                MessageListAdd(3,SignalID,35);
            }
            if(Status==0x04)
            {
                if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                {
                    return;
                }
                else
                {
                    MessageListAdd(3,SignalID,32);
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
                    MessageListAdd(3,SignalID,34);
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
                MessageListAdd(3,SignalID,36);
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
                    MessageListAdd(3,SignalID,31);
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
                    MessageListAdd(3,SignalID,33);
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
                MessageListAdd(3,SignalID,35);
            }
            if(Status==0x04)
            {
                if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                {
                    return;
                }
                else
                {
                    MessageListAdd(3,SignalID,32);
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
                    MessageListAdd(3,SignalID,34);
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
                MessageListAdd(3,SignalID,36);
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
                    MessageListAdd(3,SignalID,31);
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
                    MessageListAdd(3,SignalID,33);
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
                MessageListAdd(3,SignalID,35);
            }
            if(Status==0x04)
            {
                if(SignalsDataMap.find(SignalID).value().GreenDSStatus==0x01)
                {
                    return;
                }
                else
                {
                    MessageListAdd(3,SignalID,32);
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
                    MessageListAdd(3,SignalID,34);
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
                MessageListAdd(3,SignalID,36);
            }
        }
    }

}

//【09操作·灯丝复原】（待测试）√
void InterLock::DSFY(byte Status)
{
    int SignalID;
    QString selectsignalid =QString("select *from signalinfo WHERE signalinfo.signalregion=%1").arg(Status);
    QSqlQuery sqlsignalid(selectsignalid);
    while(sqlsignalid.next())
    {
        SignalID=sqlsignalid.value(0).toInt();
        QString BeginSignalName;
        QString selectsignalname = QString("select *from signalinfo WHERE signalinfo.SignalID=%1").arg(SignalID);
        QSqlQuery sqlName(selectsignalname);
        while(sqlName.next())
        {
            BeginSignalName=sqlName.value(3).toString();

            if(RuleMap.keys().contains(BeginSignalName))
            {
                qDebug()<<RuleMap[BeginSignalName];
                    QString type =RuleMap[BeginSignalName][0];//进路类型
                    qDebug()<<SignalsDataMap.find(SignalID).value().DSStatus;
                    if(SignalsDataMap.find(SignalID).value().RedAllDSStatus==0x01||SignalsDataMap.find(SignalID).value().GreenAllDSStatus==0x01)
                    {
                        if(SignalsDataMap.find(SignalID).value().signalType==0x03)
                        {
                            SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                        }
                        else
                            SignalsDataMap.find(SignalID).value().signalStatus=0x01;
                    }
                    else if(SignalsDataMap.find(SignalID).value().DSStatus==0x01)
                    {
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

                    if(SignalsDataMap.find(SignalID).value().signalType==0x03)
                    {
                        SignalsDataMap.find(SignalID).value().signalStatus=0x07;
                    }
                    else
                        SignalsDataMap.find(SignalID).value().signalStatus=0x01;
            }
        }
        SignalsDataMap.find(SignalID).value().DSStatus=0x00;
        SignalsDataMap.find(SignalID).value().RedAllDSStatus=0x00;
        SignalsDataMap.find(SignalID).value().GreenDSStatus=0x00;
        SignalsDataMap.find(SignalID).value().GreenAllDSStatus=0x00;
        MessageListAdd(3,SignalID,39);
    }

}

//【10操作·区段占用】 √
void InterLock::ZhanYong(byte sectionID)//nameid
{
    QString SectionName;
    QString id = QString::number(sectionID, 10);
    if(SectionsDataMap.find(id).value().sectionStatus==0x01)
    {
        MessageListAdd(1,sectionID,1);
        SectionsDataMap.find(id).value().sectionStatus=0x02;
    }
    else{
        MessageListAdd(1,sectionID,7);
        SectionsDataMap.find(id).value().sectionStatus=0x01;
    }
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
            QString signalname = ruledata.key();
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
                MessageListAdd(3,signalid,30);
            }
            else if(SectionsDataMap.find(id).value().sectionStatus==0x02)
            {
                SignalsDataMap.find(signalid).value().signalLockStatus = 0x02;
            }
        }
        //i++
    }
}

//【11操作道岔失表】 √
void InterLock::SwitchLoss(byte SwitchName)
{
    int switchid;
    int SwitchNameid;
    QString selectSwitchID = QString("select *from switch WHERE SwitchName =%1").arg(SwitchName);
    QSqlQuery sqlSwitchID(selectSwitchID);
    while(sqlSwitchID.next())
    {
        SwitchNameid = sqlSwitchID.value(5).toInt();
    }
    QString selectSwitchid = QString("select *from switch WHERE SwitchNameID =%1").arg(SwitchNameid);
    QSqlQuery sqlSwitchid(selectSwitchid);
    while(sqlSwitchid.next())
    {
        switchid = sqlSwitchid.value(0).toInt();
        SwitchDataMap.find(switchid).value().SwitchLoss=0x01;
        MessageListAdd(2,switchid,133);
        //道岔失表后道岔位置会变成定位
        if(0x00 ==SwitchDataMap.find(switchid).value().switchPos)//如果是定位就启用
        {
            SwitchDataMap.find(switchid).value().switchStates = 0x01;
        }
        else if(0x01 == SwitchDataMap.find(switchid).value().switchPos)//如果是反位就不启用
        {
            SwitchDataMap.find(switchid).value().switchStates = 0x00;
        }
    }

}

//【12操作·道岔取消失表】 √
void InterLock::SwitchNoLoss(byte SwitchName){
    int switchid;
    int switchID;
    int SwitchNameid;
    QString selectSwitchID = QString("select *from switch WHERE SwitchName =%1").arg(SwitchName);
    QSqlQuery sqlSwitchID(selectSwitchID);
    while(sqlSwitchID.next())
    {
        SwitchNameid = sqlSwitchID.value(5).toInt();
        switchID = sqlSwitchID.value(0).toInt();
    }
    QString selectSwitchid = QString("select *from switch WHERE SwitchNameID =%1").arg(SwitchNameid);
    QSqlQuery sqlSwitchid(selectSwitchid);
    while(sqlSwitchid.next())
    {
        switchid = sqlSwitchid.value(0).toInt();
        SwitchDataMap.find(switchid).value().SwitchLoss=0x02;
        MessageListAdd(2,switchID,134);
    }
}

//【13操作·模拟行车】
void InterLock::UnlockState(byte beginSignalID)
{
    QStringList OneSwitch = {"1","10","15","16","2","23","24","27","28","51","52","9"};
    QString SectionId;
    QString SectionIdB;
    QString beginSignalName = QString();
    QString selectBeginSignalStr = QString("select * from signalinfo WHERE signalinfo.SignalID = %1").arg(beginSignalID);
    QSqlQuery queryBeginSignal(selectBeginSignalStr);
    while(queryBeginSignal.next())
    {
        beginSignalName = queryBeginSignal.value(3).toString();
    }
    if(RuleMap.find(beginSignalName).key() != NULL){
       QString Sections = RuleMap.find(beginSignalName).value()[1];
       QStringList SwitchIds = RuleMap.find(beginSignalName).value()[3].split(",");
       int SwitchIndex = 0;
       QString switchid;
       QString switchidB;
       QStringList sectionlist = Sections.split(",");
       for(int i=0;i<2*sectionlist.length()-1;i++){
           int n=(i+1)/2;           //修改道岔的占用和锁闭
           SectionId = SelectIdForName(sectionlist[n]);
           if(n-1>=0)
           {
               SectionIdB = SelectIdForName(sectionlist[n-1]);
           }
           int NameCount = SelectCountForName(sectionlist[n]);
           int NameCountB = 0;
           if(n-1>=0){
               NameCountB = SelectCountForName(sectionlist[n-1]);
           }
           //修改区段的占用和锁闭
           if(i == 0)
           {
               UpdateSectionStatus(SectionId,0x01);
               if(NameCount > 1){//如果有分轨，则有所属道岔，所属道岔
                   switchid = SwitchIds[SwitchIndex];
                   UpdateSwitchOccupy(switchid.toInt(),0x01);
               }
           }
           else
           {
               UpdateSectionStatus(SectionId,0x01);
               if(NameCount > 1){//如果有分轨，则有所属道岔，所属道岔
                   switchid = SwitchIds[SwitchIndex];
                   UpdateSwitchOccupy(switchid.toInt(),0x01);
               }
               if(i % 2 == 0)
               {
                   UpdateSectionStatus(SectionIdB,0x02);
                   UpdateSectionLock(SectionIdB,0x02);
                   if(NameCountB > 1){//如果有分轨，则有所属道岔，所属道岔
                       switchid = SwitchIds[SwitchIndex];
                       UpdateSwitchOccupy(switchid.toInt(),0x02);//道岔解除占用；
                       UpdateSwitchLock(switchid.toInt(),0x02);//道岔解除锁闭；
                       if((SwitchIndex+1) < SwitchIds.length()){
                           switchidB = SwitchIds[SwitchIndex+1];
                           UpdateSwitchLock(switchidB.toInt(),0x02);//联动道岔解除锁闭；
                       }
                       if(OneSwitch.contains(switchid)){//如果是非带动道岔，自增一
                           SwitchIndex++;
                       }else{//如果是带动道岔，自增二
                           SwitchIndex+=2;
                       }
                   }
               }
               if(i % 2 != 0)
               {
                   UpdateSectionStatus(SectionIdB,0x01);
                   if(NameCountB > 1){//如果有分轨，则有所属道岔，所属道岔
                       if(OneSwitch.contains(switchid) && (SwitchIndex+1) < SwitchIds.length()){//如果是非带动道岔，自增一
                           switchidB = SwitchIds[SwitchIndex+1];
                       }else if((SwitchIndex+2) < SwitchIds.length()){//如果是带动道岔，自增二
                           switchidB = SwitchIds[SwitchIndex+2];
                       }else{
                           switchidB = "";
                       }
                       if(switchidB != ""){
                           UpdateSwitchOccupy(switchidB.toInt(),0x01);//道岔被占用；
                       }
                   }
               }
           }
           if(i == 2*sectionlist.length()-2){
                UpdateSectionLock(SectionId,0x02);
           }
           //延迟一秒
           sleep(1000);
        }
        RuleDataMap.remove(beginSignalName);
    }
}

//【1测试·模拟行车】
void InterLock::TestUnlockState(byte beginSignalID)
{
    QList<int> OneSwitch = {1,10,15,16,2,23,24,27,28,51,52,9};
    QString SectionId;
    QString SectionIdB;
    QString beginSignalName = QString();
    QString selectBeginSignalStr = QString("select * from signalinfo WHERE signalinfo.SignalID = %1").arg(beginSignalID);
    QSqlQuery queryBeginSignal(selectBeginSignalStr);
    while(queryBeginSignal.next())
    {
        beginSignalName = queryBeginSignal.value(3).toString();
    }
    if(RuleMap.find(beginSignalName).key() != NULL){
       QString Sections = RuleMap.find(beginSignalName).value()[1];
       QStringList sectionlist = Sections.split(",");
       int SwitId;
       for(int i=0;i<2*sectionlist.length()-1;i++){
           int n=(i+1)/2;           //修改道岔的占用和锁闭
           SectionId = SelectIdForName(sectionlist[n]);//目前行车的区段ID
           SwitId = TestStationSwitchFZ(SectionId,beginSignalName);//目前行车的区段ID所对应地道岔
           if(n-1>=0)
           {
               SectionIdB = SelectIdForName(sectionlist[n-1]);
           }
           int NameCount = SelectCountForName(sectionlist[n]);
           int NameCountB = 0;
           if(n-1>=0){
               NameCountB = SelectCountForName(sectionlist[n-1]);
           }
           //修改区段的占用和锁闭
           if(i == 0)
           {
               UpdateSectionStatus(SectionId,0x01);
               if(NameCount > 1){//如果有分轨，则有所属道岔，所属道岔
                   UpdateSwitchOccupy(SwitId,0x01);
               }
           }
           else
           {
               UpdateSectionStatus(SectionId,0x01);
               if(NameCount > 1){//如果有分轨，则有所属道岔，所属道岔
                   UpdateSwitchOccupy(SwitId,0x01);
               }
               if(i % 2 == 0)
               {
                   UpdateSectionStatus(SectionIdB,0x02);
                   UpdateSectionLock(SectionIdB,0x02);
                   if(NameCountB > 1){//如果有分轨，则有所属道岔，所属道岔
//                       UpdateSwitchOccupy(SwitId,0x02);//道岔解除占用；
//                       UpdateSwitchLock(SwitId,0x02);//道岔解除锁闭；
                       int SwitIdB = TestStationSwitchFZ(SectionIdB,beginSignalName);//目前行车的区段ID所对应地道岔
                       UpdateSwitchLock(SwitIdB,0x02);//联动道岔解除锁闭；
                       UpdateSwitchOccupy(SwitIdB,0x02);//道岔解除占用；
                   }
               }
               if(i % 2 != 0)
               {
                   UpdateSectionStatus(SectionIdB,0x01);
                   if(NameCountB > 1){//如果有分轨，则有所属道岔，所属道岔
                       int SwitIdB = TestStationSwitchFZ(SectionIdB,beginSignalName);//目前行车的区段ID所对应地道岔
                       if(OneSwitch.contains(SwitIdB)){//如果是非带动道岔，自增一
                           UpdateSwitchOccupy(SwitIdB,0x01);//道岔被占用；
                       }
                   }
               }
           }
           if(i == 2*sectionlist.length()-2){
                UpdateSectionLock(SectionId,0x02);
           }
           //延迟一秒
           sleep(1000);
        }
        RuleDataMap.remove(beginSignalName);
    }
}

//【2测试辅助·区段道岔相关】
int InterLock::TestStationSwitchFZ(QString SectionId,QString beginSignalName){
    QString switchid;
    int SwitchName;
    if(TestStationSwitchDataMap.keys().contains(SectionId)){//如果是有道岔的区段
         SwitchName = TestStationSwitchDataMap.find(SectionId).value();//查找区段所对应道岔的名字
         QString SectionSwitchStr = QString("SELECT * FROM switch WHERE SwitchName = %1").arg(SwitchName);
         QSqlQuery SectionSwitchQuery(SectionSwitchStr);
         while(SectionSwitchQuery.next())
         {
             switchid = SectionSwitchQuery.value(0).toString();
             if(RuleMap.find(beginSignalName).value()[3].split(",").contains(switchid)){//根据进路中所保存的道岔定反位，选择区段所对应道岔的符合进路定反位的道岔ID
                return switchid.toInt();
             }
         }
    }
}

//【14操作·红白光带故障】  √
void InterLock::HBGZ(byte sectionnameid, byte status)
{
    QString sectionName;
    int sectionmenameid;
    int sectionforswitch;
    QString sectionid;
    QString sectionendid;
    int signalid;
    QString id =QString::number(sectionnameid,10);
    QString selectmename = QString("select *from section WHERE section.sectionnameid=%1").arg(sectionnameid);
    QSqlQuery sqlmename(selectmename);
    while(sqlmename.next())
    {
        sectionmenameid = sqlmename.value(3).toInt();
    }
    if(status==0x01)//红光带故障
    {
        if(SectionsDataMap.find(id).value().SectionRedObstacle==0x01)
        {
            return;
        }
        else
        {
            MessageListAdd(1,sectionmenameid,3);
            SectionsDataMap.find(id).value().SectionRedObstacle=0x01;
        }
    }
    else if(status==0x02)//白光带故障
    {
            if(SectionsDataMap.find(id).value().SectionWhiteObstacle==0x01)
            {
                return;
            }
            else
            {
                MessageListAdd(1,sectionmenameid,4);
                SectionsDataMap.find(id).value().SectionWhiteObstacle=0x01;
            }
    }
    else if(status==0x03)
    {
        if(SectionsDataMap.find(id).value().SectionRedObstacle==0x02&&SectionsDataMap.find(id).value().SectionWhiteObstacle==0x02)
        {
            return;
        }
        else
        {
            MessageListAdd(1,sectionmenameid,8);
            SectionsDataMap.find(id).value().SectionRedObstacle=0x02;
            SectionsDataMap.find(id).value().SectionWhiteObstacle=0x02;
        }

    }
    QList<int> seforsw;
    QString selectsecName =QString("select *from section WHERE section.sectionnameid=%1").arg(id);
    QSqlQuery sqlname(selectsecName);
    while(sqlname.next())
    {
        sectionName=sqlname.value(1).toString();
        sectionforswitch =sqlname.value(4).toInt();
        if(sectionforswitch!=NULL)
        {
            seforsw.append(sectionforswitch);
        }
        for(int i=0;i<seforsw.length();i++)
        {
            if(SwitchDataMap.find(seforsw[i]).value().switchStates==0x01)//反位
            {
                if(status==0x01)
                {
                    SwitchDataMap.find(seforsw[i]).value().switchred=0x01;//红光带故障
                }
                if(status==0x02)
                {
                    SwitchDataMap.find(seforsw[i]).value().switchwhite=0x01;//白光带故障
                }
                if(status==0x03)
                {
                    SwitchDataMap.find(seforsw[i]).value().switchred=0x02;//红光带正常
                    SwitchDataMap.find(seforsw[i]).value().switchwhite=0x02;//白光带正常
                }
            }
        }
    }
    QMap<QString,QList<QString>>::iterator ruledata;
    for(ruledata = RuleMap.begin();ruledata !=RuleMap.end();++ruledata)
    {
        if(ruledata.value()[1].contains(sectionName))
        {
            QString signalname =ruledata.key();
            QString sectionNameList = ruledata.value()[1];
            QStringList sectionName = sectionNameList.split(",");
            QString sectionend = sectionName[sectionName.length()-1];
            QString selectsectionendid =QString("select *from sectioninfo WHERE sectioninfo.SectionName=\"%1\"").arg(sectionend);
            QSqlQuery sqlsectionendid(selectsectionendid);
            while(sqlsectionendid.next())
            {
                sectionendid=sqlsectionendid.value(0).toString();
            }
            for(int as=0;as<sectionName.length();as++)
            {
                QString selectsectionid =QString("select *from sectioninfo WHERE sectioninfo.SectionName=\"%1\"").arg(sectionName[as]);
                QSqlQuery sqlsectionid(selectsectionid);
                while(sqlsectionid.next())
                {
                    sectionid = sqlsectionid.value(0).toString();
                    QString selectsignalid =QString("select * from signalinfo WHERE SingalName = \"%1\"").arg(signalname);
                    QSqlQuery sqlsignalid(selectsignalid);
                    while(sqlsignalid.next())
                    {
                        signalid =sqlsignalid.value(0).toInt();
                    }
                    if(SectionsDataMap.find(sectionid).value().SectionWhiteObstacle==0x01||SectionsDataMap.find(sectionid).value().SectionRedObstacle==0x01)
                    {
                        SignalsDataMap.find(signalid).value().signalStatus=0x01;
                        SignalsDataMap.find(signalid).value().signalLockStatus = 0x01;
                        MessageListAdd(3,signalid,30);
                        return;
                    }
                    if(sectionendid==sectionid)
                    {
                        SignalsDataMap.find(signalid).value().signalLockStatus = 0x01;
                    }

                }
            }

        }
    }
}

//【01辅助·非阻塞延迟方法·模拟行车模块用】
void InterLock::sleep(unsigned int msec){
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

//【02辅助·道岔字符串信息提取·全局】
QByteArray InterLock::switchesStrextract(QString switchesStr)
{
    QByteArray switchesInfo;
    QStringList switchesSplit = switchesStr.split(",");
    for(int i=0; i<switchesSplit.length(); i++)
    {
        QByteArray switchesData;
        QRegExp rxMethod1(tr("^[\\(](\\d+)/(\\d+)[\\)]$"));//匹配(1/3)类型，反位
        if(rxMethod1.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod1.cap(1).toInt();
            switchesData[1] = 0x01;
            switchesData[2] = rxMethod1.cap(2).toInt();
            switchesData[3] = 0x01;
        }
        QRegExp rxMethod2(tr("^(\\d+)/(\\d+)$"));//匹配1/3类型，定位
        if(rxMethod2.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod2.cap(1).toInt();
            switchesData[1] = 0x02;
            switchesData[2] = rxMethod2.cap(2).toInt();
            switchesData[3] = 0x02;
        }
        QRegExp rxMethod3(tr("^[\\(](\\d+)[\\)]$"));//匹配(5)类型，单边型反位
        if(rxMethod3.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod3.cap(1).toInt();
            switchesData[1] = 0x01;
        }
        QRegExp rxMethod4(tr("^(\\d+)$"));//匹配5类型，单边型反位
        if(rxMethod4.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod4.cap(1).toInt();
            switchesData[1] = 0x02;
        }
        QRegExp rxMethod5(tr("^[\\[](\\d+)/(\\d+)[\\]]$"));//匹配[1/3]类型，防护到定位
        if(rxMethod5.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod5.cap(1).toInt();
            switchesData[1] = 0x02;
            switchesData[2] = rxMethod5.cap(2).toInt();
            switchesData[3] = 0x02;
        }
        QRegExp rxMethod6(tr("^[\\[][\\(](\\d+)/(\\d+)[\\)][\\]]$"));//匹配[(1/3)]类型 防护到反位
        if(rxMethod6.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod6.cap(1).toInt();
            switchesData[1] = 0x01;
            switchesData[2] = rxMethod6.cap(2).toInt();
            switchesData[3] = 0x01;
        }
        QRegExp rxMethod7(tr("^[\\[](\\d+)[\\]]$"));//匹配[5]类型 单边防护到定位
        if(rxMethod7.exactMatch(switchesSplit[i])) {
            switchesData[0] = rxMethod7.cap(1).toInt();
            switchesData[1] = 0x02;
        }
        QRegExp rxMethod8(tr("^[\\[][\\(](\\d+)[\\)][\\]]$"));//匹配[(5)]类型 单边防护到反位
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

//【03辅助·根据名字查询区段ID】
QString InterLock::SelectIdForName(QString Name){
    QString SectionId;
    QString SectionIdSql = QString("SELECT * FROM sectioninfo WHERE SectionName = \"%1\"").arg(Name);
    QSqlQuery querySectionId(SectionIdSql);
    while(querySectionId.next())//占用区段地分轨数量
    {
        SectionId = querySectionId.value(0).toString();
    }
    return SectionId;
}

//【04辅助·根据名字查询区段是否有分轨】
int InterLock::SelectCountForName(QString Name){
    QString SectionIdSql = QString("SELECT * FROM section WHERE SectionName = \"%1\"").arg(Name);
    QSqlQuery querySectionId(SectionIdSql);
    int count = querySectionId.size();
    return count;
}

//【05辅助·信息提示框信息增加】
void InterLock::MessageListAdd(int type,int id,int messageid){
    MessageList.append(type);
    MessageList.append(id);
    MessageList.append(messageid);
}

//【06辅助·根据名字查询区段是否有分轨】
QList<int> InterLock::SelectSwitchIdForName(QString Name){
    int switchid;
    QList<int> switchforname;
    QString SectionIdSql = QString("SELECT * FROM switch WHERE SwitchName = %1").arg(Name);
    QSqlQuery querySectionId(SectionIdSql);
    while(querySectionId.next())//占用区段地分轨数量
    {
        switchid = querySectionId.value(0).toInt();
        switchforname.append(switchid);
    }
    return switchforname;
}

//【07辅助·根据区段名给所途径道岔赋值白光带故障】
void InterLock::SwitchWhite(int count,QString SectionName,QByteArray SwitchNameAndStatus,byte data){
    if(count>1){//如果区故解区段有分轨
       QString SwitchName = SectionName.section("D",0,0);//通过截取获取道岔名字
       int switchIndex = SwitchNameAndStatus.indexOf(SwitchName.toInt());//区故解道岔地检索号
       QList<int> switchlist = SelectSwitchIdForName(SwitchName);//同道岔名的定反位ID
       if(SwitchNameAndStatus[switchIndex+1].operator == (0x02)){//定位
           UpdateSwitch(switchlist[0],"switchwhite",data,0x00);
           //SwitchDataMap.find(switchlist[0]).value().switchwhite = data;//则所经过道岔白光带故障
       }else if(SwitchNameAndStatus[switchIndex+1].operator == (0x01)){//反位
           UpdateSwitch(switchlist[1],"switchwhite",data,0x00);
           //SwitchDataMap.find(switchlist[1]).value().switchwhite = data;//则所经过道岔白光带故障
       }
       if(data == 0x02){
           UpdateSwitch(switchlist[0],"switchLock",data,0x00);
           UpdateSwitch(switchlist[1],"switchLock",data,0x00);
       }
    }
}

//【08辅助·修改道岔的占用】
void InterLock::UpdateSwitchOccupy(int switchid,byte data){
    if(SwitchDataMap.find(switchid).value().switchOccupy != data){
        SwitchDataMap.find(switchid).value().switchOccupy = data;//道岔被占用；
        if(data == 0x01){
           MessageListAdd(2,switchid,69);
        }else if(data == 0x02){
           MessageListAdd(2,switchid,70);
        }
        if(switchid == 48 || switchid == 14 || switchid == 46 || switchid == 20){//防护反位
            if(switchid == 48){
                SwitchDataMap.find(13).value().switchOccupy = data;//道岔被占用；
            }else if(switchid == 14){
                SwitchDataMap.find(47).value().switchOccupy = data;//道岔被占用；
            }else if(switchid == 46){
                SwitchDataMap.find(19).value().switchOccupy = data;//道岔被占用；
            }else if(switchid == 20){
                SwitchDataMap.find(45).value().switchOccupy = data;//道岔被占用；
            }
        }
    }
}

//【09辅助·修改道岔的锁闭】
void InterLock::UpdateSwitchLock(int switchid,byte data){
    if(SwitchDataMap.find(switchid).value().switchRoute != data){
        SwitchDataMap.find(switchid).value().switchRoute = data;//道岔被锁闭；
        if(data == 0x01){
           MessageListAdd(2,switchid,68);
        }else if(data == 0x02){
           MessageListAdd(2,switchid,71);
        }
        if(switchid == 48 || switchid == 14 || switchid == 46 || switchid == 20){//防护反位
            if(switchid == 48){
                SwitchDataMap.find(13).value().switchRoute = data;//道岔被占用；
            }else if(switchid == 14){
                SwitchDataMap.find(47).value().switchRoute = data;//道岔被占用；
            }else if(switchid == 46){
                SwitchDataMap.find(19).value().switchRoute = data;//道岔被占用；
            }else if(switchid == 20){
                SwitchDataMap.find(45).value().switchRoute = data;//道岔被占用；
            }
        }
    }
}

//【10辅助·修改区段的占用】
void InterLock::UpdateSectionStatus(QString sectionid,byte data){
    if(SectionsDataMap.find(sectionid).value().sectionStatus != data){
        SectionsDataMap.find(sectionid).value().sectionStatus = data;
        if(data == 0x01){
            MessageListAdd(1,sectionid.toInt(),1);
        }else if(data == 0x02){
            MessageListAdd(1,sectionid.toInt(),7);
        }
    }
}

//【11辅助·修改区段的锁闭】
void InterLock::UpdateSectionLock(QString sectionid,byte data){
    if(SectionsDataMap.find(sectionid).value().LockStatus != data){
        SectionsDataMap.find(sectionid).value().LockStatus = data;
        if(data == 0x01){//区段锁闭
            MessageListAdd(1,sectionid.toInt(),6);
        }else if(data == 0x02){//区段解除锁闭
            MessageListAdd(1,sectionid.toInt(),9);
        }
    }
}

//【12辅助·修改区段状态】
void InterLock::UpdateSection(QString SectionId,QString Attribute,byte data){
    if(Attribute == "LockStatus"){//修改区段锁闭
        if(SectionsDataMap.find(SectionId).value().LockStatus != data){
            SectionsDataMap.find(SectionId).value().LockStatus = data;
            if(data == 0x01){
                MessageListAdd(1,SectionId.toInt(),6);//区段锁闭
            }else if(data == 0x02){
                MessageListAdd(1,SectionId.toInt(),9);//区段解除锁闭
            }else if(data == 0x03){
                MessageListAdd(1,SectionId.toInt(),6);//区段引导锁闭
            }else if(data == 0x04){
                MessageListAdd(1,SectionId.toInt(),10);//区段隐性锁闭（带动道岔，防护道岔）
            }
        }
    }else if(Attribute == "SectionStatus"){//修改区段占用
        if(SectionsDataMap.find(SectionId).value().sectionStatus != data){
            SectionsDataMap.find(SectionId).value().sectionStatus = data;
            if(data == 0x01){
                MessageListAdd(1,SectionId.toInt(),1);//区段占用
            }else if(data == 0x02){
                MessageListAdd(1,SectionId.toInt(),7);//区段解除占用
            }else if(data == 0x03){
                MessageListAdd(1,SectionId.toInt(),11);//区段隐性占用（带动道岔，防护道岔）
            }
        }
    }else if(Attribute == "SectionWhiteObstacle"){//修改区段白光带故障
        if(SectionsDataMap.find(SectionId).value().SectionWhiteObstacle != data){
            SectionsDataMap.find(SectionId).value().SectionWhiteObstacle = data;
            if(data == 0x01){
                MessageListAdd(1,SectionId.toInt(),4);//区段被白光带故障
            }else if(data == 0x02){
                MessageListAdd(1,SectionId.toInt(),12);//区段区段白光带故障解除
            }
        }
    }else if(Attribute == "SectionRedObstacle"){//修改区段红光带故障
        if(SectionsDataMap.find(SectionId).value().SectionRedObstacle != data){
            SectionsDataMap.find(SectionId).value().SectionRedObstacle = data;
            if(data == 0x01){
                MessageListAdd(1,SectionId.toInt(),3);//区段被白光带故障
            }else if(data == 0x02){
                MessageListAdd(1,SectionId.toInt(),13);//区段区段白光带故障解除
            }
        }
    }else if(Attribute == "blockStatus"){//修改区段封锁
        if(SectionsDataMap.find(SectionId).value().blockStatus != 0x01)
        {
            SectionsDataMap.find(SectionId).value().blockStatus = 0x01;
            MessageListAdd(1,SectionId.toInt(),2);//区段被封锁
        }else{
            SectionsDataMap.find(SectionId).value().blockStatus = 0x02;
            MessageListAdd(1,SectionId.toInt(),14);//区段解除封锁
        }
    }
}

//【13辅助·根据道岔id修改道岔状态】
void InterLock::UpdateSwitch(int SwitchId,QString Attribute,byte data,byte switchPos){
    if(Attribute == "switchLock"){//修改道岔锁闭
        if(SwitchDataMap.find(SwitchId).value().switchRoute != data){
            SwitchDataMap.find(SwitchId).value().switchRoute = data;
            if(data == 0x01){
                MessageListAdd(2,SwitchId,62);//道岔锁闭
            }else if(data == 0x02){
                MessageListAdd(2,SwitchId,71);//道岔解除锁闭
            }else if(data == 0x03){
                MessageListAdd(2,SwitchId,6);//道岔隐性锁闭
            }
            if(SwitchId == 48 || SwitchId == 14 || SwitchId == 46 || SwitchId == 20){//防护反位
                if(SwitchId == 48){
                    SwitchDataMap.find(13).value().switchRoute = data;//道岔被占用；
                }else if(SwitchId == 14){
                    SwitchDataMap.find(47).value().switchRoute = data;//道岔被占用；
                }else if(SwitchId == 46){
                    SwitchDataMap.find(19).value().switchRoute = data;//道岔被占用；
                }else if(SwitchId == 20){
                    SwitchDataMap.find(45).value().switchRoute = data;//道岔被占用；
                }
            }
        }
    }else if(Attribute == "switchStates"){//修改道岔定反位
        if(SwitchDataMap.find(SwitchId).value().switchStates != data){
            if(switchPos == 0x00){//道岔定位
                if(data == 0x01){
                    SwitchDataMap.find(SwitchId).value().switchStates = 0x01;//启用该道岔的定位
                    SwitchDataMap.find(SwitchId + 1).value().switchStates = 0x00;//关闭该道岔的反位
                    MessageListAdd(2,SwitchId,60);//道岔定位操作
                }else if(data == 0x02){
                    SwitchDataMap.find(SwitchId).value().switchStates = 0x01;//启用该道岔的反位
                    SwitchDataMap.find(SwitchId - 1).value().switchStates = 0x00;//关闭该道岔的定位
                    MessageListAdd(2,SwitchId,61);//道岔反位操作
                }
            }else if(switchPos == 0x01){//道岔反位
                if(data == 0x01){
                    MessageListAdd(2,SwitchId,62);//道岔锁闭
                }else if(data == 0x02){
                    MessageListAdd(2,SwitchId,71);//道岔解除锁闭
                }else if(data == 0x03){
                    MessageListAdd(2,SwitchId,6);//道岔隐性锁闭
                }
            }
        }
    }else if(Attribute == "blockStatus"){//修改道岔封锁
        if(SwitchDataMap.find(SwitchId).value().blockStatus != data){
            SwitchDataMap.find(SwitchId).value().blockStatus = data;
            if(data == 0x01){
                MessageListAdd(2,SwitchId,65);//道岔封锁
            }else if(data == 0x02){
                MessageListAdd(2,SwitchId,73);//道岔解除封锁
            }
        }
    }else if(Attribute == "SwitchLoss"){//修改道岔失表
        if(SwitchDataMap.find(SwitchId).value().SwitchLoss != data){
            SwitchDataMap.find(SwitchId).value().SwitchLoss = data;
            if(data == 0x01){
                MessageListAdd(2,SwitchId,63);//道岔失表
            }else if(data == 0x02){
                MessageListAdd(2,SwitchId,74);//道岔解除失表
            }
        }
    }else if(Attribute == "switchOccupy"){//修改道岔占用
        if(SwitchDataMap.find(SwitchId).value().switchOccupy != data){
            SwitchDataMap.find(SwitchId).value().switchOccupy = data;
            if(data == 0x01){
                MessageListAdd(2,SwitchId,69);//道岔占用
            }else if(data == 0x02){
                MessageListAdd(2,SwitchId,70);//道岔解除占用
            }
            if(SwitchId == 48 || SwitchId == 14 || SwitchId == 46 || SwitchId == 20){//防护反位
                if(SwitchId == 48){
                    SwitchDataMap.find(13).value().switchOccupy = data;//道岔被占用；
                }else if(SwitchId == 14){
                    SwitchDataMap.find(47).value().switchOccupy = data;//道岔被占用；
                }else if(SwitchId == 46){
                    SwitchDataMap.find(19).value().switchOccupy = data;//道岔被占用；
                }else if(SwitchId == 20){
                    SwitchDataMap.find(45).value().switchOccupy = data;//道岔被占用；
                }
            }
        }
    }else if(Attribute == "switchred"){//修改道岔红光带故障
        if(SwitchDataMap.find(SwitchId).value().switchred != data){
            SwitchDataMap.find(SwitchId).value().switchred = data;
            if(data == 0x01){
                MessageListAdd(2,SwitchId,68);//道岔红光带故障
            }else if(data == 0x02){
                MessageListAdd(2,SwitchId,75);//道岔解除红光带故障
            }
        }
    }else if(Attribute == "switchwhite"){//修改道岔白光带故障
        if(SwitchDataMap.find(SwitchId).value().switchwhite != data){
            SwitchDataMap.find(SwitchId).value().switchwhite = data;
            if(data == 0x01){
                MessageListAdd(2,SwitchId,67);//道岔白光带故障
            }else if(data == 0x02){
                MessageListAdd(2,SwitchId,76);//道岔解除白光带故障
            }
        }
    }
}

//【发送信号】
void InterLock::TimerTicked()
{
    QByteArray data = SectionEncapsalutation();
    udpSocket->writeDatagram(data,QHostAddress("127.0.0.1"),4401);
    QByteArray data1 = SignalEncapsalutation();
    udpSocket->writeDatagram(data1,QHostAddress("127.0.0.1"),4402);
    QByteArray data2 = SwitchEncapsalutation();
    udpSocket->writeDatagram(data2,QHostAddress("127.0.0.1"),4403);
}

//【测试·道岔区段相关初始化】
void InterLock::TestStationSwitch(){
    QString sectionid;
    int switchname;
    QSqlQuery query("SELECT * FROM section_switch");
    while (query.next())
    {
        sectionid = query.value(2).toString();
        switchname = query.value(1).toInt();
        TestStationSwitchDataMap[query.value(2).toString()] = switchname;
    }
}

//设置信号机
void InterLock::SetupSignal(byte SignalID,byte signalStatus)
{
    //0灰色禁用，1位单红日常，2为双黄，3为双黄闪，4为单黄，5为绿黄，6为单绿,7为单蓝,8为白
    SignalsDataMap.find(SignalID).value().signalStatus = signalStatus;
}

//全站区段信息初始化
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

//全站区段信息发送
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

//全站信号机信息初始化
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

//全站信号机信息发送
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

//全站道岔信息初始化
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
        switchData.SwitchLoss          =0x02;                               //道岔失表 1为失表 2为正常
        switchData.switchred           =0x02;                               //道岔红光带故障  1为故障 2清除故障
        switchData.switchwhite         =0x02;                               //道岔白光带故障  1为故障 2清除故障
        switchData.switchRoute         =0x02;                               //道岔进路状态  1为进路 2无进路
        SwitchDataMap[querySwitch.value(0).toInt()] = switchData;
    }
}

//全站道岔信息发送
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
    frameHead[1] = (switchNr*12)+5;
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

//按钮1
void InterLock::on_pushButton_clicked()
{
    //SetupRoute(1,8);
}
//按钮2
void InterLock::on_pushButton_2_clicked()
{
    //RemoveRoute(1,1);
}
