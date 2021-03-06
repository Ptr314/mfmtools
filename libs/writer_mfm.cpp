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

#include <QFile>
#include "writer_mfm.h"
#include "mfm_formats.h"
#include "crc16.h"
#include "config.h"

WriterMFM::WriterMFM(Loader *loader, QString track_type, QJsonArray fdd_track_formats, QJsonArray fdd_track_variants)
    :Writer(loader)
{
    this->track_type = track_type;
    this->fdd_track_formats = fdd_track_formats;
    this->fdd_track_variants =  fdd_track_variants;
}

QString WriterMFM::get_file_ext(){
    return QString(".mfm");
}

QJsonObject WriterMFM::find_std_track_format(QString track_type)
{
    foreach (const QJsonValue & value, this->fdd_track_formats) {
        QJsonObject obj = value.toObject();
        if (obj["id"].toString() == track_type) return obj;
    }
    return QJsonObject();
}

QJsonObject WriterMFM::find_std_track_variant(QString track_type, QJsonObject fdd_format){
    foreach (const QJsonValue & value, this->fdd_track_variants) {
        QJsonObject obj = value.toObject();
        if (
                (obj["track_format"].toString() == track_type) &&
                (obj["sector"].toInt() == fdd_format["sector"].toInt()) &&
                (obj["sectors"].toInt() == fdd_format["sectors"].toInt())

           ) return obj;
    }
    return QJsonObject();
}

void WriterMFM::write_hxc_header(QFile *out, uint16_t track_size)
{
    HXC_MFM_HEADER      hxc_mfm_header;
    HXC_MFM_TRACK_INFO  hxc_mfm_track_info;
    //header
    strcpy((char*)&hxc_mfm_header.headername, "HXCMFM");
    hxc_mfm_header.number_of_track = this->loader->fdd_format["tracks"].toInt();
    hxc_mfm_header.number_of_side = this->loader->fdd_format["heads"].toInt();
    hxc_mfm_header.floppyRPM = this->loader->fdd_format["rpm"].toInt();
    hxc_mfm_header.floppyBitRate = this->loader->fdd_format["dtr"].toInt();
    hxc_mfm_header.floppyiftype = 0;
    hxc_mfm_header.mfmtracklistoffset = sizeof(HXC_MFM_HEADER);
    out->write((char*)(&hxc_mfm_header), sizeof(HXC_MFM_HEADER));
    //track list
    for (uint8_t track = 0; track < this->loader->fdd_format["tracks"].toInt(); track++){
        for (uint8_t head = 0; head < this->loader->fdd_format["heads"].toInt(); head++){
            hxc_mfm_track_info.track_number = track;
            hxc_mfm_track_info.side_number = head;
            hxc_mfm_track_info.mfmtracksize = track_size;
            hxc_mfm_track_info.mfmtrackoffset = 0x800 + (track*2 + head)*track_size;
            out->write((char*)(&hxc_mfm_track_info), sizeof(HXC_MFM_TRACK_INFO));
        }
    }
    uint8_t fill = 0;
    uint16_t fill_size = 0x800 - sizeof(HXC_MFM_HEADER) - sizeof(HXC_MFM_TRACK_INFO)*this->loader->fdd_format["tracks"].toInt()*this->loader->fdd_format["heads"].toInt();
    for (uint16_t i=0; i < fill_size; i++){
        out->write((char*)(&fill), 1);
    }
}

