#include "createeditinputdialog.h"
#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QDebug>
#include <QHostInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QDir>
#include "../databasetext.h"

QString settings_name="";

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->comboBoxoutputfmt->insertItems(0,outputformats);

    populatesettings();

    // NOTE: I know this is haphazard but the column width should add together to under 431 (width of QTableWidget)
    ui->outputListTable->setColumnWidth(0, 85);
    ui->outputListTable->setColumnWidth(1, 260);
    ui->outputListTable->setColumnWidth(2, 55);
    ui->outputListTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->outputListTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::populatepublicvars()
{
    msgdisplayformat=ui->comboBoxoutputfmt->currentText();
    dropnontextmsgs=ui->checkBoxdropnontextmsgs->isChecked();
    donotdisplaysus.clear();
    QRegExp rx("([\\da-fA-F]+)");
    int pos = 0;
    while ((pos = rx.indexIn(ui->lineEditdonotdisplaysus->text(), pos)) != -1)
    {
        bool ok = false;
        uint value = rx.cap(1).toUInt(&ok,16);
        if(ok)donotdisplaysus.push_back(value);
        pos += rx.matchedLength();
    }

    audioinputdevice=QAudioDeviceInfo::defaultInputDevice();
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
    {
        if(deviceInfo.deviceName()==ui->comboBoxsoundcard->currentText())
        {
            audioinputdevice=deviceInfo;
            break;
        }
    }

    loggingdirectory=ui->lineEditlogdir->text();
    loggingenable=ui->checkBoxlogenable->isChecked();
    widebandwidthenable=ui->checkBoxlogwidebandwidthenable->isChecked();

    planesfolder=ui->lineEditplanesfolder->text();
    planelookup=ui->lineEditplanelookup->text();

    onlyuselibacars=ui->checkBoxonlyuselibacars->isChecked();
    beepontextmessage=ui->checkBoxbeepontextmessage->isChecked();

    set_station_id_enabled=ui->checkSetStationId->isChecked();
    station_id=ui->lineEditstationid->text();
    
    //bottom text window output settings
    /*
     * TODO:
    QStringList hosts=ui->lineEditudpoutputdecodedmessagesaddress->text().simplified().split(" ");
    udp_for_decoded_messages_address.clear();
    udp_for_decoded_messages_port.clear();
    for(int i=0;i<hosts.size();i++)
    {
        QString host=hosts[i];
        QHostAddress hostaddress;
        quint16 hostport;
        QString hostaddress_string=host.section(':',0,0).simplified();
        if(hostaddress_string.toUpper()=="LOCALHOST")hostaddress_string="127.0.0.1";
        if(!hostaddress.setAddress(hostaddress_string))
        {
            qDebug()<<"Can't set UDP address";
        }
        hostport=host.section(':',1,1).toInt();
        if(hostport==0)
        {
            qDebug()<<"Can't set UDP port reverting to 18765";
            hostport=18765;
        }
        if(!((udp_for_decoded_messages_port.contains(hostport)&&udp_for_decoded_messages_address.contains(hostaddress))))
        {
            udp_for_decoded_messages_address.push_back(hostaddress);
            udp_for_decoded_messages_port.push_back(hostport);
        }
         else
         {
              qDebug()<<"Can't set UDP address:port as it's already used";
         }
    }
    */
    udp_for_decoded_messages_enabled=ui->checkBoxenablefeeding->isChecked();

    //ads message output using SBS1 protocol over TCP
    QString hostaddr=ui->lineEdittcpoutputadsmessagesaddress->text().section(':',0,0);
    if(!tcp_for_ads_messages_address.setAddress(hostaddr))
    {
        QString tstr=ui->lineEdittcpoutputadsmessagesaddress->text().section(':',1,1);
        ui->lineEdittcpoutputadsmessagesaddress->setText("0.0.0.0:"+tstr);
        tcp_for_ads_messages_address.clear();
        QSettings settings("Jontisoft", settings_name);
        settings.setValue("lineEdittcpoutputadsmessagesaddress", "0.0.0.0:"+tstr);
        qDebug()<<"Can't set TCP address reverting to 0.0.0.0";
    }
    tcp_for_ads_messages_port=ui->lineEdittcpoutputadsmessagesaddress->text().section(':',1,1).toInt();
    if(tcp_for_ads_messages_port==0)
    {
        qDebug()<<"Can't set TCP port reverting to 30003";
        ui->lineEdittcpoutputadsmessagesaddress->setText(hostaddr+":30003");
        QSettings settings("Jontisoft", settings_name);
        settings.setValue("lineEdittcpoutputadsmessagesaddress", hostaddr+":30003");
        tcp_for_ads_messages_port=30003;
    }
    tcp_for_ads_messages_enabled=ui->checkOutputADSMessageToTCP->isChecked();
    tcp_as_client_enabled=ui->checkTCPAsClient->isChecked();
    ui->checkTCPAsClient->setEnabled(ui->checkOutputADSMessageToTCP->isChecked());
    disablePlaneLogWindow=ui->checkBoxDisablePlaneLogWindow->isChecked();

    //Trim leading and trailing white spaces from zmq text
    ui->lineEditZMQBind->setText(ui->lineEditZMQBind->text().trimmed());
    ui->lineEditZMQBindTopic->setText(ui->lineEditZMQBindTopic->text().trimmed());
    ui->lineEditZmqConnectAddress->setText(ui->lineEditZmqConnectAddress->text().trimmed());
    ui->lineEditZmqTopic->setText(ui->lineEditZmqTopic->text().trimmed());

    localAudioOutEnabled=ui->ambeEnabled->isChecked();
    zmqAudioOutEnabled=ui->remoteAmbeEnabled->isChecked();
    zmqAudioOutBind=ui->lineEditZMQBind->text();
    zmqAudioOutTopic=ui->lineEditZMQBindTopic->text();
    zmqAudioInputAddress = ui->lineEditZmqConnectAddress->text();
    zmqAudioInputTopic = ui->lineEditZmqTopic->text();
    zmqAudioInputEnabled = ui->checkBoxZMQ->isChecked();


    ui->MQTT_host->setText(mqtt_settings_object.host);
    ui->MQTT_port->setValue(mqtt_settings_object.port);
    ui->MQTT_topic->setText(mqtt_settings_object.topic);
    ui->MQTT_clientId->setText(mqtt_settings_object.clientId);
    ui->MQTT_username->setText(mqtt_settings_object.username);
    ui->MQTT_password->setText(mqtt_settings_object.password);
    ui->MQTT_publish->setChecked(mqtt_settings_object.publish);
    ui->MQTT_encryption->setChecked(mqtt_settings_object.encryption);
    mqtt_enable=ui->MQTT_enable->isChecked();

}

