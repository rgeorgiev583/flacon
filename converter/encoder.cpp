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


#include "encoder.h"
#include "decoder.h"
#include "outformat.h"

#include <QFileInfo>
#include <QDir>
#include <QDebug>


/************************************************
 *
 ************************************************/
Encoder::Encoder(const Job &job, QObject *parent):
    QObject(parent),
    mJob(job),
    mTotal(0),
    mReady(0),
    mProgress(0)
{
    mTotal = mJob.quality.bytesPerSecond() * ((mJob.end.milliseconds() - mJob.start.milliseconds()) / 1000.0);
}


/************************************************

 ************************************************/
void Encoder::run()
{
    emit progress(mJob.trackId, Track::Encoding, 0);
    try
    {
        if (mJob.format == "WAV")
            runWav();
        else
            runProccess();

        emit progress(mJob.trackId, Track::OK, -1);
    }
    catch (FlaconError &err)
    {
        emit progress(mJob.trackId, Track::Error, -1);
        emit error(err.message());
    }

    emit finished();


    /*
    emit trackProgress(mRequest.track(), Track::Encoding, 0);
    bool debug  = QProcessEnvironment::systemEnvironment().contains("FLACON_DEBUG_ENCODER");

    // Input file already WAV, so for WAV output format we just rename file.
    if (mFormat->id() == "WAV")
    {
        runWav();
        return;
    }

    QProcess process;
    QStringList args = mFormat->encoderArgs(mRequest.track(), QDir::toNativeSeparators(mRequest.outFile()));
    QString prog = args.takeFirst();

    if (debug)
        debugArguments(prog, args);

    connect(&process, SIGNAL(bytesWritten(qint64)),
            this, SLOT(processBytesWritten(qint64)));

    process.start(QDir::toNativeSeparators(prog), args);
    process.waitForStarted();

    readInputFile(&process);
    process.closeWriteChannel();

    process.waitForFinished(-1);
    if (process.exitCode() != 0)
    {
        QTextStream(stderr) << "Encoder command: ";
        debugArguments(prog, args);
        QString msg = tr("Encoder error:\n") + "<pre>" +
                QString::fromLocal8Bit(process.readAllStandardError()) +
                "</pre>";
        error(mRequest.track(), msg);
    }

    deleteFile(mRequest.inputFile());

    emit trackReady(mRequest.track(), mRequest.outFile());
    */
}

/************************************************
 *
 ************************************************/
void Encoder::runWav()
{
    QFile outFile(mJob.outputFile);
    if (! outFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        throw EncoderError(outFile.errorString());
    }
    Decoder decoder(mJob);
    connect(&decoder, &Decoder::readyRead,
            [this, &outFile](const QByteArray &data) { proccessDecodedData(data, &outFile); });

    decoder.run();
    outFile.close();
}


/************************************************
 *
 ************************************************/
void Encoder::runProccess()
{
    OutFormat *format = OutFormat::formatForId(mJob.format);
    if (!format)
        throw EncoderError(tr("Unknown format \"%1\"").arg(mJob.format));


    bool debug  = QProcessEnvironment::systemEnvironment().contains("FLACON_DEBUG_ENCODER");
    QStringList args = format->encoderArgs(mJob.tags, QDir::toNativeSeparators(mJob.outputFile));
    QString prog = args.takeFirst();

    if (debug)
        debugArguments(prog, args);


    Decoder decoder(mJob);

    QProcess encoder;
    encoder.setProgram(QDir::toNativeSeparators(prog));
    encoder.setArguments(args);

    encoder.start();
    encoder.waitForStarted();
    connect(&decoder, &Decoder::readyRead,
           [this, &encoder](const QByteArray &data) { proccessDecodedData(data, &encoder); });


    decoder.run();

    encoder.closeWriteChannel();
    encoder.waitForFinished();
    if (encoder.exitCode() != 0)
    {
        QTextStream(stderr) << "Encoder command: ";
        debugArguments(encoder.program(), encoder.arguments());
        QString msg = tr("Encoder error:\n") +
                QString::fromLocal8Bit(encoder.readAllStandardError());
        throw EncoderError(msg);
    }
}


/************************************************
 *
 ************************************************/
void Encoder::proccessDecodedData(const QByteArray &data, QIODevice *encoder)
{
    encoder->write(data);

    mReady += data.length();
    uint p = (mReady * 100.0 ) / mTotal;
    if (p != mProgress)
    {
        mProgress = p;
        emit progress(mJob.trackId, Track::Encoding, mProgress);
    }
}


/************************************************

 ************************************************/
//void Encoder::processBytesWritten(qint64 bytes)
//{
//    mReady += bytes;
//    int p = ((mReady * 100.0) / mTotal);
//    if (p != mProgress)
//    {
//        mProgress = p;
//        emit trackProgress(mRequest.track(), Track::Encoding, mProgress);
//    }
//}


/************************************************

 ************************************************/
//void Encoder::readInputFile(QProcess *process)
//{
//    QFile file(mRequest.inputFile());
//    if (!file.open(QFile::ReadOnly))
//    {
//        error(mRequest.track(), tr("I can't read %1 file", "Encoder error. %1 is a file name.").arg(mRequest.inputFile()));
//    }

//    mProgress = -1;
//    mTotal = file.size();

//    int bufSize = int(mTotal / 200);
//    QByteArray buf;

//    while (!file.atEnd())
//    {
//        buf = file.read(bufSize);
//        process->write(buf);
//    }
//}


/************************************************

 ************************************************/
//void Encoder::runWav()
//{
//    QFile srcFile(mRequest.inputFile());
//    bool res =  srcFile.rename(mRequest.outFile());

//    if (!res)
//    {
//        error(mRequest.track(),
//              tr("I can't rename file:\n%1 to %2\n%3").arg(
//                  mRequest.inputFile(),
//                  mRequest.outFile(),
//                  srcFile.errorString()));
//    }

//    emit trackProgress(mRequest.track(), Track::Encoding, 100);
//    emit trackReady(mRequest.track(), mRequest.outFile());
//}
