#include "startexchange.h"
#include "ui_startexchange.h"
#include "gui-src/stackwidgethelper.h"
#include "gui-src/treemodel.h"

///[!] quick-collect
#include "src/zbyratortasksmedium.h"
#include "startpagepollv2.h"
#include "relaywdgt.h"
#include "metersdatetime.h"
#include "setmeteraddress.h"
#include "checkconnectiontoolwdgt.h"
#include "zbyratorservice.h"
#include "src/watersleepschedulermom.h"


///[!] matilda-conf-shared-widgets
#include "info-pgs/devicetasks.h"


#include "info-pgs/statisticofexchangewdgt.h"
#include "global-pgs/directaccessviamatilda.h"


///[!] widgets-shared
#include "prepaid-pgs/ifaceindicationwdgt.h"
#include "src/widgets/ifacelabel.h"
#include "zbyrator-src/zbyrifacesett.h"


///[!] guisett-shared
#include "zbyrator-src/src/startexchangehelper.h"



///[!] matilda-conf-widgets
#include "network-pgs/ifacesett4groups.h"

StartExchange::StartExchange(GuiHelper *gHelper, QWidget *parent) :
    MatildaConfWidget(gHelper,  parent),
    ui(new Ui::StartExchange)
{
    ui->setupUi(this);
    localPbReadLocker = false;
    ui->trDevOperation->setEnabled(false);

    hasRelayWdgt = false;


}
//-----------------------------------------------------------------------------------------------
StartExchange::~StartExchange()
{
    delete ui;
}

