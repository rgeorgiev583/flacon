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
#include "../converter/decoder.h"
#include "../formats/wav.h"
#include "../formats/flac.h"

#include <QTest>
#include <QVector>
#include <QDebug>


/************************************************
 *
 ************************************************/
void TestFlacon::testDecoder()
{
    QFETCH(QString,  audioFile);
    QFETCH(QString,  start);
    QFETCH(QString,  end);
    QFETCH(QString,  hash);

    QString outFileName = QString("%1/out.wav").arg(dir());
    QFile outFile(outFileName);
    outFile.open(QFile::WriteOnly | QFile::Truncate);

    Job job;
    job.inputFile  = audioFile;
    job.outputFile = "";
    job.start      = CueIndex(start);
    job.end        = CueIndex(end);
    job.format     = "";

    if      (QFileInfo(audioFile).baseName().startsWith("CD"))      job.quality = AudioQuality::qualityCD() ;
    else if (QFileInfo(audioFile).baseName().startsWith("24x96"))   job.quality = AudioQuality::quality24x96();
    else if (QFileInfo(audioFile).baseName().startsWith("32x192"))  job.quality = AudioQuality::quality32x192();
    else  QFAIL("Unknown file type");

    Decoder decoder(job);
    connect(&decoder, &Decoder::readyRead,
            [=,&outFile](const QByteArray &data) { outFile.write(data); } );

    try
    {
        decoder.run();
    }
    catch(FlaconError& e)
    {
        FAIL_EXCEPTION(e);
    }
    outFile.close();

    compareAudioHash(outFileName, hash);
}


/************************************************
 *
 ************************************************/
