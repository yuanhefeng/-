#include "piointerlockingdrive.h"     //PIO驱动


PioInterlockingDrive::PioInterlockingDrive(QObject *parent) : QObject(parent)
{
    DOConfigureDevice();//数据输出配置设备
    DIConfigureDevice();//数据输入配置设备
}
void PioInterlockingDrive::DOConfigureDevice()//数据输出配置设备
{
    instantDoCtrl = InstantDoCtrl::Create();
    //Device Tree是一种描述硬件的数据结构
    Array<DeviceTreeNode> *supportedDevices = instantDoCtrl->getSupportedDevices();//设备树，支持的设备
    if (supportedDevices->getCount() == 0)//支持的设备数为0
    {
    }else {
        for (int i = 0; i < supportedDevices->getCount(); i++) {
            DeviceTreeNode const &node = supportedDevices->getItem(i);//结点
            qDebug("%d, %ls\n", node.DeviceNumber, node.Description);
            DeviceInformation devInfo(node.DeviceNumber);//设备信息
            ErrorCode errorCode = instantDoCtrl->setSelectedDevice(devInfo);//错误码
            if(errorCode != 0)
            {
            }
        }
    }
    GetDOPortState();//获取输出设备端口状态
}

void PioInterlockingDrive::DIConfigureDevice()//数据输入配置设备
{
    //DI设备配置
    instantDICtrl = InstantDiCtrl::Create();
    Array<DeviceTreeNode>* sptedDiDevices = instantDICtrl->getSupportedDevices();
    if (sptedDiDevices->getCount() == 0)
    {
    }
    else {
        for (int i = 0; i < sptedDiDevices->getCount(); ++i)
        {
            DeviceTreeNode const & node = sptedDiDevices->getItem(i);
            qDebug("%d, %s\n", node.DeviceNumber, node.Description);
            DeviceInformation devInfo(node.DeviceNumber);
            ErrorCode errorCode = instantDICtrl->setSelectedDevice(devInfo);
            if (errorCode != 0) {
            }
        }
    }
    portDiCount = instantDICtrl->getFeatures()->getPortCount();
}

void PioInterlockingDrive::GetDOPortState()//获取输出设备端口状态
{
    portDoCount = instantDoCtrl->getPortCount();
    quint8 *portStates = new quint8[portDoCount];
    ErrorCode errorCode = Success;
    errorCode = instantDoCtrl->Read(0, portDoCount, portStates);
    CheckError(errorCode);
    for (int i = 0; i < portDoCount; i++)
    {
    //获取DO状态
    }
}
void PioInterlockingDrive::SetDOPortSate(int portNum,byte data) //设置输出设备端口状态
{
    int clickDoPort = portNum;
    ErrorCode errorCode = Success;
    errorCode = instantDoCtrl->Write(clickDoPort, 1, &data);
    CheckError(errorCode);
}

//void PioCollectionDrive::ReadDiPortState() {
//    //read DI data
//    quint8 *portStates = new quint8[portDiCount];
//    ErrorCode errorCode = Success;
//    errorCode = instantDICtrl->Read(0, portDiCount, portStates);
//    CheckError(errorCode);

//    for (int i = 0; i < portDiCount; i++) {
//        int bitValue = 0;
//        for (int j = 7; j >= 0; j--) {
//            bitValue = (portStates[i] >> (7 - i)) & 0x1;
//            //从板卡标号0到3
//                //得到标号从低到高每位的状态（左高位|右低位）
//        }
//    }
//}
quint8* PioInterlockingDrive::ReadDiPortState()//读取端口状态
{
    //read DI data
    portStates = new quint8[portDiCount];
    ErrorCode errorCode = Success;
    errorCode = instantDICtrl->Read(0, portDiCount, portStates);
    CheckError(errorCode);
    return portStates;
//    for (int i = 0; i < portDiCount; i++) {
//        int bitValue = 0;
//        for (int j = 7; j >= 0; j--) {
//            bitValue = (portStates[i] >> (7 - i)) & 0x1;
//            //从板卡标号0到3
//                //得到标号从低到高每位的状态（左高位|右低位）
//        }
//    }
}
void PioInterlockingDrive::CheckError(ErrorCode errorCode)
{
    if (BioFailed(errorCode))
    {

    }
}