//-----------------------------------------------------------------------------------------------
void StartExchange::initPage()
{

    guiHelper = new GuiHelper(gHelper->verboseMode, this);

    guiHelper->initObj();
    guiHelper->guiSett = gHelper->guiSett;
    guiHelper->ucDeviceTreeW = gHelper->ucDeviceTreeW;
//    guiHelper->cacheHelper = gHelper->cacheHelper;

    guiHelper->stackedWidget = gHelper->stackedWidget;
    guiHelper->parentWidget = gHelper->parentWidget;
    connect(guiHelper, SIGNAL(addWdgt2stackWdgt(QWidget*,int,bool,QString,QString)), gHelper, SIGNAL(addWdgt2stackWdgt(QWidget*,int,bool,QString,QString)) );
    connect(guiHelper, SIGNAL(showMessage(QString)), gHelper, SIGNAL(showMessage(QString)));
    connect(guiHelper, SIGNAL(showMessageCritical(QString)), gHelper, SIGNAL(showMessageCritical(QString)) );
    connect(ui->swDeviceOperations, SIGNAL(currentChanged(int)), this, SLOT(onSwDevicesCurrIndxChanged()) );
    ui->wdgtReadButton->setEnabled(false);

    connect(guiHelper, SIGNAL(showMessageExt(QString,int,QVariant)), gHelper, SIGNAL(showMessageExt(QString,int,QVariant)));

    //check the next: 'guiHelper or gHelper is here?'
    connect(gHelper, SIGNAL(setPbWriteEnableDisable(bool)), this, SLOT(checkPbReadEnabled()));// ui->pbRead, SLOT(setDisabled(bool)));
    checkPbReadEnabled();
//    ui->pbRead->setDisabled(gHelper->managerEnDisBttn.pbWriteDis);

    guiHelper->stackedWidget = ui->swDeviceOperations;
    guiHelper->parentWidget = ui->swDeviceOperations;
    ui->trDevOperation->setContextMenuPolicy(Qt::CustomContextMenu);

    TreeModel * modelDevOptTree = new TreeModel(this);
    ui->trDevOperation->setModel(modelDevOptTree);
    //ui->trDevOperation->setRootIndex();

    modelDevOptTree->setDefSize4itms(QSize(ui->trDevOperation->width(), ui->pbStop->height() * 1.2));

    guiHelper->managerEnDisBttn.pbWriteDis = false;
    connect(metersListMedium, &ZbyrMeterListMedium::onConnectionStateChanged, guiHelper, &GuiHelper::setPbWriteEnableDisableSlot);
//    connect(metersListMedium, &ZbyrMeterListMedium::setLblWaitTxt, ui->lblOperationInProgress, &QLabel::setText);

    ui->pbStop->setEnabled(guiHelper->managerEnDisBttn.pbWriteDis);
    connect(metersListMedium, &ZbyrMeterListMedium::onConnectionStateChanged, ui->pbStop, &QPushButton::setEnabled);
    connect(guiHelper, SIGNAL(pbStopAnimateClick()), ui->pbStop, SLOT(animateClick()));

    connect(ui->pbStop, SIGNAL(clicked(bool)), metersListMedium, SIGNAL(killUconTasks()) );


//    connect(metersListMedium, SIGNAL(setLblWaitTxt(QString)), this, SLOT(updateScrollAreaHeight()) );
//    connect(metersListMedium, &ZbyrMeterListMedium::updateHashSn2meter, guiHelper, &GuiHelper::updateHashSn2meter);
//    connect(metersListMedium, &ZbyrMeterListMedium::setElectricityPowerCenters, guiHelper, &GuiHelper::setElectricityPowerCenters);
//    connect(metersListMedium, &ZbyrMeterListMedium::setWaterPowerCenters, guiHelper, &GuiHelper::setWaterPowerCenters);
    guiHelper->setObjectName("StartExchange");

    QStringList listIcos, chListNames;


    //, QStringList chListNames, QStringList listIcos, int buttonH)


    const QStringList chListData = StartExchangeHelper::getChListData(listIcos, chListNames);

    QStringList ignoreList = QString("Meter address;Check Connection Tool").split(";");
    for(int i = 0, imax = listIcos.size(); i < imax; i++){
        if(ignoreList.contains( chListData.at(i)))
            continue;
        modelDevOptTree->appendItem(chListNames.at(i), chListData.at(i), listIcos.at(i), QString());
    }

    ui->trDevOperation->setCurrentIndex(modelDevOptTree->index(0,0));
    lastWdgtAccessibleName = chListData.first();

//    showWdgtByName(chListData.first(), chListNames.first(), QIcon(listIcos.first()));
    QTimer::singleShot(333, this, SLOT(unlockWdgts()) );



    if(true){
        IfaceIndicationWdgt *w = new IfaceIndicationWdgt(guiHelper, this);
        ui->hl4wdgt->addWidget(w);
//        w->disableLogMode(true);
        connect(metersListMedium, SIGNAL(onReadWriteOperation(bool)), w, SLOT(onReadWriteOperation(bool)) );

//        connect(this, &StartExchange::mIfaceLog, w, &IfaceIndicationWdgt::mIfaceLog);
        connect(metersListMedium, &ZbyrMeterListMedium::ifaceLogStrFromZbyrator, w, &IfaceIndicationWdgt::appendLogLines);
        connect(metersListMedium, &ZbyrMeterListMedium::ifaceLogStrFromDA, w, &IfaceIndicationWdgt::appendLogLines);

    }

    if(true){
        IfaceLabel *l = new IfaceLabel(this);
        connect(metersListMedium, &ZbyrMeterListMedium::onIfaceSett, l, &IfaceLabel::onIfaceSett);
//        connect(l, &IfaceLabel::onTextChanged, ui->lblIface, &QLabel::setText);
//        ui->lblIface->setText(l->text());
        ui->hl4lbl->addWidget(l);
        QTimer::singleShot(2222, metersListMedium, SLOT(sendMeIfaceSett()));
    }

    connect(metersListMedium, SIGNAL(command4devStr(quint16,QString)), this, SLOT(onCommandStarted()));
    connect(metersListMedium, SIGNAL(command4dev(quint16,QVariantMap)), this, SLOT(onCommandStarted()));

    connect(this, SIGNAL(onRequest2pollThese(QStringList,quint8)), this, SLOT(activateStartPollWdgt()));
     unlockTmpLock();//init state
//    emit pageReady();
    QTimer::singleShot(555, this, SIGNAL(pageReady()));

}
//-----------------------------------------------------------------------------------------------
void StartExchange::unlockWdgts()
{
    //it calls from initPage
    ui->trDevOperation->setEnabled(true);
//    const QString named = lastWdgtAccessibleName;
    lastWdgtAccessibleName.clear();
//    showWdgtByNameData(named);
    activateStartPollWdgt();

    updateScrollAreaHeight();
}
//-----------------------------------------------------------------------------------------------
void StartExchange::showWdgtByName(const QString &wdgtAccessibleName, const QString &wdgtTitle)
{
    showWdgtByName(wdgtAccessibleName, wdgtTitle, QIcon());


}
//-----------------------------------------------------------------------------------------------
void StartExchange::showWdgtByName(const QString &wdgtAccessibleName, const QString &wdgtTitle, const QIcon &itemIcon)
{
    emit killOldWdgtSingleShot();

    if(!ui->trDevOperation->isEnabled())
        return;

    if(wdgtAccessibleName != lastWdgtAccessibleName && wdgtAccessibleName != "Interface"){
//        gHelper->closeYourPopupsSlot();
        lastWdgtAccessibleName = wdgtAccessibleName;
    }
    if(WidgetsHelper::stackContainsThisWdgt(ui->swDeviceOperations, wdgtAccessibleName, true, 0)){
        MatildaConfWidget *w = currentMatildaWidget();
        if(w){
            w->pageActivated();
//            w->setupGlobalLblMessage(ui->lblPageMess);
        }
    }else{
        addWdgt2devStack(wdgtAccessibleName, wdgtTitle, itemIcon);
    }

}

