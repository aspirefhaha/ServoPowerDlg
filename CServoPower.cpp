#include "CServoPower.h"
#include <qDebug>
#include <QSettings>
#include <QApplication>
#include <QDir>
#include <QDateTime>

#define PW_IDLETIME 100


CServoPower::CServoPower(QObject * parent)
    :QThread(parent),
    m_Quit(false),
    mNeedSend(false),
    spstat(SPHEADADDR1),
    mPeerPort(0),
    mLocalPort(0),
    pdulen(0),
    datalen(0)
	, m_AskItem(SASTATUS)
{
    iniFile = "./" + QApplication::applicationName() + ".ini";
    QSettings configIni(iniFile, QSettings::IniFormat);

    QString saveDiSPath = configIni.value("Store/Dir").toString();
    if (saveDiSPath.isEmpty())
        saveDiSPath = "C:/SERVOPOWERDATASAVE";
    QDir saveDir(saveDiSPath);
	if (!saveDir.exists())
        saveDir.mkpath(saveDiSPath);
	QDateTime nowtime = QDateTime::currentDateTime();
	QString subdirname = nowtime.toString("yyyy_MM_dd_hh");
    saveDiSPath = saveDiSPath + "/" + subdirname;
    saveDir.setPath(saveDiSPath);
	if (!saveDir.exists())
        saveDir.mkpath(saveDiSPath);

	QString powersavefilename = configIni.value("Store/FileName").toString();
	if (powersavefilename.isEmpty()) {
        powersavefilename = saveDiSPath +  "/" + nowtime.toString("hh_mm_ss") + "PW.dat";
	}
	else {
        powersavefilename = saveDiSPath + "/" + powersavefilename;
	}
	m_file.setFileName(powersavefilename);
	m_file.open(QIODevice::WriteOnly);
    QString peerAddrStr = configIni.value("ServoPower/UDPAddr").toString();
    if(peerAddrStr.isEmpty()){
        peerAddrStr = "192.168.1.92";
    }
    mPeerAddr.setAddress(peerAddrStr);
    mPeerPort = configIni.value("ServoPower/UDPPort").toUInt() & 0xffff;
    if(mPeerPort==0){
        mPeerPort = 502;
    }
    mLocalPort = configIni.value("ServoPower/LocalPort").toUInt() & 0xffff;
    if(mLocalPort ==0){
        //mLocalPort = 4032;
    }
    
	if (!mSocket.bind(QHostAddress("0.0.0.0"), mLocalPort))
	{
		qDebug() << "bind  Failed";
	}
	
    //mSocket.setPeerAddress(mPeerAddr);
    //mSocket.setPeeSPort(mPeerPort);
}

bool CServoPower::sendData(QByteArray & ba)
{
    baMutex.lock();
	m_sendBa.append(ba);
    baMutex.unlock();
	return true;
}

CServoPower::~CServoPower()
{

    if(m_Quit!=true){
        m_Quit = true;
    }
	
}



//void CServoPower::sltSwitchPower(int idx, int state)
//{
//	sdMutex.lock();
//	if (idx == 0) {
//		if (state == 0) {
//			m_sendBa.append((const char *)switchOff1, sizeof(switchOff1));
//		}
//		else {
//			m_sendBa.append((const char *)switchOn1, sizeof(switchOn1));
//		}
//	}
//	else if (idx == 1) {
//		if (state == 0) {
//			m_sendBa.append((const char *)switchOff2, sizeof(switchOff2));
//		}
//		else {
//			m_sendBa.append((const char *)switchOn2, sizeof(switchOn2));
//		}
//	}
//	else if (idx == 2) {
//		if (state == 0) {
//			m_sendBa.append((const char *)switchOff3, sizeof(switchOff3));
//		}
//		else {
//			m_sendBa.append((const char *)switchOn3, sizeof(switchOn3));
//		}
//	}
//	sdMutex.unlock();
//}
//void CServoPower::sltChangeVoltage(int idx, int vol)
//{

//}
quint8 switchOn[] = { 0x00,0x01,0x00,0x00,0x00,0x06,0x01,0x05,0x00,0x85u,0xffu,0x00 };
quint8 switchOff[] = { 0x00,0x01,0x00,0x00,0x00,0x06,0x01,0x05,0x00,0x85u,0x00,0x00};
void CServoPower::sltSwitchPower(int state)
{

    QByteArray ba ;
	m_AskItem = SASWITCHONOFF;
    if(state){
        ba.append((char *)switchOn,sizeof(switchOn));

    }
    else{
        ba.append((char *)switchOff,sizeof(switchOff));
    }
    sendData(ba);
	qDebug() << "Send Switch" << QString::number(state);
}

