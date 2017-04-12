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

#ifndef CONVERTER_H
#define CONVERTER_H

#include <QLibraryInfo>

#include "libs/disk_formats.h"

#define FDD_CONV_OK             0
#define FDD_CONV_UNKNOWN_INPUT  10
#define FDD_CONV_READ_ERROR     11
#define FDD_CONV_UNKNOWN_OUTPUT 12
#define FDD_CONV_WRITE_ERROR    13

#define FDD_CONV_CHECK          0
#define FDD_CONV_CONVERT        1

uint8_t file_convert(QJsonObject inp_fddf, QJsonObject inp_ft, QJsonObject out_ft, QJsonArray fdd_track_formats, QJsonArray fdd_track_variants, QString mfm_track_id, QString inFile, QString outDir, uint8_t convert_mode, QString * log);

void encode_MFM_byte(uint8_t data, uint8_t clock, uint8_t * last_byte, uint8_t * buffer);
void write_mfm_array(QFile *out, uint8_t data, uint8_t clock, uint16_t count, uint8_t * last_byte);
void write_mfm_data(QFile *out, uint8_t *data, uint8_t clock, uint16_t count, uint8_t * last_byte);

uint8_t mfm_sector_size_code(uint16_t sector_size);

#endif // CONVERTER_H
