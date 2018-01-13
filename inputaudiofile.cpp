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


#include "inputaudiofile.h"
#include "decoder.h"
#include "format.h"
#include <settings.h>
#include <QProcess>
#include <QStringList>
#include <QByteArray>
#include <QTextStream>
#include <QDebug>
#include <QFileInfo>
#include <QDir>


/************************************************

 ************************************************/
InputAudioFile::InputAudioFile(const QString &fileName):
    mFileName(fileName),
    mValid(false),
    mSampleRate(0),
    mCdQuality(false),
    mDuration(0),
    mFormat(0)

{
    mValid = load();
}


/************************************************

 ************************************************/
InputAudioFile::InputAudioFile(const InputAudioFile &other)
{
    mFileName    = other.mFileName;
    mValid       = other.mValid;
    mErrorString = other.mErrorString;
    mSampleRate  = other.mSampleRate;
    mCdQuality   = other.mCdQuality;
    mDuration    = other.mDuration;
    mFormat      = other.mFormat;
    mQuality     = other.mQuality;

}

InputAudioFile &InputAudioFile::operator =(const InputAudioFile &other)
{
    mFileName    = other.mFileName;
    mValid       = other.mValid;
    mErrorString = other.mErrorString;
    mSampleRate  = other.mSampleRate;
    mCdQuality   = other.mCdQuality;
    mDuration    = other.mDuration;
    mFormat      = other.mFormat;
    mQuality     = other.mQuality;
    return *this;
}


/************************************************

 ************************************************/
bool InputAudioFile::load()
{
    if (mFileName == "")
    {
        qWarning() << "The audio file name is not set";
        mErrorString = QObject::tr("The audio file name is not set");
        return false;
    }

    if (!QFileInfo(mFileName).exists())
    {
        qWarning() << QString("The audio file <b>\"%1\"</b> does not exist").arg(mFileName);
        mErrorString = QObject::tr("The audio file <b>\"%1\"</b> does not exist").arg(mFileName);
        return false;
    }


    mFormat = AudioFormat::formatForFile(mFileName);
    if (!mFormat)
    {
        mErrorString = QObject::tr("File <b>%1</b> is not a supported audio file. <br>"
                                   "<br>Verify that all required programs are installed and in your preferences.").arg(mFileName);

        return false;
    }

    // See also `mediainfo -f  CD.wav` & `mediainfo --Info-Parameters`
    QProcess proc;
    QStringList args;
    args << "--Inform=Audio;"
            "D:%Duration%\\n"
            "S:%SamplingRate%\\n"
            "B:%Resolution%\\n"
            "C:%Channels%";

    args << QDir::toNativeSeparators(mFileName);

    proc.start("mediainfo", args);
    proc.waitForFinished();

    if (proc.exitCode() != 0)
    {
        mErrorString = QObject::tr("File <b>%1</b> is not a supported audio file. <br>"
                                   "<br>Verify that all required programs are installed and in your preferences.").arg(mFileName);
        mErrorString += ": " + proc.readAllStandardError();
        return false;
    }

    bool ok = false;
    foreach (QByteArray line, proc.readAllStandardOutput().split('\n'))
    {
        if (!line.length())
            continue;


        if (line.startsWith("D:"))
        {
            mDuration = line.mid(2).toInt(&ok);

            if (!ok)
                break;
        }

        if (line.startsWith("S:"))
        {
            this->mQuality.setSampleRate(line.mid(2).toInt(&ok));

            if (!ok)
                break;
        }

        if (line.startsWith("B:"))
        {
            this->mQuality.setBitsPerSample(line.mid(2).toInt(&ok));

            if (!ok)
                break;
        }

        if (line.startsWith("C:"))
        {
            this->mQuality.setNumChannels(line.mid(2).toInt(&ok));

            if (!ok)
                break;
        }
    }

    if (!ok)
    {
        mErrorString = QObject::tr("File <b>%1</b> is not a supported audio file. <br>"
                               "<br>Verify that all required programs are installed and in your preferences.").arg(mFileName);
        return false;
    }

    mSampleRate = mQuality.sampleRate();
    mCdQuality  = mQuality.bitsPerSample() == 16;

    return true;
}

