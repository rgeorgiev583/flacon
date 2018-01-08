#ifndef JOB_H
#define JOB_H

#include "types.h"
#include "cue.h"
#include "outformat.h"

struct Job
{
    quint64 trackId;
    QString inputFile;
    QString outputFile;
    CueIndex start;
    CueIndex end;
    AudioQuality quality;
    QString format;
    Tags    tags;
};

#endif // JOB_H