uint8_t WriterMFM::write(QString FileName)
{
    QFile out(FileName);

    if (!out.open(QIODevice::WriteOnly)) {
        return FDD_WRITE_ERROR;
    }

    bool write_hxc_mfm_header = true;

    if (this->track_type == "TRACK_IBM_MFM" or this->track_type == "TRACK_DVK_MY") {
        uint8_t sector_size_code = mfm_sector_size_code(this->loader->fdd_format["sector"].toInt());

        QJsonObject track_format = find_std_track_format(this->track_type);
        QJsonObject track_variant = find_std_track_variant(this->track_type, this->loader->fdd_format);
        if (track_variant["gap4_size"].toInt() == 0) {
            track_variant["gap4_size"] =
                    track_format["track_len"].toInt() -
                    (
                        track_variant["gap0_size"].toInt() +
                        track_format["isync_size"].toInt() +
                        track_format["imark1_size"].toInt() +
                        track_format["imark2_size"].toInt() +
                        track_variant["gap1_size"].toInt() +
                        (
                            track_format["hsync_size"].toInt() +
                            track_format["amark1_size"].toInt() +
                            track_format["amark2_size"].toInt() +
                            6 + //track, side, sector, sector size, crc1, crc2
                            track_format["hdesync_size"].toInt() +
                            track_variant["gap2_size"].toInt() +
                            track_format["dsync_size"].toInt() +
                            track_format["dmark1_size"].toInt() +
                            track_format["dmark2_size"].toInt() +
                            track_variant["sector"].toInt() +
                            2 + //crc1, crc2
                            track_format["ddesync_size"].toInt() +
                            track_variant["gap3_size"].toInt()
                        ) * track_variant["sectors"].toInt()
                    );
        }

        if (write_hxc_mfm_header){
            write_hxc_header(&out, track_format["track_len"].toInt()*2);
        }

        for (uint8_t track = 0; track < this->loader->fdd_format["tracks"].toInt(); track++){
            for (uint8_t head = 0; head < this->loader->fdd_format["heads"].toInt(); head++){
                write_std_track(&out, track_format, track_variant, head, track, sector_size_code);
            }
        };
    } else
    if (this->track_type == "TRACK_DVK_MX_OLD" or this->track_type == "TRACK_DVK_MX_NEW") {
        if (write_hxc_mfm_header){
            write_hxc_header(&out, 12800);
        }
        for (uint8_t track = 0; track < this->loader->fdd_format["tracks"].toInt(); track++){
            for (uint8_t head = 0; head < this->loader->fdd_format["heads"].toInt(); head++){
                write_mx_track(&out, head, track);
            }
        }
    } else {
        out.close();
        return FDD_WRITE_UNKNOWN_TRACK;
    };
    out.close();
    return FDD_WRITE_OK;
}

void WriterMFM::encode_MFM_byte(uint8_t data, uint8_t clock, uint8_t * last_byte, uint8_t *buffer)
{
    uint16_t mfm_encoded;
    mfm_encoded = (uint16_t)(MFM_tab[data] & CLK_tab[clock] & ~(MFM_tab[*last_byte]<<15));
    buffer[1] = (uint8_t)(mfm_encoded & 0xFF);
    buffer[0] = (uint8_t)(mfm_encoded >> 8);

    *last_byte = data;
}

void WriterMFM::write_mfm_array(QFile *out, uint8_t data, uint8_t clock, uint16_t count, uint8_t * last_byte)
{
    uint16_t mfm_word;
    for (int i=0; i<count; i++) {
        encode_MFM_byte(data, clock, &*last_byte, (uint8_t*)(&mfm_word));
        out->write((char*)(&mfm_word), sizeof(mfm_word));
    }
}

void WriterMFM::write_mfm_data(QFile *out, uint8_t * data, uint8_t clock, uint16_t count, uint8_t * last_byte)
{
    uint16_t mfm_word;
    for (int i=0; i<count; i++) {
        encode_MFM_byte(data[i], clock, &*last_byte, (uint8_t*)(&mfm_word));
        out->write((char*)(&mfm_word), sizeof(mfm_word));
    }
}

uint8_t WriterMFM::mfm_sector_size_code(uint16_t sector_size)
{
    uint8_t i = 0;
    uint16_t s = sector_size >> 7;
    while (s > 1) {s >>= 1; i++; };
    return i;
}