void StartExchange::showLastWdgt()
{
    showWdgtByNameData(lastWdgtAccessibleName);

}

void StartExchange::updateScrollAreaHeight()
{
    ui->scrollArea->setMaximumHeight( ui->pbStop->height() * 2.3);// (title.isEmpty() ? 1.3 : 2.3));

}

void StartExchange::appendShowMessagePlain(QString m)
{
    const QString s = m.split("\n",
                                                                                              #if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
                                                                                                  Qt::SkipEmptyParts
                                                                                              #else
                                                                                                  QString::SkipEmptyParts
                                                                                              #endif
                                                                                                      ).last();

    if(!s.isEmpty()){
        const QStringList l = s.split(": ");
        if(l.size() > 1)
            ui->lblCurrentMeter->setText(l.mid(1).join(": "));
    }
}

void StartExchange::addWdgt2stackWdgt(QWidget *w, const int &wdgtType, bool oneShot, QString actTxt, QString actIco)
{
    emit killOldWdgtSingleShot();
    const QString wdgtAccessibleName = QString("wdgt_%1").arg(wdgtType);
    if(oneShot)
        connect(this, &StartExchange::killOldWdgtSingleShot, w, &QWidget::deleteLater);

    connect(w, SIGNAL(destroyed(QObject*)), this, SLOT(showLastWdgt()));

    if(!oneShot && WidgetsHelper::stackContainsThisWdgt(ui->swDeviceOperations, wdgtAccessibleName, true, 0)){
        MatildaConfWidget *ww = currentMatildaWidget();
        if(ww){
            ww->pageActivated();
            w->deleteLater();

//            w->setupGlobalLblMessage(ui->lblPageMess);
        }
    }else{
        addWdgt2devStackExt(w, wdgtAccessibleName, actTxt, QIcon(actIco));
    }
}

void StartExchange::lockPbRead(bool disable)
{
    localPbReadLocker = disable;
    checkPbReadEnabled();

}



//-----------------------------------------------------------------------------------------------
void StartExchange::on_tbIfaceSett_clicked()
{
    //open iface settings widget

//    showWdgtByNameData("Interface");
    showWdgtByName("Interface", tr("Interface"));

//    TreeModel * modelDevOptTree = new TreeModel(this);

}

