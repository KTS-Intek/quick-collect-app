#include "selectmeters4poll.h"

#include <QShortcut>
#include <QKeySequence>

///[!] quick-collect
#include "zbyrator-src/src/getreadymetersdata.h"

///[!] widgets-shared
#include "src/widgets/selectionchecker.h"

///[!] guisett-shared
#include "src/nongui/settloader.h"

///[!] zbyrator-shared
#include "src/zbyrator-v2/quickpollhelper.h"

#include "myucdevicetypes.h"


//----------------------------------------------------------------------------------------------

GetDataFromDbDoneSignalizator::GetDataFromDbDoneSignalizator(QObject *parent) : QObject(parent)
{
    connect(this, SIGNAL(onDbSelectorKickedOff()), this, SLOT(deleteLater()));
    connect(this, SIGNAL(destroyed(QObject*)), this, SIGNAL(stopDbSelector()));
}


//----------------------------------------------------------------------------------------------


SelectMeters4poll::SelectMeters4poll(GetDataFromDbDoneSignalizator *signalizator, GuiHelper *gHelper, QWidget *parent) :

    ReferenceWidgetClassGui(gHelper,  parent), signalizator(signalizator)
{
    topWdgt = new SelectMeters4pollTopWdgt(this);
    getVLayoutMain()->insertWidget(0, topWdgt);

    connect(topWdgt->getPbCancel(), SIGNAL(clicked(bool)), this, SLOT(deleteLater()) );

    connect(new QShortcut(QKeySequence::Cancel, this), SIGNAL(activated()), this, SLOT(deleteLater()) );

    hidePbAdd();
}


//----------------------------------------------------------------------------------------------

void SelectMeters4poll::setPollSett(const QDateTime &dtFrom, const QDateTime &dtTo, const quint8 &pollCode, const quint8 &deviceType, const int &go2sleepSeconds, const bool &enCheckSleepProfile, const bool &ignoreRetr)
{
    //start select data
    lPollSett.dtFrom = dtFrom;
    lPollSett.dtTo = dtTo;
    lPollSett.pollCode = pollCode;
    lPollSett.deviceType = deviceType;
    lPollSett.go2sleepSeconds = go2sleepSeconds;
    lPollSett.enCheckSleepProfile = enCheckSleepProfile;
    lPollSett.ignoreRetr = ignoreRetr;
}

//----------------------------------------------------------------------------------------------

void SelectMeters4poll::setPollSettElectric(const QDateTime &dtFrom, const QDateTime &dtTo
                                            , const quint8 &pollCode, const bool &ignoreRetr)
{
    setPollSett(dtFrom, dtTo, pollCode, UC_METER_ELECTRICITY, 0, false, ignoreRetr);
}

//----------------------------------------------------------------------------------------------

void SelectMeters4poll::setPollSettWater(const QDateTime &dtFrom, const QDateTime &dtTo, const quint8 &pollCode
                                         , const bool &enSleepCommand, const int &go2sleepSeconds, const bool &enCheckSleepProfile, const bool &ignoreRetr)
{
    setPollSett(dtFrom, dtTo, pollCode, UC_METER_WATER, enSleepCommand ? go2sleepSeconds : 0, enCheckSleepProfile, ignoreRetr );
}

//----------------------------------------------------------------------------------------------

void SelectMeters4poll::setPollSettGas(const QDateTime &dtFrom, const QDateTime &dtTo, const quint8 &pollCode, const bool &ignoreRetr)
{
    //it can come with a sleep mode, but I'm not sure
    setPollSett(dtFrom, dtTo, pollCode, UC_METER_GAS, 0, false, ignoreRetr);
}

//----------------------------------------------------------------------------------------------

void SelectMeters4poll::setPollSettPulse(const QDateTime &dtFrom, const QDateTime &dtTo, const quint8 &pollCode, const bool &ignoreRetr)
{
    setPollSett(dtFrom, dtTo, pollCode, UC_METER_PULSE, 0, false, ignoreRetr);
}

//----------------------------------------------------------------------------------------------

