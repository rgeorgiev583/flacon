/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2013
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "in_mp3.h"

REGISTER_INPUT_FORMAT(Format_Mp3)

bool Format_Mp3::checkMagic(const QByteArray &data) const
{
    auto offset = magicOffset();
    return data.mid(offset, magicNoId3.length()) == magicNoId3 ||
           data.mid(offset, magicWithId3v1_1.length()) == magicWithId3v1_1 ||
           data.mid(offset, magicWithId3v1_2.length()) == magicWithId3v1_2 ||
           data.mid(offset, magicWithId3v2.length()) == magicWithId3v2;
}

/************************************************
 *
 ************************************************/
QStringList Format_Mp3::decoderArgs(const QString &fileName) const
{
    QStringList args;
    args << fileName;
    args << "-t";
    args << "wav";
    args << "-";

    return args;
}