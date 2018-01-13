#ifndef ENCODER_H
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


#define ENCODER_H
#include <QObject>
#include <QProcess>

#include "track.h"
#include "types.h"
#include "job.h"

class QProcess;
class OutFormat;

class Encoder: public QObject
{
    Q_OBJECT
public:
    Encoder(const Job &job,  QObject *parent = 0);

    //QString outFile() const { return mOutFile; }
public slots:
    void run();

signals:
    void progress(quint64 trackId, Track::Status status, int percent);
    void finished();
    void error(const QString &message);

private slots:

    //void processBytesWritten(qint64 bytes);
    void proccessDecodedData(const QByteArray &data, QIODevice *encoder);

private:
    const Job mJob;
//    const WorkerRequest mRequest;
//    const OutFormat *mFormat;
//    QString mOutFile;
    uint mTotal;
    uint mReady;
    uint mProgress;
//    CueIndex mStart;
//    CueIndex mEnd;
//    AudioQuality mQuality;

    //void readInputFile(QProcess *process);
    void runWav();
    void runProccess();
};

class EncoderError: public FlaconError
{
public:
    EncoderError(const QString &message): FlaconError(message, "Encoder") {}
};


#endif // ENCODER_H
