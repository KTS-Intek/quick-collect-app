#include "qcmainwindow.h"
#include "ui_qcmainwindow.h"
#include "src/matilda/settloader.h"
#include "main-pgs/langdialog.h"
#include <QDebug>
#include <QTimer>
#include <QLocale>
#include <QDesktopWidget>
#include <QStyleFactory>
#include "zbyrator-src/createtoolbar.h"
#include "src/shared/stackwidgethelper.h"

#include "dataconcetrator-pgs/meterlistwdgt.h"
#include "src/matilda/moji_defy.h"
#include "dataconcetrator-pgs/databasewdgt.h"
#include "dataconcetrator-pgs/editenergywidget.h"
#include "dataconcetrator-pgs/meterjournalform.h"
#include "template-pgs/smplptewdgt.h"
#include "zbyrator-src/ktsconnectwdgt.h"
#include "zbyrator-src/startexchange.h"
#include "src/zbyrator-v2/metermanager.h"
#include "zbyrator-src/zbyratorprocessmanager.h"
#include "zbyrator-src/wdgt/addeditmacform.h"
#include "src/matilda/guihelperdefines.h"
#include "main-pgs/custommessagewidget.h"


#include "main-pgs/scanipwidget.h"
#include "src/matilda/classmanagerudpscanner.h"


QcMainWindow::QcMainWindow(const QFont &font4log, const int &defFontPointSize,  QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QcMainWindow)
{
    ui->setupUi(this);

    guiSett = new GuiSett4all(this);
    guiSett->font4log = font4log;
    guiSett->defFontPointSize = defFontPointSize;

    loadMainSett();


    GuiSett4all::setKtsIntekDefautStyle();

    QTimer::singleShot(999, this, SLOT(initializeZbyrator())) ;
}

//---------------------------------------------------------------------

QcMainWindow::~QcMainWindow()
{
    delete ui;
}

//---------------------------------------------------------------------

void QcMainWindow::loadMainSett()
{

    QRect rect = SettLoader::loadSett(SETT_MAIN_GEOMETRY).toRect();
    if(!rect.isNull()){
        qint32 desktopW = qApp->desktop()->width();
        qint32 desktopH = qApp->desktop()->height();

        if(rect.x()< 10)
            rect.setX(10);
        if(rect.y()<30)
            rect.setY(31);
        if(rect.x()>(desktopW)  || rect.y()>(desktopH)) {
            int lastW = rect.width();
            int lastH = rect.height();
            rect.setX(10);
            rect.setY(31);
            rect.setWidth(lastW);
            rect.setHeight(lastH);
        }
        setGeometry(rect);
    }


    if(SettLoader::loadSett(SETT_MAIN_LANG_SELECTED).toBool()){
         loadLanguage(SettLoader::loadSett(SETT_MAIN_CURRLANG).toString());

    }else{
        LangDialog *d = new LangDialog(this);
        connect(d, SIGNAL(onLangSelected(QString)), this, SLOT(onLangSelected(QString)) );
        if(d->exec() == QDialog::Rejected)
            QTimer::singleShot(11, this, SLOT(loadFontSize()) );
        d->deleteLater();
    }
}

//---------------------------------------------------------------------

