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

#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QIcon>

#define CODEC_AUTODETECT "AUTODETECT"

typedef quint16 DiscNum;
typedef quint16 DiscCount;
typedef quint16 TrackNum;
typedef quint16 TrackCount;
typedef uint    Duration;

enum class PreGapType {
    Skip,
    ExtractToFile,
    AddToFirstTrack
};

QString    preGapTypeToString(PreGapType type);
PreGapType strToPreGapType(const QString &str);

enum class GainType {
    Disable,
    Track,
    Album
};

QString  gainTypeToString(GainType type);
GainType strToGainType(const QString &str);

enum class CoverMode {
    Disable,
    OrigSize,
    Scale
};

QString   coverModeToString(CoverMode mode);
CoverMode strToCoverMode(const QString &str);

unsigned int levenshteinDistance(const QString &s1, const QString &s2);

class FlaconError : public std::runtime_error
{
public:
    explicit FlaconError(const char *msg) :
        std::runtime_error(msg) { }
    explicit FlaconError(const std::string &msg) :
        std::runtime_error(msg) { }
    explicit FlaconError(const QString &msg) :
        std::runtime_error(msg.toStdString()) { }
};

class CueIndex
{
public:
    explicit CueIndex(const QString &str = "");

    bool    isNull() const { return mNull; }
    QString toString(bool cdQuality = true) const;

    CueIndex operator-(const CueIndex &other) const;
    bool     operator==(const CueIndex &other) const;
    bool     operator!=(const CueIndex &other) const;

    uint milliseconds() const { return mHiValue; }
    uint frames() const { return mCdValue; }

private:
    bool mNull;
    int  mCdValue;
    int  mHiValue;

    bool parse(const QString &str);
};

typedef CueIndex CueTime;

enum class TrackState {
    NotRunning = 0,
    Canceled   = 1,
    Error      = 2,
    Aborted    = 3,
    OK         = 4,
    Splitting  = 5,
    Encoding   = 6,
    Queued     = 7,
    WaitGain   = 8,
    CalcGain   = 9,
    WriteGain  = 10
};

Q_DECLARE_METATYPE(TrackState)

enum class BitsPerSample {
    AsSourcee = 0,
    Bit_16    = 16,
    Bit_24    = 24,
    Bit_32    = 32,
    Bit_64    = 64,
};
Q_DECLARE_METATYPE(BitsPerSample)

enum class SampleRate {
    AsSource  = 0,
    Hz_44100  = 44100,
    Hz_48000  = 48000,
    Hz_96000  = 96000,
    Hz_192000 = 192000,
    Hz_384000 = 384000,
    Hz_768000 = 768000,
};
Q_DECLARE_METATYPE(SampleRate)

enum class FormatOption {
    NoOptions   = 0x0,
    Lossless    = 0x1,
    SupportGain = 0x2,
};

Q_DECLARE_FLAGS(FormatOptions, FormatOption)
Q_DECLARE_OPERATORS_FOR_FLAGS(FormatOptions)
inline bool operator&&(const FormatOptions &flags, const FormatOption flag) noexcept
{
    return flags.testFlag(flag);
}

typedef quint8  Percent;
typedef quint64 TrackId;

QByteArray leftPart(const QByteArray &line, const QChar separator);
QByteArray rightPart(const QByteArray &line, const QChar separator);
void       initTypes();

QString safeString(const QString &str);
QString debugProgramArgs(const QString &prog, const QStringList &args);

class Messages
{
public:
    static void error(const QString &message);

    class Handler
    {
    public:
        virtual void showErrorMessage(const QString &message) = 0;
    };

    static void setHandler(Handler *handler);

private:
    static Handler *mHandler;
};

#endif // TYPES_H