quint8 setvolCmd[] = {0x0 , 0x1, 0x0, 0x0,0x0,0x6,0x1,0x6,0x0,0x95,0x0,0x0};
void CServoPower::sltChangeVoltage(unsigned short vol)
{
	if (vol > 3000) {
		emit error(tr("Voltage %1 Overflow").arg(vol));
		return;
	}
	setvolCmd[10] = (vol >> 8) & 0xff;
	setvolCmd[11] = (vol & 0xff);
	QByteArray ba;
	m_AskItem = SASETVOLTAGE;
	ba.append((char *)setvolCmd, sizeof(setvolCmd));
	sendData(ba);

}

quint8 setcurCmd[] = { 0x0 , 0x1, 0x0, 0x0,0x0,0x6,0x1,0x6,0x0,0x96,0x0,0x0 };
void CServoPower::sltChangeCurrent(unsigned short cur)
{
	if (cur > 8500) {
		emit error(tr("Current %1 Overflow").arg(cur));
		return;
	}
	setcurCmd[10] = (cur >> 8) & 0xff;
	setcurCmd[11] = (cur & 0xff);
	QByteArray ba;
	m_AskItem = SASETCURRENT;
	ba.append((char *)setcurCmd, sizeof(setcurCmd));
	sendData(ba);
}
void CServoPower::sltChangeRatedVoltage(int vol)
{

}
void CServoPower::sltChagneRatedCurrent(int cur)
{

}

quint8 askVoltageCurrent[] = { 0x00,0x01,0x00,0x00,0x00,0x06,0x01,0x04,0x00,0x64u,0x00u,0x02 };
void CServoPower::sltGetCurrent()
{
	QByteArray ba;
	m_AskItem = SAVOLTAGECURRENT;
	ba.append((char *)askVoltageCurrent, sizeof(askVoltageCurrent));
	sendData(ba);
}

void CServoPower::sltGetVoltage()
{
	QByteArray ba;
	m_AskItem = SAVOLTAGECURRENT;
	ba.append((char *)askVoltageCurrent, sizeof(askVoltageCurrent));
	sendData(ba);
}
quint8 askRatedVoltageCurrent[] = { 0x00,0x01,0x00,0x00,0x00,0x06,0x01,0x04,0x00,0x67u,0x00u,0x02 };
void CServoPower::sltGetRatedVoltage()
{
	QByteArray ba;
	m_AskItem = SARVOLTAGECURRENT;
	ba.append((char *)askRatedVoltageCurrent, sizeof(askRatedVoltageCurrent));
	sendData(ba);
}
void CServoPower::sltGetRatedCurrent()
{
	QByteArray ba;
	m_AskItem = SARVOLTAGECURRENT;
	ba.append((char *)askRatedVoltageCurrent, sizeof(askRatedVoltageCurrent));
	sendData(ba);
}

quint8 askStatus[] = { 0x00,0x01,0x00,0x00,0x00,0x06,0x01,0x01,0x00,0x85u,0x00u,0x01 };
void CServoPower::sltRefreshStatus()
{
	QByteArray ba;
	m_AskItem = SASTATUS;
	ba.append((char *)askStatus, sizeof(askStatus));
	sendData(ba);
}
void CServoPower::sltGetPower()
{
	quint8 getpower[] = { 0x00,0x01,0x00,0x00,0x00,0x06,0x01,0x04,0x00,0x6eu,0x00u,0x03 };
	m_AskItem = SAPOWER;
	QByteArray ba;
	ba.append((char*)getpower, sizeof(getpower));
	sendData(ba);
	
}

#define GETHIGHVALUE(left,val)	do{left = (val & 0xff);\
                                    m_buf[m_buflen++] = data;\
									left <<= 8; }while(0)

#define GETLOWVALUE(left,val)	do{ left &= 0xff00; \
                                    m_buf[m_buflen++] = data;\
									left |= (val & 0xff);}while(0)

int CServoPower::parseModbus(quint8 data)
{
    switch(spstat){
    case SPHEADADDR1:
        if(data == 0x00){
            spstat = SPHEADADDR2;
			m_buflen = 1;
			datalen = 0;
            m_buf[0] = data;
        }
        break;
    case SPHEADADDR2:
        if(data == 0x01){
            spstat = SPPROTOCODE1;
            m_buf[m_buflen++] = data;
        }
        else{
            spstat = SPHEADADDR1;
            return -1;
        }
        break;
    case SPPROTOCODE1:
        if(data == 0x00){
            spstat = SPPROTOCODE2;
            m_buf[m_buflen++] = data;
        }
        else{
            spstat = SPHEADADDR1;
            return -1;
        }
        break;
    case SPPROTOCODE2:
        if(data == 0x00){
            spstat = SPLEN1;
            m_buf[m_buflen++] = data;
        }
        else{
            spstat = SPHEADADDR1;
            return -1;
        }
        break;
    case SPLEN1:
        GETHIGHVALUE(pdulen, data);
        spstat = SPLEN2;
        break;
    case SPLEN2:
        GETLOWVALUE(pdulen, data);
        spstat = SPPOWERADDR;
        break;
    case SPPOWERADDR:
        if(data == 0x01){
            spstat = SPFUNCCODE;
            poweraddr=data;
            pdulen--;
            m_buf[m_buflen++] = data;
        }
        else{
            spstat = SPHEADADDR1;
            return -1;
        }
        break;
    case SPFUNCCODE:
        funccode = data;
        m_buf[m_buflen++] = data;
        pdulen--;
        spstat = SPDATAS;
        break;
    case SPDATAS:
        pdulen--;
        m_buf[m_buflen++] = data;
        m_data[datalen++] = data;
        if(pdulen==0){
            spstat = SPHEADADDR1;
            pdulen = 0;
            m_buflen = 0;
            return 0;
        }
		break;
    }
    return -1;
}


