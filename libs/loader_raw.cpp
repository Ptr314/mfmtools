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

#include <QApplication>
#include <QLibraryInfo>
#include <QFileInfo>
#include "loader_raw.h"

uint8_t var_sides[]   = {1, 2};
uint8_t var_tracks[]  = {40, 41, 42, 43, 80, 81, 82, 83};
uint8_t var_sectors[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
uint16_t var_sector[] = {128, 256, 512, 1024, 2048, 4096, 8192};
uint16_t mfm_max_bytes = 9216;

LoaderRAW::LoaderRAW(QJsonObject format, QString FileName, bool msb_first)
    :Loader(format, FileName)
{
    this->msb_first = msb_first;
}

uint8_t LoaderRAW::load(bool check_only, bool ignore_warnings, QString *log)
{
    uint8_t res = FDD_LOAD_OK;

    Loader::load(check_only, ignore_warnings, log);

    QFile file(this->file_name);

    if (!file.open(QIODevice::ReadOnly)) {
        log->append(qApp->translate("RAW", "File open error")+"\r\n");
        return FDD_LOAD_ERROR;
    } else {
        log->append(QString(qApp->translate("RAW", "Actual file size: %1")+"\r\n").arg(QString::number(file.size())));
        log->append(QString(qApp->translate("RAW", "Expected file size: %1")+"\r\n").arg(QString::number(this->image_size)));
        if (file.size() < this->image_size) {
            log->append(qApp->translate("RAW", "Warning: File size is too small")+"\r\n");
            res = FDD_LOAD_SIZE_SMALLER;
        } else
        if (file.size() > this->image_size) {
            log->append(qApp->translate("RAW", "Warning: File size is too large")+"\r\n");
            res = FDD_LOAD_SIZE_LARGER;
        } else res = FDD_LOAD_OK;
    }
    if (file.size() != this->image_size) {
        log->append(qApp->translate("RAW", "Trying to calculate variants")+"\r\n");
        int found = 0;
        for (uint8_t l = 0; l < sizeof(var_sector)/sizeof(var_sector[0]); l++)
            for (uint8_t i = 0; i < sizeof(var_sides)/sizeof(var_sides[0]); i++)
                for (uint8_t j = 0; j < sizeof(var_tracks)/sizeof(var_tracks[0]); j++)
                    for (uint8_t k = 0; k < sizeof(var_sectors)/sizeof(var_sectors[0]); k++) {
                        uint32_t projected_size = (uint32_t)var_sides[i] * var_tracks[j] * var_sectors[k] * var_sector[l];
                        if ((projected_size == file.size()) && (var_sectors[k] * var_sector[l] <= mfm_max_bytes)) {
                            found++;
                            log->append(QString(qApp->translate("RAW", "#%1: SIDES: %2; TRACKS: %3; SECTORS: %4; SECTOR SIZE: %5")+"\r\n").arg(QString::number(found)).arg(QString::number(var_sides[i])).arg(QString::number(var_tracks[j])).arg(QString::number(var_sectors[k])).arg(QString::number(var_sector[l])));
                        }
                    }
        if (found == 0) {
            log->append(qApp->translate("RAW", "No possible variants found")+"\r\n");
        } else {
            log->append(QString(qApp->translate("RAW", "Total variants found: %1")+"\r\n").arg(QString::number(found)));
        }
    }

    if (res==FDD_LOAD_OK) {
        log->append(qApp->translate("RAW", "No problems found")+"\r\n");
    }
    if (res != FDD_LOAD_OK && !ignore_warnings) {
        file.close();
        return res;
    }

    if (!check_only) {
        this->buffer = new uint8_t[this->image_size];
        file.read((char*)this->buffer, this->image_size);
        file.close();

        //swap byte order for LSB-first files
        if (!this->msb_first) {
            for (uint16_t i=0; i < this->image_size/2; i++) {
                uint8_t tmp = this->buffer[i*2];
                this->buffer[i] = this->buffer[i*2+1];
                this->buffer[i*2+1] = tmp;
            }
        }

        this->loaded = true;
    }

    return FDD_LOAD_OK;
}

