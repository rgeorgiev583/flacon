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


#include "diskpipline.h"
//#include "splitter.h"
#include "encoder.h"
#include "gain.h"
#include "cuecreator.h"
#include "copycover.h"
#include "project.h"
#include "settings.h"
#include "outformat.h"
#include "../inputaudiofile.h"


#include <QThread>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>
#include <QThreadPool>

class Runnable: public QRunnable
{
public:
    Runnable(const Job &job):
        QRunnable(),
        mJob(job)
    {

    }

    void run()
    {
        qDebug() << "START Runnable" << mJob.tags.trackNum();
        try
        {
//            emit progress(1234);
            //emit error("QQQ");
            Encoder encoder(mJob);

  //          connect(&encoder, &Encoder::progress,
    //                this,     &Thread::progress);

            encoder.run();
        }
        catch (FlaconError &e)
        {
            qWarning() << "Error" << e.message();
      //      emit error(e.message());
        }

        qDebug() << "STOP Runnable" << mJob.tags.trackNum();
    }

private:
    Job mJob;

};

Thread::Thread(const Job &job, QObject *parent):
    QThread(parent),
    mJob(job)
{

}

Thread::~Thread()
{
    //qDebug() << Q_FUNC_INFO << mJob.tags.trackNum();
    quit();
    if (!wait(3000))
    {
        //qWarning() << "Can't quit from thread" << mWorker;
        terminate();
        //if (!wait(3000))
        //    qWarning() << "Can't terminate from thread" << mWorker;
    }
}

void Thread::run()
{
    try
    {
        qDebug() << "START THREAD" << mJob.tags.trackNum();
        emit progress(1234);
        //emit error("QQQ");
        Encoder encoder(mJob);

        connect(&encoder, &Encoder::progress,
                this,     &Thread::progress);

        encoder.run();
        qDebug() << "STOP THREAD" << mJob.tags.trackNum();
    }
    catch (FlaconError &e)
    {
        qWarning() << "Error" << e.message();
        emit error(e.message());
    }
}


/************************************************
 *
 ************************************************/
class WorkerThread: public QThread
{
public:
    explicit WorkerThread(Worker *worker, QObject *parent = nullptr):
        QThread(parent),
        mWorker(worker)
    {
        worker->moveToThread(this);
    }

    virtual ~WorkerThread()
    {
        quit();
        if (!wait(3000))
        {
            qWarning() << "Can't quit from thread" << mWorker;
            terminate();
            if (!wait(3000))
                qWarning() << "Can't terminate from thread" << mWorker;
        }
        mWorker->deleteLater();
    }

    void run()
    {
        mWorker->run();
    }


private:
    Worker *mWorker;
};


/************************************************
 *
// ************************************************/
//class DiskPipeline::Data
//{
//public:
//    Data():
//        pipeline(nullptr),
//        disk(nullptr),
//        //needStartSplitter(true),
//        interrupted(false)
//       // preGapType(PreGapType::Skip)
//    {
//    }

//    DiskPipeline *pipeline;
//    const Disk *disk;
//    QList<const Track*> tracks;
//    //bool needStartSplitter;
//    QHash<const Track*, Track::Status> trackStatuses;
//    //QList<WorkerRequest> encoderRequests;
//    QList<WorkerRequest> gainRequests;
//    bool interrupted;
//    QString workDir;
//    //PreGapType preGapType;


//    void interrupt(Track::Status status);
//    //void startSplitterThread();
//    //void startEncoderThread(const WorkerRequest &req);
//    //void startTrackGainThread(const WorkerRequest &req);
//    //void startAlbumGainThread(QList<WorkerRequest> &reqs);
//    bool createDir(const QString &dirName) const;
//    bool createCue() const;
//    bool copyCoverImage() const;
//};



/************************************************
 *
 ************************************************/
//bool DiskPipeline::Data::createDir(const QString &dirName) const
//{
//    QDir dir(dirName);

//    if (! dir.mkpath("."))
//    {
//        Project::error(QObject::tr("I can't create directory \"%1\".").arg(dir.path()));
//        return false;
//    }

//    if (!QFileInfo(dir.path()).isWritable())
//    {
//        Project::error(QObject::tr("I can't write to directory \"%1\".").arg(dir.path()));
//        return false;
//    }

//    return true;
//}


/************************************************
 *
 ************************************************/