void SettingsDialog::populatesettings()
{

    //populate soundcard
    ui->comboBoxsoundcard->clear();
    foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
        ui->comboBoxsoundcard->addItem(deviceInfo.deviceName());

    //load settings
    QSettings settings("Jontisoft", settings_name);
    ui->comboBoxoutputfmt->setCurrentIndex(settings.value("comboBoxDisplayformat",2).toInt());
    ui->lineEditdonotdisplaysus->setText(settings.value("lineEditdonotdisplaysus","26 0A C0 00 14 16").toString());
    ui->checkBoxdropnontextmsgs->setChecked(settings.value("checkBoxdropnontextmsgs",true).toBool());
    ui->comboBoxsoundcard->setCurrentText(settings.value("comboBoxsoundcard","").toString());
    ui->lineEditlogdir->setText(settings.value("lineEditlogdir",QStandardPaths::standardLocations(APPDATALOCATIONS)[0]+"/logs").toString());
    ui->checkBoxlogenable->setChecked(settings.value("checkBoxlogenable",false).toBool());
    ui->checkBoxlogwidebandwidthenable->setChecked(settings.value("checkBoxlogwidebandwidthenable",false).toBool());
    ui->lineEditplanesfolder->setText(settings.value("lineEditplanesfolder",QStandardPaths::standardLocations(APPDATALOCATIONS)[0]+"/planes").toString());
    ui->lineEditplanelookup->setText(settings.value("lineEditplanelookup","http://www.flightradar24.com/data/airplanes/{REG}").toString());
    ui->checkBoxonlyuselibacars->setChecked(settings.value("checkBoxonlyuselibacars",false).toBool());
    ui->checkBoxbeepontextmessage->setChecked(settings.value("checkBoxbeepontextmessage",true).toBool());

    ui->outputListTable->setRowCount(0);

    int size = settings.beginReadArray("feeders");
    if (size>0)
    {
        for (int i=0;i<size;i++)
        {
            settings.setArrayIndex(i);

            ui->outputListTable->insertRow(i);
            ui->outputListTable->setItem(i,0,new QTableWidgetItem(outputformats.at(settings.value("format",0).toInt())));
            ui->outputListTable->setItem(i,1,new QTableWidgetItem(settings.value("host","localhost").toString()));
            ui->outputListTable->setItem(i,2,new QTableWidgetItem(settings.value("port",18765).toInt()));
        }
        settings.endArray();
    }
    else
    {
        QStringList hosts=settings.value("lineEditudpoutputdecodedmessagesaddress","").toString().simplified().split(" ");
        for (int i=0;i<hosts.size();i++)
        {
            QString host=hosts[i];
            if (host.size()>0)
            {
                QString hostaddress_string=host.section(':',0,0).simplified();
                quint16 hostport=host.section(':',1,1).toInt();
                if (hostport==0) hostport=18765;

                ui->outputListTable->insertRow(i);
                ui->outputListTable->setItem(i,0,new QTableWidgetItem(outputformats.at(ui->comboBoxoutputfmt->currentIndex())));
                ui->outputListTable->setItem(i,1,new QTableWidgetItem(hostaddress_string));
                ui->outputListTable->setItem(i,2,new QTableWidgetItem(QString("%1").arg(hostport)));
            }
        }
    }

    ui->checkBoxenablefeeding->setChecked(settings.value("checkOutputDecodedMessageToUDPPort",false).toBool());
    ui->lineEdittcpoutputadsmessagesaddress->setText(settings.value("lineEdittcpoutputadsmessagesaddress","0.0.0.0:30003").toString());
    ui->checkOutputADSMessageToTCP->setChecked(settings.value("checkOutputADSMessageToTCP",false).toBool());
    ui->checkTCPAsClient->setChecked(settings.value("checkTCPAsClient",false).toBool());
    ui->checkBoxDisablePlaneLogWindow->setChecked(settings.value("checkBoxDisablePlaneLogWindow",false).toBool());
    ui->checkSetStationId->setChecked(settings.value("checkSetStationId",false).toBool());
    ui->lineEditstationid->setText(settings.value("lineEditstationid",QHostInfo::localHostName()).toString());

    ui->ambeEnabled->setChecked(settings.value("localAudioOutEnabled", true).toBool());
    ui->remoteAmbeEnabled->setChecked(settings.value("remoteAudioOutEnabled", false).toBool());
    if(!ui->remoteAmbeEnabled->isChecked())
    {
        ui->lineEditZMQBind->setEnabled(false);
        ui->lineEditZMQBindTopic->setEnabled(false);
    }
    ui->lineEditZMQBind->setText(settings.value("remoteAudioOutBindAddress", "tcp://*:5551").toString());
    ui->lineEditZMQBindTopic->setText(settings.value("remoteAudioOutBindTopic", "JAERO").toString());
    ui->checkBoxZMQ->setChecked(settings.value("zmqAudioInputEnabled", false).toBool());
    ui->lineEditZmqConnectAddress->setText(settings.value("zmqAudioInputReceiveAddress", "tcp://127.0.0.1:6003").toString());
    QString default_topic = settings_name;
    default_topic.remove(QRegExp( "JAERO \\[" )).remove(QRegExp( "\\]" ));
    default_topic=default_topic.trimmed();
    ui->lineEditZmqTopic->setText(settings.value("zmqAudioInputReceiveTopic", default_topic).toString());

    ui->checkBoxlogwidebandwidthenable->setEnabled(!ui->checkBoxZMQ->isChecked());
    ui->comboBoxsoundcard->setEnabled(!ui->checkBoxZMQ->isChecked());
    ui->lineEditZmqConnectAddress->setEnabled(ui->checkBoxZMQ->isChecked());
    ui->lineEditZmqTopic->setEnabled(ui->checkBoxZMQ->isChecked());

    //new simple object method for lots of settings
    mqtt_settings_object.fromQByteArray(settings.value("mqtt_settings_object", mqtt_settings_object.toQByteArray()).toByteArray());
    ui->MQTT_enable->setChecked(settings.value("MQTT_enable",false).toBool());

    on_lineEditlogdir_editingFinished();

    populatepublicvars();
}

