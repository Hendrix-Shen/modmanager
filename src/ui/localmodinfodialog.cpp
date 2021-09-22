#include "localmodinfodialog.h"
#include "ui_localmodinfodialog.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QDebug>

#include "localfileitemwidget.h"
#include "local/localmod.h"
#include "curseforge/curseforgemod.h"
#include "modrinth/modrinthmod.h"
#include "curseforgemodinfodialog.h"
#include "modrinthmodinfodialog.h"
#include "util/funcutil.h"
#include "util/websiteicon.h"

LocalModInfoDialog::LocalModInfoDialog(QWidget *parent, LocalMod *mod) :
    QDialog(parent),
    ui(new Ui::LocalModInfoDialog),
    file_(mod->modFile()),
    mod_(mod)
{
    ui->setupUi(this);
    //not use it currently
    ui->disableButton->setVisible(false);
    ui->aliasText->setVisible(false);

    //init info
    onCurrentModChanged();

    //signals / slots
    connect(mod_, &LocalMod::curseforgeReady, this, [=](bool bl){
        ui->curseforgeButton->setEnabled(bl);
    });
    connect(mod_, &LocalMod::modrinthReady, this, [=](bool bl){
        ui->modrinthButton->setEnabled(bl);
    });
    connect(mod_, &LocalMod::modFileUpdated, this, &LocalModInfoDialog::onCurrentModChanged);
    connect(ui->aliasText, &QLineEdit::returnPressed, this, [=]{
        ui->editAliasButton->setChecked(false);
    });
    connect(ui->fileBaseNameText, &QLineEdit::returnPressed, this, [=]{
        ui->editFileNameButton->setChecked(false);
    });
}

LocalModInfoDialog::~LocalModInfoDialog()
{
    delete ui;
}

void LocalModInfoDialog::onCurrentModChanged()
{
    file_ = mod_->modFile();
    if(mod_->curseforgeMod()) ui->curseforgeButton->setEnabled(true);
    if(mod_->modrinthMod()) ui->modrinthButton->setEnabled(true);
    ui->modName->setText(mod_->alias());
    ui->aliasText->setText(mod_->alias());

    ui->versionSelect->clear();
    ui->versionSelect->addItem(mod_->modFile()->commonInfo()->version());
    for(auto file : mod_->oldFiles())
        ui->versionSelect->addItem(file->commonInfo()->version());
    //TODO: if this version was deleted
    ui->versionSelect->setCurrentText(file_->commonInfo()->version());

    onCurrentFileChanged();
}

void LocalModInfoDialog::onCurrentFileChanged()
{
    ui->rollbackButton->setVisible(file_ != mod_->modFile());
    if(mod_->alias().isEmpty()) ui->modName->setText(file_->commonInfo()->name());
    ui->aliasText->setPlaceholderText(file_->commonInfo()->name());

    auto str = ui->modName->text();
    if(file_ == mod_->modFile())
        str.append(" - " + tr("Local"));
    else
        str.append(" - " + tr("Local Old"));
    setWindowTitle(str);

    if(mod_->isDisabled())
        ui->modName->setText(ui->modName->text() + tr(" (Disabled)"));
    ui->modAuthors->setText(file_->commonInfo()->authors().join(", "));
    ui->modDescription->setText(file_->commonInfo()->description());

    ui->homepageButton->setEnabled(false);
    ui->sourceButton->setEnabled(false);
    ui->issueButton->setEnabled(false);

    if(!file_->commonInfo()->homepage().isEmpty()){
        ui->homepageButton->setEnabled(true);
        auto homepageIcon = new WebsiteIcon(this);
        connect(homepageIcon, &WebsiteIcon::iconGot, this, [=](const auto &icon){
            ui->homepageButton->setIcon(icon);
        });
        homepageIcon->get(file_->commonInfo()->homepage());
    }

    if(!file_->commonInfo()->sources().isEmpty()){
        ui->sourceButton->setEnabled(true);
        auto homepageIcon = new WebsiteIcon(this);
        connect(homepageIcon, &WebsiteIcon::iconGot, this, [=](const auto &icon){
            ui->sourceButton->setIcon(icon);
        });
        homepageIcon->get(file_->commonInfo()->sources());
    }

    if(!file_->commonInfo()->issues().isEmpty()){
        ui->issueButton->setEnabled(true);
        auto homepageIcon = new WebsiteIcon(this);
        connect(homepageIcon, &WebsiteIcon::iconGot, this, [=](const auto &icon){
            ui->issueButton->setIcon(icon);
        });
        homepageIcon->get(file_->commonInfo()->issues());
    }

    QPixmap pixelmap;
    pixelmap.loadFromData(file_->commonInfo()->iconBytes());
    ui->modIcon->setPixmap(pixelmap.scaled(80, 80, Qt::KeepAspectRatio));
    ui->modIcon->setCursor(Qt::ArrowCursor);

    //file info
    auto fileInfo = file_->fileInfo();
    auto [ baseName, suffix ] = file_->baseNameFullSuffix();
    ui->fileBaseNameText->setText(baseName);
    ui->fileSuffixText->setText(suffix);
    ui->sizeText->setText(numberConvert(fileInfo.size(), "B"));
    ui->createdTimeText->setText(fileInfo.fileTime(QFile::FileBirthTime).toString());
    ui->modifiedTimeText->setText(fileInfo.fileTime(QFile::FileModificationTime).toString());

    //if disabled
    ui->disableButton->setChecked(mod_->isDisabled());
}