DiskPipeline::DiskPipeline(const Disk *disk, const AudioQuality &quality, const OutFormat &format, QObject *parent) :
    QObject(parent),
    mDisk(disk),
    mQuality(quality)
{
    mPreGapType = settings->createCue() ? settings->preGapType() : PreGapType::Skip;
    // If the first track starts with zero second, doesn't make sense to create pregap track.
    bool extractPregapTrack = (mPreGapType == PreGapType::ExtractToFile && mDisk->track(0)->cueIndex(1).milliseconds() > 0);

    Track *track;
    if (extractPregapTrack)
    {
        track = mDisk->preGapTrack();
        CueIndex start = mDisk->track(0)->cueIndex(0);
        CueIndex end   = mDisk->track(0)->cueIndex(1);

        Job job;
        job.trackId    = track->id();
        job.inputFile  = mDisk->audioFileName();
        job.outputFile = track->resultFilePath();
        job.start      = start;
        job.end        = end;
        job.quality    = quality;
        job.format     = format.id();
        job.tags       = track->tags();
        mJobs << job;
    }

    for (int i=0; i<mDisk->count(); ++i)
    {
        track = mDisk->track(i);

        CueIndex start, end;
        if (i==0 && mPreGapType == PreGapType::AddToFirstTrack)
            start = CueTime("00:00:00");
        else
            start = track->cueIndex(1);

        if (i<mDisk->count()-1)
            end = mDisk->track(i+1)->cueIndex(01);
        else
            end = CueIndex::fromMilliseconds(mDisk->audioFile()->duration());


        Job job;
        job.trackId    = track->id();
        job.inputFile  = mDisk->audioFileName();
        job.outputFile = track->resultFilePath();
        job.start      = start;
        job.end        = end;
        job.quality    = quality;
        job.format     = format.id();
        job.tags       = track->tags();
        mJobs << job;
    }

    mCnt = mJobs.count();
}


/************************************************

 ************************************************/
DiskPipeline::~DiskPipeline()
{
//    delete mTmpDir;
}


/************************************************
 *
 ************************************************/
bool DiskPipeline::init()
{
//    if (!settings->tmpDir().isEmpty())
//    {
//        if (!mData->createDir(settings->tmpDir()))
//            return false;
//        mTmpDir = new QTemporaryDir(QString("%1/flacon.").arg(settings->tmpDir()));
//        mTmpDir->setAutoRemove(true);
//        mData->workDir = mTmpDir->path();
//    }
//    else
//        mData->workDir = QFileInfo(mData->disk->track(0)->resultFilePath()).dir().absolutePath();

//    Splitter splitter(mData->disk, mData->workDir, mData->preGapType);
//    mData->tracks = splitter.tracks();

//    foreach (const Track *track, mData->tracks)
//    {
//        mData->trackStatuses.insert(track, Track::NotRunning);
//    }

//    if (!mData->createDir(mData->workDir))
//        return false;

    foreach (const Job &job, mJobs)
    {
        QString dir = QFileInfo(job.outputFile).absoluteDir().path();
        if (!createDir(dir))
            return false;
    }

    return true;
}


/************************************************
 CREATE WORKER CHAINS
 ************************************************
              +--> Encoder ---> Track gain -->+
   Splitter ->+            ...                +-> Album gain --> this
              +--> Encoder ---> Track gain -->+

                                optional step    optional step

 ************************************************/
int DiskPipeline::startWorker(int count)
{
    return 0;
//    if (mData->interrupted)
//        return count;

    if (count <= 0)
        return 0;

    while (count > 0 && !mJobs.isEmpty())
    {
        startThread(mJobs.takeFirst());
        --count;
    }

    return count;
//    if (settings->outFormat()->gainType() == GainType::Track)
//    {
//        while (count > 0 && !mData->gainRequests.isEmpty())
//        {
//            mData->startTrackGainThread(mData->gainRequests.takeFirst());
//            --count;
//        }
//    }
//    else if (settings->outFormat()->gainType() == GainType::Album)
//    {
//        if (count > 0 && mData->gainRequests.count() == mData->tracks.count())
//        {
//            mData->startAlbumGainThread(mData->gainRequests);
//            mData->gainRequests.clear();
//            --count;
//        }
//    }

//    while (count > 0 && !mRequests.isEmpty())
//    {
//        startThread(mRequests.takeFirst());
//        --count;
//    }

//    return count;
}


