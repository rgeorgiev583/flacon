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

#include "gain.h"
#include "profiles.h"

#include <QProcess>
#include <QDir>
#include <QTextStream>
#include <QDebug>

/************************************************
 *
 ************************************************/
Gain::Gain(const Profile &profile, QObject *parent) :
    Worker(parent),
    mProfile(profile)
{
}

/************************************************
 *
 ************************************************/
void Gain::run()
{
    QStringList files;
    for (const GainTrack &track : mTracks) {
        emit trackProgress(track.track, TrackState::CalcGain, 0);
        files << QDir::toNativeSeparators(track.file);
    }

    QStringList args = mProfile.gainArgs(files);
    QString     prog = args.takeFirst();

    qDebug() << "Start gain:" << debugProgramArgs(prog, args);

    QProcess process;

    process.start(prog, args);
    process.waitForFinished(-1);

    if (process.exitCode() != 0) {
        qWarning() << "Gain command failed: " << debugProgramArgs(prog, args);
        QString msg = tr("Gain error:\n") + QString::fromLocal8Bit(process.readAllStandardError());
        emit    error(mTracks.first().track, msg);
    }

    for (const GainTrack &track : mTracks) {
        emit trackProgress(track.track, TrackState::WriteGain, 100);
        emit trackReady(track.track, track.file);
    }
}
