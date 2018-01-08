/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2017-2018
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
#include "../converter/encoder.h"

#include <QTest>
#include <QDebug>


/************************************************
 *
 ************************************************/
void TestFlacon::testEncoder()
{
    QFETCH(QString,  inQuality);
    QFETCH(QString,  outQuality);
    QFETCH(QString,  start);
    QFETCH(QString,  end);
    QFETCH(QString,  hash);

    QString inFile;
    if      (inQuality == "CD")      inFile = mAudio_cd_wav;
    else if (inQuality == "24x96")   inFile = mAudio_24x96_wav;
    else if (inQuality == "32x192")  inFile = mAudio_32x192_wav;
    else                             QFAIL("Unknown inQuality");
    QString inFileBaseName = inFile.left(inFile.length()-3);


    AudioQuality quality;
    if      (outQuality == "CD")     quality = AudioQuality::qualityCD();
    else if (outQuality == "24x96")  quality = AudioQuality::quality24x96();
    else if (outQuality == "32x192") quality = AudioQuality::quality32x192();
    else                             QFAIL("Unknown outQuality");


    QStringList inFormats;
    inFormats << "wav";
    inFormats << "ape";
    inFormats << "flac";
    inFormats << "tta";
    inFormats << "wv";

    QStringList outFormats;
    outFormats << "WAV";
    outFormats << "FLAC";
    outFormats << "WV";

    foreach (QString inFormat, inFormats)
    {
        QString inFile = inFileBaseName + inFormat;

        foreach (QString outFormatId, outFormats)
        {
            OutFormat *outFormat = OutFormat::formatForId(outFormatId);
            QString outFile = QString("%1/out_%2.%3").arg(dir(), inFormat, outFormat->ext());

            Job job;
            job.inputFile  = inFile;
            job.outputFile = outFile;
            job.start   = CueIndex(start);
            job.end     = CueIndex(end);
            job.format  = outFormat->id();
            job.quality = quality;
            job.tags.setTrackNum(1);
            job.tags.setTrackCount(1);

            Encoder enc(job);

            try
            {
                enc.run();
            }
            catch(FlaconError& e)
            {
                FAIL_EXCEPTION(e);
            }

            if (!QFileInfo::exists(outFile))
                QFAIL(QString("File %1 not exists.\n").arg(outFile).toLocal8Bit().data());

            compareAudioHash(outFile, hash);

        }
    }
}


/************************************************
 *
 ************************************************/
void TestFlacon::testEncoder_data()
{
    QTest::addColumn<QString>("inQuality");
    QTest::addColumn<QString>("outQuality");
    QTest::addColumn<QString>("start");
    QTest::addColumn<QString>("end");
    QTest::addColumn<QString>("hash");

    QTest::newRow("001") << "CD"    << "CD"     << "00:00:00" << "00:30:00" << "7d6351521a02b625905edd28970b5a73";
    QTest::newRow("002") << "CD"    << "CD"     << "00:30:00" << "01:30:00" << "ac122fd6541d84bd3fad555f3f0a67df";
    QTest::newRow("003") << "CD"    << "CD"     << "01:30:00" << "02:30:00" << "128aa3a57539d70cdb225a9b1b76a3c2";

    QTest::newRow("004") << "CD"    << "CD"     << "00:00:10" << "00:30:00" << "2310ce664e1dc134ccbf8af5b52710bc";
    QTest::newRow("005") << "CD"    << "CD"     << "00:30:00" << "01:30:20" << "26575693c3c50c4f91563769ec9dee02";
    QTest::newRow("006") << "CD"    << "CD"     << "01:30:20" << "02:30:30" << "f0c8971a53aa4be86093da31145b5d87";


    QTest::newRow("007") << "24x96" << "24x96"  << "00:00:000" << "00:30:000" << "a20d655209861b734d96e79e80e967cd";
    QTest::newRow("008") << "24x96" << "24x96"  << "00:30:000" << "01:30:000" << "f53a6b3612b407fc1c42a755d1130e62";
    QTest::newRow("009") << "24x96" << "24x96"  << "01:30:000" << "02:30:000" << "ac3eb3dec93094791e5358f9151fadd0";
}
