#ifndef METERSDATETIME_H
#define METERSDATETIME_H

#include "gui-src/referencewidgetclass.h"

namespace Ui {
class MetersDateTime;
}

class MetersDateTime : public ReferenceWidgetClass
{
    Q_OBJECT

public:
    explicit MetersDateTime(GuiHelper *gHelper, QWidget *parent = 0);
    ~MetersDateTime();

    QVariant getPageSett4read(bool &ok, QString &mess);

signals:
    ///map 4 exchange stat
    void setTableDataExt(const MPrintTableOut &table, const QStringList &header, const int &keycol);

    void setModelHeaderDataRoles(QString columnroles);// list joined with '\n'

    void showThisDeviceKeyValue(QString keyvalue);

    void setDefaultDataFilterSettings(QVariantMap maponeprofile, QString profilename);


    void showMapEs(QString lastLang);


    void onReloadAllMeters();


    void command4dev(quint16 command, QVariantMap mapArgs);//pollCode args

    void setLastPageId(QString name);
    void lockButtons(bool disable);

    void lockActions(bool disable);


public slots:
    void clearPage();

//    void setPageSett(const MyListStringList &listRows, const QVariantMap &col2data, const QStringList &headerH, const QStringList &header, const bool &hasHeader);

    void onModelChanged();

    void meterDateTimeDstStatus(QString ni, QDateTime dtLocal, QString stts);


    void showThisDev(QString ni);
    void showContextMenu4thisDev(QString ni);
    void showThisDevInSource(QString ni);

    void onWdgtLock(bool disable);
    void onButtonLock(bool disable);

    void sendActLock(const bool &isWdgtDisabled, const bool &isButtonDisabled);

private slots:

    void initPage();



    void on_tbShowList_clicked();

    void on_tbShowMap_clicked();


    void on_tvTable_customContextMenuRequested(const QPoint &pos);

    void onPbReadAll_clicked();

    void on_pbCorrectionAll_clicked();

    void on_pbRead_clicked();

    void on_pbWrite_clicked();

private:
    Ui::MetersDateTime *ui;

    void startOperation(const QStringList &listni, const quint8 &operation);

    QList<QAction*> getDateTimeActions();


    bool isMapReady;
    QString lastDateTimeMask;
};

#endif // METERSDATETIME_H
