#include "deviceinfoparser.h"
#include <QObject>
#include <sys/utsname.h>
#include <iostream>
#include <QFile>
#include <QProcess>
#include <QRegExp>
#include <DLog>
#include <com_deepin_daemon_power.h>
#include "hwinfohandler.h"
#include "deviceattributedefine.h"

using PowerInter = com::deepin::daemon::Power;

DCORE_USE_NAMESPACE

static QProcess process_;

const QString& DeviceInfoParser::qureyData(const QString& toolname, const QString& firstKey, const QString& secondKey)
{
    static QString result = DeviceAttributeUnknown;
    if(false == toolDatabase_.contains(toolname))
    {
        return result;
    }

    if(false == toolDatabase_[toolname].contains(firstKey))
    {
        return result;
    }

    if(false == toolDatabase_[toolname][firstKey].contains(secondKey))
    {
        return result;
    }

    return toolDatabase_[toolname][firstKey][secondKey];
}

const QString& DeviceInfoParser::fuzzyQueryData(const QString& toolname, const QString& firstKey, const QString& secondKey)
{
    static QString result = DeviceAttributeUnknown;
    if(false == toolDatabase_.contains(toolname))
    {
        return result;
    }

    foreach(const QString& fk, toolDatabase_[toolname].uniqueKeys() )
    {
        if( fk.contains(firstKey) )
        {
            foreach(const QString& sk, toolDatabase_[toolname][fk].uniqueKeys() )
            {
                if( sk.contains(secondKey) )
                {
                    return toolDatabase_[toolname][fk][sk];
                }
            }
        }
    }

    return result;
}

QStringList DeviceInfoParser::getMemorynameList()
{
    QStringList memList;

    if(false == toolDatabase_.contains("dmidecode"))
    {
        return memList;
    }

    foreach(const QString& fk, toolDatabase_["dmidecode"].uniqueKeys() )
    {
        if( fk == "Memory Device" || fk.contains("Memory Device_"))
        {
            memList.push_back(fk);
        }
    }

    return memList;
}

QStringList DeviceInfoParser::getDisknameList()
{
    QStringList diskList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return diskList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        if( fk.contains("disk") && false == fk.contains("volume"))
        {
            diskList.push_back(fk);
        }
    }

    return diskList;
}

QStringList DeviceInfoParser::getDiaplayadapterList()
{
    QStringList displayadapterList;

    if(false == toolDatabase_.contains("lspci"))
    {
        return displayadapterList;
    }

    foreach(const QString& fk, toolDatabase_["lspci"].uniqueKeys() )
    {
        if( fk.contains("VGA compatible controller") )
        {
            displayadapterList.push_back(fk);
        }
    }

    return displayadapterList;
}

QStringList DeviceInfoParser::getDisplayInterfaceList()
{
    QStringList interfaceList;

    if(false == toolDatabase_.contains("xrandr"))
    {
        return interfaceList;
    }

    foreach(const QString& fk, toolDatabase_["xrandr"].uniqueKeys() )
    {
        int index = fk.indexOf(Devicetype_Xrandr_Disconnected);
        if(index < 1)
        {
            index = fk.indexOf(Devicetype_Xrandr_Connected);
        }

        if( index < 0 )
        {
            continue;
        }

        QString interface = fk.mid(0, index).trimmed();
        index = interface.indexOf('-');
        if( index > 0 )
        {
            interface = interface.mid(0, index);
        }
        if( false == interfaceList.contains(interface) )
        {
            interfaceList.push_back(interface);
        }
    }

    return interfaceList;
}

QStringList DeviceInfoParser::getMonitorList()
{
    QStringList monitorList;

    if(false == toolDatabase_.contains("hwinfo"))
    {
        return monitorList;
    }

    foreach(const QString& fk, toolDatabase_["hwinfo"].uniqueKeys() )
    {
        monitorList.push_back(fk);
    }

    return monitorList;
}

QStringList DeviceInfoParser::getConnectedMonitorList()
{
    QStringList connectedMonitorList;

    if(false == toolDatabase_.contains("xrandr"))
    {
        return connectedMonitorList;
    }

    foreach(const QString& fk, toolDatabase_["xrandr"].uniqueKeys() )
    {
        int index = fk.indexOf(Devicetype_Xrandr_Connected);
        if( index < 0 )
        {
            continue;
        }

        QString interface = fk.mid(0, index).trimmed();
        index = interface.indexOf('-');
        if( index > 0 )
        {
            interface = interface.mid(0, index);
        }

        connectedMonitorList.push_back(interface);
    }

    return connectedMonitorList;
}

