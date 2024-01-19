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

#ifndef WRITER_MFM_H
#define WRITER_MFM_H

#include <QFile>
#include <QJsonObject>
#include <QJsonArray>

#include "writer.h"
#include "mfm_formats.h"

class WriterMFM:public Writer
{
protected:
    QString track_type;
    QJsonArray fdd_track_formats;
    QJsonArray fdd_track_variants;
    QJsonObject find_std_track_format(QString track_type);
    QJsonObject find_std_track_variant(QString track_type, QJsonObject fdd_format);
    void write_std_track(QFile *out, QJsonObject track_format, QJsonObject track_variant, uint8_t head, uint8_t track, uint8_t sector_size_code);
    void encode_MFM_byte(uint8_t data, uint8_t clock, uint8_t * last_byte, uint8_t *buffer);
    void write_mfm_array(QFile *out, uint8_t data, uint8_t clock, uint16_t count, uint8_t * last_byte);
    void write_mfm_data(QFile *out, uint8_t * data, uint8_t clock, uint16_t count, uint8_t * last_byte);
    uint8_t mfm_sector_size_code(uint16_t sector_size);

    void write_mx_track(QFile *out, uint8_t head, uint8_t track);
    void encode_FM_byte(uint8_t data, uint8_t clock, uint8_t *buffer);
    void write_fm_array(QFile *out, uint8_t data, uint8_t clock, uint16_t count);
    void write_fm_data(QFile *out, uint8_t * data, uint8_t clock, uint16_t count);

    void write_gcr62_track(QFile *out, QJsonObject track_variant, uint8_t head, uint8_t track);

    void write_hxc_header(QFile *out, uint16_t track_size);
public:
    WriterMFM(Loader * loader, QString track_type, QJsonArray fdd_track_formats, QJsonArray fdd_track_variants);
    virtual QString get_file_ext() override;
    virtual uint8_t write(QString FileName) override;
};

QByteArray code44(const uint8_t buffer[], const int len);

void encode_gcr62(const uint8_t data_in[], uint8_t * data_out, const int len);

#endif // WRITER_MFM_H