//-----------------------------------------------------------------------------------------------
MatildaConfWidget *StartExchange::currentMatildaWidget()
{
    QScrollArea *sa = qobject_cast<QScrollArea*>(ui->swDeviceOperations->currentWidget());
    if(sa){
        MatildaConfWidget *w = qobject_cast<MatildaConfWidget*>(sa->widget());
        if(w)
            return w;
    }
    return 0;
}
//-----------------------------------------------------------------------------------------------
MatildaConfWidget *StartExchange::createStartPagePoll(GuiHelper *gHelper, QWidget *parent)
{
    StartPagePollV2 *w = new StartPagePollV2(gHelper,  parent);
    w->metersListMedium = metersListMedium;


    connect(metersListMedium, SIGNAL(appendData2model(QVariantHash)), w, SLOT(setPageSett(QVariantHash)) );

    connect(metersListMedium, &ZbyrMeterListMedium::onUpdatedSavedList, w, &StartPagePollV2::onUpdatedSavedList);
    connect(metersListMedium, &ZbyrMeterListMedium::onExternalCommandProcessed, w, &StartPagePollV2::onExternalCommandProcessed);
    connect(metersListMedium, &ZbyrMeterListMedium::requestToSwitchIgnoreCycles, w, &StartPagePollV2::requestToSwitchIgnoreCycles);

    connect(w, &StartPagePollV2::lockPbRead, this, &StartExchange::lockPbRead);
    connect(w, &StartPagePollV2::onCbxIgnoreRetr  , metersListMedium, &ZbyrMeterListMedium::setIgnoreCycles);
    connect(w, &StartPagePollV2::onCbxOnlyGlobalConnection  , metersListMedium, &ZbyrMeterListMedium::setOnlyGlobalConnection);


    connect(w, &StartPagePollV2::onPollStarted    , metersListMedium, &ZbyrMeterListMedium::onPollStarted     );

//    connect(w, &StartPagePollV2)

//    connect(w, SIGNAL(command4dev(quint16,QString))    , metersListMedium, SLOT(command4devStrSlot(quint16,QString)) );
    connect(w, &StartPagePollV2::command4devStr, metersListMedium, &ZbyrMeterListMedium::command4devStrSlot);
    connect(w, &StartPagePollV2::command4dev, metersListMedium, &ZbyrMeterListMedium::command4devSlot);

    connect(w, &StartPagePollV2::addWdgt2stackWdgt, this, &StartExchange::addWdgt2stackWdgt);
    connect(this, &StartExchange::lockButtons, w, &StartPagePollV2::lockButtons);

    connect(w, &StartPagePollV2::pageEndInit, this, &StartExchange::checkPbReadEnabled);
    connect(w, &StartPagePollV2::checkPbStartPollEnabled, this, &StartExchange::checkPbReadEnabled);
    connect(w, &StartPagePollV2::onPbStartPoll, this, &StartExchange::on_pbRead_clicked);


    connect(this, &StartExchange::onRequest2pollThese, w, &StartPagePollV2::onRequest2pollThese);
    connect(w, SIGNAL(killCurrentTask()), ui->pbStop, SLOT(animateClick()));


    connect(w, &StartPagePollV2::onRequest2GetDataTheseFromDb, this, &StartExchange::onRequest2GetDataThese);
    connect(w, &StartPagePollV2::onRequest2pollTheseFromDb, this, &StartExchange::onRequest2pollThese);

    connect(this, &StartExchange::receivedKillSignal, w, &StartPagePollV2::kickOffAll);

    return w;
}
//-----------------------------------------------------------------------------------------------
MatildaConfWidget *StartExchange::createWaterSleepSchedulerWdgt(GuiHelper *gHelper, QWidget *parent)
{
    WaterSleepSchedulerMom *w = new WaterSleepSchedulerMom(gHelper,  parent);

//    connect(w, &WaterSleepSchedulerMom::onReloadAllMeters, metersListMedium, &ZbyrMeterListMedium::onReloadAllMeters);
    connect(w, &WaterSleepSchedulerMom::setLastPageId, metersListMedium, &ZbyrMeterListMedium::setLastPageId);
    connect(w, SIGNAL(command4dev(quint16,QVariantMap)), metersListMedium, SLOT(command4devSlot(quint16,QVariantMap)) );

    connect(w, &WaterSleepSchedulerMom::reloadSavedSleepProfiles, metersListMedium, &ZbyrMeterListMedium::reloadSavedSleepProfiles);
//    connect(metersListMedium, SIGNAL(setWaterMeterSchedulerPageSett(MyListStringList,QVariantMap,QStringList,QStringList,bool)),
//            w, SLOT(setPageSett(MyListStringList,QVariantMap,QStringList,QStringList,bool)) );

    connect(this, SIGNAL(lockButtons(bool)), w, SIGNAL(lockButtons(bool)));
    return w;

}
//-----------------------------------------------------------------------------------------------
MatildaConfWidget *StartExchange::createRelayWdgt(GuiHelper *gHelper, QWidget *parent)
{
    hasRelayWdgt = true;
    RelayWdgt *w = new RelayWdgt(gHelper,  parent);
//    connect(metersListMedium, &ZbyrMeterListMedium::setRelayPageSett, w, &RelayWdgt::setp);

//    connect(metersListMedium, SIGNAL(setRelayPageSett(MyListStringList,QVariantMap,QStringList,QStringList,bool)), w, SLOT(setPageSett(MyListStringList,QVariantMap,QStringList,QStringList,bool)) );
//    connect(metersListMedium, &ZbyrMeterListMedium::meterRelayStatus, w, &RelayWdgt::meterRelayStatus);

    connect(w, &RelayWdgt::setLastPageId, metersListMedium, &ZbyrMeterListMedium::setLastPageId);
//    connect(w, SIGNAL(command4dev(quint16,QVariantMap)), metersListMedium, SLOT(command4devSlot(quint16,QVariantMap)) );
    connect(w, &RelayWdgt::command4dev, metersListMedium, &ZbyrMeterListMedium::command4devSlot);
    connect(w, &RelayWdgt::command4devStr, metersListMedium, &ZbyrMeterListMedium::command4devStrSlot);

    connect(this, SIGNAL(lockButtons(bool)), w, SIGNAL(lockButtons(bool)));
    return w;
}

