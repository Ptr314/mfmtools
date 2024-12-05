/*
 * English:
 *
 *       This program is free software: you can redistribute it and/or modify
 *       it under the terms of the GNU General Public License as published by
 *       the Free Software Foundation, either version 3 of the License, or
 *       (at your option) any later version.
 *
 *       This program is distributed in the hope that it will be useful,
 *       but WITHOUT ANY WARRANTY; without even the implied warranty of
 *       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *       GNU General Public License for more details.
 *
 *       You should have received a copy of the GNU General Public License
 *       along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *       Author: Panther <http://www.emuverse.ru/wiki/User:Panther>
 *
 * Russian:
 *       Этот файл является свободным программным обеспечением, вы можете
 *       распространять и изменять его на условиях лицензии GNU General Public
 *       License, опубликованной Free Software Foundation, версии 3, или
 *       более поздней, на ваше усмотрение.
 *
 *       Программа распространяется с надеждой, что она окажется полезной,
 *       но БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ, в том числе подразумеваемых гарантий
 *       КОММЕРЧЕСКОЙ ЦЕННОСТИ или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ.
 *       Подробнее смотрите текст лицензии GNU General Public License.
 *
 *       Копия текста лицензии должна поставляться вместе с этим файлом,
 *       в противном случае вы можете получить ее по адресу
 *       <http://www.gnu.org/licenses/>
 *
 *       Автор: Panther <http://www.emuverse.ru/wiki/User:Panther>
 */

#include <QMessageBox>
#include <QJsonObject>
#include "diskparamsdlg.h"
#include "ui_diskparamsdlg.h"

#include "libs/disk_formats.h"

DiskParamsDlg::DiskParamsDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiskParamsDlg)
{
    ui->setupUi(this);

    valuesChanged = false;
}

DiskParamsDlg::~DiskParamsDlg()
{
    delete ui;
}

void DiskParamsDlg::recalcSize(void)
{
    uint32_t new_size = ui->sidesSpin->value() *
                        ui->tracksSpin->value() *
                        ui->sectorsSpin->value() *
                        (1<<ui->sectorSizeCombo->currentIndex())*128;

    ui->ImageSizeEdit->setText(QString::number(new_size));
}

void DiskParamsDlg::on_sidesSpin_valueChanged(int arg1)
{
    valuesChanged = true;
    recalcSize();
}

void DiskParamsDlg::on_tracksSpin_valueChanged(int arg1)
{
    valuesChanged = true;
    recalcSize();
}

void DiskParamsDlg::on_sectorsSpin_valueChanged(int arg1)
{
    valuesChanged = true;
    recalcSize();
}

void DiskParamsDlg::on_sectorSizeCombo_activated(int index)
{
    valuesChanged = true;
    recalcSize();
}

void DiskParamsDlg::setValues(QJsonObject * fddf)
{
    this->fddf = fddf;

    ui->sidesSpin->setValue(fddf->value("heads").toInt());
    ui->tracksSpin->setValue(fddf->value("tracks").toInt());
    ui->sectorsSpin->setValue(fddf->value("sectors").toInt());
    QString modulation = fddf->value("modulation").toString();
    if (modulation == "MODULATION_FM")
        ui->modulationCombo->setCurrentIndex(0);
    else
    if (modulation == "MODULATION_MFM")
        ui->modulationCombo->setCurrentIndex(1);
    else
        if (modulation == "MODULATION_GCR")
            ui->modulationCombo->setCurrentIndex(2);
    else
        QMessageBox::critical(0, qApp->translate("Main", "Error"), qApp->translate("Main", "Unknown modulation type"));
    switch (fddf->value("rpm").toInt()) {
        case 300: ui->rpmCombo->setCurrentIndex(0);
    };
    switch (fddf->value("dtr").toInt()) {
        case 250: ui->dtrCombo->setCurrentIndex(0);
    }

    ui->sectorSizeCombo->setCurrentIndex(sector_size_to_code(fddf->value("sector").toInt()));
    recalcSize();
    valuesChanged = false;
}

void DiskParamsDlg::done(int r)
{
    if (r == QDialog::Accepted) {
        if (valuesChanged) {
            int res = QMessageBox::warning(0,
                                     qApp->translate("DiskParamsDlg", "Warning"),
                                     qApp->translate("DiskParamsDlg", "Disk parameters are changed. Continue?"),
                                     QMessageBox::Yes | QMessageBox::No
                               );
            if (res == QMessageBox::Yes) {
                (*this->fddf)["heads"]   = ui->sidesSpin->value();
                (*this->fddf)["tracks"]  = ui->tracksSpin->value();
                (*this->fddf)["sectors"] = ui->sectorsSpin->value();
                (*this->fddf)["sector"]   = code_to_sector_size(ui->sectorSizeCombo->currentIndex());
                switch (ui->modulationCombo->currentIndex()) {
                    case 0: (*this->fddf)["modulation"] = "MODULATION_FM";
                    case 1: (*this->fddf)["modulation"] = "MODULATION_MFM";
                }
                switch (ui->rpmCombo->currentIndex()) {
                    case 0: (*this->fddf)["rpm"] = 300;
                }
                switch (ui->dtrCombo->currentIndex()) {
                    case 0: (*this->fddf)["dtr"] = 250;
                }

                QDialog::done(DLG_RESULT_CHANGED);
            };
        } else QDialog::done(r);
    } else QDialog::done(r);
}
