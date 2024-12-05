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

#ifndef LOADER_TD0_H
#define LOADER_TD0_H

#include <QLibraryInfo>
#include <QJsonObject>
#include "loader.h"

#pragma pack(1)

typedef struct TD0_main_header_
{
    uint8_t     signature[2];
    uint8_t     volume;
    uint8_t     chunk;
    uint8_t     version;
    uint8_t     density;
    uint8_t     device_type;
    uint8_t     flags;
    uint8_t     dos;
    uint8_t     heads;
    uint16_t    crc;

}TD0_main_header;

typedef struct TD0_comment_header_
{
    uint16_t    crc;
    uint16_t    length;
    uint8_t     year, month, day, hour, min, sec;

}TD0_comment_header;

typedef struct TD0_track_header_
{
    uint8_t     data_records;
    uint8_t     track;
    uint8_t     head;
    uint8_t     crc;

}TD0_track_header;

typedef struct TD0_sector_header_
{
    uint8_t     track;
    uint8_t     head;
    uint8_t     sector;
    uint8_t     size_code;
    uint8_t     control;
    uint8_t     crc;

}TD0_sector_header;

typedef struct TD0_repeat_pattern_
{
    uint16_t     count;
    uint16_t     pattern;

}TD0_repeat_pattern;

typedef struct TD0_pattern_
{
    uint8_t     flag;
    uint8_t     count;

}TD0_pattern;

class LoaderTD0:public Loader
{
private:
    uint8_t  readSector(uint8_t * file_data, uint32_t * file_data_ptr, uint8_t * buffer, uint16_t data_len);

public:
    LoaderTD0(QJsonObject format, QString FileName);
    virtual uint8_t load(bool check_only, bool ignore_warnings, QString * log) override;
};

#endif // LOADER_TD0_H
