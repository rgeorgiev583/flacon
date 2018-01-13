/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2017
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


#include "testflacon.h"
#include "tools.h"
#include "../formats/format.h"
#include "../inputaudiofile.h"

#include <QTest>
#include <QString>
#include <QBuffer>
#include <QDebug>


void TestFlacon::testInputAudioFile()
{
    QFETCH(QString, fileName);
    QFETCH(int,     bitsPersample);
    QFETCH(int,     sampleRate);
    QFETCH(int,     channels);
    QFETCH(int,     duration);
    QFETCH(QString, formatId);


    InputAudioFile ia(fileName);
    if (!ia.isValid())
    {
        FAIL(QString("Can't open '%1': %2").arg(fileName, ia.errorString()));
        return;
    }

    QCOMPARE(ia.quality().sampleRate(),    uint(sampleRate));
    QCOMPARE(ia.quality().bitsPerSample(), uint(bitsPersample));
    QCOMPARE(ia.quality().numChannels(),   quint16(channels));
    QCOMPARE(ia.duration(),                uint(duration));
    QCOMPARE(ia.format()->name(), formatId);
}


void TestFlacon::testInputAudioFile_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<int>("bitsPersample");
    QTest::addColumn<int>("sampleRate");
    QTest::addColumn<int>("channels");
    QTest::addColumn<int>("duration");
    QTest::addColumn<QString>("formatId");

    //                                     File             bits    rate        chanels duration    format
    QTest::newRow("01 mAudio_cd_ape")   << mAudio_cd_ape    << 16   << 44100    << 2    << 900000 << "APE";
    QTest::newRow("02 mAudio_cd_flac")  << mAudio_cd_flac   << 16   << 44100    << 2    << 900000 << "FLAC";
    QTest::newRow("03 mAudio_cd_tta")   << mAudio_cd_tta    << 16   << 44100    << 2    << 900000 << "TTA";
    QTest::newRow("04 mAudio_cd_wav")   << mAudio_cd_wav    << 16   << 44100    << 2    << 900000 << "WAV";
    QTest::newRow("05 mAudio_cd_wv")    << mAudio_cd_wv     << 16   << 44100    << 2    << 900000 << "WavPack";

    QTest::newRow("06 mAudio_24x96_ape") << mAudio_24x96_ape    << 24   << 96000    << 2    << 900000 << "APE";
    QTest::newRow("07 mAudio_24x96_flac")<< mAudio_24x96_flac   << 24   << 96000    << 2    << 900000 << "FLAC";
    QTest::newRow("08 mAudio_24x96_tta") << mAudio_24x96_tta    << 24   << 96000    << 2    << 900000 << "TTA";
    QTest::newRow("09 mAudio_24x96_wav") << mAudio_24x96_wav    << 24   << 96000    << 2    << 900000 << "WAV";
    QTest::newRow("10 mAudio_24x96_wv")  << mAudio_24x96_wv     << 24   << 96000    << 2    << 900000 << "WavPack";

    QTest::newRow("11 mAudio_32x192_wav") << mAudio_32x192_wav  << 32   << 192000   << 2    << 900000 << "WAV";
}