void QcMainWindow::initializeZbyrator()
{
    qRegisterMetaType<QHash<QString,QString> >("QHash<QString,QString>");
    qRegisterMetaType<MyListStringList>("MyListStringList");
    qRegisterMetaType<MyListHashString>("MyListHashString");

    guiHelper = new GuiHelper(this);

    guiHelper->initObj();
    guiHelper->guiSett = guiSett;
    guiHelper->lDevInfo = new LastDevInfo(this);
    guiHelper->lDevInfo->matildaDev.protocolVersion = MATILDA_PROTOCOL_VERSION;
    guiHelper->cacheHelper = new MatildaCacheHelper(this);
    guiHelper->stackedWidget = ui->stackedWidget;
    guiHelper->parentWidget = this;

    connect(ui->stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(onStackedWidgetCurrentChanged(int)) );
    connect(guiHelper, SIGNAL(addWdgt2stackWdgt(QWidget*,int,bool,QString,QString)), this ,SLOT(addWdgt2stackWdgt(QWidget*,int,bool,QString,QString)) );

    connect(guiHelper, SIGNAL(showMess(QString)), this, SLOT(showMess(QString)));
    connect(guiHelper, SIGNAL(showMessCritical(QString)), this, SLOT(showMess(QString)));
    connect(guiHelper, SIGNAL(showMessExt(QString,int,QVariant)), this, SLOT(showMess(QString,int,QVariant)));

//    connect(guiHelper, SIGNAL(addWdgt2stackWdgt(QWidget*,int)   ), this, SLOT(addWdgt2stackWdgt(QWidget*,int))  );
//    connect(guiHelper, SIGNAL(loadDevList()                     ), this, SLOT(loadDevList())                    );

    connect(guiSett, SIGNAL(changeLanguate(QString)), this, SLOT(onLangSelected(QString)) );
    connect(guiSett, SIGNAL(updateStyle(QString))   , this, SLOT(setStyleSheet(QString))  );


    StackWidgetHelper *swh = new StackWidgetHelper(this);
    swh->setStackWdgt(ui->stackedWidget);
    connect(this, SIGNAL(addWdgt2history()), swh, SLOT(onAddWdgt2history()) );
    connect(ui->stackedWidget, SIGNAL(destroyed(QObject*)), swh, SLOT(onStackKickedOff()));

    createToolBar();
    createZbyrProcManager();
    createMeterManager();


}

//---------------------------------------------------------------------

void QcMainWindow::onLangSelected(QString lang)
{
    if(lang.isEmpty())
        return;

    SettLoader::saveSett(SETT_MAIN_LANG_SELECTED, true);
    SettLoader::saveSett(SETT_MAIN_CURRLANG, lang);
    guiSett->currLang.clear();
    loadLanguage(lang);
    //    loadMainSett();

    if(guiSett->verboseMode)
        qDebug() << "onLang selected" << lang;
}

//---------------------------------------------------------------------

void QcMainWindow::loadFontSize()
{
    int fontSize = SettLoader::loadSett(SETT_OPTIONS_APPFONT, guiSett->defFontPointSize).toInt();
    if(fontSize < 6 || fontSize > 300)
        fontSize = guiSett->defFontPointSize;
    if(fontSize > 6 && fontSize < 300)
        setStyleSheet(QString("font-size:%1pt;").arg(fontSize));
}

//---------------------------------------------------------------------

void QcMainWindow::onActivateThisWdgt(QString tabData)
{
    //stackContainsThisWdgt(QStackedWidget *stackedWidget, const QString &wdgtName, const bool andChange, const int indxFrom)
    if(!GuiHelper::stackContainsThisWdgt(ui->stackedWidget, tabData, true))
        createOneOfMainWdgt(tabData);

    emit addWdgt2history();
}

//---------------------------------------------------------------------

