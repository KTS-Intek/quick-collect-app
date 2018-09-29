#ifndef SELECTMETERS4POLL_H
#define SELECTMETERS4POLL_H

#include "src/shared/referencewidgetclass.h"
#include "src/zbyrator-v2/zbyratordatatypehelper.h"

namespace Ui {
class SelectMeters4poll;
}

class SelectMeters4poll : public ReferenceWidgetClass
{
    Q_OBJECT

public:
    explicit SelectMeters4poll(LastDevInfo *lDevInfo, GuiHelper *gHelper, GuiSett4all *gSett4all, QWidget *parent = 0);
    ~SelectMeters4poll();

signals:
    void onReloadAllMeters();

    void onAllMeters(UniversalMeterSettList allMeters);

//    void command4dev(quint16 command, QString args);//pollCode args
    void command4dev(quint16 command, QVariantMap mapArgs);//pollCode args

public slots:
    void setPollSett(const QDateTime &dtFrom, const QDateTime &dtTo, const quint8 &pollCode, const quint8 &meterType, const int &go2sleepSeconds, const bool &enCheckSleepProfile);

    void setPollSettElectric(const QDateTime &dtFrom, const QDateTime &dtTo, const quint8 &pollCode);

    void setPollSettWater(const QDateTime &dtFrom, const QDateTime &dtTo, const quint8 &pollCode, const bool &enSleepCommand, const int &go2sleepSeconds, const bool &enCheckSleepProfile);

    void initPage();


    void setPageSett(const MyListStringList &listRows, const QVariantMap &col2data, const QStringList &headerH, const QStringList &header, const bool &hasHeader);

    void onProcessingEnds(QStringList listMissingData);

private slots:
    void on_pbPollAll_clicked();



    void on_pbSelected_clicked();

    void on_pbCheckedOn_clicked();

private:
    void sendStartPoll(const QStringList &listni);

    struct LastPollSett{
        QDateTime dtFrom;
        QDateTime dtTo;
        quint8 pollCode;
        quint8 meterType;
//        bool enablePowerManagement;
        int go2sleepSeconds;
        bool enCheckSleepProfile;
    } lPollSett;

    Ui::SelectMeters4poll *ui;
};

#endif // SELECTMETERS4POLL_H