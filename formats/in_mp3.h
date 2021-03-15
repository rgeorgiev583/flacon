/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2017
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

#ifndef IN_MP3_H
#define IN_MP3_H

#include "informat.h"

class Format_Mp3 : public InputFormat
{
    const QString magicNoId3 = "\xFF\xFB", magicWithId3v1_1 = "\xFF\xF3", magicWithId3v1_2 = "\xFF\xF2", magicWithId3v2 = "\x49\x44\x33";

public:
    virtual QString name() const override { return "MP3"; }
    virtual QString ext() const override { return "mp3"; }
    virtual bool    isInputFormat() const override { return true; }

    virtual QString     decoderProgramName() const override { return "sox"; }
    virtual QStringList decoderArgs(const QString &fileName) const override;

    virtual QByteArray magic() const override { return ""; }

protected:
    virtual bool checkMagic(const QByteArray &data) const override;
};

#endif // IN_MP3_H
