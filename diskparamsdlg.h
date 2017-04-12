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

#ifndef DISKPARAMSDLG_H
#define DISKPARAMSDLG_H

#include <QDialog>
#include <QJsonObject>
#include "libs/disk_formats.h"

#define DLG_RESULT_CHANGED 100

namespace Ui {
class DiskParamsDlg;
}

class DiskParamsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit DiskParamsDlg(QWidget *parent = 0);
    ~DiskParamsDlg();

    void setValues(QJsonObject *fddf);

private slots:

    void on_sidesSpin_valueChanged(int arg1);

    void on_tracksSpin_valueChanged(int arg1);

    void on_sectorsSpin_valueChanged(int arg1);

    void on_sectorSizeCombo_activated(int index);

private:
    Ui::DiskParamsDlg *ui;

    bool valuesChanged;
    QJsonObject * fddf;
    void recalcSize(void);
    virtual void done(int r) override;
};

#endif // DISKPARAMSDLG_H