void TestFlacon::testDecoder_data()
{
    QTest::addColumn<QString>("audioFile");
    QTest::addColumn<QString>("start");
    QTest::addColumn<QString>("end");
    QTest::addColumn<QString>("hash");

    QTest::newRow("001") << mAudio_cd_wav    << "00:00:00" << "00:30:00" << "7d6351521a02b625905edd28970b5a73";
    QTest::newRow("002") << mAudio_cd_wav    << "00:30:00" << "01:30:00" << "ac122fd6541d84bd3fad555f3f0a67df";
    QTest::newRow("003") << mAudio_cd_wav    << "01:30:00" << "02:30:00" << "128aa3a57539d70cdb225a9b1b76a3c2";

    QTest::newRow("004") << mAudio_cd_wav    << "00:00:10" << "00:30:00" << "2310ce664e1dc134ccbf8af5b52710bc";
    QTest::newRow("005") << mAudio_cd_wav    << "00:30:00" << "01:30:20" << "26575693c3c50c4f91563769ec9dee02";
    QTest::newRow("006") << mAudio_cd_wav    << "01:30:20" << "02:30:30" << "f0c8971a53aa4be86093da31145b5d87";

    QTest::newRow("007") << mAudio_24x96_wav << "00:00:000" << "00:30:000" << "a20d655209861b734d96e79e80e967cd";
    QTest::newRow("008") << mAudio_24x96_wav << "00:30:000" << "01:30:000" << "f53a6b3612b407fc1c42a755d1130e62";
    QTest::newRow("009") << mAudio_24x96_wav << "01:30:000" << "02:30:000" << "ac3eb3dec93094791e5358f9151fadd0";

    QTest::newRow("010") << mAudio_cd_flac   << "00:00:00" << "00:30:00" << "7d6351521a02b625905edd28970b5a73";
    QTest::newRow("011") << mAudio_cd_flac   << "00:30:00" << "01:30:00" << "ac122fd6541d84bd3fad555f3f0a67df";
    QTest::newRow("012") << mAudio_cd_flac   << "01:30:00" << "02:30:00" << "128aa3a57539d70cdb225a9b1b76a3c2";

    QTest::newRow("013") << mAudio_cd_flac   << "00:00:10" << "00:30:00" << "2310ce664e1dc134ccbf8af5b52710bc";
    QTest::newRow("014") << mAudio_cd_flac   << "00:30:00" << "01:30:20" << "26575693c3c50c4f91563769ec9dee02";
    QTest::newRow("015") << mAudio_cd_flac   << "01:30:20" << "02:30:30" << "f0c8971a53aa4be86093da31145b5d87";

    QTest::newRow("016") << mAudio_24x96_ape << "00:00:000" << "00:30:000" << "a20d655209861b734d96e79e80e967cd";
    QTest::newRow("017") << mAudio_24x96_ape << "00:30:000" << "01:30:000" << "f53a6b3612b407fc1c42a755d1130e62";
    QTest::newRow("018") << mAudio_24x96_ape << "01:30:000" << "02:30:000" << "ac3eb3dec93094791e5358f9151fadd0";

    QTest::newRow("019") << mAudio_cd_ape    << "00:00:00" << "00:30:00" << "7d6351521a02b625905edd28970b5a73";
    QTest::newRow("020") << mAudio_cd_ape    << "00:30:00" << "01:30:00" << "ac122fd6541d84bd3fad555f3f0a67df";
    QTest::newRow("021") << mAudio_cd_ape    << "01:30:00" << "02:30:00" << "128aa3a57539d70cdb225a9b1b76a3c2";

    QTest::newRow("022") << mAudio_cd_ape    << "00:00:10" << "00:30:00" << "2310ce664e1dc134ccbf8af5b52710bc";
    QTest::newRow("023") << mAudio_cd_ape    << "00:30:00" << "01:30:20" << "26575693c3c50c4f91563769ec9dee02";
    QTest::newRow("024") << mAudio_cd_ape    << "01:30:20" << "02:30:30" << "f0c8971a53aa4be86093da31145b5d87";

    QTest::newRow("025") << mAudio_24x96_ape << "00:00:000" << "00:30:000" << "a20d655209861b734d96e79e80e967cd";
    QTest::newRow("026") << mAudio_24x96_ape << "00:30:000" << "01:30:000" << "f53a6b3612b407fc1c42a755d1130e62";
    QTest::newRow("027") << mAudio_24x96_ape << "01:30:000" << "02:30:000" << "ac3eb3dec93094791e5358f9151fadd0";

    QTest::newRow("028") << mAudio_cd_tta    << "00:00:00" << "00:30:00" << "7d6351521a02b625905edd28970b5a73";
    QTest::newRow("029") << mAudio_cd_tta    << "00:30:00" << "01:30:00" << "ac122fd6541d84bd3fad555f3f0a67df";
    QTest::newRow("030") << mAudio_cd_tta    << "01:30:00" << "02:30:00" << "128aa3a57539d70cdb225a9b1b76a3c2";

    QTest::newRow("031") << mAudio_cd_tta    << "00:00:10" << "00:30:00" << "2310ce664e1dc134ccbf8af5b52710bc";
    QTest::newRow("032") << mAudio_cd_tta    << "00:30:00" << "01:30:20" << "26575693c3c50c4f91563769ec9dee02";
    QTest::newRow("033") << mAudio_cd_tta    << "01:30:20" << "02:30:30" << "f0c8971a53aa4be86093da31145b5d87";

    QTest::newRow("034") << mAudio_24x96_tta << "00:00:000" << "00:30:000" << "a20d655209861b734d96e79e80e967cd";
    QTest::newRow("035") << mAudio_24x96_tta << "00:30:000" << "01:30:000" << "f53a6b3612b407fc1c42a755d1130e62";
    QTest::newRow("036") << mAudio_24x96_tta << "01:30:000" << "02:30:000" << "ac3eb3dec93094791e5358f9151fadd0";

    QTest::newRow("037") << mAudio_cd_wv     << "00:00:00" << "00:30:00" << "7d6351521a02b625905edd28970b5a73";
    QTest::newRow("038") << mAudio_cd_wv     << "00:30:00" << "01:30:00" << "ac122fd6541d84bd3fad555f3f0a67df";
    QTest::newRow("039") << mAudio_cd_wv     << "01:30:00" << "02:30:00" << "128aa3a57539d70cdb225a9b1b76a3c2";

    QTest::newRow("040") << mAudio_cd_wv     << "00:00:10" << "00:30:00" << "2310ce664e1dc134ccbf8af5b52710bc";
    QTest::newRow("041") << mAudio_cd_wv     << "00:30:00" << "01:30:20" << "26575693c3c50c4f91563769ec9dee02";
    QTest::newRow("042") << mAudio_cd_wv     << "01:30:20" << "02:30:30" << "f0c8971a53aa4be86093da31145b5d87";

    QTest::newRow("043") << mAudio_24x96_wv  << "00:00:000" << "00:30:000" << "a20d655209861b734d96e79e80e967cd";
    QTest::newRow("044") << mAudio_24x96_wv  << "00:30:000" << "01:30:000" << "f53a6b3612b407fc1c42a755d1130e62";
    QTest::newRow("045") << mAudio_24x96_wv  << "01:30:000" << "02:30:000" << "ac3eb3dec93094791e5358f9151fadd0";
}

