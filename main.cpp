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

#include <QCommandLineParser>
#include <QApplication>
#include <application.h>
#include "mainwindow.h"
#include "settings.h"
#include "converter/converter.h"
#include "project.h"
#include "scanner.h"
#include "consoleout.h"

#include <QString>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QLoggingCategory>

#ifdef MAC_UPDATER
#include "updater/updater.h"
#endif

static bool quiet;
static bool progress;

/************************************************
 *
 ************************************************/
void printHelp(FILE *file)
{
    QTextStream out(file);
    out << R"(
Usage: flacon [options] [file]
Flacon extracts individual tracks from one big audio file
    out << endl;

Generic options:
  -s --start                Start to convert immediately.
  -c --config <file>        Specify an alternative configuration file.
  -q --quiet                Quiet mode (no output).
  -p --progress             Show progress during conversion.
    out << endl;
  -h, --help                Show help about options
  --version                 Show version information
  --debug                   Enable debug output

Arguments:
  file                      CUE or Audio file

Environment variables:
  FLACON_DEBUG           If variable is set, flacon prints debugging information
                         to the console.)";
    out << endl;
}

/************************************************
 *
 ************************************************/
void printVersion()
{
    QTextStream out(stdout);

#ifndef GIT_BRANCH
    out << "flacon " << FLACON_VERSION << endl;
#else
    out << "flacon " << FLACON_VERSION << " + git " << GIT_BRANCH << " " << GIT_COMMIT_HASH << endl;
#endif
    out << "Copyright (c) 2013-" << QDate::currentDate().year() << " Alexander Sokolov" << endl;
    out << "   https://github.com/flacon/flacon" << endl;
    out << endl;
    out << "License LGPLv2.1+: GNU GNU Lesser General Public License version 2.1" << endl;
    out << "or later <http://www.gnu.org/licenses/lgpl-2.1.html>." << endl;
    out << "This is free software: you are free to change and redistribute it." << endl;
    out << "There is NO WARRANTY, to the extent permitted by law." << endl;
}

/************************************************
 *
 ************************************************/
void consoleErroHandler(const QString &message)
{
    QString msg(message);
    msg.remove(QRegExp("<[^>]*>"));
    msg.replace("\\n", "\n");
    QTextStream(stderr) << msg.toLocal8Bit() << endl;
}

/************************************************
 *
 ************************************************/
void guiErrorHandler(const QString &message)
{
    consoleErroHandler(message);
    QString msg(message);
    msg.replace("\n", "<br>");
    msg.replace(" ", "&nbsp;");
    QMessageBox::critical(nullptr, QObject::tr("Flacon", "Error"), "<html>" + msg + "</html>");
}

/************************************************
 *
 ************************************************/
void translate(QApplication *app)
{
#ifdef MAC_BUNDLE
    QString appDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#else
    QString appDir = TRANSLATIONS_DIR;
#endif

    QString locale = QLocale::system().name();

    QTranslator *qtTranslator = new QTranslator(app);
    qtTranslator->load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app->installTranslator(qtTranslator);

    QTranslator *appTranslator = new QTranslator(app);
    appTranslator->load(QString("flacon_%2.qm").arg(locale)) || appTranslator->load(QString("%1/flacon_%2.qm").arg(appDir, locale));
    app->installTranslator(appTranslator);
}

/************************************************
 *
 ************************************************/
