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

#include <QLibraryInfo>
#include <QFileInfo>
#include <QDebug>
#include <QTextCodec>
#include <QCoreApplication>

#include "loader_td0.h"
#include "crc16.h"
#include "lzss.h"

LoaderTD0::LoaderTD0(QJsonObject format, QString FileName)
    :Loader(format, FileName)
{

}

uint8_t LoaderTD0::load(bool check_only, bool ignore_warnings, QString *log)
{
    Loader::load(check_only, ignore_warnings, log);

    QFile ffile(this->file_name);
    if (!ffile.open(QIODevice::ReadOnly)) {
        return FDD_LOAD_ERROR;
    }
    uint8_t * file_data = (uint8_t*)malloc(ffile.size()+512);
    uint32_t file_data_size = ffile.read((char*)file_data, ffile.size());
    ffile.close();
    if ((ffile.size() > 0) && (file_data_size!=ffile.size())) {
        return FDD_LOAD_ERROR;
    }

    QJsonObject detected_format;
    //memset(&detected_format, 0, sizeof(detected_format));

    TD0_main_header main_header;
    TD0_comment_header comment_header;
    TD0_track_header track_header;
    TD0_sector_header sector_header;
    int file_type;
    bool Terminate = false;
    uint32_t file_data_ptr = 0;

    if (!check_only) {
        this->buffer = new uint8_t[this->image_size];
        memset(this->buffer, 0, this->image_size);
    }

    //Header
    memcpy(&main_header, &file_data[file_data_ptr], sizeof(main_header));
    file_data_ptr+=sizeof(main_header);
    if (main_header.signature[0] == 0x54 && main_header.signature[1] == 0x44) file_type = 1; //TD
    else
    if (main_header.signature[0] == 0x74 && main_header.signature[1] == 0x64) file_type = 2; //td
    else file_type = 0;

    if (file_type) {
        log->append(tr(">>>HEADER<<<\r\n"));
        if (file_type == 1) {
            log->append(tr("Signature: \"TD\" (RLE compression)\r\n"));
        } else {
            log->append(tr("Signature: \"td\" (RLE+LZSS compression)\r\n"));
        }
        log->append(QString(tr("Volume:\t%1\r\n")).arg(QString::number(main_header.volume)));
        log->append(QString(tr("Chunk:\t%1\r\n")).arg(QString::number(main_header.chunk)));
        log->append(QString(tr("Version:\t%1\r\n")).arg(QString::number(main_header.version)));
        log->append(QString(tr("Density:\t%1\r\n")).arg(QString::number(main_header.density)));
        log->append(QString(tr("Type:\t%1\r\n")).arg(QString::number(main_header.device_type)));
        log->append(QString(tr("Flags:\t$%1\r\n")).arg(QString::number(main_header.flags, 16)));
        if (main_header.flags and 0x80)
            log->append(QString(tr("\tComment presents ($80)\r\n")));
        log->append(QString(tr("DOS:\t%1\r\n")).arg(QString::number(main_header.dos)));
        log->append(QString(tr("Heads:\t%1\r\n")).arg(QString::number(main_header.heads)));
        log->append(QString(tr("CRC16:\t$%1 ")).arg(QString::number(main_header.crc, 16).rightJustified(4, '0').toUpper()));

        detected_format["heads"] = main_header.heads;

        uint16_t header_crc = 0;
        CRC16((uint8_t*)&main_header, sizeof(main_header)-sizeof(main_header.crc), &header_crc, 0xA097);
        if (main_header.crc == header_crc)
            log->append(QString(tr("(OK)\r\n")));
        else {
            log->append(QString(tr("ERROR! Expected $%1 ")).arg(QString::number(header_crc, 16).rightJustified(4, '0').toUpper()));
            return FDD_LOAD_FILE_CORRUPT;
        }
        if (file_type == 2) {
            log->append(tr("\r\nPerforming LZSS decompression\r\n"));
            file_data = lzss_unpack(file_data, file_data_size, sizeof(main_header), &file_data_size);
            file_data_ptr = sizeof(main_header);
            log->append(QString(tr("Decompressed file size: %1 bytes\r\n")).arg(QString::number(file_data_size)));
        }
        //Comment
        if (main_header.flags and 0x80) {
            memcpy(&comment_header, &file_data[file_data_ptr], sizeof(comment_header));
            file_data_ptr+=sizeof(comment_header);
            log->append(tr("\r\n>>>COMMENT<<<\r\n"));
            log->append(QString(tr("Length:\t%1\r\n")).arg(QString::number(comment_header.length)));
            log->append(QString(tr("Date:\t%1-%2-%3 %4:%5:%6\r\n")).arg(QString::number(comment_header.year+1900)).arg(QString::number(comment_header.month+1).rightJustified(2, '0')).arg(QString::number(comment_header.day).rightJustified(2, '0')).arg(QString::number(comment_header.hour)).arg(QString::number(comment_header.min)).arg(QString::number(comment_header.sec)));
            uint8_t * comment = new uint8_t[comment_header.length];
            memcpy(comment, &file_data[file_data_ptr], comment_header.length);
            file_data_ptr+=comment_header.length;
            log->append(tr("Comment body:\r\n"));

            QTextCodec *codec = QTextCodec::codecForName("IBM 866");
            log->append(tr("\t"));
            log->append(codec->toUnicode(QByteArray((char*)comment, comment_header.length).replace(QChar::Null, QByteArray("\r\n\t"))).trimmed());
            log->append(tr("\r\n"));
            log->append(QString(tr("CRC16:\t$%1 ")).arg(QString::number(comment_header.crc, 16).rightJustified(4, '0').toUpper()));
            uint16_t comment_crc = 0;
            CRC16((uint8_t*)&comment_header.length, sizeof(comment_header)-sizeof(comment_header.crc), &comment_crc, 0xA097);
            CRC16((uint8_t*)comment, comment_header.length, &comment_crc, 0xA097);
            if (comment_header.crc == comment_crc)
                log->append(QString(tr("(OK)\r\n")));
            else {
                log->append(QString(tr("ERROR! Expected $%1 ")).arg(QString::number(comment_crc, 16).rightJustified(4, '0').toUpper()));
                return FDD_LOAD_FILE_CORRUPT;
            }
        }
        //Tracks
        log->append(tr("\r\n>>>TRACKS<<<\r\n"));
        //while (!file.atEnd()) {
        while ((file_data_ptr < file_data_size) && !Terminate) {
            uint8_t sectors_per_track = 0;
            log->append(QString(tr("$%1: ")).arg(QString::number(file_data_ptr, 16).rightJustified(6, '0').toUpper()));
            memcpy(&track_header, &file_data[file_data_ptr], sizeof(track_header));
            file_data_ptr+=sizeof(track_header);
            LO_HI track_header_crc;
            track_header_crc.word = 0;
            CRC16((uint8_t*)&track_header, sizeof(track_header)-sizeof(track_header.crc), &track_header_crc.word, 0xA097);
            log->append(QString(tr("TRACK: %1; ")).arg(QString::number(track_header.track)));
            log->append(QString(tr("HEAD: %1; ")).arg(QString::number(track_header.head)));
            log->append(QString(tr("SECTOR RECORDS: %1; ")).arg(QString::number(track_header.data_records)));
            if (track_header.data_records == 0xFF) {
                log->append(tr("\r\nSECTOR RECORDS == $FF, finishing\r\n"));
                Terminate = true;
            }
            if (!Terminate) {
                log->append(QString(tr("CRC: $%1 ")).arg(QString::number(track_header.crc, 16).rightJustified(2, '0').toUpper()));
                if (track_header.crc == track_header_crc.byte.lo)
                    log->append(tr("(OK)\r\n"));
                else {
                    log->append(QString(tr("ERROR! Expected $%1\r\n")).arg(QString::number(track_header_crc.byte.lo, 16).rightJustified(2, '0').toUpper()));
                    return FDD_LOAD_FILE_CORRUPT;
                }
                for (int i=0; i < track_header.data_records; i++) {
                    if (file_data_ptr >= file_data_size) {
                        log->append(tr("ERROR! Unexpected end of file\r\n"));
                        return FDD_LOAD_FILE_CORRUPT;
                    }
                    log->append(QString(tr("  $%1: ")).arg(QString::number(file_data_ptr, 16).rightJustified(6, '0').toUpper()));
                    memcpy(&sector_header, &file_data[file_data_ptr], sizeof(sector_header));
                    file_data_ptr+=sizeof(sector_header);
                    LO_HI sector_header_crc;
                    sector_header_crc.word = 0;
                    CRC16((uint8_t*)&sector_header, sizeof(sector_header)-sizeof(sector_header.crc), &sector_header_crc.word, 0xA097);
                    log->append(QString(tr("SECTOR: %1; ")).arg(QString::number(sector_header.sector)));
                    log->append(QString(tr("HEAD: %1; ")).arg(QString::number(sector_header.head)));
                    log->append(QString(tr("TRACK: %1; ")).arg(QString::number(sector_header.track)));
                    log->append(QString(tr("SIZE: %1; ")).arg(QString::number(code_to_sector_size(sector_header.size_code))));
                    log->append(QString(tr("FLAGS: $%1; ")).arg(QString::number(sector_header.control, 16).rightJustified(2, '0').toUpper()));
                    //log->append(QString(tr("CRC: $%1 ")).arg(QString::number(sector_header.crc, 16).rightJustified(2, '0').toUpper()));

                    if (sector_header.sector == 0x65) {
                        log->append(tr("\r\nSECTOR# == $65, finishing\r\n"));
                        Terminate = true;
                        break;
                    } else {
                        if (!(sector_header.control & 0x30) && !(sector_header.size_code & 0xF8)) {
                            if (sector_header.control == 0x10) {
                                //Sector is empty
                                log->append(tr("TYPE: Empty; "));
                                //Fill data with zeroes!
                                //if (!check_only) memset(&(this->buffer[p], 0, code_to_sector_size(sector_header.size_code));
                                sectors_per_track++;
                            } else {
                                uint16_t data_len;
                                memcpy(&data_len, &file_data[file_data_ptr], sizeof(data_len));
                                file_data_ptr+=sizeof(data_len);
                                log->append(QString(tr("DATALEN: %1; ")).arg(QString::number(data_len)));
                                if (sector_header.sector < 100) {
                                    log->append(tr("TYPE: Normal; "));
                                    //Here we read data
                                    if (!check_only) {
                                        uint32_t p = ((sector_header.track * main_header.heads + sector_header.head)*this->fdd_format["sectors"].toInt() + sector_header.sector-1)*code_to_sector_size(sector_header.size_code);
                                        uint8_t res= readSector((uint8_t*)file_data, &file_data_ptr, &this->buffer[p], data_len);
                                        LO_HI data_crc;
                                        data_crc.word = 0;
                                        CRC16((uint8_t*)&this->buffer[p], code_to_sector_size(sector_header.size_code), &data_crc.word, 0xA097);
                                        if (sector_header.crc == data_crc.byte.lo)
                                            log->append(tr("CRC OK"));
                                        else
                                            log->append(tr("CRC ERROR"));
                                        if ((res != FDD_LOAD_OK) ||(sector_header.crc != data_crc.byte.lo) ) {
                                            log->append(tr("ERROR! Sector data is corrupt\r\n"));
                                            return res;
                                        }
                                    } else {
                                        file_data_ptr += data_len;
                                    }
                                    sectors_per_track++;
                                } else {
                                    log->append(tr("TYPE: Ghost; "));
                                    //Here we have to skip data?
                                    file_data_ptr += data_len;
                                }
                                detected_format["sector"] = code_to_sector_size(sector_header.size_code);
                                if (sector_header.track+1 > detected_format["tracks"].toInt()) detected_format["tracks"] = sector_header.track+1;
                            }
                        }
                    }
                    log->append(tr("\r\n"));
                }//For
                if (sectors_per_track > detected_format["sectors"].toInt()) detected_format["sectors"] = sectors_per_track;
            } //Terminate
        } //While not EOF
        log->append(tr("\r\n>>>DETECTED DISK PARAMETERS<<<\r\n"));
        log->append(QString(tr("Heads:\t%1\r\n")).arg(QString::number(detected_format["heads"].toInt())));
        log->append(QString(tr("Tracks:\t%1\r\n")).arg(QString::number(detected_format["tracks"].toInt())));
        log->append(QString(tr("Sectors:\t%1\r\n")).arg(QString::number(detected_format["sectors"].toInt())));
        log->append(QString(tr("Sector size: %1\r\n")).arg(QString::number(detected_format["sector"].toInt())));
        log->append(QString(tr("Image size: %1 bytes\r\n")).arg(QString::number((uint32_t)detected_format["sector"].toInt() * detected_format["sectors"].toInt() * detected_format["tracks"].toInt() * detected_format["heads"].toInt())));

        if ((detected_format["heads"].toInt() != this->fdd_format["heads"].toInt()) || (detected_format["sector"].toInt() != this->fdd_format["sector"].toInt()) || (detected_format["sectors"].toInt() != this->fdd_format["sectors"].toInt()) || (detected_format["tracks"].toInt() != this->fdd_format["tracks"].toInt()) ) {
            return FDD_LOAD_PARAMS_MISMATCH;
        }
        if (!check_only) this->loaded = true;
        return FDD_LOAD_OK;
    } else {
        return FDD_LOAD_INCORRECT_FILE;
    }
}
uint8_t LoaderTD0::readSector(uint8_t * file_data, uint32_t * file_data_ptr, uint8_t * buffer, uint16_t data_len)
{
    uint8_t block_type;

    //file->read((char*)&block_type, 1);
    memcpy(&block_type, &file_data[*file_data_ptr], 1);
    *file_data_ptr += 1;

    switch (block_type) {
    case 0:
    {
        //file->read((char*)buffer, data_len-1);
        memcpy(buffer, &file_data[*file_data_ptr], data_len-1);
        *file_data_ptr += data_len-1;
        break;
    }
    case 1:
    {
        TD0_repeat_pattern repeat_pattern;
        //file->read((char*)&repeat_pattern, sizeof(repeat_pattern));
        memcpy(&repeat_pattern, &file_data[*file_data_ptr], sizeof(repeat_pattern));
        *file_data_ptr += sizeof(repeat_pattern);
        for (int i=0; i < repeat_pattern.count; i++)
            memcpy(&buffer[i*2], &repeat_pattern.pattern, 2);
        break;
    }
    case 2:
    {
        int processed = 0;
        int p = 0;
        TD0_pattern pattern;
        while (processed < data_len-1) {
            //file->read((char*)&pattern, sizeof(pattern));
            memcpy(&pattern, &file_data[*file_data_ptr], sizeof(pattern));
            *file_data_ptr += sizeof(pattern);
            processed += sizeof(pattern);
            if (pattern.flag == 0) {
                //file->read((char*)&buffer[p], pattern.count);
                memcpy(&buffer[p], &file_data[*file_data_ptr], pattern.count);
                *file_data_ptr += pattern.count;
                processed += pattern.count;
                p += pattern.count;
            } else
            if (pattern.flag < 5) {
                uint8_t tmp[1024];
                uint16_t pattern_size = 1 << pattern.flag;
                //file->read((char*)&tmp, pattern_size);
                memcpy(&tmp, &file_data[*file_data_ptr], pattern_size);
                *file_data_ptr += pattern_size;

                for (int i=0; i < pattern.count; i++) {
                    memcpy(&buffer[p], &tmp, pattern_size);
                    p += pattern_size;
                }
                processed += pattern_size;

            } else return FDD_LOAD_FILE_CORRUPT;
        }
        break;
    }
    default:
    {
        return FDD_LOAD_FILE_CORRUPT;
        break;
    }
    }
    return FDD_LOAD_OK;
}

/*
 * Think about this:
            if(td_sector_header->Syndrome & 0x04)
            {
                sectorconfig[i].use_alternate_datamark=1;
                sectorconfig[i].alternate_datamark=0xF8;
            }

            if(td_sector_header->Syndrome & 0x02)
            {
                sectorconfig[i].use_alternate_data_crc=2;
            }

            if(td_sector_header->Syndrome & 0x20)
            {
                sectorconfig[i].missingdataaddressmark=1;
}
*/
