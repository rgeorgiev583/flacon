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


#ifndef DISKPIPLINE_H
#define DISKPIPLINE_H

#include <QObject>
#include <QTemporaryDir>
#include <QThread>
#include "track.h"
#include "worker.h"
#include "types.h"
#include "job.h"

class Disk;
class Project;
class WorkerThread;
class QThreadPool;

class Thread: public QThread
{
    Q_OBJECT
public:
    Thread(const Job &job, QObject *parent = nullptr);
    virtual ~Thread();

protected:
    void run();

signals:
    void error(const QString &message);
    void progress(int percent);
private:
    Job mJob;
};

class DiskPipeline : public QObject
{
    Q_OBJECT
public:
    explicit DiskPipeline(const Disk *disk, const AudioQuality &quality, const OutFormat &format, QObject *parent = 0);
    virtual ~DiskPipeline();

    bool init();
    int startWorker(int count);
    void stop();
    bool isRunning() const;
    int runningThreadCount() const;
    void start(QThreadPool *pool);
signals:
    void readyStart();
    void threadFinished();
    void finished();
    void threadQuit();
    void trackFinished(quint64 trackId);

private slots:
    void encoderProgress(quint64 trackId, Track::Status status, int percent);
    void trackError(const Track *track, const QString &message);

    //void addEncoderRequest(const Track *track, const QString &inputFile);
    //void addGainRequest(const Track *track, const QString &fileName);
    //void trackDone(const Track *track, const QString &outFileName);

private:
    class Data;
   // Data *mData;
//    QTemporaryDir *mTmpDir;
    QVector<WorkerThread*> mThreads;
    QList<Job> mJobs;
    const Disk *mDisk;
    PreGapType mPreGapType;
    AudioQuality mQuality;
    int mCnt;
    void startThread(const Job &job);
    bool createDir(const QString &dirName) const;


};


class DiskPipelineError: public FlaconError
{
public:
    DiskPipelineError(const QString &message) :
        FlaconError(message, "DiskPipeline")
    {
    }
};


#endif // DISKPIPLINE_H
