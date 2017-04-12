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

#ifndef LOADER_H
#define LOADER_H

#include <QLibraryInfo>
#include <QObject>
#include <QJsonObject>
#include "inttypes.h"

#include "disk_formats.h"

#define FDD_LOAD_OK                 0
#define FDD_LOAD_ERROR              1
#define FDD_LOAD_SIZE_SMALLER       2
#define FDD_LOAD_SIZE_LARGER        3
#define FDD_LOAD_PARAMS_MISMATCH    4
#define FDD_LOAD_INCORRECT_FILE     5
#define FDD_LOAD_FILE_CORRUPT       6

class Loader:public QObject
{
protected:
    uint8_t     *buffer;
    QString     file_name;
    uint32_t    image_size;
    bool        loaded;

public:
    QJsonObject  fdd_format;
    Loader(QJsonObject format, QString FileName);
    Loader(Loader * loader);
    ~Loader();

    virtual uint8_t load(bool check_only, bool ignore_warnings, QString * log);
    //virtual uint8_t check(void);
    uint8_t     *get_sector_data(uint8_t head, uint8_t track, uint8_t sector);

};

#endif // LOADER_H