void SettingsDialog::accept()
{    

    //save settings
    QSettings settings("Jontisoft", settings_name);
    settings.setValue("comboBoxDisplayformat", ui->comboBoxoutputfmt->currentIndex());
    settings.setValue("lineEditdonotdisplaysus", ui->lineEditdonotdisplaysus->text());
    settings.setValue("checkBoxdropnontextmsgs", ui->checkBoxdropnontextmsgs->isChecked());
    settings.setValue("comboBoxsoundcard", ui->comboBoxsoundcard->currentText());
    settings.setValue("lineEditlogdir", ui->lineEditlogdir->text());
    settings.setValue("checkBoxlogenable", ui->checkBoxlogenable->isChecked());
    settings.setValue("checkBoxlogwidebandwidthenable", ui->checkBoxlogwidebandwidthenable->isChecked());
    settings.setValue("lineEditplanesfolder", ui->lineEditplanesfolder->text());
    settings.setValue("lineEditplanelookup", ui->lineEditplanelookup->text());
    settings.setValue("checkBoxonlyuselibacars", ui->checkBoxonlyuselibacars->isChecked());
    settings.setValue("checkBoxbeepontextmessage", ui->checkBoxbeepontextmessage->isChecked());

    settings.beginWriteArray("feeders");
    for (int i=0;i<ui->outputListTable->rowCount();i++)
    {
        settings.setArrayIndex(i);

        int idx=outputformats.indexOf(ui->outputListTable->item(i,0)->text());
        if (idx==-1) idx=outputformats.count()-1;

        settings.setValue("format", idx);
        settings.setValue("host", ui->outputListTable->item(i,1)->text());
        settings.setValue("port", ui->outputListTable->item(i,2)->text());
    }
    settings.endArray();

    settings.setValue("checkOutputDecodedMessageToUDPPort", ui->checkBoxenablefeeding->isChecked());
    settings.setValue("lineEdittcpoutputadsmessagesaddress", ui->lineEdittcpoutputadsmessagesaddress->text());
    settings.setValue("checkOutputADSMessageToTCP", ui->checkOutputADSMessageToTCP->isChecked());
    settings.setValue("checkTCPAsClient", ui->checkTCPAsClient->isChecked());
    settings.setValue("checkBoxDisablePlaneLogWindow",ui->checkBoxDisablePlaneLogWindow->isChecked());
    settings.setValue("checkSetStationId",ui->checkSetStationId->isChecked());
    settings.setValue("lineEditstationid",ui->lineEditstationid->text());

    settings.setValue("localAudioOutEnabled", ui->ambeEnabled->isChecked());
    settings.setValue("remoteAudioOutEnabled", ui->remoteAmbeEnabled->isChecked());
    settings.setValue("remoteAudioOutBindAddress",ui->lineEditZMQBind->text());
    settings.setValue("remoteAudioOutBindTopic",ui->lineEditZMQBindTopic->text());

    settings.setValue("zmqAudioInputEnabled",  ui->checkBoxZMQ->isChecked());
    settings.setValue("zmqAudioInputReceiveAddress",  ui->lineEditZmqConnectAddress->text());
    settings.setValue("zmqAudioInputReceiveTopic", ui->lineEditZmqTopic->text());

    //new object method for lots of settings. doesn't really fit the old schema but oh well
    mqtt_settings_object.host=ui->MQTT_host->text();
    mqtt_settings_object.port=ui->MQTT_port->value();
    mqtt_settings_object.topic=ui->MQTT_topic->text();
    mqtt_settings_object.clientId=ui->MQTT_clientId->text();
    mqtt_settings_object.username=ui->MQTT_username->text();
    mqtt_settings_object.password=ui->MQTT_password->text().toLatin1();
    mqtt_settings_object.publish=ui->MQTT_publish->isChecked();
    mqtt_settings_object.subscribe=!ui->MQTT_publish->isChecked();
    mqtt_settings_object.encryption=ui->MQTT_encryption->isChecked();
    settings.setValue("mqtt_settings_object", mqtt_settings_object.toQByteArray());
    settings.setValue("MQTT_enable", ui->MQTT_enable->isChecked());

    populatepublicvars();
    QDialog::accept();
}