void LocalModInfoDialog::on_curseforgeButton_clicked()
{
    auto curseforgeMod = mod_->curseforgeMod();
    if(!curseforgeMod->modInfo().hasBasicInfo())
        curseforgeMod->acquireBasicInfo();
    auto dialog = new CurseforgeModInfoDialog(this, curseforgeMod, mod_);
    dialog->show();
}

void LocalModInfoDialog::on_modrinthButton_clicked()
{
    auto modrinthMod = mod_->modrinthMod();
    if(!modrinthMod->modInfo().hasBasicInfo())
        modrinthMod->acquireFullInfo();
    auto dialog = new ModrinthModInfoDialog(this, modrinthMod, mod_);
    dialog->show();
}

void LocalModInfoDialog::on_homepageButton_clicked()
{
    QDesktopServices::openUrl(file_->commonInfo()->homepage());
}

void LocalModInfoDialog::on_sourceButton_clicked()
{
    QDesktopServices::openUrl(file_->commonInfo()->sources());
}

void LocalModInfoDialog::on_issueButton_clicked()
{
    QDesktopServices::openUrl(file_->commonInfo()->issues());
}

void LocalModInfoDialog::on_disableButton_toggled(bool checked)
{
    ui->disableButton->setEnabled(false);
    mod_->setEnabled(!checked);
    ui->disableButton->setEnabled(true);
}

void LocalModInfoDialog::on_editAliasButton_toggled(bool checked)
{
    ui->aliasText->setVisible(checked);
    ui->modName->setVisible(!checked);
    if(checked)
        //start edit
        ui->aliasText->setFocus();
    else{
        //finish edit
        mod_->setAlias(ui->aliasText->text());
    }
}

void LocalModInfoDialog::on_editFileNameButton_toggled(bool checked)
{
    ui->fileBaseNameText->setEnabled(checked);
    if(checked){
        //start edit
        ui->fileBaseNameText->setFocus();
    } else{
        QString baseName;
        if(isRenaming) return;
        baseName = std::get<0>(file_->baseNameFullSuffix());
        if(ui->fileBaseNameText->text() == baseName)
            return;
        isRenaming = true;
        if(!file_->rename(ui->fileBaseNameText->text())) {
            ui->fileBaseNameText->setText(baseName);
            QMessageBox::information(this, tr("Rename Failed"), tr("Rename failed!"));
        }
        isRenaming = false;
    }
}

void LocalModInfoDialog::on_versionSelect_currentIndexChanged(int index)
{
    if(index < 0 || index > mod_->oldFiles().size()) return;
    if(index == 0)
        file_ = mod_->modFile();
    else
        file_ = mod_->oldFiles().at(index - 1);
    onCurrentFileChanged();
}

void LocalModInfoDialog::on_rollbackButton_clicked()
{
    mod_->rollback(file_);
}
