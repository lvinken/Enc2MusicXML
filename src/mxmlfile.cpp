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

#include <QFile>
#include <QtDebug>

#include "encfile.h"
#include "mxmlfile.h"


//---------------------------------------------------------
// faceValue2duration - convert Encore note type to duration
//---------------------------------------------------------

static int faceValue2duration(const quint8 faceValue)
{
    switch (faceValue) {
    //case 0: return "0";
    case 1: return 1920;
    case 2: return  960;
    case 3: return  480;
    case 4: return  240;
    case 5: return  120;
    case 6: return   60;
    case 7: return   30;
    case 8: return   15;
    }
    return 0;
}


//---------------------------------------------------------
// faceValue2xml - convert Encore to MusicXML note type
//---------------------------------------------------------

static QString faceValue2xml(const quint8 faceValue)
{
    switch (faceValue) {
    //case 0: return "0";
    case 1: return "whole";
    case 2: return "half";
    case 3: return "quarter";
    case 4: return "eighth";
    case 5: return "16th";
    case 6: return "32nd";
    case 7: return "64th";
    case 8: return "128th";
    }
    return "???";
}

//---------------------------------------------------------
// midipitch2xml
//---------------------------------------------------------

static int alterTab[12] = { 0,   1,   0,   1,   0,  0,   1,   0,   1,   0,   1,   0 };
static char noteTab[12] = { 'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B' };

static void midipitch2xml(const quint8 pitch, char& step, int& alter, int& octave)
{
    // 60 = C 4
    step   = noteTab[pitch % 12];
    alter  = alterTab[pitch % 12];
    octave = pitch / 12 - 1;
    //qDebug("midipitch2xml(pitch %d) step %c, alter %d, octave %d", pitch, c, alter, octave);
}


//---------------------------------------------------------
// TupletHandler - handle tuplet state
//---------------------------------------------------------

TupletHandler::type TupletHandler::newNote(const quint8 actualNotes, const quint8 normalNotes, const quint8 faceValue)
{
    TupletHandler::type res { TupletHandler::type::NONE };
    char const * printableRes { nullptr };
    if (actualNotes <= 0 || normalNotes <= 0) {
        printableRes = "none";
        m_count = 0;
    }
    else {
        if (m_count == 0) {
            printableRes = "start";
            res = TupletHandler::type::START;
            ++m_count;
            m_value = faceValue;
        }
        else {
            int count = 1;
            int value = faceValue;
            while (value > m_value) {
                m_count *= 2;
                ++m_value;
            }
            while (m_value > value) {
                count *= 2;
                ++value;
            }
            m_count += count;
            if (m_count >= actualNotes) {
                printableRes = "stop";
                res = TupletHandler::type::STOP;
                m_count = 0;
            }
            else {
                res = TupletHandler::type::MID;
            }
        }
    }
    /*
    qDebug()
            << "TupletHandler::newNote()"
            << "actualNotes" << actualNotes
            << "normalNotes" << normalNotes
            << "faceValue" << faceValue
            << "res" << printableRes
               ;
               */
    return res;
}


//---------------------------------------------------------
// MxmlFile - write MusicXML
//---------------------------------------------------------

MxmlFile::MxmlFile(const EncFile& ef)
    : m_ef(ef)
{

}


void MxmlFile::write()
{
    qDebug() << "MxmlFile::write()";
    QFile outFile;
    outFile.open(stdout, QFile::WriteOnly);
    m_out.setDevice(&outFile);
    m_out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          << "<!DOCTYPE score-partwise PUBLIC"
          << " \"-//Recordare//DTD MusicXML 3.1 Partwise//EN\""
          << " \"http://www.musicxml.org/dtds/partwise.dtd\">\n";
    m_out << "<score-partwise>\n";
    writeWork();
    writeIdentification();
    writePartList();
    writeParts();
    m_out << "</score-partwise>\n";
}


void MxmlFile::writeAttributes()
{
    m_out << "      <attributes>\n";
    m_out << "        <divisions>" << 480 << "</divisions>\n";
    writeKey();
    writeTime();
    writeClef();
    m_out << "      </attributes>\n";
}


void MxmlFile::writeClef()
{
    // TBD (too) simple implementation: use timesig of first measure only
    // m_ef.measures().at(0).m_timeSigNum
    const bool hasMeasures = m_ef.measures().size() > 0;
    if (hasMeasures) {
        m_out << "        <clef>\n";
        m_out << "          <sign>G</sign>\n";
        m_out << "          <line>2</line>\n";
        m_out << "        </clef>\n";
    }
}