void SettingsDialog::on_lineEditlogdir_editingFinished()
{
    QFile file(ui->lineEditlogdir->text());
    ui->lineEditlogdir->setText(file.fileName());
}

void SettingsDialog::on_lineEditplanesfolder_editingFinished()
{
    QFile file(ui->lineEditplanesfolder->text());
    ui->lineEditplanesfolder->setText(file.fileName());
}

void SettingsDialog::on_checkOutputADSMessageToTCP_stateChanged(int arg1)
{
    if(!arg1)ui->checkTCPAsClient->setEnabled(false);
     else ui->checkTCPAsClient->setEnabled(true);
}

void SettingsDialog::on_newEntryButton_clicked()
{
    CreateEditInputDialog::addEntry(this, ui->outputListTable, outputformats);
}

void SettingsDialog::on_editEntryButton_clicked()
{
    QModelIndexList selection = ui->outputListTable->selectionModel()->selectedRows();

    // NOTE: we explicitly disabled multiselection and made sure edit/remove buttons are disabled if no selection is active

    CreateEditInputDialog::editEntry(this, ui->outputListTable, outputformats, selection.at(0).row());
}

void SettingsDialog::on_removeEntryButton_clicked()
{
    QModelIndexList selection = ui->outputListTable->selectionModel()->selectedRows();

    // NOTE: we explicitly disabled multiselection and made sure edit/remove buttons are disabled if no selection is active

    ui->outputListTable->removeRow(selection.at(0).row());

    if (ui->outputListTable->rowCount() == 0) {
        ui->editEntryButton->setDisabled(true);
        ui->removeEntryButton->setDisabled(true);
    }
}

void SettingsDialog::on_outputListTable_itemSelectionChanged()
{
    if (ui->outputListTable->selectionModel()->selectedRows().count() > 0) {
        ui->editEntryButton->setDisabled(false);
        ui->removeEntryButton->setDisabled(false);
    }
}

