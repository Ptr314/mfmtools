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

const uint8_t m_write_translate_table[64] =
    {
        0x96,0x97,0x9A,0x9B,0x9D,0x9E,0x9F,0xA6,
        0xA7,0xAB,0xAC,0xAD,0xAE,0xAF,0xB2,0xB3,
        0xB4,0xB5,0xB6,0xB7,0xB9,0xBA,0xBB,0xBC,
        0xBD,0xBE,0xBF,0xCB,0xCD,0xCE,0xCF,0xD3,
        0xD6,0xD7,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,
        0xDF,0xE5,0xE6,0xE7,0xE9,0xEA,0xEB,0xEC,
        0xED,0xEE,0xEF,0xF2,0xF3,0xF4,0xF5,0xF6,
        0xF7,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
};

static const unsigned char FlipBit1[4] = { 0, 2,  1,  3  };
static const unsigned char FlipBit2[4] = { 0, 8,  4,  12 };
static const unsigned char FlipBit3[4] = { 0, 32, 16, 48 };

static const uint8_t agat_sector_translate[]={
    0x00,0x0D,0x0B,0x09,0x07,0x05,0x03,0x01,0x0E,0x0C,0x0A,0x08,0x06,0x04,0x02,0x0F
};

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
    int track_offset_mult = (hxc_mfm_header.number_of_side==2)?2:1;
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
            hxc_mfm_track_info.mfmtrackoffset = 0x800 + (track*track_offset_mult + head)*track_size;
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
    } else
    if (this->track_type == "TRACK_AGAT_140") {
        QJsonObject track_variant = find_std_track_variant(track_type, loader->fdd_format);

        if (track_variant["sector"].toInt() != 256)
            return FDD_WRITE_UNSUPPORTED_FORMAT;

        int track_len = 6400;

        if (track_variant["gap3_size"].toInt() == 0) {
            track_variant["gap3_size"] = track_len - (
                        track_variant["gap0_size"].toInt()            // GAP 0
                        + track_variant["sectors"].toInt() * (
                              3 +                                       // Address prologue
                              8 +                                       // Address
                              3 +                                       // Address epilogue
                              track_variant["gap1_size"].toInt() +      // GAP 1
                              3 +                                       // Data prologue
                              343 +                                     // Data
                              3 +                                       // Data epilogue
                              track_variant["gap2_size"].toInt()        // GAP 2
                          )
                        );
        }
        if (write_hxc_mfm_header){
            write_hxc_header(&out, track_len);
        }
        for (uint8_t track = 0; track < loader->fdd_format["tracks"].toInt(); track++){
            write_gcr62_track(&out, track_variant, 0, track);
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

QByteArray code44(const uint8_t buffer[], const int len)
{
    QByteArray result;
    for (int i=0; i<len; i++) {
        //result.append(static_cast<char>( ((buffer[i] >> 1) & 0x55) | 0xaa));
        //result.append(static_cast<char>( ( buffer[i]       & 0x55) | 0xaa));
        result.append(static_cast<char>( (buffer[i] >> 1) | 0xaa));
        result.append(static_cast<char>(  buffer[i]       | 0xaa));
    }

    return result;
}

void WriterMFM::write_gcr62_track(QFile *out, QJsonObject track_variant, uint8_t head, uint8_t track)
{
    //track_variant["gap3_size"].toInt()
    char gap_bytes[256];
    memset(&gap_bytes, 0xFF, sizeof(gap_bytes));

    //int encoded_size = 342+2; //ceil(track_variant["sector"].toDouble() * 4 / 3)+2;
    //uint8_t * encoded_sector = new uint8_t[encoded_size];
    uint8_t encoded_sector[344];

    // GAP 0
    out->write(gap_bytes, track_variant["gap0_size"].toInt());

    // Agat counts sectors from 0?
    for (uint8_t sector = 0; sector < track_variant["sectors"].toInt(); sector++) {
        // Prologue
        out->write(QByteArray("\xD5\xAA\x96"));
        // Address
        uint8_t volume = 0xFE;
        uint8_t sector_t = agat_sector_translate[sector];
        uint8_t address_field[4] = {volume, track, sector_t, static_cast<uint8_t>(volume ^ track ^ sector_t)};
        out->write(code44(address_field, 4));
        // Epilogue
        out->write(QByteArray("\xDE\xAA\xEB"));
        // GAP 1
        out->write(gap_bytes, track_variant["gap1_size"].toInt());
        // Data field
        // Prologue
        out->write(QByteArray("\xD5\xAA\xAD"));
        // Data + CRC
        uint8_t * data = loader->get_sector_data(head, track, sector+1);        // Other code counts sectors from 1
        encode_gcr62(data, encoded_sector, track_variant["sector"].toInt());
        out->write(reinterpret_cast<char*>(encoded_sector), 343);
        // Epilogue
        out->write(QByteArray("\xDE\xAA\xEB"));
        // GAP 2
        out->write(gap_bytes, track_variant["gap2_size"].toInt());
    }
    // GAP 3
    out->write(gap_bytes, track_variant["gap3_size"].toInt());

    //delete[] encoded_sector;
}

// void encode_gcr62(const uint8_t data_in[], uint8_t * data_out, const int len)
// {
//     // The algorytm is taken from https://github.com/allender/apple2emu/blob/master/src/disk_image.cpp
//     memcpy(&data_out[0x56], data_in, len);
//     data_out[342]=0;                        //Extra byte should be zero

//     uint8_t offset = 0x0;
//     while (offset < 0x56) {
//         uint8_t val = (((data_in[offset + 0xac] & 0x1) << 1) | ((data_in[offset + 0xac] & 0x2) >> 1)) << 6;
//         val = val | ((((data_in[offset + 0x56] & 0x1) << 1) | ((data_in[offset + 0x56] & 0x2) >> 1)) << 4);
//         val = val | (((data_in[offset] & 0x1) << 1) | ((data_in[offset] & 0x2) >> 1)) << 2;
//         data_out[offset++] = val;
//     }
//     data_out[offset-1] &= 0x3f;
//     data_out[offset-2] &= 0x3f;
//     uint8_t xor_value = 0;
//     for (auto i = 0; i <= 343; i++) {
//         auto prev_val = data_out[i];
//         data_out[i] = data_out[i] ^ xor_value;
//         xor_value = prev_val;
//     }
//     for (auto i = 0; i <= 342; i++) {
//         data_out[i] = m_write_translate_table[data_out[i] >> 2];
//     }
// }

void encode_gcr62(const uint8_t data_in[], uint8_t * data_out, const int len)
{

    memset(data_out, 0, 0x157);
    int ecx;
    uint8_t al;
    uint8_t cy;

    uint8_t bl = 2;                                     // mov ebx, 2h
    do {                                                // @l2:
        ecx = 0x55;                                     // mov ecx, 55h
        do {                                            // @l1
            bl--;                                       // dec bl
            al = data_in[bl];                           // mov al, [esi+ebx]
            cy = al & 0x01; al >>= 1;                   // shr al, 1
            data_out[ecx] = (data_out[ecx] << 1) | cy;  // rcl byte ptr [edi+ecx], 1
            cy = al & 0x01; al >>= 1;                   // shr al, 1
            data_out[ecx] = (data_out[ecx] << 1) | cy;  // rcl byte ptr [edi+ecx], 1
            data_out[0x56 + bl] = al;                   // mov byte ptr [edi+56h+ebx], al
            ecx--;                                      // dec ecx
        } while (ecx >= 0);                             // jns l1
    } while(bl != 0);                                   // or ebx, ebx; jne	l2

    al = 0;                                             // xor al, al
    ecx = 0;                                            // xor ecx, ecx
    bl = 0;                                             // xor ebx, ebx
    uint8_t ah;
    do {                                                // @l4:
        ah = data_out[ecx];                             // mov ah, [edi+ecx]
        bl = ah;                                        // mov bl, ah
        bl ^= al;                                       // xor bl, al
        al = ah;                                        // mov al, ah
        bl = m_write_translate_table[bl];               // mov bl, CodeTabl[ebx]
        data_out[ecx] = bl;                             // mov [edi+ecx], bl
        ecx++;                                          // inc ecx
    } while (ecx != 0x156);                             // cmp ecx, 156h; jne l4
    bl = al;                                            // mov bl, al
    bl = m_write_translate_table[bl];                   // mov bl, CodeTabl[ebx]
    data_out[ecx] = bl;                                 // mov [edi+ecx], bl
}



// void encode_gcr62_(const uint8_t data_in[], uint8_t * data_out, const int len)
// {
//     // https://tulip-house.ddo.jp/digital/SDISK2/english.html (dsk2nic.cpp)
//     uint8_t src[256+2];
//     memcpy(src, data_in, 256);
//     src[256] = src[257] = 0;

//     uint8_t vx = 0;

//     uint8_t v;
//     for (int i = 0; i < 86; i++) {
//         v = (FlipBit1[src[i]&3] | FlipBit2[src[i+86]&3] | FlipBit3[src[i+172]&3]);
//         *data_out++ = m_write_translate_table[(v ^ vx) & 0x3F];
//         vx = v;
//     }
//     for (int i = 0; i < 256; i++) {
//         v = src[i] >> 2;
//         *data_out++ = m_write_translate_table[(v ^ vx) & 0x3F];
//         vx = v;
//     }
//     *data_out++ = m_write_translate_table[vx & 0x3F];
// }
