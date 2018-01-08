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


#ifndef DECODER_H
#define DECODER_H

#include "wavheader.h"
#include <QObject>
#include <QString>
#include <QByteArray>
#include "../cue.h"
#include "../formats/format.h"
#include "types.h"
#include "job.h"

class QProcess;

class Decoder : public QObject
{
    Q_OBJECT
public:
    explicit Decoder(const Job &job, QObject *parent = 0);
    virtual ~Decoder();

    void run();

signals:
    void readyRead(const QByteArray &data);

private slots:
    void readStandardOutput();

private:
    Job        mJob;
    QProcess  *mProcess;
    quint64    mPos;
    quint64    mBytesStart;
    quint64    mBytesEnd;
};


class QIODevice;


class DecoderOld : public QObject
{
    Q_OBJECT
public:
    explicit DecoderOld(QObject *parent = 0);
    explicit DecoderOld(const AudioFormat &format, QObject *parent = 0);
    virtual ~DecoderOld();

    bool open(const QString fileName);
    void close();

    bool extract(const CueTime &start, const CueTime &end, QIODevice *outDevice);
    bool extract(const CueTime &start, const CueTime &end, const QString &outFileName);

    // Duration of audio in milliseconds.
    uint duration() const { return mWavHeader.duration(); }

    QString errorString() const { return mErrorString; }

    WavHeader wavHeader() const { return mWavHeader; }

signals:
    void progress(int percent);

private slots:
    void readStandardError();

private:
    const AudioFormat *mFormat;
    QProcess  *mProcess;
    QString    mInputFile;
    QFile     *mFile;
    WavHeader  mWavHeader;
    QString    mErrorString;
    quint64    mPos;
    QByteArray mErrBuff;

    bool openFile();
    bool openProcess();
};


class DecoderError: public FlaconError
{
public:
    DecoderError(const QString &message): FlaconError(message, "Decoder") {}
};


#endif // DECODER_H