void WriterMFM::write_std_track(QFile *out, QJsonObject track_format, QJsonObject track_variant, uint8_t head, uint8_t track, uint8_t sector_size_code)
{
    uint8_t last_byte = 0;
    //gap 0
    write_mfm_array(out, track_format["gap0_byte"].toInt(), 0xFF, track_variant["gap0_size"].toInt(), &last_byte);
    //index sync
    write_mfm_array(out, track_format["isync_byte"].toInt(), 0xFF, track_format["isync_size"].toInt(), &last_byte);
    //index mark 1
    write_mfm_array(out, track_format["imark1_byte"].toInt(), track_format["imark1_sync"].toInt(), track_format["imark1_size"].toInt(), &last_byte);
    //index mark 2
    write_mfm_array(out, track_format["imark2_byte"].toInt(), track_format["imark2_sync"].toInt(), track_format["imark2_size"].toInt(), &last_byte);
    //gap 1
    write_mfm_array(out, track_format["gap1_byte"].toInt(), 0xFF, track_variant["gap1_size"].toInt(), &last_byte);

    for (uint8_t sector = 1; sector <= this->loader->fdd_format["sectors"].toInt(); sector++){
        //uint32_t offset = ((track * fdd_format.heads  + head) * fdd_format.sectors + (sector-1)) * fdd_format.sector;
        //sector header sync
        write_mfm_array(out, track_format["hsync_byte"].toInt(), 0xFF, track_format["hsync_size"].toInt(), &last_byte);

        LO_HI crc;

        crc.word = track_format["crc_init"].toInt();

        //address mark 1
        write_mfm_array(out, track_format["amark1_byte"].toInt(), track_format["amark1_sync"].toInt(), track_format["amark1_size"].toInt(), &last_byte);
        CRC16_add_bytes(track_format["amark1_byte"].toInt(), track_format["amark1_size"].toInt(), &crc.word, track_format["crc_poly"].toInt());

        //address mark 2
        write_mfm_array(out, track_format["amark2_byte"].toInt(), track_format["amark2_sync"].toInt(), track_format["amark2_size"].toInt(), &last_byte);
        CRC16_add_bytes(track_format["amark2_byte"].toInt(), track_format["amark2_size"].toInt(), &crc.word, track_format["crc_poly"].toInt());

        //track number
        write_mfm_array(out, track, 0xFF, 1, &last_byte);
        CRC16_add_bytes(track, 1, &crc.word, track_format["crc_poly"].toInt());

        //head number
        write_mfm_array(out, head, 0xFF, 1, &last_byte);
        CRC16_add_bytes(head, 1, &crc.word, track_format["crc_poly"].toInt());

        //sector number
        write_mfm_array(out, sector, 0xFF, 1, &last_byte);
        CRC16_add_bytes(sector, 1, &crc.word, track_format["crc_poly"].toInt());

        //sector size
        write_mfm_array(out, sector_size_code, 0xFF, 1, &last_byte);
        CRC16_add_bytes(sector_size_code, 1, &crc.word, track_format["crc_poly"].toInt());

        //header checksum
        write_mfm_array(out, crc.byte.hi, 0xFF, 1, &last_byte);
        write_mfm_array(out, crc.byte.lo, 0xFF, 1,& last_byte);

        //gap 2
        write_mfm_array(out, track_format["gap2_byte"].toInt(), 0xFF, track_variant["gap2_size"].toInt(), &last_byte);

        //sector data sync
        write_mfm_array(out, track_format["dsync_byte"].toInt(), 0xFF, track_format["dsync_size"].toInt(), &last_byte);

        crc.word = track_format["crc_init"].toInt();

        //data mark 1
        write_mfm_array(out, track_format["dmark1_byte"].toInt(), track_format["dmark1_sync"].toInt(), track_format["dmark1_size"].toInt(), &last_byte);
        CRC16_add_bytes(track_format["dmark1_byte"].toInt(), track_format["dmark1_size"].toInt(), &crc.word, track_format["crc_poly"].toInt());

        //data mark 2
        write_mfm_array(out, track_format["dmark2_byte"].toInt(), track_format["dmark2_sync"].toInt(), track_format["dmark2_size"].toInt(), &last_byte);
        CRC16_add_bytes(track_format["dmark2_byte"].toInt(), track_format["dmark2_size"].toInt(), &crc.word, track_format["crc_poly"].toInt());

        //data
        uint8_t *data = loader->get_sector_data(head, track, sector);
        write_mfm_data(out, data, 0xFF, this->loader->fdd_format["sector"].toInt(), &last_byte);
        CRC16(data, this->loader->fdd_format["sector"].toInt(), &crc.word, track_format["crc_poly"].toInt());

        //data checksum
        write_mfm_array(out, crc.byte.hi, 0xFF, 1, &last_byte);
        write_mfm_array(out, crc.byte.lo, 0xFF, 1,& last_byte);

        //gap 3
        //if (sector <  this->loader->fdd_format.sectors)
             write_mfm_array(out, track_format["gap3_byte"].toInt(), 0xFF, track_variant["gap3_size"].toInt(), &last_byte);
    }
    //gap 4
    write_mfm_array(out, track_format["gap4_byte"].toInt(), 0xFF, track_variant["gap4_size"].toInt(), &last_byte);

}

