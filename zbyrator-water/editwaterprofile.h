#ifndef EDITWATERPROFILE_H
#define EDITWATERPROFILE_H

#include "src/matilda-conf-wdgt/confpopupwdgt.h"

namespace Ui {
class EditWaterProfile;
}

class EditWaterProfile : public ConfPopupWdgt
{
    Q_OBJECT

public:
    explicit EditWaterProfile(QWidget *parent = 0);
    ~EditWaterProfile();


signals:
    void onSaveProfile(QString name, QVariantHash profile);

    void setProfile(const QVariantHash &profile);

    void setProfileName(QString name);

    void deleteProfileName();

    void ready2edit();


private slots:

    void on_pbDefault_clicked();

    void on_pbSave_clicked();



public slots:
    void setProfileSlot(const QString &name, const QVariantHash &profile);


    void onNewProfile(const QVariantHash &profile);


    void onTbClearPressed();

    void onTbUndoPressed();

    void setupPage();

private:
    Ui::EditWaterProfile *ui;

    QString lastProfileName;
    QVariantHash lastProfile;


};

#endif // EDITWATERPROFILE_H