void QcMainWindow::addWdgt2stackWdgt(QWidget *w, int wdgtTag, bool oneShot, QString actTxt, QString actIco)
{
    emit killOldWdgt();

    const QString wdgtName = QString("Wdgt4stack_%1").arg(wdgtTag);
    for(int i = 1, iMax = ui->stackedWidget->count(); i < iMax; i++){
        if(ui->stackedWidget->widget(i)->accessibleName() == wdgtName){
            ui->stackedWidget->widget(i)->deleteLater();
            break;
        }
    }

    w->setAccessibleName(wdgtName);

//    QAction *a = new QAction(w);


//    switch(oneShot){
//    case WDGT_TYPE_EDITENERGYWIDGET     : a->setText(tr("Edit Energy"))         ; a->setIcon(QIcon(":/katynko/svg/document-edit.svg")); break;
//    case WDGT_TYPE_RENAMEOBJECTWIDGET   : a->setText(tr("Rename Object"))       ; a->setIcon(QIcon(":/katynko/svg/address-book-new.svg")); break;
//    case WDGT_TYPE_SCANIPWIDGET         : a->setText(tr("Scan"))                ; a->setIcon(QIcon(":/katynko/svg/irc-voice.svg")); break;
//    default: oneShot = true; break;
//    }

    if(oneShot)
        connect(this, SIGNAL(killOldWdgt()), w, SLOT(deleteLater()) );//залишу тільки для окремих випадків


    if(!actTxt.isEmpty()){
        QAction *a = new QAction(QIcon(actIco), actTxt, w);
        a->setData(wdgtTag);
        connect(a, SIGNAL(triggered(bool)), this, SLOT(onActImitatorClck()) );

        QAction *s = ui->mainToolBar->addSeparator();
        connect(a, SIGNAL(destroyed(QObject*)), s, SLOT(deleteLater()) );

        ui->mainToolBar->addAction(a);
    }

    if(wdgtTag == WDGT_TYPE_DEVICES){
        ui->stackedWidget->addWidget(w);
        ui->stackedWidget->setCurrentWidget(w);
    }else{
        QScrollArea *sa = StackWidgetHelper::addWdgtWithScrollArea(this, w, wdgtName);
        ui->stackedWidget->addWidget(sa);
        ui->stackedWidget->setCurrentWidget(sa);
    }
    emit addWdgt2history();
}
//---------------------------------------------------------------------
void QcMainWindow::openEditMacProfileWdgt(bool isEditMode, QLineEdit *le)
{
    if(GuiHelper::stackContainsThisWdgtType(ui->stackedWidget, WDGT_TYPE_EDIT_OBJET_CONNECTION, true)){
        emit setThisObjProfileName( (isEditMode && le) ? le->text() : "");

    }else{

        AddEditMacForm *w = new AddEditMacForm(metersListMedium->ifaceLoader, (le && isEditMode) ? le->text() : "", this);
//        connect(this, SIGNAL(setThisMacAndObjName(QString,QString)), w, SLOT(setThisMacAndObjName(QString,QString)) );
//        connect(this, SIGNAL(setThisObjNameIpPort(QString,QString,quint16)), w, SLOT(setThisObjNameIpPort(QString,QString,quint16)) );
        connect(this, SIGNAL(setThisObjProfileName(QString)), w,SLOT(preapryPage(QString)) );

        connect(w, SIGNAL(showMess(QString)), this, SLOT(showMess(QString)) );
        connect(w, SIGNAL(openScanWdgt(int,IfaceSettLoader*)), this, SLOT(onScanClicked(int,IfaceSettLoader*)) );
        if(le)
            connect(w, SIGNAL(setLblTxt(QString)), le, SLOT(setText(QString)) );

        addWdgt2stackWdgt(w, WDGT_TYPE_EDIT_OBJET_CONNECTION, false, tr("M2M profile"), ":/katynko/svg/irc-voice.svg");
    }
}

//---------------------------------------------------------------------

void QcMainWindow::showMess(QString mess)
{
    showMess(mess, CSTM_MB_INFORMATION);
}

//---------------------------------------------------------------------

void QcMainWindow::showMessSmpl(QString mess)
{
    showMess(mess, CSTM_MB_INFORMATION);
}

//---------------------------------------------------------------------

void QcMainWindow::showMessCritical(QString mess)
{
    showMess(mess, CSTM_MB_CRITICAL);
}

//---------------------------------------------------------------------

void QcMainWindow::showMess(QString mess, int messType, const QVariant customData)
{
    if(GuiHelper::stackContainsThisWdgtType(ui->stackedWidget, WDGT_TYPE_CUSTOMMESSAGEWIDGET))
        emit killOldWdgt();

    CustomMessageWidget *w = new CustomMessageWidget(messType, mess, this);
    if(customData.isValid())
        w->setCustomData(customData);//:/katynko/svg2/lc_extendedhelp.svg
    addWdgt2stackWdgt(w, WDGT_TYPE_CUSTOMMESSAGEWIDGET, true, tr("Message"), ":/katynko/svg2/lc_extendedhelp.svg");
}

//---------------------------------------------------------------------