void CServoPower::run()
{
    while(!m_Quit){
        QThread::msleep(PW_IDLETIME);
        baMutex.lock();
        if(!m_ba.isEmpty()){
            if(-1 == mSocket.write(m_ba)){
                emit error(tr("UDP Send Failed"));
                m_ba.clear();
            }
            else{
                m_ba.clear();
            }
        }
        baMutex.unlock();
        QByteArray ba;
        if(mSocket.hasPendingDatagrams())
        {
            ba.resize(mSocket.pendingDatagramSize());
			mSocket.readDatagram(ba.data(), ba.size());
			if (m_file.isOpen()) {
				m_file.write(ba.data(), ba.size());
			}
            for(int i = 0 ;i<ba.size();i++){
                unsigned char data = (unsigned char)ba.at(i);
                if(parseModbus(data)!=-1){
					for (int i = 0; i < m_buflen; i++) {
						qDebug() << " " << QString::number(m_buf[i],16);
					}
					unsigned char poweraddress = m_data[0];
					switch (funccode) {
                    case 0x3:
                        break;
                    case 0x4:
					{
						switch (m_AskItem) {
						case SARVOLTAGECURRENT:
						{
							unsigned char bytecode = m_data[0];
							if (bytecode == 4) {
								unsigned short voltage = m_data[1];
								voltage <<= 8;
								voltage |= m_data[2] & 0xff;
								unsigned short current = m_data[3];
								current <<= 8;
								current |= m_data[4] & 0xff;
								emit powerRatedCurrent(current);
								emit powerRatedVoltage(voltage);
							}
						}
							break;
						case SAVOLTAGECURRENT:
						{
							unsigned char bytecode = m_data[0];
							if (bytecode == 4) {
								unsigned short voltage = m_data[1];
								voltage <<= 8;
								voltage |= m_data[2] & 0xff;
								unsigned short current = m_data[3];
								current <<= 8;
								current |= m_data[4] & 0xff;
								emit powerCurrent(current);
								emit powerVoltage(voltage);
							}
						}
							break;
						case SAPOWER:
						{
							unsigned char bytecode = m_data[0];
							if (bytecode == 6) {
								unsigned short thd = m_data[1];
								thd <<= 8;
								thd |= m_data[2] & 0xff;
								unsigned short walt = m_data[3];
								walt <<= 8;
								walt |= m_data[4];
								unsigned short mwalt = m_data[5];
								mwalt <<= 8;
								mwalt |= m_data[6];
								emit powerPower(thd, walt, mwalt);
							}
						}
							break;
						default:
							break;
						}
					}
                        break;
                    case 0x1:
					{
						switch (m_AskItem) {
						case SASTATUS:
						{
							if (datalen == 2) {

								unsigned char onoff = m_data[1];
								emit powerState(onoff);
							}
						}
							break;
						}
					}
                        break;
                    case 0x6:
                        break;
                    case 0x10:
						break;
					case 0x5:
					{
						unsigned short regaddr = m_data[0];
						regaddr <<= 8;
						regaddr |= (m_data[1] & 0xff);
						switch (regaddr) {
							case 0x0085:
							{
								unsigned short onoff = m_data[2];
								onoff <<= 8;
								onoff |= (m_data[3] & 0xff);
								emit powerState(onoff);
							}
							break;
						}
						
					}
                        break;
                    }
                }
            }
        }
		else {

			baMutex.lock();
			if (!m_sendBa.isEmpty())
			{
                mSocket.writeDatagram(m_sendBa, mPeerAddr, mPeerPort);
				//QThread::msleep(50);
				m_sendBa.clear();
			}
			baMutex.unlock();
//            {
//                static int stsendcount= 0;
//                if(stsendcount++%10==0){

//                    const unsigned char readStateData[] = { 0x01,0x03,0,0,0,7,0x4,0x8 };
//                    mSocket.writeDatagram((const char *)&readStateData,sizeof(readStateData), mPeerAddr,mPeerPort);
//                }
//            }
		}

    }
	if (m_file.isOpen()) {
		m_file.close();
	}
}
