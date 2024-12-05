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

#include <QtCore>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>

#include "converter.h"
#include "libs/mfm_formats.h"
#include "libs/crc16.h"
#include "libs/disk_formats.h"
#include "libs/loader_raw.h"
#include "libs/loader_td0.h"
#include "libs/writer_mfm.h"
#include "libs/writer_raw.h"

#include "libs/config.h"

uint8_t file_convert(QJsonObject inp_fddf, QJsonObject inp_ft, QJsonObject out_ft, QJsonArray fdd_track_formats, QJsonArray fdd_track_variants, QString mfm_track_id, QString inFile, QString outDir, uint8_t convert_mode, QString * log)
{
    Loader * loader;
    Writer * writer;
    uint8_t err;

    QString input_format_id = inp_ft["id"].toString();
    if (input_format_id == "FILE_RAW_MSB") {
        loader = new LoaderRAW(inp_fddf, inFile, true);
    } else
    if (input_format_id == "FILE_RAW_LSB") {
        loader = new LoaderRAW(inp_fddf, inFile, false);
    } else
    if (input_format_id == "FILE_TD0") {
        loader = new LoaderTD0(inp_fddf, inFile);
    } else {
        return FDD_CONV_UNKNOWN_INPUT;
    };

    switch (convert_mode){
        case FDD_CONV_CHECK:
        {
            return loader->load(true, false, log);
            break;
        }
        case FDD_CONV_CONVERT:
        {
            if (loader->load(false, true, log) != FDD_LOAD_OK) return FDD_CONV_READ_ERROR;

            QString output_format_id = out_ft["id"].toString();
            if (output_format_id == "FILE_MFM_HFE" || output_format_id == "FILE_MFM_NIC") {
                writer = new WriterMFM(loader, mfm_track_id, fdd_track_formats, fdd_track_variants, output_format_id);
            } else
            if (output_format_id == "FILE_RAW_MSB") {
                writer = new WriterRAW(loader);
            } else {
                delete loader;
                return FDD_CONV_UNKNOWN_OUTPUT;
            }

            QFileInfo ifi = QFileInfo(inFile);
            QString outFile = outDir + "/" + ifi.baseName() + writer->get_file_ext();

            if (writer->write(outFile) != FDD_WRITE_OK) return FDD_CONV_WRITE_ERROR;
            break;
        }
    }
    delete loader;
    return FDD_CONV_OK;
}

