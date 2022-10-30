#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QLabel>

#include "audiooqpskdemodulator.h"
#include "audiomskdemodulator.h"
#include "audioburstoqpskdemodulator.h"
#include "audioburstmskdemodulator.h"
#include "gui_classes/settingsdialog.h"
#include "aerol.h"
#include "gui_classes/planelog.h"
#include <QFile>
#include <QSound>
#include "sbs1.h"
#include "gui_classes/qled.h"

#include "databasetext.h"

#include "arincparse.h"

#include "audiooutdevice.h"
#include "compressedaudiodiskwriter.h"
#include "zmq_audioreceiver.h"
#include "zmq_audiosender.h"

#include "mqttsubscriber.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:

private:
    enum class LedState{Disable,Off,On,Overload};

    enum DemodType{NoDemodType,MSK,OQPSK,BURSTOQPSK,BURSTMSK};
    Ui::MainWindow *ui;
    AudioMskDemodulator *audiomskdemodulator;
    AudioMskDemodulator::Settings audiomskdemodulatorsettings;
    QLabel *ebnolabel;
    QLabel *freqlabel;
    QLabel *sourcelabel;
    QUdpSocket *udpsocket;

    //OQPSK add
    AudioOqpskDemodulator *audiooqpskdemodulator;
    AudioOqpskDemodulator::Settings audiooqpskdemodulatorsettings;
    //

    //Burst OQPSK add
    AudioBurstOqpskDemodulator *audioburstoqpskdemodulator;
    AudioBurstOqpskDemodulator::Settings audioburstoqpskdemodulatorsettings;
    //

    //Burst MSK add
    AudioBurstMskDemodulator *audioburstmskdemodulator;
    AudioBurstMskDemodulator::Settings audioburstmskdemodulatorsettings;

    QList<QPointer<QUdpSocket> > feeder_udp_socks;

    //ambe->pcm->vorbis->ogg->disk
    QObject *ambe;
    AudioOutDevice *audioout;
    CompressedAudioDiskWriter *compresseddiskwriter;

    AeroL *aerol;
    AeroL *aerol2;

    MqttSubscriber *mqttsubscriber;

    SettingsDialog *settingsdialog;

    SBS1 *sbs1;

    void acceptsettings();

    PlaneLog *planelog;

    void log(QString &text);
    QFile filelog;
    QTextStream outlogstream;

    DemodType typeofdemodtouse;

    void selectdemodulatorconnections(DemodType demodtype);

    ArincParse arincparser;

    QSound *beep;

    //ZeroMQ Audio Receiver
    ZMQAudioReceiver *zmq_audio_receiver;
    ZMQAudioSender   *zmq_audio_sender;

    bool last_dcd;
    double last_frequency;
    double last_EbNo;

    void setLedState(QLed *led, LedState state);
    bool formatACARSItem(const ACARSItem &acarsitem, const QString &msgfmt, QString &humantext);

protected:
    void closeEvent(QCloseEvent *event);


private slots:
    void DataCarrierDetectStatusSlot(bool dcd);
    void SignalStatusSlot(bool signal);
    void EbNoSlot(double EbNo);
    void WarningTextSlot(QString warning);
    void PeakVolumeSlot(double Volume);
    void PlottablesSlot(double freq_est,double freq_center,double bandwidth);
    void AboutSlot();
    void on_comboBoxbps_currentIndexChanged(const QString &arg1);
    void on_comboBoxlbw_currentIndexChanged(const QString &arg1);
    void on_comboBoxafc_currentIndexChanged(const QString &arg1);
    void on_actionCleanConsole_triggered();
    void on_comboBoxdisplay_currentIndexChanged(const QString &arg1);
    void on_actionConnectToUDPPort_toggled(bool arg1);
    void on_actionRawOutput_triggered();
    void on_action_Settings_triggered();
    void on_action_PlaneLog_triggered();
    void ACARSslot(ACARSItem &acarsitem);
    void CChannelAssignmentSlot(CChannelAssignmentItem &item);
    void ERRorslot(QString &error);

   // void result(bool ok, int ref, const QStringList &result);

    void on_tabWidget_currentChanged(int index);
    void on_actionSound_Out_toggled(bool mute);
    void on_actionReduce_CPU_triggered(bool checked);

    void statusToUDPifJSONset();

    void onMqttConnectionStateChange(MqttSubscriber::ConnectionState state);

};

#endif // MAINWINDOW_H