/************************************************
 *
 ************************************************/
//bool DiskPipeline::Data::createCue() const
//{
////    if (!settings->createCue())
////        return true;

////    CueCreator cue(disk, preGapType);
////    if (!cue.write())
////    {
////        pipeline->trackError(tracks.first(), cue.errorString());
////        return false;
////    }

//    return true;
//}


/************************************************
 *
 ************************************************/
//bool DiskPipeline::Data::copyCoverImage() const
//{
//    CoverMode mode = settings->coverMode();

//    if (mode == CoverMode::Disable)
//        return true;

//    int size = 0;
//    if (mode == CoverMode::Scale)
//        size = settings->coverImageSize();

//    QString dir = QFileInfo(disk->track(0)->resultFilePath()).dir().absolutePath();

//    CopyCover copyCover(disk, dir, "cover", size);
//    bool res = copyCover.run();

//    if (!res)
//        Project::error(copyCover.errorString());

//    return res;
//}


/************************************************
 *
 ************************************************/
/*void DiskPipeline::Data::startSplitterThread()
{
    Splitter *worker = new Splitter(disk, workDir, preGapType);

    WorkerThread *thread = new WorkerThread(worker, pipeline);

    connect(pipeline, SIGNAL(threadQuit()),
            thread,   SLOT(terminate()));

    connect(worker,   SIGNAL(trackProgress(const Track*,Track::Status,int)),
            pipeline, SLOT(trackProgress(const Track*,Track::Status,int)));

    connect(worker,   SIGNAL(error(const Track*,QString)),
            pipeline, SLOT(trackError(const Track*,QString)));

    connect(worker,   SIGNAL(trackReady(const Track*,QString)),
            pipeline, SLOT(addEncoderRequest(const Track*,QString)));

    connect(thread,   SIGNAL(finished()),
            pipeline, SIGNAL(threadFinished()));

    pipeline->mThreads << thread;
    thread->start();

    needStartSplitter = false;
    trackStatuses.insert(disk->track(0), Track::Splitting);

    createCue();
    copyCoverImage();
}*/


/************************************************
 *
 ************************************************/
//void DiskPipeline::Data::startEncoderThread(const WorkerRequest &req)
//{
//    Encoder *worker = new Encoder(req, settings->outFormat());
//    WorkerThread *thread = new WorkerThread(worker, pipeline);

//    connect(pipeline, SIGNAL(threadQuit()),
//            thread,   SLOT(terminate()));

//    connect(worker,   SIGNAL(trackProgress(const Track*,Track::Status,int)),
//            pipeline, SLOT(trackProgress(const Track*,Track::Status,int)));

//    connect(worker, SIGNAL(error(const Track*,QString)),
//            pipeline, SLOT(trackError(const Track*,QString)));

//    if (settings->outFormat()->gainType() == GainType::Disable)
//    {
//        connect(worker, SIGNAL(trackReady(const Track*,QString)),
//                pipeline, SLOT(trackDone(const Track*,QString)));
//    }
//    else
//    {
//        connect(worker, SIGNAL(trackReady(const Track*,QString)),
//                pipeline, SLOT(addGainRequest(const Track*,QString)));
//    }

//    connect(thread,   SIGNAL(finished()),
//            pipeline, SIGNAL(threadFinished()));

//    pipeline->mThreads << thread;
//    thread->start();
//}


/************************************************
 *
 ************************************************/
//void DiskPipeline::Data::startTrackGainThread(const WorkerRequest &req)
//{
//    Gain *worker = new Gain(req, settings->outFormat());
//    WorkerThread *thread = new WorkerThread(worker, pipeline);

//    connect(pipeline, SIGNAL(threadQuit()),
//            thread,   SLOT(terminate()));

//    connect(worker,   SIGNAL(trackProgress(const Track*,Track::Status,int)),
//            pipeline, SLOT(trackProgress(const Track*,Track::Status,int)));

//    connect(worker,   SIGNAL(error(const Track*,QString)),
//            pipeline, SLOT(trackError(const Track*,QString)));

//    connect(worker,   SIGNAL(trackReady(const Track*,QString)),
//            pipeline, SLOT(trackDone(const Track*,QString)));

//    connect(thread,   SIGNAL(finished()),
//            pipeline, SIGNAL(threadFinished()));

//    pipeline->mThreads << thread;
//    thread->start();
//}