QStringList DeviceInfoParser::getInputdeviceList()
{
    QStringList inputdeviceList;

    if(false == toolDatabase_.contains("catinput"))
    {
        return inputdeviceList;
    }

    foreach(const QString& fk, toolDatabase_["catinput"].uniqueKeys() )
    {
        inputdeviceList.push_back(fk);
    }

    return inputdeviceList;
}

QStringList DeviceInfoParser::getNetworkadapterList()
{
    QStringList networkadapterList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return networkadapterList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        if( fk.contains("network") )
        {
            networkadapterList.push_back(fk);
        }
    }

    return networkadapterList;
}

QStringList DeviceInfoParser::getBluetoothList()
{
    QStringList bluetoothList;

    if(false == toolDatabase_.contains("hciconfig"))
    {
        return bluetoothList;
    }

    foreach(const QString& fk, toolDatabase_["hciconfig"].uniqueKeys() )
    {
        bluetoothList.push_back(fk);
    }

    return bluetoothList;
}

QStringList DeviceInfoParser::getCameraList()
{
    QStringList cameraList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return cameraList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        if(false == toolDatabase_["lshw"][fk].contains("product"))
        {
            continue;
        }

        QString product =toolDatabase_["lshw"][fk]["product"];
        if(product.contains("Camera", Qt::CaseInsensitive))
        {
            cameraList.push_back(fk);
        }
    }

    return cameraList;
}

QStringList DeviceInfoParser::getUsbdeviceList()
{
    QStringList usbdeviceList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return usbdeviceList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        if(fk.contains("usb:", Qt::CaseInsensitive))
        {
            usbdeviceList.push_back(fk);
        }
    }

    return usbdeviceList;
}

QStringList DeviceInfoParser::getSwitchingpowerList()
{
    QStringList switchingpowerList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return switchingpowerList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        if(fk.contains("power", Qt::CaseInsensitive))
        {
            switchingpowerList.push_back(fk);
        }
    }

    return switchingpowerList;
}

QStringList DeviceInfoParser::getBatteryList()
{
    QStringList batteryList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return batteryList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        if(fk.contains("battery", Qt::CaseInsensitive))
        {
            batteryList.push_back(fk);
        }
    }

    return batteryList;
}

QStringList DeviceInfoParser::getOtherInputdeviceList()
{
    QStringList otherInputdeviceList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return otherInputdeviceList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        QString product =toolDatabase_["lshw"][fk]["product"];
        QString description =toolDatabase_["lshw"][fk]["description"];

        if( product.contains("touchpad", Qt::CaseInsensitive) || description.contains("touchpad", Qt::CaseInsensitive) || \
            product.contains("scanner", Qt::CaseInsensitive) || description.contains("scanner", Qt::CaseInsensitive) || \
            product.contains("Joystick", Qt::CaseInsensitive) || description.contains("Joystick", Qt::CaseInsensitive) ||
            product.contains("Handwriting", Qt::CaseInsensitive) || description.contains("Handwriting", Qt::CaseInsensitive) ||
            product.contains("Voice input", Qt::CaseInsensitive) || description.contains("Voice input", Qt::CaseInsensitive)  )
        {
            otherInputdeviceList.push_back(fk);
        }
    }

    return otherInputdeviceList;
}

QStringList DeviceInfoParser::getOtherPciDeviceList()
{
    QStringList otherPcideviceList;

    if(false == toolDatabase_.contains("lshw"))
    {
        return otherPcideviceList;
    }

    foreach(const QString& fk, toolDatabase_["lshw"].uniqueKeys() )
    {
        QRegExp re("^[\\s\\S]*_pci_[\\S\\s]*pci:[\\d]?_[\\S\\s]*$");

        if( re.exactMatch(fk) )
        {
            if( fk.endsWith("display", Qt::CaseInsensitive) || fk.endsWith("network", Qt::CaseInsensitive) )
            {
                continue;
            }

            otherPcideviceList.push_back(fk);
        }
    }

    return otherPcideviceList;
}

QStringList DeviceInfoParser::getPortsList()
{
    QStringList portsList;

    if(false == toolDatabase_.contains("dmidecode"))
    {
        return portsList;
    }

    foreach(const QString& fk, toolDatabase_["dmidecode"].uniqueKeys() )
    {
        if( fk.contains("Port Connector Information", Qt::CaseInsensitive) )
        {
            portsList.push_back(fk);
        }
    }

    return portsList;
}

