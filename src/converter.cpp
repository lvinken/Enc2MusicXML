#include <QtDebug>
#include <QFile>

#include "converter.h"
#include "encfile.h"
#include "mxmlconverter.h"

//---------------------------------------------------------
// read_file - read an Encore file into ef
//---------------------------------------------------------

// copied from main.cpp
// TODO: remove duplicated code

static QString read_file(const QString& filename, EncFile& ef)
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
        return "ERROR: ZBOT format detected.";
    }

    QDataStream data(&fileData, QIODevice::ReadOnly);
    ef.read(data);
}

void Converter::convert(const QUrl& inUrl, const QUrl& outUrl){
    qDebug("Converter: string inUrl %s outUrl %s", qPrintable(inUrl.toString()), qPrintable(outUrl.toString()));
    qDebug("Converter: displaystring inUrl %s outUrl %s", qPrintable(inUrl.toDisplayString()), qPrintable(outUrl.toDisplayString()));
    qDebug("Converter: localfile inUrl %s outUrl %s", qPrintable(inUrl.toLocalFile()), qPrintable(outUrl.toLocalFile()));
    EncFile ef;
    m_result = read_file(inUrl.toLocalFile(), ef);
    if (m_result != "") {
        return;
    }
    QFile outFile(outUrl.toLocalFile());
    outFile.open(QFile::WriteOnly);
    MxmlConverter mf(ef, &outFile);
    mf.convertEncToMxml();
}