///************************************************
// *
// ************************************************/
//void DiskPipeline::Data::startAlbumGainThread(QList<WorkerRequest> &reqs)
//{
//    Gain *worker = new Gain(reqs, settings->outFormat());
//    WorkerThread *thread = new WorkerThread(worker, pipeline);

//    connect(pipeline, SIGNAL(threadQuit()),
//            thread,   SLOT(terminate()));

//    connect(worker,   SIGNAL(trackProgress(const Track*,Track::Status,int)),
//            pipeline, SLOT(trackProgress(const Track*,Track::Status,int)));

//    connect(worker,   SIGNAL(error(const Track*,QString)),
//            pipeline, SLOT(trackError(const Track*,QString)));

//    connect(worker,   SIGNAL(trackReady(const Track*,QString)),
//            pipeline, SLOT(trackDone(const Track*,QString)));

//    connect(thread,   SIGNAL(finished()),
//            pipeline, SIGNAL(threadFinished()));

//    pipeline->mThreads << thread;
//    thread->start();
//}


/************************************************
 *
 ************************************************/
//void DiskPipeline::addEncoderRequest(const Track *track, const QString &inputFile)
//{
//    trackProgress(track, Track::Queued, 0);
//    QFileInfo trackFile(track->resultFilePath());
//    QString outFile = trackFile.dir().filePath(
//                QFileInfo(inputFile).baseName() +
//                ".encoded." +
//                trackFile.suffix());

//    mData->encoderRequests << WorkerRequest(track, inputFile, outFile);
//    emit readyStart();
//}


/************************************************
 *
 ************************************************/
//void DiskPipeline::addGainRequest(const Track *track, const QString &fileName)
//{
//    if (settings->outFormat()->gainType() == GainType::Album)
//        trackProgress(track, Track::WaitGain, 0);
//    else
//        trackProgress(track, Track::Queued, 0);

//    mData->gainRequests << WorkerRequest(track, fileName, fileName);
//    emit readyStart();
//}


/************************************************
 *
 ************************************************/
//void DiskPipeline::trackDone(const Track *track, const QString &outFileName)
//{
    // Track is ready, rename the file to the final name.
//    QFile::remove(track->resultFilePath());
//    QFile(outFileName).rename(track->resultFilePath());


//    mData->trackStatuses.insert(track, Track::OK);
//    const_cast<Track*>(track)->setProgress(Track::OK);

//    emit threadFinished();

//    if (!isRunning())
//        emit finished();
//}


/************************************************
 *
 ************************************************/
void DiskPipeline::startThread(const Job &job)
{
    Track *track = nullptr;
    for (int i=0; i<mDisk->count(); ++i)
    {
        if (mDisk->track(i)->trackNum() == job.tags.trackNum())
            track = mDisk->track(i);
    }


    Thread *thread = new Thread(job, this);
    if (track)
    {
        connect(thread, SIGNAL(progress(int)),
                this, SLOT(encoderProgress(int)));
    }
        connect(thread, &Thread::finished,
                [this]() {

            --mCnt;
qDebug() << "STOPPPP" << mCnt;
        });

        //connect(thread, &Thread::progress,
        //        [&track](int percent) { qDebug() << percent; }); //track->setProgress(Track::Encoding, percent); });

//    connect(this, SIGNAL(threadQuit()),
//            thread,   SLOT(terminate()));

//    connect(worker,   SIGNAL(trackProgress(const Track*,Track::Status,int)),
//            this, SLOT(trackProgress(const Track*,Track::Status,int)));

//    connect(worker, SIGNAL(error(const Track*,QString)),
//            this, SLOT(trackError(const Track*,QString)));

//    //    if (settings->outFormat()->gainType() == GainType::Disable)
//    //    {
//            connect(worker, SIGNAL(trackReady(const Track*,QString)),
//                    this, SLOT(trackDone(const Track*,QString)));
//    //    }
//    //    else
//    //    {
//    //        connect(worker, SIGNAL(trackReady(const Track*,QString)),
//    //                this, SLOT(addGainRequest(const Track*,QString)));
//    //    }

//        connect(thread,   SIGNAL(finished()),
//                this, SIGNAL(threadFinished()));

//        mThreads << thread;
    thread->start();
}


/************************************************
 *
 ************************************************/