void SelectMeters4poll::initPage()
{
    createModelsV2("SelectMeters4poll", true);
    connectMessageSignal();
    allow2useDefaultContextMenu(true);

    connect(lastTv, &QTableView::doubleClicked, this, &SelectMeters4poll::ontvTable_doubleClicked);



//    setupObjects(0, ui->tvTable, ui->tbFilter, ui->cbFilterMode, ui->leFilter, SETT_FILTERS_ZBYR_SELMTRS);
//    connect(this, SIGNAL(openContextMenu(QPoint)), this, SLOT(on_tvTable_customContextMenuRequested(QPoint)));
//    StandardItemModelHelper::setModelHorizontalHeaderItems(model, QStringList());


    SelectionChecker *tmr = new SelectionChecker(this);
    tmr->setWatchTable(lastTv, topWdgt->getPbSelected());
//    setHasDataFromRemoteDevice();

    GetReadyMetersData *d = new GetReadyMetersData;
    QThread *t = new QThread;
    t->setObjectName("GetReadyMetersData");
    d->setPollSett(lPollSett.dtFrom, lPollSett.dtTo, lPollSett.pollCode, lPollSett.deviceType);

    d->moveToThread(t);

    connect(signalizator, &GetDataFromDbDoneSignalizator::stopDbSelector, d, &GetReadyMetersData::stopAllDirect, Qt::DirectConnection);
    connect(t, SIGNAL(destroyed(QObject*)), signalizator, SIGNAL(onDbSelectorKickedOff()));

    connect(topWdgt->getPbCancel(), SIGNAL(clicked(bool)), d, SLOT(stopAllDirect()), Qt::DirectConnection);
    connect(this, SIGNAL(destroyed(QObject*)), d, SLOT(stopAllDirect()), Qt::DirectConnection);
    connect(d, SIGNAL(destroyed(QObject*)), t, SLOT(quit()));
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()) );

    connect(t, SIGNAL(started()), d, SLOT(onThreadStarted()) );
    connect(t, SIGNAL(started()), this, SIGNAL(onReloadAllMeters()) );


    connect(d, &GetReadyMetersData::allMeters2selectWdgtV2, this, &SelectMeters4poll::allMeters2selectWdgtV2);
//    connect(d, SIGNAL(allMeters2selectWdgt(MyListStringList,QVariantMap,QStringList,QStringList,bool)), this, SLOT(setPageSett(MyListStringList,QVariantMap,QStringList,QStringList,bool)));
    connect(d, SIGNAL(onProcessingEnds(QStringList)), this, SLOT(onProcessingEnds(QStringList)));

    connect(this, SIGNAL(onAllMeters(UniversalMeterSettList)), d, SLOT(setMetersList(UniversalMeterSettList)));

    t->start();

    topWdgt->getCbxIgnoreExistingData()->setChecked(SettLoader::loadSett(SETT_ZBRTR_IGNOREEXISTING, true).toBool());


    connect(topWdgt->getPbCheckedOn(), SIGNAL(clicked(bool)), this, SLOT(onpbCheckedOn_clicked()));
    connect(topWdgt->getPbPollAll(), SIGNAL(clicked(bool)), this, SLOT(onpbPollAll_clicked()));
    connect(topWdgt->getPbSelected(), SIGNAL(clicked(bool)), this, SLOT(onpbSelected_clicked()));
    //    emit onPageCanReceiveData();
}

//----------------------------------------------------------------------------------------------

void SelectMeters4poll::allMeters2selectWdgtV2(MPrintTableOut table, QStringList humanHeader, QStringList listGrpsHuman)
{
    setTableData4amodelExt6woValidator(table, humanHeader);
    listGrpsHuman.prepend("");

    disconnect(topWdgt->getCbxGroups(), SIGNAL(currentIndexChanged(QString)), this, SLOT(onCbxGroups_currentIndexChanged(QString)));
    proxy_model->setSpecFilter(5, "+");
    topWdgt->getCbxGroups()->clear();
    topWdgt->getCbxGroups()->addItems(listGrpsHuman);
    topWdgt->getCbxGroups()->setCurrentIndex(-1);
    connect(topWdgt->getCbxGroups(), SIGNAL(currentIndexChanged(QString)), this, SLOT(onCbxGroups_currentIndexChanged(QString)));
    topWdgt->getCbxGroups()->setCurrentIndex(0);


    const bool enableBnts = !table.isEmpty();

    topWdgt->getPbCheckedOn()->setEnabled(enableBnts);
    topWdgt->getPbPollAll()->setEnabled(enableBnts);


}

//----------------------------------------------------------------------------------------------

//void SelectMeters4poll::setPageSett(const MyListStringList &listRows, const QVariantMap &col2data, const QStringList &headerH, const QStringList &header, const bool &hasHeader)
//{

////    const QString currNi = headerH.contains("NI") ? TableViewHelper::getCellValueOfcurrentRow(ui->tvTable, headerH.indexOf("NI")) : "";