void QcMainWindow::onScanClicked(const int &mode, IfaceSettLoader *connWdgt)
{
    if(!GuiHelper::stackContainsThisWdgtType(ui->stackedWidget, WDGT_TYPE_SCANIPWIDGET)){

        ScanIpWidget *w = new ScanIpWidget(mode, this);// connWdgt->getTabWidget_2(), this);
        if(connWdgt){
            connect(w, SIGNAL(setThisIp(QString))       , connWdgt, SLOT(setThisIp(QString)) );
            connect(w, SIGNAL(setThisObjName(QString))  , connWdgt, SLOT(setThisObjName(QString)) );
            connect(w, SIGNAL(setThisPort(int))         , connWdgt, SLOT(setThisPort(int)) );
            connect(w, SIGNAL(setThisMac(QString))      , connWdgt, SLOT(setThisMac(QString)) );
            connect(w, SIGNAL(setThisObjName_2(QString)), connWdgt, SLOT(setThisObjName_2(QString)) );
            connect(w, SIGNAL(setThisMode(int))         , connWdgt, SLOT(setThisMode(int)) );
        }

        connect(w, SIGNAL(showMess(QString))        , this, SLOT(appendShowMess(QString)) );

        ClassManagerUdpScanner *classManager = new ClassManagerUdpScanner;

        QThread *t = new QThread;
        classManager->moveToThread(t);

        connect(t, SIGNAL(started()) , classManager, SLOT(initializeScanner()) );
        connect(t, SIGNAL(finished()), t      , SLOT(deleteLater())       );
        connect(classManager, SIGNAL(destroyed(QObject*)), t, SLOT(quit()) );


        connect(w, SIGNAL(startUdpScan(quint16,bool))   , classManager, SIGNAL(startUdpScan(quint16,bool)) );
        connect(w, SIGNAL(stopUdpScan())                , classManager, SIGNAL(stopUdpScan())              );
        connect(w, SIGNAL(onCheckTcpChanged(bool))      , classManager, SIGNAL(onCheckTcpChanged(bool))       );
        connect(w, SIGNAL(destroyed(QObject*))          , classManager, SLOT(kickMe()) );


        connect(classManager, SIGNAL(findedDevice(QString,QString,QString)), w, SLOT(findedDevice(QString,QString,QString)) );
        connect(classManager, SIGNAL(scanStarted()                        ), w, SLOT(onScanStarted())                       );
        connect(classManager, SIGNAL(scanFinished()                       ), w, SLOT(onScanStopped())                       );
        connect(classManager, SIGNAL(onUserOnline(QString,QString,QString)), w, SLOT(onUserOnline(QString,QString,QString)) );

        connect(classManager, SIGNAL(showMess(QString)                    ), w, SIGNAL(showMess(QString)) );


        t->start();

        addWdgt2stackWdgt(w, WDGT_TYPE_SCANIPWIDGET, false, tr("Scan"), ":/katynko/svg/irc-voice.svg");
    }else{
        emit addWdgt2history();

    }
}

void QcMainWindow::appendShowMess(QString m)
{

}

void QcMainWindow::appendShowMessPlain(QString m)
{

}

void QcMainWindow::onActImitatorClck()
{
    QAction *a = qobject_cast<QAction*>(sender());
    if(a && a->data().toInt() > 0){
        const int wdgtType = a->data().toInt();
        if(!GuiHelper::stackContainsThisWdgtType(ui->stackedWidget, wdgtType, true))
            a->deleteLater();
        else
            emit addWdgt2history();

    }
}

//---------------------------------------------------------------------