bool DiskPipeline::createDir(const QString &dirName) const
{
    QDir dir(dirName);

    if (! dir.mkpath("."))
    {
        Project::error(QObject::tr("I can't create directory \"%1\".").arg(dir.path()));
        return false;
    }

    if (!QFileInfo(dir.path()).isWritable())
    {
        Project::error(QObject::tr("I can't write to directory \"%1\".").arg(dir.path()));
        return false;
    }

    return true;
}


/************************************************

 ************************************************/
//void DiskPipeline::Data::interrupt(Track::Status status)
//{
////    interrupted = true;
////    //encoderRequests.clear();

////    QHash<const Track*, Track::Status>::iterator it;
////    for (it = trackStatuses.begin(); it != trackStatuses.end(); ++it)
////    {
////        switch (it.value())
////        {
////        case Track::Splitting:
////        case Track::Encoding:
////        case Track::Queued:
////        case Track::WaitGain:
////        case Track::CalcGain:
////        case Track::WriteGain:
////        case Track::NotRunning:
////            it.value() = status;
////            const_cast<Track*>(it.key())->setProgress(status);
////            break;


////        case Track::Canceled:
////        case Track::Error:
////        case Track::Aborted:
////        case Track::OK:
////            break;
////        }
////    }
//}


/************************************************

 ************************************************/
void DiskPipeline::stop()
{
//    mData->interrupt(Track::Aborted);
//    emit threadQuit();
//    emit threadFinished();

//    emit finished();
}


/************************************************

 ************************************************/
void DiskPipeline::trackError(const Track *track, const QString &message)
{
//    mData->trackStatuses.insert(track, Track::Error);
//    const_cast<Track*>(track)->setProgress(Track::Error);
//    mData->interrupt(Track::Aborted);
//    emit threadQuit();
//    emit threadFinished();

//    emit finished();
//    Project::error(message);
}


/************************************************

 ************************************************/
bool DiskPipeline::isRunning() const
{
    qDebug() << Q_FUNC_INFO << (mCnt > 0);
    return mCnt > 0;
//    QHash<const Track*, Track::Status>::const_iterator it;
//    for (it = mData->trackStatuses.begin(); it != mData->trackStatuses.end(); ++it)
//    {
//        switch (it.value())
//        {
//        case Track::Splitting:
//        case Track::Encoding:
//        case Track::Queued:
//        case Track::WaitGain:
//        case Track::CalcGain:
//        case Track::WriteGain:
//            return true;

//        case Track::NotRunning:
//        case Track::Canceled:
//        case Track::Error:
//        case Track::Aborted:
//        case Track::OK:
//            break;
//        }
//    }

    return false;
}


/************************************************
 *
 ************************************************/
int DiskPipeline::runningThreadCount() const
{
    int res = 0;
    foreach (WorkerThread *thread, mThreads)
    {
        if (thread->isRunning())
            ++res;
    }
    return res;
}

void DiskPipeline::start(QThreadPool *pool)
{

    //QtConcurrent::run([=]() {
       // Code in this block will run in another thread
   //});
//    pool->setMaxThreadCount(2);
    foreach (Job job, mJobs)
    {
        QtConcurrent::run(pool, [this, job]() {

            qDebug() << "START QtConcurrent" << job.tags.trackNum();
            try
            {
                Encoder encoder(job);

                connect(&encoder, &Encoder::progress,
                        this,     &DiskPipeline::encoderProgress);

                connect(&encoder, &Encoder::finished,
                        [this, job]() { emit trackFinished(job.trackId); });


                encoder.run();
            }
            catch (FlaconError &e)
            {
                qWarning() << "Error" << e.message();
          //      emit error(e.message());
            }

            qDebug() << "STOP QtConcurrent" << job.tags.trackNum();



        });



//        Runnable *runnable = new Runnable(job);
//        pool->start(runnable);
    }
}

/************************************************
 *
 ************************************************/
void DiskPipeline::encoderProgress(quint64 trackId, Track::Status status, int percent)
{
    for (int i=0; i<mDisk->count(); ++i)
    {
        Track *track = mDisk->track(i);
        if (track->id() == trackId)
            track->setProgress(status, percent);
    }
}


/************************************************

 ************************************************/
//void DiskPipeline::trackProgress(const Track *track, Track::Status status, int percent)
//{
////    if (mData->interrupted)
////        return;

////    if (!track)
////        return;

////    mData->trackStatuses.insert(track, status);
////    const_cast<Track*>(track)->setProgress(status, percent);
//}

