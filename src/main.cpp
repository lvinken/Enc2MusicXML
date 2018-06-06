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

#include <QCoreApplication>
#include <QDataStream>
#include <QFile>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include <QtDebug>

#include "encfile.h"
#include "mxmlfile.h"
#include "textfile.h"


//---------------------------------------------------------
// read_file - read an Encore file into ef
//---------------------------------------------------------

static void read_file(const QString& filename, EncFile& ef)
{
    qDebug() << "processing file" << filename;
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QDataStream data(&file);
    ef.read(data);
}

//---------------------------------------------------------
// main - handle command line arguments
//---------------------------------------------------------

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Enc2MusicXML");
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser clp;
    clp.setApplicationDescription("Enc2MusicXML converts Encore files to LilyPond.");
    clp.addHelpOption();
    clp.addOptions({
                       {{"a", "analyse"},
                        QCoreApplication::translate("main", "Analyse file(s).")},
                       {{"m", "convert-to-MusicXML"},
                        QCoreApplication::translate("main", "Convert file(s) to MusicXML format.")},
                   });
    clp.process(app);
    if (clp.isSet("h") || (!clp.isSet("a") && !clp.isSet("m"))) {
        clp.showHelp();
        Q_UNREACHABLE();
    }
    if (clp.isSet("a")) {
        for (const auto& s : clp.positionalArguments()) {
            EncFile ef;
            read_file(s, ef);
            TextFile tf(ef);
            tf.write();
        }
    }
    if (clp.isSet("m")) {
        for (const auto& s : clp.positionalArguments()) {
            EncFile ef;
            read_file(s, ef);
            MxmlFile mf(ef);
            mf.write();
        }
    }

    return 0;
}