int runConsole(int argc, char *argv[], const QStringList &files)
{
    QCoreApplication app(argc, argv);

    auto addFile = [&](const QString &file, bool showError = false) {
        try {
            QFileInfo fi = QFileInfo(file);
            DiscList  discs;
            if (fi.size() > 102400)
                discs << project->addAudioFile(file);
            else
                discs << project->addCueFile(file);
        }

        catch (FlaconError &err) {
            if (showError)
                qWarning() << "Error: " << err.what();
        }
    };

    for (QString file : files) {
        QFileInfo fi = QFileInfo(file);

        if (fi.isDir()) {
            Scanner scanner;
            scanner.connect(&scanner, &Scanner::found, addFile);
            scanner.start(fi.absoluteFilePath());
        }
        else {
            addFile(file, true);
        }
    }

    if (project->count() == 0)
        return 10;

    ConsoleOut out;
    Converter  converter;
    if (!quiet) {
        QObject::connect(&converter, &Converter::started,
                         &out, &ConsoleOut::converterStarted);

        QObject::connect(&converter, &Converter::finished,
                         &out, &ConsoleOut::converterFinished);

        QObject::connect(&converter, &Converter::destroyed,
                         &out, &ConsoleOut::printStatistic);

        if (progress) {
            QObject::connect(&converter, &Converter::trackProgress,
                             &out, &ConsoleOut::trackProgress);
        }
    }

    app.connect(&converter, SIGNAL(finished()),
                &app, SLOT(quit()));

    converter.start(Settings::i()->currentProfile());
    if (!converter.isRunning())
        return 11;

    return app.exec();
}

/************************************************
 *
 ************************************************/
int runGui(int argc, char *argv[], const QStringList &files)
{
    Application app(argc, argv);
    translate(&app);

    MainWindow window;

    foreach (QString file, files)
        window.addFileOrDir(file);

    QObject::connect(&app, SIGNAL(openFile(QString)),
                     &window, SLOT(addFileOrDir(QString)));

    window.show();

#ifdef MAC_UPDATER
    Updater &updater = Updater::sharedUpdater();
    if (updater.automaticallyChecksForUpdates())
        updater.checkForUpdatesInBackground();
#endif

    return app.exec();
}

/************************************************
 *
 ************************************************/
int main(int argc, char *argv[])
{
    initTypes();
    QCommandLineParser parser;

    parser.addPositionalArgument("file", "CUE or Audio file.");

    parser.addOption(QCommandLineOption(QStringList() << "h"
                                                      << "help",
                                        ""));
    parser.addOption(QCommandLineOption("version", ""));
    parser.addOption(QCommandLineOption(QStringList() << "s"
                                                      << "start",
                                        ""));
    parser.addOption(QCommandLineOption(QStringList() << "c"
                                                      << "config",
                                        "", "config file"));
    parser.addOption(QCommandLineOption(QStringList() << "q"
                                                      << "quiet",
                                        ""));
    parser.addOption(QCommandLineOption(QStringList() << "p"
                                                      << "progress",
                                        ""));
    parser.addOption(QCommandLineOption("debug", ""));

    QStringList args;
    for (int i = 0; i < argc; ++i)
        args << QString::fromLocal8Bit(argv[i]);

    if (!parser.parse(args)) {
        QTextStream(stderr) << parser.errorText() << endl
                            << endl;
        printHelp(stderr);
        return 1;
    }

    if (parser.isSet("help")) {
        printHelp(stdout);
        return 0;
    }

    if (parser.isSet("version")) {
        printVersion();
        return 0;
    }

    if (!parser.value("config").isEmpty()) {
        Settings::setFileName(parser.value("config"));
    }

    if (parser.isSet("debug") || getenv("FLACON_DEBUG")) {
        qSetMessagePattern(":::%{if-category}%{category}: %{endif}%{if-warning}Warning: %{endif}%{if-critical}Error: %{endif}%{if-fatal}Error: %{endif} [%{threadid}] %{message}");
    }
    else {
        qSetMessagePattern("%{if-warning}Warning: %{endif}%{if-critical}Error: %{endif}%{if-fatal}Error: %{endif}%{message}");
        QLoggingCategory::setFilterRules("*.debug=false");
    }

    quiet    = parser.isSet("quiet");
    progress = parser.isSet("progress");

#ifndef GIT_BRANCH
    qInfo() << "Start flacon " << FLACON_VERSION;
#else
    qInfo() << "Start flacon " << FLACON_VERSION << " + git " << GIT_BRANCH << " " << GIT_COMMIT_HASH;
#endif

    if (parser.isSet("start"))
        return runConsole(argc, argv, parser.positionalArguments());
    else
        return runGui(argc, argv, parser.positionalArguments());
}
