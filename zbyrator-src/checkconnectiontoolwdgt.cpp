#include "checkconnectiontoolwdgt.h"
#include "ui_checkconnectiontoolwdgt.h"

CheckConnectionToolWdgt::CheckConnectionToolWdgt(LastDevInfo *lDevInfo, GuiHelper *gHelper, GuiSett4all *gSett4all, QWidget *parent) :
    ReferenceWidgetClass(lDevInfo, gHelper, gSett4all, parent),
    ui(new Ui::CheckConnectionToolWdgt)
{
    ui->setupUi(this);
}

CheckConnectionToolWdgt::~CheckConnectionToolWdgt()
{
    delete ui;
}

void CheckConnectionToolWdgt::on_pushButton_clicked()
{

}