//    StandardItemModelHelper::append2model(listRows, col2data, headerH, header, hasHeader, model);


////    if(!currNi.isEmpty())
////        TableViewHelper::selectRowWithThisCell(ui->tvTable, currNi, headerH.indexOf("NI"));

//    QStringList powerins = col2data.value("\r\ngroupsext\r\n").toStringList();//<NI> - <memo>
//    powerins.prepend("");

//    disconnect(ui->cbxGroups, SIGNAL(currentIndexChanged(QString)), this, SLOT(onCbxGroups_currentIndexChanged(QString)));
//    proxy_model->setSpecFilter(5, "+");
//    ui->cbxGroups->addItems(powerins);
//    ui->cbxGroups->setCurrentIndex(-1);
//    connect(ui->cbxGroups, SIGNAL(currentIndexChanged(QString)), this, SLOT(onCbxGroups_currentIndexChanged(QString)));
//    ui->cbxGroups->setCurrentIndex(0);




//    const bool enableBnts = !listRows.isEmpty();

//    ui->pbCheckedOn->setEnabled(enableBnts);
//    ui->pbPollAll->setEnabled(enableBnts);

//    ui->tbFilter->setEnabled(!header.isEmpty());

//    emit resizeTv2content(ui->tvTable);
//}

//----------------------------------------------------------------------------------------------

void SelectMeters4poll::onProcessingEnds(QStringList listMissingData)
{//    //tr("Meter;NI;Memo;Poll;Has data").split(";"));
    topWdgt->getLblDatabase()->hide();
    for(int i = 0, imax = model->rowCount(); i < imax; i++){
         if(listMissingData.contains(model->item(i,1)->text())){
            listMissingData.removeOne(model->item(i,1)->text());
            model->item(i, 3)->setText("+");
            model->item(i, 4)->setText(tr("not all"));
        }else{
             model->item(i, 3)->setText("-");
             model->item(i, 4)->setText(tr("all"));
        }
    }
}

//----------------------------------------------------------------------------------------------

void SelectMeters4poll::onpbPollAll_clicked()
{
    QStringList l;
    for(int i = 0, imax = proxy_model->rowCount(); i < imax; i++){
        const int row = proxy_model->mapToSource(proxy_model->index(i, 0)).row();
        if(row < 0)
            continue;

        l.append(model->item(row, 1)->text());
    }
    sendStartPoll(l);
}

//----------------------------------------------------------------------------------------------

void SelectMeters4poll::sendStartPoll(const QStringList &listni)
{
    if(listni.isEmpty())
        return;

    QString mess;
    const QVariantMap map = QuickPollHelper::createPollMap(listni, lPollSett.dtTo, lPollSett.dtFrom, topWdgt->getCbxIgnoreExistingData()->isChecked()
                                                           , lPollSett.go2sleepSeconds, lPollSett.enCheckSleepProfile, lPollSett.ignoreRetr, mess);

    if(map.isEmpty())
        return;


    emit command4dev(lPollSett.pollCode, map);
    SettLoader::saveSett(SETT_ZBRTR_IGNOREEXISTING, topWdgt->getCbxIgnoreExistingData()->isChecked());

    QTimer::singleShot(11, this, SLOT(deleteLater()) );
}

//----------------------------------------------------------------------------------------------

void SelectMeters4poll::onpbSelected_clicked()
{
    sendStartPoll(TableViewHelper::getSelectedRowsText(lastTv, 1));
}

//----------------------------------------------------------------------------------------------

void SelectMeters4poll::onpbCheckedOn_clicked()
{
    QStringList l;
    for(int i = 0, imax = proxy_model->rowCount(); i < imax; i++){
        const int row = proxy_model->mapToSource(proxy_model->index(i, 0)).row();
        if(row < 0)
            continue;

        if(model->item(row, 3)->text() == "+")
            l.append(model->item(row, 1)->text());
    }
    sendStartPoll(l);
}

//----------------------------------------------------------------------------------------------

void SelectMeters4poll::onCbxGroups_currentIndexChanged(const QString &arg1)
{
    proxy_model->setSpecFilter(5, arg1.split(" ").first());

}

//----------------------------------------------------------------------------------------------

void SelectMeters4poll::ontvTable_doubleClicked(const QModelIndex &index)
{
    if(index.isValid()){
        topWdgt->getPbSelected()->setEnabled(true);
        topWdgt->getPbSelected()->animateClick();
//        ui->pbSelected->setEnabled(true);
//        ui->pbSelected->animateClick();

    }

}

//----------------------------------------------------------------------------------------------