bool DeviceInfoParser::getOSInfo(QString& osInfo)
{
    struct utsname kernel_info;
    int ret = uname(&kernel_info);
    if (ret == 0) {
        char kversion[100] = { 0 };
        strncpy(kversion, kernel_info.release, strlen(kernel_info.release));
    }

    if( false == executeProcess("cat /proc/version") )
    {
        osInfo = "Unknow";
        return false;
    }

     QString linuxCoreVerson;
     QString releaseVersion;

     QRegExp rx("^([\\s\\S]*)\\([\\w!#$%&'*+/=?^_`{|}~-]+(?:\\.[\\w!#$%&'*+/=?^_`{|}~-]+)*@(?:[\\w](?:[\\w-]*[\\w])?\\.)+[\\w](?:[\\w-]*[\\w])?\\)([\\s\\S]*)$");
     if( rx.exactMatch(standOutput_) )
     {
        linuxCoreVerson = rx.cap(1).trimmed();
        releaseVersion = rx.cap(2).trimmed();

        rx.setPattern("^(\\(gcc version [\\d-.]*)[\\s\\S]*$");
        if( rx.exactMatch( releaseVersion ) )
        {
            releaseVersion.remove(rx.cap(1));
            int index = releaseVersion.indexOf(")");
            releaseVersion.remove( index, 1 );
        }

        osInfo = linuxCoreVerson + releaseVersion;
     }
     else
     {
        osInfo = standOutput_;
     }

     osInfo.remove("version");

    return true;
}

