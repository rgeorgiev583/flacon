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

enum class PreGapType
{
    Skip,
    ExtractToFile,
    AddToFirstTrack
};

QString preGapTypeToString(PreGapType type);
PreGapType strToPreGapType(const QString &str);


enum class GainType
{
    Disable,
    Track,
    Album
};

QString gainTypeToString(GainType type);
GainType strToGainType(const QString &str);


enum class CoverMode
{
    Disable,
    OrigSize,
    Scale
};

QString coverModeToString(CoverMode mode);
CoverMode strToCoverMode(const QString &str);


unsigned int levenshteinDistance(const QString &s1, const QString & s2);
QIcon loadIcon(const QString &iconName, bool loadDisable = true);
void debugArguments(const QString &prog, const QStringList &args);

class FlaconError: public std::exception
{
public:
    FlaconError(const QString &message, const QString &module):
        std::exception(),
        mMessage(message),
        mModule(module)
    {
    }

    QString message() const { return mMessage; }
    QString module()  const { return mModule; }

    const char* what() const noexcept override
    {
        return QString("[%1] %2").arg(mModule).arg(mMessage).toLocal8Bit().data();
    }

private:
    QString mMessage;
    QString mModule;
};

class AudioQuality
{
public:
    AudioQuality();
    AudioQuality(quint32 bitsPerSample, quint32 sampleRate, quint16 numChannels);

    bool operator==(const AudioQuality &other) const;
    bool operator!=(const AudioQuality &other) const;

    quint16 numChannels() const { return mNumChannels; }
    void setNumChannels(quint16 value) { mNumChannels = value; }

    quint32 sampleRate() const { return mSampleRate; }
    void setSampleRate(quint32 value) { mSampleRate = value; }

    quint32 bitsPerSample() const { return mBitsPerSample; }
    void setBitsPerSample(quint32 value) { mBitsPerSample = value; }

    quint32 bytesPerSecond() const { return mSampleRate * mBitsPerSample / 8 * mNumChannels; }


    static AudioQuality qualityCD()     { return AudioQuality(16, 44100, 2);  }
    static AudioQuality quality24x96()  { return AudioQuality(24, 96000, 2);  }
    static AudioQuality quality32x192() { return AudioQuality(32, 192000, 2); }

private:
    quint32 mBitsPerSample;
    quint32 mSampleRate;
    quint16 mNumChannels;
};

class Tags
{
public:
    Tags();
    Tags(const Tags &other);
    Tags &operator=(const Tags &other);

    QString artist() const { return mArtist; }
    void setArtist(const QString &value) { mArtist = value; }

    QString album() const { return mAlbum; }
    void setAlbum(const QString &value) { mAlbum = value; }

    QString comment() const { return mComment; }
    void setComment(const QString &value) { mComment = value; }

    QString title() const { return mTitle; }
    void setTitle(const QString &value) { mTitle = value; }

    QString genre() const { return mGenre; }
    void setGenre(const QString &value) { mGenre = value; }

    QString date() const { return mDate; }
    void setDate(const QString &value) { mDate = value; }

    QString diskId() const { return mDiskId; }
    void setDiskId(const QString &value) { mDiskId = value; }

    uint trackNum() const { return mTrackNum; }
    void setTrackNum(uint value) { mTrackNum = value; }

    uint trackCount() const { return mTrackCount; }
    void setTrackCount(uint value) { mTrackCount = value; }
private:
    QString mArtist;
    QString mAlbum;
    QString mComment;
    QString mTitle;
    QString mGenre;
    QString mDate;
    QString mDiskId;
    uint mTrackNum;
    uint mTrackCount;
};

#endif // TYPES_H