//-----------------------------------------------------------------------------------------------

MatildaConfWidget *StartExchange::createMetersDateTime(GuiHelper *gHelper, QWidget *parent)
{
    MetersDateTime *w = new MetersDateTime(gHelper,  parent);

    connect(w, SIGNAL(command4dev(quint16,QVariantMap)), metersListMedium, SLOT(command4devSlot(quint16,QVariantMap)) );
    connect(w, &MetersDateTime::setLastPageId           , metersListMedium, &ZbyrMeterListMedium::setLastPageId     );
    connect(metersListMedium, &ZbyrMeterListMedium::meterDateTimeDstStatus, w, &MetersDateTime::meterDateTimeDstStatus);
    connect(this, SIGNAL(lockButtons(bool)), w, SIGNAL(lockButtons(bool)));
    return w;
}
//-----------------------------------------------------------------------------------------------
MatildaConfWidget *StartExchange::createZbyrIfaceSett(GuiHelper *gHelper, QWidget *parent)
{
    ZbyrIfaceSett * w = new ZbyrIfaceSett(false, tr("Global interface settings"), gHelper,  parent);

    connect(metersListMedium, SIGNAL(setIfaceSett(QVariantHash)), w, SLOT(setPageSett(QVariantHash)) );
    connect(w, SIGNAL(setNewSettings(QVariantHash)), metersListMedium, SLOT(setNewSettings(QVariantHash)));
    connect(w, SIGNAL(sendMeIfaceSett()), metersListMedium, SLOT(sendMeIfaceSett()) );
    connect(w, SIGNAL(setNewSettings(QVariantHash)), this, SLOT(showLastWdgt()) );
    connect(w, SIGNAL(closeThisPage())              , this, SLOT(showLastWdgt()));

    connect(w, SIGNAL(sendMeTheTcpHistory(QLineEdit*))      , metersListMedium, SLOT(openTcpServerDlg(QLineEdit*)) );
    connect(w, SIGNAL(sendMeTheM2mHistory(QLineEdit*))      , metersListMedium, SLOT(openM2mDlg(QLineEdit*)));
    connect(w, SIGNAL(openEditMacProfileWdgt(bool,QLineEdit*)), this, SIGNAL(openEditMacProfileWdgt(bool,QLineEdit*)));

    return w;
}
//-----------------------------------------------------------------------------------------------
MatildaConfWidget *StartExchange::createStatisticWdgt(GuiHelper *gHelper, QWidget *parent)
{
    StatisticOfExchangeWdgt *w = new StatisticOfExchangeWdgt( gHelper,  parent);

//    connect(w, SIGNAL(onRequest2pollThese(QStringList,quint8)), this, SIGNAL(killOldWdgtStartPoll()) );

    connect(w, &StatisticOfExchangeWdgt::onRequest2pollThese, this, &StartExchange::onRequest2pollThese);
    connect(w, &StatisticOfExchangeWdgt::onRequest2GetDataThese, this, &StartExchange::onRequest2GetDataThese);
//    connect(w, &StatisticOfExchangeWdgt::checkDbPageIsReady, this, &StartExchange::checkDbPageIsReady);

//    connect(w, SIGNAL(onRequest2GetDataThese(QStringList,quint8)), this, SLOT(activatePageDb()));

    return w;

}
//-----------------------------------------------------------------------------------------------
MatildaConfWidget *StartExchange::createZbyratorTaskWdgt(GuiHelper *gHelper, QWidget *parent)
{

    ZbyratorTasksMedium *m = new ZbyratorTasksMedium;
    QThread *t = new QThread;
    t->setObjectName("ZbyratorTasksMediumT");

    m->moveToThread(t);
    connect(this, SIGNAL(receivedKillSignal()), m, SLOT(deleteLater()));
    connect(m, SIGNAL(destroyed(QObject*)), t, SLOT(quit()) );
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()) );
    connect(t, SIGNAL(started()), m, SLOT(onThreadStarted()) );

    DeviceTasks *f = new DeviceTasks(      gHelper,  parent);
    f->setAccessibleName("DeviceTasksMeter");

    connect(f, &DeviceTasks::onPageCanReceiveData, [=](){
        connect(gHelper->ucDeviceTreeW, &UCDeviceTreeWatcher::onUCTaskTableStateMeterChanged, f, &DeviceTasks::onUCTaskTableStateChanged);

        if(gHelper->ucDeviceTreeW->getCachedUCTaskTableStateMeters().validator.dtlastupdate.isValid())
             f->onUCTaskTableStateChanged(gHelper->ucDeviceTreeW->getCachedUCTaskTableStateMeters());

        f->onCanIkillTasks(true);
    });


    connect(f, &DeviceTasks::killTheseIDs, [=](const QList<quint32> &taskIdDecimal){

        QString m;
        if(!gHelper->ucDeviceTreeW->command2killMeterTasks(taskIdDecimal, f->accessibleName(), m))
            gHelper->showMessageSlot(m);
    });

    connect(f, &DeviceTasks::vanishFinishedTasks, metersListMedium, &ZbyrMeterListMedium::vanishFinishedTasks);


    connect(metersListMedium, &ZbyrMeterListMedium::onTaskTableChanged, m, &ZbyratorTasksMedium::onTaskTableChanged);


    connect(m, &ZbyratorTasksMedium::setUCTaskTableStateMeter, gHelper->ucDeviceTreeW, &UCDeviceTreeWatcher::setUCTaskTableStateMeter);
    connect(f, SIGNAL(pageEndInit()), t, SLOT(start()));

    return f;


}