void WriterMFM::write_mx_track(QFile *out, uint8_t head, uint8_t track)
{
    //gap 1
    if (this->track_type == "TRACK_DVK_MX_OLD") {
        write_fm_array(out, 0x00, 0xFF, 60);
    } else {
        write_fm_array(out, 0x00, 0xFF, 16);
    }
    //track number marker 00
    write_fm_array(out, 0x00, 0xFF, 1);
    write_fm_array(out, 0xF3, 0xFF, 1);
    //track number
    write_fm_array(out, 0x00, 0xFF, 1);
    write_fm_array(out, track, 0xFF, 1);
    //sectors
    for (uint8_t sector = 1; sector <= this->loader->fdd_format["sectors"].toInt(); sector++){
        LO_HI crc;
        crc.word = 0;
        //data
        uint8_t *data = loader->get_sector_data(head, track, sector);
        write_fm_data(out, data, 0xFF, this->loader->fdd_format["sector"].toInt());
        //control sum
        for (int i=0; i < this->loader->fdd_format["sector"].toInt() / 2; i++){
            crc.word = (crc.word + (uint16_t)(data[i]<<8) + (uint16_t)(data[i+1])) & 0xFFFF;
        }
        write_fm_array(out, crc.byte.hi, 0xFF, 1);
        write_fm_array(out, crc.byte.lo, 0xFF, 1);
    }
    if (this->track_type == "TRACK_DVK_MX_OLD") {
        write_fm_array(out, 0x83, 0xFF, 1);
        write_fm_array(out, 0x01, 0xFF, 1);
        write_fm_array(out, 0x83, 0xFF, 1);
        write_fm_array(out, 0x01, 0xFF, 1);
        write_fm_array(out, 0xFF, 0xFF, 294);
    } else {
        LO_HI trk_side;
        trk_side.word = (0x8300 + track*2 + head) & 0xFFFF;
        for (int i=0; i<3; i++) {
            write_fm_array(out, trk_side.byte.hi, 0xFF, 1);
            write_fm_array(out, trk_side.byte.lo, 0xFF, 1);
        }
        write_fm_array(out, 0x00, 0xFF, 336);
    }

}

void WriterMFM::encode_FM_byte(uint8_t data, uint8_t clock, uint8_t *buffer)
{
    uint32_t data_encoded = 0;
    uint8_t data_tmp = data;
    uint8_t clock_tmp = clock;
    for (int i=0; i<8; i++) {
        data_encoded |= (uint32_t)(data_tmp & 1)<<(i*4);
        data_tmp >>= 1;
        data_encoded |= (uint32_t)(clock_tmp & 1)<<(i*4+2);
        clock_tmp >>= 1;
    }
    buffer[0] = (data_encoded>>24) & 0xFF;
    buffer[1] = (data_encoded>>16) & 0xFF;
    buffer[2] = (data_encoded>>8) & 0xFF;
    buffer[3] = data_encoded & 0xFF;
}

void WriterMFM::write_fm_array(QFile *out, uint8_t data, uint8_t clock, uint16_t count)
{
    uint32_t fm_dword;
    for (int i=0; i<count; i++) {
        encode_FM_byte(data, clock, (uint8_t*)(&fm_dword));
        out->write((char*)(&fm_dword), sizeof(fm_dword));
    }
}

void WriterMFM::write_fm_data(QFile *out, uint8_t * data, uint8_t clock, uint16_t count)
{
    uint32_t fm_dword;
    for (int i=0; i<count; i++) {
        encode_FM_byte(data[i], clock, (uint8_t*)(&fm_dword));
        out->write((char*)(&fm_dword), sizeof(fm_dword));
    }
}

