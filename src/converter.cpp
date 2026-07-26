#include "converter.h"

static int count { -1 };

void Converter::convert(const QUrl& inUrl, const QUrl& outUrl){
    qDebug("Converter: string inUrl %s outUrl %s", qPrintable(inUrl.toString()), qPrintable(outUrl.toString()));
    qDebug("Converter: displaystring inUrl %s outUrl %s", qPrintable(inUrl.toDisplayString()), qPrintable(outUrl.toDisplayString()));
    qDebug("Converter: localfile inUrl %s outUrl %s", qPrintable(inUrl.toLocalFile()), qPrintable(outUrl.toLocalFile()));
    ++count;
    qDebug("Converter: count %d", count);
    QString res;
    switch (count % 3) {
    case 0: res = "success"; break;
    case 1: res = "read error"; break;
    case 2: res = "write error"; break;
    default: res = "<none>"; break;
    }
    qDebug("Converter: res '%s' m_result '%s'", qPrintable(res), qPrintable(m_result));
    if (res != m_result) {
        m_result = res;
        qDebug("Converter: m_result %s", qPrintable(m_result));
        emit resultChanged(m_result);
    }
}