//-----------------------------------------------------------------------------------------------

MatildaConfWidget *StartExchange::createZbyratorServiceWdgt(GuiHelper *gHelper, QWidget *parent)
{
    ZbyratorService *w = new ZbyratorService(    gHelper,  parent);

    connect(w, SIGNAL(command4dev(quint16,QVariantMap)), metersListMedium, SLOT(command4devSlot(quint16,QVariantMap)) );
    connect(this, SIGNAL(lockButtons(bool)), w, SIGNAL(lockButtons(bool)));

    return w;

}

MatildaConfWidget *StartExchange::createQuickDirectAccessWdgt(GuiHelper *gHelper, QWidget *parent)
{
    DirectAccessViaMatilda *w = new DirectAccessViaMatilda(gHelper, parent);

//    connect(metersListMedium, &ZbyrMeterListMedium::onQuickCollectDaStateChanged    , w, &DirectAccessViaMatilda::onDaServiceState  );
//    connect(metersListMedium, &ZbyrMeterListMedium::onQuickCollectDaStateChangedStr , w, &DirectAccessViaMatilda::onStateChanged    );
//    connect(metersListMedium, &ZbyrMeterListMedium::onDasStopped                    , w, &DirectAccessViaMatilda::onDasStopped      );
//    connect(metersListMedium, &ZbyrMeterListMedium::onDasStarted                    , w, &DirectAccessViaMatilda::onDasStarted      );
//    connect(metersListMedium, &ZbyrMeterListMedium::append2logDirectAccess          , w, &DirectAccessViaMatilda::append2log        );



//    connect(w, &DirectAccessViaMatilda::startDaServer   , metersListMedium, &ZbyrMeterListMedium::startApiAddressatorSlot   );
//    connect(w, &DirectAccessViaMatilda::stopDaServer    , metersListMedium, &ZbyrMeterListMedium::stopApiAddressator    );
//    connect(w, &DirectAccessViaMatilda::setDaForwardNI  , metersListMedium, &ZbyrMeterListMedium::setDaForwardNI        );

//    connect(w, &DirectAccessViaMatilda::pageEndInit , [=]{
//        metersListMedium->sendForcedQuickCollectDaChangingState();
//        metersListMedium->sendForcedQuickCollectDaState();
//    });

    return w;

}
//-----------------------------------------------------------------------------------------------
MatildaConfWidget *StartExchange::createIfaceSett4groupsWdgt(GuiHelper *gHelper, QWidget *parent)
{
    IfaceSett4groups * w = new IfaceSett4groups(gHelper, parent);


    connect(w, SIGNAL(sendMeTheTcpHistory(QLineEdit*))      , metersListMedium, SLOT(openTcpServerDlg(QLineEdit*)) );
    connect(w, SIGNAL(sendMeTheM2mHistory(QLineEdit*))      , metersListMedium, SLOT(openM2mDlg(QLineEdit*)));
    connect(w, SIGNAL(openEditMacProfileWdgt(bool,QLineEdit*)), this, SIGNAL(openEditMacProfileWdgt(bool,QLineEdit*)));

    connect(w, SIGNAL(addIfaceSett2history(QVariantHash)), metersListMedium, SLOT(addIfaceSett2history(QVariantHash)));


    //there is no signal to tell to update NI channels
    connect(w, SIGNAL(addIfaceSett2history(QVariantHash)), metersListMedium, SIGNAL(reloadIfaceChannels()));

    return w;

}
//-----------------------------------------------------------------------------------------------
void StartExchange::showWdgtByNameData(const QString &named)
{
    QStringList listIcos, chListNames;


    //, QStringList chListNames, QStringList listIcos, int buttonH)


    const QStringList chListData = StartExchangeHelper::getChListData(listIcos, chListNames);

    const int row = chListData.indexOf(named);
    if(row < 0)
        return;
//QString("Poll;Relay;Queue;Statistic of the exchange;Date and time;Meter address;Check Connection Tool;Other;Interface").split(";");

    showWdgtByName(chListData.at(row), chListNames.at(row), QIcon(listIcos.at(row)));
//ui->trDevOperation->model()
    TreeModel *modelDevOptTree { static_cast<TreeModel *>(ui->trDevOperation->model())};
    if(modelDevOptTree)
        ui->trDevOperation->setCurrentIndex(modelDevOptTree->index(row,0));


}

