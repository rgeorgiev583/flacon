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

#include "project.h"
#include "settings.h"
#include "cue.h"
#include "inputaudiofile.h"

#include <QDebug>
#include <QApplication>
#include <QMessageBox>
#include <QDir>

/************************************************

 ************************************************/
void Project::clear()
{
    QList<Disc *> discs;
    for (int i = 0; i < count(); ++i)
        discs << disc(i);

    removeDisc(&discs);
}

/************************************************

 ************************************************/
Project *Project::instance()
{
    static Project *inst = nullptr;
    if (!inst)
        inst = new Project();

    return inst;
}

/************************************************

 ************************************************/
Project::Project(QObject *parent) :
    QObject(parent)
{
}

/************************************************

 ************************************************/
Disc *Project::disc(int index) const
{
    return mDiscs.at(index);
}

/************************************************

 ************************************************/
int Project::count() const
{
    return mDiscs.count();
}

/************************************************

 ************************************************/
int Project::insertDisc(Disc *disc, int index)
{
    if (index < 0)
        index = mDiscs.count();

    mDiscs.insert(index, disc);

    emit layoutChanged();
    return index;
}

/************************************************

 ************************************************/
void Project::removeDisc(const QList<Disc *> *discs)
{
    for (int i = 0; i < discs->count(); ++i) {
        Disc *disc = discs->at(i);
        emit  beforeRemoveDisc(disc);
        if (mDiscs.removeAll(disc))
            disc->deleteLater();

        emit afterRemoveDisc();
    }
}

/************************************************

 ************************************************/
int Project::indexOf(const Disc *disc) const
{
    return mDiscs.indexOf(const_cast<Disc *>(disc));
}

/************************************************
 *
 ************************************************/
bool Project::discExists(const QString &cueUri)
{
    foreach (const Disc *d, mDiscs) {
        if (d->cueFile() == cueUri)
            return true;
    }
    return false;
}

/************************************************

 ************************************************/
Disc *Project::addAudioFile(const QString &fileName)
{

    QString canonicalFileName = QFileInfo(fileName).canonicalFilePath();

    for (int i = 0; i < count(); ++i) {
        if (disc(i)->audioFileName() == canonicalFileName)
            return nullptr;
    }

    InputAudioFile audio(QFileInfo(fileName).absoluteFilePath());
    if (!audio.isValid()) {
        throw FlaconError(audio.errorString());
    }

    Disc *disc = new Disc();
    disc->setAudioFile(audio);
    addDisc(disc);

    return disc;
}

/************************************************

 ************************************************/
DiscList Project::addCueFile(const QString &fileName)
{
    DiscList res;
    try {
        QVector<CueDisc> discs = CueReader().load(fileName);

        for (int i = 0; i < discs.count(); ++i) {
            if (discExists(discs.at(i).uri()))
                continue;

            Disc *disc = new Disc();
            disc->loadFromCue(discs.at(i));
            mDiscs << disc;
            res << disc;
        }
        emit layoutChanged();
    }
    catch (FlaconError &err) {
        emit layoutChanged();
        qWarning() << err.what();
        throw err;
    }

    return res;
}

/************************************************

 ************************************************/
void Project::emitDiscChanged(Disc *disc) const
{
    emit discChanged(disc);
}

/************************************************

 ************************************************/
void Project::emitLayoutChanged() const
{
    emit layoutChanged();
}
