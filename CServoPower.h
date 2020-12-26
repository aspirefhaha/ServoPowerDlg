#ifndef CSERVOPOWER_H
#define CSERVOPOWER_H
#include <QThread>
#include <QUdpSocket>
#include <QMutex>
#include <QFile>

enum SPProtState {
    SPHEADADDR1=0,
    SPHEADADDR2,
    SPPROTOCODE1,
    SPPROTOCODE2,
    SPLEN1,
    SPLEN2,
    SPPOWERADDR,
    SPFUNCCODE,
    SPDATAS
};

enum SAITEM {
	SASTATUS,
	SARVOLTAGECURRENT,
	SAVOLTAGECURRENT,
	SAPOWER,
	SASWITCHONOFF,
	SASETVOLTAGE,
	SASETCURRENT
};

class CServoPower : public QThread
{
    Q_OBJECT
public:
    explicit CServoPower (QObject * parent = nullptr);
    ~CServoPower ();
    void try_quit()
    {
        m_Quit = true;
    }

    bool sendData(QByteArray & ba);

signals:
    void error(QString );
    void powerMsg(QString );
    void powerState(int onoff);
    void powerVoltage(int vol);
    void powerRatedVoltage(int vol);
    void powerPower(unsigned short thousand, unsigned short walt, unsigned short mwalt);
    void powerCurrent(int cur);
    void powerRatedCurrent(int cur);

public slots:
    void sltSwitchPower(int state);
    void sltChangeVoltage(unsigned short vol);
    void sltChangeCurrent(unsigned short cur);
    void sltChangeRatedVoltage(int vol);
    void sltChagneRatedCurrent(int cur);
	void sltGetCurrent();
	void sltGetVoltage();
	void sltGetRatedVoltage();
	void sltGetRatedCurrent();
	void sltGetPower();
	void sltRefreshStatus();

protected:
    void run() override;

private:
    bool m_Quit;
	enum SAITEM m_AskItem;
    QUdpSocket mSocket;
    QHostAddress mPeerAddr;
    quint16 mPeerPort;
    quint16 mLocalPort;
    QString iniFile;
    bool mNeedSend;
    QByteArray m_ba;
    QMutex baMutex;
	QMutex stateMutex;
    int parseModbus(quint8 data);
    SPProtState spstat;
    unsigned short datalen;
    unsigned short pdulen;
    unsigned char poweraddr;
    unsigned char funccode;
    unsigned char m_buf[256];
    unsigned char m_data[256];
	int m_buflen;
    QFile m_file;

	QMutex sdMutex;
	QByteArray m_sendBa;
};

#endif // CSERVOPOWER_H