void QcMainWindow::changeEvent(QEvent *event)
{
    if(event) {
        switch(event->type()) {
        // this event is send if a translator is loaded
        case QEvent::LanguageChange: {
            ui->retranslateUi(this);
            break; }
            // this event is send, if the system, language changes
        case QEvent::LocaleChange: {
            QString locale = QLocale::system().name();
            locale.truncate(locale.lastIndexOf('_'));
            loadLanguage(locale);
            break;}
        default: break;
        }
    }
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------

void QcMainWindow::loadLanguage(const QString &rLanguage)
{
    if(guiSett->currLang != rLanguage) {
        guiSett->currLang = rLanguage;
        QString langPath = QApplication::applicationDirPath();
        QLocale locale = QLocale(rLanguage);
        QLocale::setDefault(locale);

        qApp->removeTranslator(&translator);

        if(translator.load(QString("%1/lang/%2").arg(langPath).arg( QString("lang_%1.qm").arg(rLanguage)))){
            qApp->installTranslator(&translator);
        }else{
            if(guiSett->verboseMode)
                qDebug() << "errr load " << QString("%1/lang/%2").arg(langPath).arg( QString("lang_%1.qm").arg(rLanguage));
        }
        QTimer::singleShot(32, this, SLOT(loadFontSize()) );

    }
}

//---------------------------------------------------------------------

void QcMainWindow::createOneOfMainWdgt(const QString &tabData)
{
    qDebug() << "create " << tabData;

    LastDevInfo *lDevInfo = guiHelper->lDevInfo;
    GuiHelper *gHelper = guiHelper;
    GuiSett4all *gSett4all = guiSett;

    const int row = CreateToolBar::getTabsData().indexOf(tabData);
    MatildaConfWidget *w = 0;

    switch(row){
    case 0: w = createStartExchangeWdgt(lDevInfo, gHelper, gSett4all, this); break;
    case 1: w = createMeterListWdgt(    lDevInfo, gHelper, gSett4all, this); break;

    case 2: w = new DataBaseWdgt(       lDevInfo, gHelper, gSett4all, this) ; break;  //    l.append( QString("Database") );
    case 3: w = new MeterJournalForm(   lDevInfo, gHelper, gSett4all, this) ; break; //    l.append( QString("Meter logs") );
    case 4: w = new KtsConnectWdgt(     lDevInfo, gHelper, gSett4all, this); break;
    case 5: w = createPageLog(lDevInfo, gHelper, gSett4all, this)  ; break;   //    l.append( QString("State")                   );

    }

    if(w){
        try{
            w->objectName();
        }catch(...){
            w = 0;
            return;
        }
    }

    if(w){
        const TabName2icosPaths ttn = CreateToolBar::getTabs().at(row);

        w->setWindowTitle(ttn.tabName);
        w->setWindowIcon(QIcon(ttn.path));


//        connect(this, SIGNAL(killMyChild()), w, SLOT(deleteLater()) );
//        connect(this, SIGNAL(onOperationNError(int)), w, SLOT(onOperationNError(int)) );
//        w->setRwCommand(MatildaDeviceTree::getPageCanRead().at(row), MatildaDeviceTree::getPageCanWrite().at(row));

        w->setRwCommand(1, 0xFFFE);
        w->setHasDataFromRemoteDevice();
        w->setupGlobalLblMessage(ui->lblPageMess);
        QWidget *sa = StackWidgetHelper::addWdgtWithScrollArea(this, w, tabData);
        ui->stackedWidget->addWidget(sa);
        ui->stackedWidget->setCurrentWidget(sa);
    }
}

//---------------------------------------------------------------------

void QcMainWindow::createToolBar()
{

    CreateToolBar *c = new CreateToolBar(this);
    connect(c, SIGNAL(onActivateThisWdgt(QString)), this, SLOT(onActivateThisWdgt(QString)) );
    c->createToolBarItems(ui->mainToolBar);
}

//---------------------------------------------------------------------

void QcMainWindow::createZbyrProcManager()
{
    ZbyratorProcessManager *m = new ZbyratorProcessManager;
    QThread *t = new QThread;

    m->moveToThread(t);

    connect(this, &QcMainWindow::destroyed, m, &ZbyratorProcessManager::deleteLater);
    connect(m, &ZbyratorProcessManager::destroyed, t, &QThread::quit);
    connect(t, &QThread::finished, t, &QThread::deleteLater);

    connect(t, &QThread::started, m, &ZbyratorProcessManager::onThreadStarted);

    QTimer::singleShot(11, t, SLOT(start()));


}

//---------------------------------------------------------------------

void QcMainWindow::createMeterManager()
{

    MeterManager *zbyrator = new MeterManager(true, ZbyrConnSett());

    QThread *thread = new QThread;

    zbyrator->registerMyTypes();

    zbyrator->moveToThread(thread);
    connect(thread, SIGNAL(started()), zbyrator, SLOT(initObject()));

//    connect(extSocket, &ZbyratorSocket::appendDbgExtData, this, &ZbyratorManager::appendDbgExtData );

    connect(this, &QcMainWindow::command4dev     , zbyrator, &MeterManager::command4dev      );


//    connect(zbyrator, &MeterManager::command2extensionClient, extSocket, &ZbyratorSocket::command2extensionClient   );
//    connect(zbyrator, &MeterManager::onAboutZigBee          , extSocket, &ZbyratorSocket::sendAboutZigBeeModem      );

    metersListMedium = new ZbyrMeterListMedium(this);
    connect(metersListMedium, &ZbyrMeterListMedium::onReloadAllMeters   , zbyrator, &MeterManager::onReloadAllMeters    );
    connect(metersListMedium, &ZbyrMeterListMedium::onConfigChanged     , zbyrator, &MeterManager::onConfigChanged      );
    connect(metersListMedium, &ZbyrMeterListMedium::sendMeAlistOfMeters , zbyrator, &MeterManager::sendMeAlistOfMeters  );
    connect(metersListMedium, &ZbyrMeterListMedium::command4dev         , zbyrator, &MeterManager::command4dev          );

    connect(metersListMedium, &ZbyrMeterListMedium::setThisIfaceSett    , zbyrator, &MeterManager::setThisIfaceSett     );
    connect(metersListMedium, &ZbyrMeterListMedium::setPollSaveSettings , zbyrator, &MeterManager::setPollSaveSettings  );

    connect(metersListMedium, &ZbyrMeterListMedium::giveMeYourCache     , zbyrator, &MeterManager::giveMeYourCache      );

    connect(metersListMedium, SIGNAL(showMess(QString)), this, SLOT(showMess(QString)) );

    connect(zbyrator, SIGNAL(checkThisMeterInfo(UniversalMeterSett)), metersListMedium, SIGNAL(onReloadAllMeters()) );

    connect(zbyrator, &MeterManager::onAllMeters    , metersListMedium, &ZbyrMeterListMedium::onAllMeters       );
    connect(zbyrator, &MeterManager::onAlistOfMeters, metersListMedium, &ZbyrMeterListMedium::onAlistOfMeters   );
    connect(zbyrator, &MeterManager::ifaceLogStr    , metersListMedium, &ZbyrMeterListMedium::ifaceLogStr       );

    connect(zbyrator, &MeterManager::appendMeterData, metersListMedium, &ZbyrMeterListMedium::appendMeterData   );


    QTimer::singleShot(1111, thread, SLOT(start()) );
}

//---------------------------------------------------------------------

MatildaConfWidget *QcMainWindow::createStartExchangeWdgt(LastDevInfo *lDevInfo, GuiHelper *gHelper, GuiSett4all *gSett4all, QWidget *parent)
{
    StartExchange *w = new StartExchange(lDevInfo, gHelper, gSett4all, parent);
    w->metersListMedium = metersListMedium;

    connect(w, SIGNAL(openEditMacProfileWdgt(bool,QLineEdit*)), this, SLOT(openEditMacProfileWdgt(bool,QLineEdit*)));

    return w;
}

//---------------------------------------------------------------------

MatildaConfWidget *QcMainWindow::createMeterListWdgt(LastDevInfo *lDevInfo, GuiHelper *gHelper, GuiSett4all *gSett4all, QWidget *parent)
{

    MeterListWdgt *w = new MeterListWdgt(lDevInfo, gHelper, gSett4all, parent);
    connect(w, &MeterListWdgt::onReloadAllMeters, metersListMedium, &ZbyrMeterListMedium::onReloadAllMeters);
    connect(w, &MeterListWdgt::meterModelChanged, metersListMedium, &ZbyrMeterListMedium::meterModelChanged);
//    connect(metersListMedium, &ZbyrMeterListMedium::setMeterListPageSett, w, &MeterListWdgt::setPageSett);
    connect(metersListMedium, SIGNAL(setMeterListPageSett(MyListStringList,QVariantMap,QStringList,QStringList,bool)), w, SLOT(setPageSett(MyListStringList,QVariantMap,QStringList,QStringList,bool)) );

    return w;


}

MatildaConfWidget *QcMainWindow::createPageLog(LastDevInfo *lDevInfo, GuiHelper *gHelper, GuiSett4all *gSett4all, QWidget *parent)
{
    SmplPteWdgt *w = new SmplPteWdgt(tr("Log") , true, true, lDevInfo, gHelper, gSett4all, false, parent);
    connect(w, &SmplPteWdgt::giveMeYourCache, metersListMedium, &ZbyrMeterListMedium::giveMeYourCache);
    connect(metersListMedium, &ZbyrMeterListMedium::ifaceLogStr, w, &SmplPteWdgt::appendPteText);
    return w;
}

//---------------------------------------------------------------------

void QcMainWindow::onStackedWidgetCurrentChanged(int arg1)
{
    if(arg1 < 0 || !ui->stackedWidget->currentWidget())
        return;
    const QString wdgtAccessibleName = ui->stackedWidget->currentWidget()->accessibleName();
    if(wdgtAccessibleName != lastWdgtAccessibleName){
        guiHelper->closeYourPopupsSlot();
        lastWdgtAccessibleName = wdgtAccessibleName;
    }
}

//---------------------------------------------------------------------
