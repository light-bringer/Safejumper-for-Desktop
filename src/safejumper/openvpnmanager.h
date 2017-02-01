#ifndef OPENVPN_MANAGER_H
#define OPENVPN_MANAGER_H

#include <memory>
#include <QObject>
#include <QProcess>
#include <QTcpSocket>
#include <QFileSystemWatcher>
#include <QTemporaryFile>

#define G_Delay_PortQuestion 60
#define G_Delay_PortIteration 80

// only 5 seconds for each TCP connection
#define G_Delay_OneCheck 30
#define G_Max_Reconnect 3

class QTimer;

class OpenvpnManager: public QObject
{
    Q_OBJECT
public:
    enum OvState {
        ovsDisconnected = 0,
        ovsConnecting,
        ovsConnected,
        ovsTotal
    };

    ~OpenvpnManager();
    static OpenvpnManager * instance();
    static void cleanup();
    static bool exists();

    void start();
    void startWithServer(size_t srv);
    void cancel(const QString & msg);
    void stop();			// normal stop of executed via this process
    void killRunningOpenvpn();		// kill process executed previously: connect to it first, if fails: kill -9

    bool openvpnRunning();

    OvState state();

    void startPortLoop(bool port);		// true - cycle ports; false - cycle nodes

signals:
    void gotNewIp(QString ip);

private slots:
    void socketConnected();
    void socketError(QAbstractSocket::SocketError error);
    void socketReadyRead();
    void reconnectTimeout();
    void openvpnLogfileChanged(const QString & pfn);		// OpenVpn log file
    void checkState();		// timer calls it

    void processError(QProcess::ProcessError error);
    void processStarted();
    void processStateChanged(QProcess::ProcessState st);
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void logStderr();
    void logStdout();

private:
    OpenvpnManager();
    void connectToOpenvpnSocket();	// attach to OpenVPN management socket
    void disconnectFromOpenvpnSocket();
    void removeProcess();
    void setupFileWatcher();
    void parseOpenvpnLogLine(const QString & s);
    void parseNewIp(const QString & s);
    void gotConnected(const QString & s);
    void gotTunErr(const QString & s);
    void showErrorMessageCleanup(QString msg);
    void launchOpenvpn();
    bool writeConfigFile(); // Write openvpn options to ovpn file
    QStringList getOpenvpnArgs(); // Arguments to pass to openvpn
    void tryNextPort();

    void parseSocketLine(QString s);
    void parseSocketQueryWord(const QString & word, const QString & s);
    void parseSocketPlainWord(const QString & word, const QString & s);
    void parseSocketStateWord(const QString & word, const QString & s);

    void startTimer();
    void startReconnectTimer();

    void setState(OvState st);

    static std::auto_ptr<OpenvpnManager> mInstance;

    std::auto_ptr<QFileSystemWatcher> mFileSystemWatcher;	// OpenVpn log file
    std::auto_ptr<QTemporaryFile> mParametersTempFile;
    std::auto_ptr<QProcess> mProcess;
    OvState mState;
    QTimer *mStateTimer;
    int mPID;		// for running process (run safejumper after crash)
    std::auto_ptr<QTcpSocket> mSocket;

    qint64 mLogFilePosition;
    bool mProcessing;
    bool mTunError;
    bool mError;

    QString mPreviousStateWord;

    int mReconnectAttempt;
    bool mPortDialogShown;
    uint mStartDateTime;
    bool mInPortLoop;
    bool mChangingPorts;
};

#endif // OPENVPNMANAGER_H