//-----------------------------------------------------------------------------------------------

void StartExchange::on_trDevOperation_clicked(const QModelIndex &index)
{
    if(index.isValid()){
//        if(true){
//            MatildaConfWidget *w = currentMatildaWidget();
//            if(w)
//                w->disableGlobalLblMessage(ui->lblPageMess);
//        }


        const QString s = index.data(Qt::UserRole).toString();

        if(!s.isEmpty()){
            //real page name
            showWdgtByName(s, index.data(Qt::DisplayRole).toString(), index.data(Qt::DecorationRole).value<QIcon>());


        }
    }

}

void StartExchange::addWdgt2devStack(const QString &realPageName, const QString &wdgtTitle, const QIcon &itemIcon)
{

    QStringList listIcos, chListNames;


    //, QStringList chListNames, QStringList listIcos, int buttonH)


    const QStringList chListData = StartExchangeHelper::getChListData(listIcos, chListNames);

//    return QString("Poll;Scheduler for water meters;Relay;Queue;Statistic of the exchange;Date and time;Meter address;Check Connection Tool;Other;Interface").split(";");

    MatildaConfWidget *w = 0;

    const int row = chListData.indexOf(realPageName);
    switch(row){
    case 0: w = createStartPagePoll(            guiHelper, this); break;
    case 1: w = createWaterSleepSchedulerWdgt(  guiHelper, this); break;

    case 2: w = createRelayWdgt(                guiHelper, this); break;

    case 3: w = createZbyratorTaskWdgt(         guiHelper, this); break;
    case 4: w = createStatisticWdgt(            guiHelper, this); break;

    case 5: w = createMetersDateTime(           guiHelper, this); break;
    case 6: w = new SetMeterAddress(            guiHelper, this); break;

    case 7: w = new CheckConnectionToolWdgt(    guiHelper, this); break;
    case 8: w = createQuickDirectAccessWdgt(    guiHelper, this); break;
    case 9: w = createZbyratorServiceWdgt(      guiHelper, this); break;
    case 10: w = createIfaceSett4groupsWdgt(    guiHelper, this); break;
    case 11: w = createZbyrIfaceSett(           guiHelper, this); break;

    }

    if(w){
        try{
            w->objectName();
        }catch(...){
            w = 0;
            return;
        }

//        w->setRwCommand( hasReadButton ? (10 + row) : 0, 1);// MatildaDeviceTree::getPageCanRead().at(row), MatildaDeviceTree::getPageCanWrite().at(row));
        addWdgt2devStackExt(w, realPageName, wdgtTitle, itemIcon);

    }



}

