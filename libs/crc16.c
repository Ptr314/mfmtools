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

#include "CRC16.h"

void CRC16(uint8_t * buffer, uint16_t count, uint16_t * crc, uint16_t polynome)
{

    for (int i = 0; i < count; i++)
    {
        *crc ^= (uint16_t)(buffer[i] << 8); /* move byte into MSB of 16bit CRC */

        for (int j = 0; j < 8; j++) {
            if ((*crc & 0x8000) != 0) { /* test for MSB = bit 15 */
                *crc = (uint16_t)((*crc << 1) ^ polynome);
            } else {
                *crc <<= 1;
            }
        }
    }
}

void CRC16_add_bytes(uint8_t data, uint8_t count, uint16_t * crc, uint16_t polynome)
{
    for (int i = 0; i < count; i++) CRC16(&data, 1, crc, polynome);
}
