/*****************************************************************************/
/*  Enc2MusicXML - Converts musical notation from Encore to MusicXML.        */
/*  Copyright (C) 2018 Leon Vinken                                           */
/*                                                                           */
/*  This program is free software; you can redistribute it and/or modify     */
/*  it under the terms of the GNU General Public License as published by     */
/*  the Free Software Foundation; either version 3, or (at your option)      */
/*  any later version.                                                       */
/*                                                                           */
/*  This program is distributed in the hope that it will be useful,          */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/*  GNU General Public License for more details.                             */
/*                                                                           */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program; if not, write to the Free Software Foundation,  */
/*  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.           */
/*****************************************************************************/

#include <QDataStream>
#include <QFile>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include <QtDebug>

#include "analysisfile.h"
#include "converter.h"
#include "encfile.h"
#include "mxmlconverter.h"
#include "textfile.h"

static const QString applicationName { "Enc2MusicXML" };
static const QString applicationVersion { "0.7" };

//---------------------------------------------------------
// read_file - read an Encore file into ef
//---------------------------------------------------------

static void read_file(const QString& filename, EncFile& ef)
{
    qDebug() << "processing file" << filename;
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QByteArray fileData = file.readAll();
    file.close();

    // Detectar formato ZBOT (cifrado) - magic "ZBOT" = 0x5A424F54
    if (fileData.size() >= 4 && fileData.startsWith("ZBOT")) {
        qWarning() << "ERROR: ZBOT format detected.";
        qWarning() << "ZBOT files are encrypted and cannot be converted directly.";
        qWarning() << "Please convert the file to SCOW format using Encore 5.x:";
        qWarning() << "  1. Open the .enc file in Encore 5.x on Windows";
        qWarning() << "  2. Save it (the new version will be in SCOW format)";
        qWarning() << "  3. Use Enc2MusicXML with the converted file";
        return;
    }

    QDataStream data(&fileData, QIODevice::ReadOnly);
    ef.read(data);
}

//---------------------------------------------------------
// main - handle command line arguments
//---------------------------------------------------------

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(applicationName);
    QGuiApplication::setApplicationVersion(applicationVersion);

    QCommandLineParser clp;
    clp.setApplicationDescription("Enc2MusicXML converts Encore files to MusicXML.");
    clp.addHelpOption();
    clp.addVersionOption();
    clp.addOptions({
        {{"a", "analyse"},
         QCoreApplication::translate("main", "Analyse file(s).")},
        {{"d", "dump"},
         QCoreApplication::translate("main", "Dump file(s). Similar to enc2ly's --dump option.")},
        {{"m", "convert-to-MusicXML"},
         QCoreApplication::translate("main", "Convert file(s) to MusicXML format.")},
        });
    clp.process(app);
    if (clp.isSet("h")
        || (clp.isSet("a") && clp.positionalArguments().count() < 1)
        || (clp.isSet("d") && clp.positionalArguments().count() < 1)
        || (clp.isSet("m") && clp.positionalArguments().count() < 1)
        || (!clp.isSet("a") && !clp.isSet("d") && !clp.isSet("m") && clp.positionalArguments().count() != 0)) {
        clp.showHelp();
        Q_UNREACHABLE();
    }
    if (clp.isSet("a")) {
        for (const auto& s : clp.positionalArguments()) {
            EncFile ef;
            read_file(s, ef);
            AnalysisFile af(ef);
            af.write();
        }
    }
    else if (clp.isSet("d")) {
        for (const auto& s : clp.positionalArguments()) {
            EncFile ef;
            read_file(s, ef);
            TextFile tf(ef);
            tf.write();
        }
    }
    else if (clp.isSet("m")) {
        for (const auto& s : clp.positionalArguments()) {
            EncFile ef;
            read_file(s, ef);
            QFile outFile;
            outFile.open(stdout, QFile::WriteOnly);
            MxmlConverter mf(ef, &outFile);
            mf.convertEncToMxml();
        }
    }
    else {
        qDebug() << "main() using GUI";
        QQmlApplicationEngine engine;
        Converter converter;
        engine.rootContext()->setContextProperty("converter", &converter);
        engine.rootContext()->setContextProperty("applicationName", applicationName);
        engine.rootContext()->setContextProperty("applicationVersion", applicationVersion);
        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
        if (engine.rootObjects().isEmpty()) {
            qDebug() << "no root objects";
        }
        return app.exec();
    }

    return 0;
}