void MxmlFile::writeIdentification()
{
    m_out << "  <identification>\n";
    const EncTitle& ttl = m_ef.title();
    if (ttl.m_author.size() > 0)
        m_out << "    <creator type=\"composer\">" << ttl.m_author.at(0) << "</creator>\n";
    if (ttl.m_copyright.size() > 0)
        m_out << "    <rights>" << ttl.m_copyright.at(0) << "</rights>\n";
    m_out << "    <encoding>\n";
    m_out << "      <software>Enc2MusicXML</software>\n";
    // TODO fill in real date
    // m_out << "      <encoding-date>TBD</encoding-date>\n";
    m_out << "    </encoding>\n";
    m_out << "  </identification>\n";
}


int encKeyToFifths(unsigned int key)
{
    std::vector<int> v =
    {
        //   c   f  bf  ef  af  df  gf  cf  g   d   a   e   b  fs  cs
        /**/ 0, -1, -2, -3, -4, -5, -6, -7, 1,  2,  3,  4,  5,  6,  7
    };
    return v.at(key);
}


void MxmlFile::writeKey()
{
    // TBD (too) simple implementation: use keysig of first staff of first measure only
    const bool hasMeasures = m_ef.measures().size() > 0;
    if (hasMeasures) {
        const auto& line = m_ef.lines().at(0);
        const auto& data = line.lineStaffData().at(0);
        quint8 kcType = data.m_key;
        const auto fifths = encKeyToFifths(kcType);
        m_out << "        <key>" << endl;
        m_out << "          <fifths>" << fifths << "</fifths>" << endl;
        m_out << "        </key>" << endl;
    }
}


const EncMeasureElemKeyChange* findKeyChange(const EncMeasure& m)
{
    for (const auto elem : m.measureElems()) {
        if (const EncMeasureElemKeyChange* const key = dynamic_cast<const EncMeasureElemKeyChange* const>(elem))
            return key;
    }
    return nullptr;
}


void MxmlFile::writeKeyChange(const EncMeasureElemKeyChange* keyChange)
{
    quint8 kcType = keyChange->m_tipo;
    const auto fifths = encKeyToFifths(kcType);
    qDebug()
            << "writeKeyChange"
            << "kcType" << kcType
            << "fifths" << fifths
               ;
    m_out << "      <attributes>" << endl;
    m_out << "        <key>" << endl;
    m_out << "          <fifths>" << fifths << "</fifths>" << endl;
    m_out << "        </key>" << endl;
    m_out << "      </attributes>" << endl;
}


void MxmlFile::writeMeasure(const size_t measureNr)
{
    if (measureNr >= m_ef.measures().size())
        return;

    m_out << "    <measure number=\"" << measureNr + 1 << "\">\n";

    const auto& m = m_ef.measures().at(measureNr);
    const auto keyChange = findKeyChange(m);

    if (measureNr == 0)
        writeAttributes();
    else if (keyChange)
        writeKeyChange(keyChange);

    TupletHandler th;
    for (int i = 0; i < 1 /*m_ef.header().m_staffPerSystem*/; ++i) {
        std::multimap<quint8 , const EncMeasureElem* const> mmap;
        for (const auto e : m.measureElems()) {
            if (i == e->m_staffIdx)
                mmap.insert({e->m_xoffset, e});
        }
        for (const auto& e : mmap) {
            const auto elem = e.second;
            if (const EncMeasureElemNote* const note = dynamic_cast<const EncMeasureElemNote* const>(elem)) {
                writeNote(note, th);
            }
            else if (const EncMeasureElemRest* const rest = dynamic_cast<const EncMeasureElemRest* const>(elem)) {
                writeRest(rest, th);
            }
        }
    }

    // TODO fix overly simplistic implementation
    if (m.m_barTypeEnd == 5) {
        m_out << "      <barline location=\"right\">\n";
        m_out << "        <bar-style>light-heavy</bar-style>\n";
        m_out << "      </barline>\n";
    }

    m_out << "    </measure>" << endl;
}