void StartExchange::addWdgt2devStackExt(QWidget *w, const QString &realPageName, const QString &wdgtTitle, const QIcon &itemIcon)
{
    if(w){
        w->setWindowTitle(wdgtTitle);
        w->setWindowIcon(itemIcon);

//        connect(this, SIGNAL(onCOMMAND2GUI(quint16,QString)), w, SLOT(onCOMMAND2GUI(quint16,QString)) );
//        connect(this, SIGNAL(onCOMMAND2GUI(quint16,QVariantHash)), w, SLOT(onCOMMAND2GUI(quint16,QVariantHash)) );

//        connect(w, SIGNAL(onCOMMAND2GUIready()), this, SIGNAL(onCOMMAND2GUIready()) );

//        connect(this, SIGNAL(killMyChild()), w, SLOT(deleteLater()) );
//        connect(this, SIGNAL(onOperationNError(int)), w, SLOT(onOperationNError(int)) );

        w->setAccessibleName(realPageName);
        qDebug() << "realPageName " <<  realPageName << wdgtTitle ;

//        w->setupGlobalLblMessage(ui->lblPageMess);
        QWidget *sa = StackWidgetHelper::addWdgtWithScrollArea(this, w, realPageName);
        ui->swDeviceOperations->addWidget(sa);
        ui->swDeviceOperations->setCurrentWidget(sa);

        QTimer::singleShot(999, this, SLOT(checkPbReadEnabled()));

    }
}


void StartExchange::on_pbStop_clicked()
{
    ui->pbStop->setEnabled(false);
}

void StartExchange::on_pbRead_clicked()
{
    if(!ui->pbRead->isEnabled()){
        checkPbReadEnabled();
        return;
    }

    MatildaConfWidget *w = currentMatildaWidget();

    if(w){
//        ui->pbRead->setEnabled(false);
        onCommandStarted();

        QString message;
        const QString wname = w->updatePageContent(message);
        if(wname.isEmpty() && !message.isEmpty())
            gHelper->showMessageSlot(message);
//        w->getPageSett4read(ok, mess);



        QTimer::singleShot(555, this, SLOT(checkPbReadEnabled()));
    }
}

void StartExchange::checkPbReadEnabled()
{

    const bool isProcessing = (guiHelper->managerEnDisBttn.pbWriteDis || localPbReadLocker || lockByCommandTmp);
    ui->pbRead->setDisabled(isProcessing);
    emit lockButtons(isProcessing);
    qDebug() << "checkPbReadEnabled " << isProcessing << guiHelper->managerEnDisBttn.pbWriteDis << localPbReadLocker << lockByCommandTmp << guiHelper->managerEnDisBttn.pbWriteOnlyRootDis;

}

void StartExchange::onCommandStarted()
{
    lockByCommandTmp = true;
    QTimer::singleShot(1111, this, SLOT(unlockTmpLock()));

}

void StartExchange::unlockTmpLock()
{
    lockByCommandTmp = false;
    checkPbReadEnabled();

}

void StartExchange::onSwDevicesCurrIndxChanged()
{
    MatildaConfWidget *w = currentMatildaWidget();

    if(w){
            w->checkHasReadWriteButtons();
//        const int readCommand = 0;// w->getReadCommand();
//        const int writeCommand = w->getWriteCommand();

        ui->wdgtReadButton->setEnabled(w->getHasReadButton());// readCommand > 0);

    }
}

void StartExchange::activateStartPollWdgt()
{
    if(!hasRelayWdgt){
        showWdgtByNameData("Relay");
        lastWdgtAccessibleName.clear();
    }

    if(lastWdgtAccessibleName == "Poll")
        return;


    lastWdgtAccessibleName = "Poll";
    showLastWdgt();

//    lastWdgtAccessibleName


}
