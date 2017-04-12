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

#include "loader.h"

Loader::Loader(QJsonObject format, QString FileName)
{
    this->fdd_format = format;
    this->file_name = FileName;
    this->image_size = format["heads"].toInt() * format["tracks"].toInt() * format["sectors"].toInt() * format["sector"].toInt();
    this->loaded = false;
}

Loader::~Loader()
{
    delete[] this->buffer;
}

uint8_t Loader::load(bool check_only, bool ignore_warnings, QString *log)
{
    return FDD_LOAD_ERROR;
}

uint8_t * Loader::get_sector_data(uint8_t head, uint8_t track, uint8_t sector){
    uint32_t offset = ((track * this->fdd_format["heads"].toInt()  + head) * this->fdd_format["sectors"].toInt() + (sector-1)) * this->fdd_format["sector"].toInt();
    return &this->buffer[offset];
}