void MxmlFile::writeNote(const EncMeasureElemNote* const note, TupletHandler& th)
{
    char step = ' ';
    int alter = 0;
    int octave = 0;
    midipitch2xml(note->m_semiTonePitch, step, alter, octave);
    m_out << "      <note>\n";
    m_out << "        <pitch>\n";
    m_out << "          <step>" << step << "</step>\n";
    if (alter) m_out << "          <alter>" << alter << "</alter>\n";
    m_out << "          <octave>" << octave << "</octave>\n";
    m_out << "        </pitch>\n";

    int duration = faceValue2duration(note->m_faceValue & 0x0F);
    for (int i = 0; i < (note->m_dotControl & 3); ++i) {
        duration *= 3;
        duration /= 2;
    }
    if (note->actualNotes() > 0 && note->normalNotes() > 0) {
        duration *= note->normalNotes();
        duration /= note->actualNotes();
    }
    m_out << "        <duration>" << duration << "</duration>\n";
    m_out << "        <type>" << faceValue2xml(note->m_faceValue & 0x0F) << "</type>\n";
    for (int i = 0; i < (note->m_dotControl & 3); ++i)
        m_out << "        <dot/>\n";
    if (note->actualNotes() > 0 && note->normalNotes() > 0) {
        m_out << "        <time-modification>\n";
        m_out << "          <actual-notes>" << note->actualNotes() << "</actual-notes>\n";
        m_out << "          <normal-notes>" << note->normalNotes() << "</normal-notes>\n";
        m_out << "        </time-modification>\n";
    }
    const auto tupletState = th.newNote(note->actualNotes(), note->normalNotes(), note->m_faceValue & 0x0F);
    if (tupletState == TupletHandler::type::START || tupletState == TupletHandler::type::STOP) {
        m_out << "        <notations>\n";
        if (tupletState == TupletHandler::type::START)
            m_out << "          <tuplet type=\"start\"/>\n";
        if (tupletState == TupletHandler::type::STOP)
            m_out << "          <tuplet type=\"stop\"/>\n";
        m_out << "        </notations>\n";
    }
    m_out << "      </note>\n";
}


void MxmlFile::writePart(const int n)
{
    m_out << "  <part id=\"P" << n + 1 << "\">\n";
    for (unsigned int i = 0; i < m_ef.measures().size(); ++i)
        writeMeasure(i);
    m_out << "  </part>" << endl;
}


void MxmlFile::writePartList()
{
    m_out << "  <part-list>\n";
    int count = 0;
    for (const auto& s : m_ef.staves()) {
        ++count;
        writeScorePart(count, s);
    }
    m_out << "  </part-list>\n";
}


void MxmlFile::writeParts()
{
    for (unsigned int count = 0; count < m_ef.staves().size(); ++count) {
        writePart(count);
    }
}


void MxmlFile::writeRest(const EncMeasureElemRest* const rest, TupletHandler &th)
{
    m_out << "      <note>\n";
    m_out << "        <rest/>\n";
    int duration = faceValue2duration(rest->m_faceValue & 0x0F);
    for (int i = 0; i < (rest->m_dotControl & 3); ++i) {
        duration *= 3;
        duration /= 2;
    }
    if (rest->actualNotes() > 0 && rest->normalNotes() > 0) {
        duration *= rest->normalNotes();
        duration /= rest->actualNotes();
    }
    m_out << "        <duration>" << duration << "</duration>\n";
    m_out << "        <type>" << faceValue2xml(rest->m_faceValue & 0x0F) << "</type>\n";
    for (int i = 0; i < (rest->m_dotControl & 3); ++i)
        m_out << "        <dot/>\n";
    if (rest->actualNotes() > 0 && rest->normalNotes() > 0) {
        m_out << "        <time-modification>\n";
        m_out << "          <actual-notes>" << rest->actualNotes() << "</actual-notes>\n";
        m_out << "          <normal-notes>" << rest->normalNotes() << "</normal-notes>\n";
        m_out << "        </time-modification>\n";
    }
    const auto tupletState = th.newNote(rest->actualNotes(), rest->normalNotes(), rest->m_faceValue & 0x0F);
    if (tupletState == TupletHandler::type::START || tupletState == TupletHandler::type::STOP) {
        m_out << "        <notations>\n";
        if (tupletState == TupletHandler::type::START)
            m_out << "          <tuplet type=\"start\"/>\n";
        if (tupletState == TupletHandler::type::STOP)
            m_out << "          <tuplet type=\"stop\"/>\n";
        m_out << "        </notations>\n";
    }
    m_out << "      </note>\n";
}


void MxmlFile::writeScorePart(const int n, const EncStaff& staff)
{
    m_out << "    <score-part id=\"P" << n << "\">\n";
    m_out << "      <part-name>" << staff.m_name << "</part-name>\n";
    m_out << "    </score-part>\n";
}


void MxmlFile::writeTime()
{
    // (too) simple implementation: use timesig of first measure only
    const bool hasMeasures = m_ef.measures().size() > 0;
    if (hasMeasures) {
        m_out << "        <time>\n";
        m_out << "          <beats>" << m_ef.measures().at(0).m_timeSigNum << "</beats>\n";
        m_out << "          <beat-type>" << m_ef.measures().at(0).m_timeSigDen << "</beat-type>\n";
        m_out << "        </time>\n";
    }
}


void MxmlFile::writeWork()
{
    m_out << "  <work>\n";
    const EncTitle& ttl = m_ef.title();
    m_out << "    <work-title>" << ttl.m_title << "</work-title>\n";
    m_out << "  </work>\n";
}