bool DeviceInfoParser::loadDemicodeDatabase()
{
    if( false == executeProcess("sudo dmidecode"))
    {
        return false;
    }

    QString dmidecodeOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile dmidecodeFile("//home//archermind//Desktop//dmidecode.txt");
    if( false == dmidecodeFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    dmidecodeOut = dmidecodeFile.readAll();
#endif

    // dimdecode
    DatabaseMap dimdecodeDatabase_;

    int startIndex = 0;
    int lineNumber = 0;
    QString deviceType;
    QString childDeviceType;
    QString childDeviceContent;

    QMap<QString, QString> DeviceInfoMap;

    for( int i = 0; i < dmidecodeOut.size(); ++i )
    {
        if( dmidecodeOut[i] != '\n' && i != dmidecodeOut.size() -1 )
        {
            continue;
        }

        QString line = dmidecodeOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;
        ++lineNumber;

        if( line.trimmed().isEmpty() )
        {
            if(deviceType.isEmpty() == false)
            {
                static int endIndex = 1;
                if(dimdecodeDatabase_.contains(deviceType))
                {
                    QString insteadDeviceType;
                    endIndex = 1;
                    do
                    {
                        insteadDeviceType = deviceType;
                        insteadDeviceType += "_";
                        insteadDeviceType += QString::number(endIndex++);
                    }while(dimdecodeDatabase_.contains(insteadDeviceType));
                    deviceType = insteadDeviceType;
                }

                if(DeviceInfoMap.size() > 0)
                {
                    dimdecodeDatabase_[deviceType] = DeviceInfoMap;
                }
                deviceType = "";
            }

            continue;
        }

        if( lineNumber < 5 )
        {
            //# dmidecode 3.2
            QRegExp rx("^# dmidecode ([\\d]*).([\\d]*)$");
            if( rx.indexIn(line) > -1 )
            {
                DeviceInfoMap["version"] = rx.cap(1)+ "." + rx.cap(2);
                dimdecodeDatabase_["dmidecode"] = DeviceInfoMap;
                DeviceInfoMap.clear();
                continue;
            }

            //  SMBIOS 3.0.0 present.
            rx.setPattern("^SMBIOS ([\\d])*.([\\d])*.([\\d])* present.$");
            if( rx.indexIn(line) > -1 )
            {
                DeviceInfoMap["version"] = rx.cap(1)+ "." + rx.cap(2) + "." + rx.cap(1);
                dimdecodeDatabase_["SMBIOS"] = DeviceInfoMap;
                DeviceInfoMap.clear();
                continue;
            }
        }
        // Handle 0x0002, DMI type 2, 15 bytes
        QRegExp rx("^Handle 0x[0-9a-fA-F]{1,4}, DMI type [\\d]*, [\\d]* bytes$");
        if( rx.exactMatch(line) )
        {
            DeviceInfoMap.clear();
            continue;
        }

        if( line.startsWith("\t\t") && childDeviceType.isEmpty() == false )
        {
            if(childDeviceContent.isEmpty() ==false)
            {
                childDeviceContent += ", ";
            }
            childDeviceContent += line.trimmed();
            continue;
        }

        if( line.startsWith('\t') )
        {
            if(childDeviceType.isEmpty() == false)
            {
                DeviceInfoMap[childDeviceType] = childDeviceContent;
                childDeviceType.clear();
                childDeviceContent.clear();
            }

            if( line.contains(':') )
            {
                QStringList strList = line.split(':');
                if(strList.last().trimmed().isEmpty() == true)
                {
                    childDeviceType = strList[0].trimmed();
                }
                else if(strList.size() == 2)
                {
                    DeviceInfoMap[strList[0].trimmed()] = strList[1].trimmed();
                }

            }
            continue;
        }

        //BIOS Information
        if( false == line.endsWith('.') )
        {
            if(deviceType.isEmpty() == false)
            {
                static int endIndex = 1;
                if(dimdecodeDatabase_.contains(deviceType))
                {
                    QString insteadDeviceType;
                    endIndex = 1;
                    do
                    {
                        insteadDeviceType = deviceType;
                        insteadDeviceType += "_";
                        insteadDeviceType += QString::number(endIndex++);
                    }while(dimdecodeDatabase_.contains(insteadDeviceType));
                    deviceType = insteadDeviceType;
                }

                if(DeviceInfoMap.size() > 0)
                {
                    dimdecodeDatabase_[deviceType] = DeviceInfoMap;
                }
            }

            deviceType = line.trimmed();
            DeviceInfoMap.clear();
            continue;
        }
    }

    toolDatabase_[dmidecodeToolName] = dimdecodeDatabase_;
    return  true;
}

bool DeviceInfoParser::loadLshwDatabase()
{
    if( false == executeProcess("sudo lshw"))
    {
        return false;
    }

    QString lshwOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile lshwFile("//home//archermind//Desktop//lshw.txt");
    if( false == lshwFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    lshwOut = lshwFile.readAll();
#endif
    // lshw
    DatabaseMap lshwDatabase_;

    int startIndex = 0;
    int lineNumber = -1;
    QStringList deviceType;
    QMap<QString, QString> DeviceInfoMap;

    for( int i = 0; i < lshwOut.size(); ++i )
    {
        if( lshwOut[i] != '\n' && i != lshwOut.size() -1 )
        {
            continue;
        }

        ++lineNumber;

        QString line = lshwOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;

        if( line.trimmed().isEmpty() )
        {
            dWarning("DeviceInfoParser::loadLshwDatabase lshw output contains empty line!");
            continue;
        }

        if(lineNumber == 0)
        {
            DeviceInfoMap[Devicetype_Name] = line.trimmed();
            deviceType.push_back(Devicetype_lshw_Class_Prefix+Deviceype_Computer);
            continue;
        }

        if(line.contains(Devicetype_lshw_Class_Prefix))
        {
            QString deviceTypeName;
            foreach(auto deviceType, deviceType)
            {
                if(deviceTypeName.isEmpty() == false)
                {
                    deviceTypeName += Devicetype_Stitching_Symbol;
                }
                deviceTypeName += deviceType.trimmed().remove(Devicetype_lshw_Class_Prefix);
            }

            lshwDatabase_[deviceTypeName] = DeviceInfoMap;

            DeviceInfoMap.clear();
            while( deviceType.size() > 0 )
            {
                if( deviceType.last().indexOf(Devicetype_lshw_Class_Prefix) >= line.indexOf(Devicetype_lshw_Class_Prefix) )
                {
                    deviceType.pop_back();
                }
                else
                {
                    break;
                }
            }

            if( line.contains(Devicetype_Separator) )
            {
                QStringList strList = line.split(Devicetype_Separator);
                DeviceInfoMap[strList.first().trimmed().remove(Devicetype_lshw_Class_Prefix)] = strList.last().trimmed();
            }
            deviceType.push_back(line);
            continue;
        }

        int index = line.indexOf(Devicetype_Separator);
        if( index > 0 )
        {
            DeviceInfoMap[line.mid(0,index).trimmed().remove(Devicetype_lshw_Class_Prefix)] = line.mid(index+1).trimmed();
            continue;
        }
    }

    //last device
    {
        QString deviceTypeName;
        foreach(auto deviceType, deviceType)
        {
            if(deviceTypeName.isEmpty() == false)
            {
                deviceTypeName += Devicetype_Stitching_Symbol;
            }
            deviceTypeName += deviceType.trimmed().remove(Devicetype_lshw_Class_Prefix);
        }

        lshwDatabase_[deviceTypeName] = DeviceInfoMap;
    }

    toolDatabase_[lshwToolname] = lshwDatabase_;
    return true;
}

bool DeviceInfoParser::loadLscpuDatabase()
{
    if( false == executeProcess("sudo lscpu"))
    {
        return false;
    }

    QString lscpuOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile lscpuFile("//home//archermind//Desktop//lscpu.txt");
    if( false == lscpuFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    lscpuOut = lscpuFile.readAll();
#endif

    // lscpu
    QMap<QString, QString> lscpuDatabase_;

    int startIndex = 0;

    for( int i = 0; i < lscpuOut.size(); ++i )
    {
         if( lscpuOut[i] != '\n' && i != lscpuOut.size() -1 )
         {
             continue;
         }

         QString line = lscpuOut.mid(startIndex, i - startIndex);
         startIndex = i + 1;

         if( line.contains(Devicetype_Separator) )
         {
             QStringList strList = line.split(Devicetype_Separator);
             lscpuDatabase_[strList.first().trimmed().remove(Devicetype_lshw_Class_Prefix)] = strList.last().trimmed();
         }
    }

    DatabaseMap rootlscuDb;
    rootlscuDb[lscpuToolname] = lscpuDatabase_;
    toolDatabase_[lscpuToolname] = rootlscuDb;
    return true;
}

bool DeviceInfoParser::loadSmartctlDatabase()
{
    if( false == executeProcess("sudo smartctl --all /dev/sda"))
    {
        return false;
    }

    QString smartctlOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile smartctlFile("//home//archermind//Desktop//smartctl.txt");
    if( false == smartctlFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    smartctlOut = smartctlFile.readAll();
#endif

    // smartctl
    QMap<QString, QString> smartctlDatabase_;
    int startIndex = 0;

    for( int i = 0; i < smartctlOut.size(); ++i )
    {
         if( smartctlOut[i] != '\n' && i != smartctlOut.size() -1 )
         {
             continue;
         }

         QString line = smartctlOut.mid(startIndex, i - startIndex);
         startIndex = i + 1;

         if( line.contains(Devicetype_Separator) )
         {
             int index = line.indexOf(Devicetype_Separator);
             smartctlDatabase_[line.mid(0, index).trimmed()] = line.mid(index+1).trimmed();
             continue;
         }

         QRegExp rx("^[ ]*[0-9]+[ ]+([\\w-_]+)[ ]+0x[0-9a-fA-F-]+[ ]+[0-9]+[ ]+[0-9]+[ ]+[0-9]+[ ]+[\\w-]+[ ]+[\\w-]+[ ]+[\\w-]+[ ]+([0-9\\/w-]+[ ]*[ 0-9\\/w-()]*)$");
         if( rx.indexIn(line) > -1 )
         {
             smartctlDatabase_[rx.cap(1)] = rx.cap(2);
         }
    }

    DatabaseMap rootsmartctlDb;
    rootsmartctlDb[smartctlToolname] = smartctlDatabase_;
    toolDatabase_[smartctlToolname] = rootsmartctlDb;

    return true;
}

bool DeviceInfoParser::loadCatInputDatabase()
{
    if( false == executeProcess("cat /proc/bus/input/devices"))
    {
        return false;
    }

    QString inputDeviceOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile inputDeviceFile("//home//archermind//Desktop//input.txt");
    if( false == inputDeviceFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    inputDeviceOut = inputDeviceFile.readAll();
#endif

    // cat /proc/bus/input/devices
    DatabaseMap catInputDeviceDatabase_;

    QMap<QString, QString> DeviceInfoMap;
    int startIndex = 0;

    for( int i = 0; i < inputDeviceOut.size(); ++i )
    {
         if( inputDeviceOut[i] != '\n')
         {
             continue;
         }

         if( i != inputDeviceOut.size() -1 && inputDeviceOut[i+1] != '\n')
         {
             continue;
         }

         if( DeviceInfoMap.size() > 0 )
         {
            QString deviceSysfs = "InputDevice";
            if( DeviceInfoMap.contains(Devicetype_CatDeviceSysfs) )
            {
                deviceSysfs = DeviceInfoMap[Devicetype_CatDeviceSysfs];
            }

            static int deviceIndex;
            deviceIndex = 0;
            while( catInputDeviceDatabase_.contains(deviceSysfs) )
            {
                ++deviceIndex;
                deviceSysfs = "InputDevice" + QString::number(deviceIndex);
            }

            catInputDeviceDatabase_[deviceSysfs] = DeviceInfoMap;
         }

         DeviceInfoMap.clear();

         QString paragraph = inputDeviceOut.mid(startIndex, i - startIndex);
         startIndex = i + 1;

         foreach( const QString& line,  paragraph.split('\n'))
         {
            QString cutLine = line.mid(line.indexOf(Devicetype_Separator)+1);
            int first_index = cutLine.indexOf(DeviceType_CatDevice_Separator);
            if( first_index < 0 )
            {
                continue;
            }

            if( cutLine.indexOf(DeviceType_CatDevice_Separator, first_index+1) < 0)
            {
                QStringList strList = cutLine.split(DeviceType_CatDevice_Separator);
                DeviceInfoMap[strList.first().trimmed()] = strList.last().trimmed();
                continue;
            }

            foreach( const QString& typeStr, cutLine.split(' ') )
            {
                if( false == typeStr.contains(DeviceType_CatDevice_Separator) )
                {
                    continue;
                }

                QStringList strList = typeStr.split(DeviceType_CatDevice_Separator);
                DeviceInfoMap[strList.first().trimmed()] = strList.last().trimmed();
            }
         }
    }

    toolDatabase_[catInputToolname] = catInputDeviceDatabase_;
    return true;
}

bool DeviceInfoParser::loadXrandrDatabase()
{
    if( false == executeProcess("sudo xrandr --verbose"))
    {
        return false;
    }

    QString xrandrOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile xrandrFile("//home//archermind//Desktop//xrandr.txt");
    if( false == xrandrFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    xrandrOut = xrandrFile.readAll();
#endif

    // xrandr --verbose
    DatabaseMap xrandrDatabase_;
    QMap<QString, QString> DeviceInfoMap;

    int startIndex = 0;
    QString title;
    QString deviceType;
    QString content;

    for( int i = 0; i < xrandrOut.size(); ++i )
    {

        if( xrandrOut[i] != '\n' && i != xrandrOut.size() -1)
        {
            continue;
        }

        QString line = xrandrOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;

        if( i == xrandrOut.size() -1)
        {
            content += line;
            DeviceInfoMap[deviceType] = content.trimmed();
            if(title.isEmpty() == false && DeviceInfoMap.size() > 0 )
            {
                xrandrDatabase_[title] = DeviceInfoMap;
            }
            break;
        }

        if( line.startsWith(Devicetype_Xrandr_Screen) && line.contains(Devicetype_Separator) )
        {
            QStringList strList = line.split(Devicetype_Separator);

            DeviceInfoMap.clear();
            foreach(const QString& screenStr, strList.last().split(Devicetype_Xrandr_Screen_Separator) )
            {
                QString contentStr = screenStr.trimmed();
                int index = contentStr.indexOf(Devicetype_Xrandr_Space);
                if( index > 0 )
                {
                    DeviceInfoMap[contentStr.mid(0, index).trimmed()] = contentStr.mid(index+1).trimmed();
                }
            }

            xrandrDatabase_[strList.first().trimmed()] = DeviceInfoMap;
            continue;
        }

        if( line.startsWith(Devicetype_Xrandr_Twotab) || line.startsWith(Devicetype_Xrandr_Twospace) || line.startsWith(Devicetype_Xrandr_TabAndspace))
        {
            content += line;
            continue;
        }

        if( line.startsWith(Devicetype_Xrandr_Tab) )
        {
            if(deviceType.isEmpty() == false)
            {
                DeviceInfoMap[deviceType] = content.trimmed();
            }
            int index = line.indexOf(Devicetype_Separator);
            if(index > 0)
            {
                deviceType = line.mid(0, index).trimmed();
            }

            content = line.mid(index+1);
            continue;
        }

        if( line.contains(Devicetype_Xrandr_Connected) || line.contains(Devicetype_Xrandr_Disconnected) )
        {
            if(deviceType.isEmpty() == false)
            {
                DeviceInfoMap[deviceType] = content;
            }

            if(title.isEmpty() == false && DeviceInfoMap.size() > 0 )
            {
                xrandrDatabase_[title] = DeviceInfoMap;
            }

            DeviceInfoMap.clear();

            title = line.trimmed();
            continue;
        }
    }

    toolDatabase_[xrandrToolname] = xrandrDatabase_;
    return true;
}

bool DeviceInfoParser::parseXrandrData()
{

    return true;
}

bool DeviceInfoParser::parseEDID( )
{


    return true;
}

bool DeviceInfoParser::loadPowerSettings()
{
    PowerInter power("com.deepin.daemon.Power", "/com/deepin/daemon/Power", QDBusConnection::sessionBus(), nullptr);
    // switchingpower
    switchingpowerScreenSuspendDelay_ = power.linePowerScreenBlackDelay();     //screen suspend delay seconds
    switchingpowerComputerSuspendDelay_ = power.linePowerSleepDelay();   //computer suspend delay seconds
    switchingpowerAutoLockScreenDelay_ = power.linePowerLockDelay();        //auto lock screen delay seconds
    // Battery
    batteryScreenSuspendDelay_ = power.batteryScreenBlackDelay();
    batteryComputerSuspendDelay_ = power.batterySleepDelay();
    batteryAutoLockScreenDelay_ = power.batteryLockDelay();
    return true;
}

bool DeviceInfoParser::loadLspciDatabase()
{
    if( false == executeProcess("sudo lspci -v"))
    {
        return false;
    }
    QString lspciOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile lspciFile("//home//archermind//Desktop//lspci.txt");
    if( false == lspciFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }
    lspciOut = lspciFile.readAll();
#endif
    // lspci --verbose
    DatabaseMap lspciDatabase_;
    QMap<QString, QString> DeviceInfoMap;
    QString deviceName;

    int startIndex = 0;

    for( int i = 0; i < lspciOut.size(); ++i )
    {

        if( lspciOut[i] != '\n' && i != lspciOut.size() -1)
        {
            continue;
        }

        QString line = lspciOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;

        if(line.contains(Devicetype_Lspci_Tab))
        {
            int index = line.indexOf(Devicetype_Lspci_Seperator);
            if( index > 0 )
            {
                DeviceInfoMap[line.mid(0,index).trimmed()] = line.mid(index+1).trimmed();
            }
            else if(line.contains(Devicetype_Lspci_Memory) )
            {
                if( line.contains(Devicetype_Lspci_non_prefetchable) )
                {
                    QRegExp rx("^[\\s\\S]*\\[size=([\\d]*M)\\]$");
                    if( rx.exactMatch(line) )
                    {
                        DeviceInfoMap[Devicetype_Lspci_Memory] = QString(rx.cap(1)).trimmed();
                    }
                }
                else if( line.contains(Devicetype_Lspci_prefetchable) )
                {
                    QRegExp rx("^[\\s\\S]*\\[size=([\\d]*M)\\]$");
                    if( rx.exactMatch(line) )
                    {
                        DeviceInfoMap[Devicetype_Lspci_Memory] = QString(rx.cap(1)).trimmed();
                    }
                }
            }
            else
            {
                DeviceInfoMap[line.trimmed()] = "";
            }
            continue;
        }

        if(line.trimmed().isEmpty())
        {
            lspciDatabase_[deviceName] = DeviceInfoMap;
            DeviceInfoMap.clear();
            deviceName = "";
            continue;
        }

        int index = line.indexOf(Devicetype_Lspci_Seperator);
        if( index > 0 )
        {
            if(deviceName.isEmpty() == false)
            {
                lspciDatabase_[deviceName] = DeviceInfoMap;
                DeviceInfoMap.clear();
                deviceName = "";
            }
            DeviceInfoMap[Devicetype_Name] = line.mid(index+1).trimmed();
            deviceName = line.mid(0,index);
            continue;
        }

        if( i == lspciOut.size() -1 )
        {
            if(deviceName.isEmpty() == false)
            {
                lspciDatabase_[deviceName] = DeviceInfoMap;
            }
        }
    }

    toolDatabase_[lspciToolname] = lspciDatabase_;
    return true;
}

bool DeviceInfoParser::loadHciconfigDatabase()
{
    if( false == executeProcess("sudo hciconfig -a"))
    {
        return false;
    }

    QString hciconfigOut = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile xrandrFile("//home//archermind//Desktop//xrandr.txt");
    QFile hciconfigFile("//home//archermind//Desktop//hciconfig.txt");
    if( false == hciconfigFile.open(QIODevice::ReadOnly) )
    {
        return false;
    }

    hciconfigOut = hciconfigFile.readAll();
#endif

    // hciconfig
    DatabaseMap hciconfigDatabase_;
    QMap<QString, QString> DeviceInfoMap;
    QString deviceName;
    int startIndex = 0;

    for( int i = 0; i < hciconfigOut.size(); ++i )
    {
        if( hciconfigOut[i] != '\n' && i != hciconfigOut.size() -1)
        {
            continue;
        }

        QString line = hciconfigOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;

        if(line.startsWith(Devicetype_Hciconfig_Hci))
        {
            if(deviceName.isEmpty() == false)
            {
                hciconfigDatabase_[deviceName] = DeviceInfoMap;
            }
            DeviceInfoMap.clear();
            deviceName = line.mid(0, line.indexOf(Devicetype_Separator));
            continue;
        }

        if(line.startsWith(Devicetype_Hciconfig_Multispace))
        {
            int index = line.indexOf(Devicetype_Separator);
            if( index > 0 )
            {
                DeviceInfoMap[line.mid(0, index).trimmed()] = line.mid(index+1).trimmed();
            }
            else
            {
                DeviceInfoMap[line.trimmed()] = "";
            }
        }

        if( i == hciconfigOut.size() -1 || line.trimmed().isEmpty() )
        {
            if(deviceName.isEmpty() == false)
            {
                hciconfigDatabase_[deviceName] = DeviceInfoMap;
            }
            DeviceInfoMap.clear();
            deviceName = "";
            continue;
        }
    }

    toolDatabase_[hciconfigToolname] = hciconfigDatabase_;
    return true;
}

bool DeviceInfoParser::loadLsusbDatabase()
{
    // lsusb
    DatabaseMap lsusbDatabase_;
    return true;
}

bool DeviceInfoParser::loadHwinfoDatabase()
{
    if( false == executeProcess("sudo hwinfo --monitor"))
    {
        return false;
    }

    QString hwOut = standOutput_;

    //QString hwOut = getHwMonitorString();
    if( hwOut.size() < 1 )
    {
        return false;
    }

    // hciconfig
    DatabaseMap hwinfoDatabase_;
    QMap<QString, QString> DeviceInfoMap;
    QString deviceName;
    int startIndex = 0;

    for( int i = 0; i < hwOut.size(); ++i )
    {
        if( hwOut[i] != '\n' && i != hwOut.size() -1)
        {
            continue;
        }

        QString line = hwOut.mid(startIndex, i - startIndex);
        startIndex = i + 1;

        if( i == hwOut.size() -1 || line.trimmed().isEmpty() )
        {
            if(deviceName.isEmpty() == false)
            {
                hwinfoDatabase_[deviceName] = DeviceInfoMap;
            }

            DeviceInfoMap.clear();
            deviceName = "";
            continue;
        }

        if(line.startsWith(Devicetype_HwInfo_Fourspace))
        {
            int index = line.indexOf(": ");
            if(index > 0)
            {
                if( line.trimmed().contains(Devicetype_HwInfo_Resolution) )
                {
                    if(DeviceInfoMap.contains(Devicetype_HwInfo_Resolution))
                    {
                        DeviceInfoMap[Devicetype_HwInfo_Currentresolution] += ",";
                        DeviceInfoMap[Devicetype_HwInfo_Currentresolution] +=line.mid(index+1).trimmed();
                    }
                    else
                    {
                        DeviceInfoMap[Devicetype_HwInfo_Currentresolution] = line.mid(index+1).trimmed();
                    }

                    continue;
                }

                DeviceInfoMap[ line.mid(0, index).trimmed()] = line.mid(index+1).trimmed();
            }
            continue;
        }

        if(line.startsWith(Devicetype_HwInfo_Twospace))
        {
            int index = line.indexOf(": ");
            if(index > 0)
            {
                if( line.contains(Devicetype_HwInfo_Resolution) )
                {
                    if(DeviceInfoMap.contains(Devicetype_HwInfo_ResolutionList))
                    {
                        DeviceInfoMap[Devicetype_HwInfo_ResolutionList] += ",";
                        DeviceInfoMap[Devicetype_HwInfo_ResolutionList] +=line.mid(index+1).trimmed();
                    }
                    else
                    {
                        DeviceInfoMap[Devicetype_HwInfo_ResolutionList] = line.mid(index+1).trimmed();
                    }

                    continue;
                }

                DeviceInfoMap[ line.mid(0, index).trimmed()] = line.mid(index+1).trimmed();
            }
            continue;
        }



        deviceName = line.trimmed();
    }

    toolDatabase_[hwinfoToolname] = hwinfoDatabase_;
    return true;
}

bool DeviceInfoParser::executeProcess(const QString& cmd)
{
    /*int res = */process_.start(cmd);
    bool res = process_.waitForFinished();
    standOutput_ = process_.readAllStandardOutput();
    std::cout << standOutput_.toStdString() << std::endl;

    return res;
}
