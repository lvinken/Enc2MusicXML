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

static bool encClef2xml(const clefType ct, QString& sign, int& line, int& octCh)
{
    sign = "";
    line = 0;
    octCh = 0;

    bool res { true };

    switch (ct) {
    case clefType::G: sign = "G"; line = 2; break;
    case clefType::F: sign = "F"; line = 4; break;
    case clefType::C3L: sign = "C"; line = 3; break;
    case clefType::C4L: sign = "C"; line = 4; break;
    case clefType::G8P: sign = "G"; line = 2; octCh = 1; break;
    case clefType::G8M: sign = "G"; line = 2; octCh = -1; break;
    case clefType::F8M: sign = "F"; line = 4; octCh = -1; break;
    case clefType::PERC: sign = "percussion"; line = 2; break;
    case clefType::TAB: sign = "TAB"; line = 5; break;
    default:
        res = false;
        qDebug()
                << "encClef2xml: clef type"
                << static_cast<int>(ct)
                << "not supported"
                   ;
        break;
    }

    qDebug()
            << "xxx_encClef2xml"
            << "clef type" << static_cast<int>(ct)
            << "res" << res
            << "sign" << sign
            << "line" << line
            << "octCh" << octCh
               ;

    return res;
}

//---------------------------------------------------------
// midipitch2xml
//---------------------------------------------------------

static int alterTab[12] = { 0,   1,   0,   1,   0,  0,   1,   0,   1,   0,   1,   0 };
static char noteTab[12] = { 'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B' };

// determine pitch spelling based on pitch, accidental and key
// (too) simple algorithm: fails e.g. for two b flats with same pitch in one measure:
// - first (with accid) will be OK
// - second (without accid) will not

static void midipitch2xml(const quint8 pitch, const accidentalType accid, const int fifths,
                          char& step, int& alter, int& octave)
{
    // 60 = C 4
    if (accid == accidentalType::FLAT) {
        // explicit flat
        step   = noteTab[(pitch + 1) % 12];
        alter  = -1;
        octave = (pitch + 1) / 12 - 1;
    }
    else if (alterTab[pitch % 12] && accid == accidentalType::NONE && fifths < 0) {
        // need alter but no accid -> choose flat if fifths < 0
        step   = noteTab[(pitch + 1) % 12];
        alter  = -1;
        octave = (pitch + 1) / 12 - 1;
    }
    else {
        // all others none or sharp
        step   = noteTab[pitch % 12];
        alter  = alterTab[pitch % 12];
        octave = pitch / 12 - 1;
    }
    qDebug("xxx_midipitch2xml(pitch %d, accid %d, fifths %d) step %c, alter %d, octave %d",
           pitch, static_cast<unsigned int>(accid), fifths, step, alter, octave);
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
// MxmlFile - MusicXML file writer constructor
//---------------------------------------------------------

MxmlFile::MxmlFile(const EncFile& ef)
    : m_ef(ef)
{

}


//---------------------------------------------------------
// write - write the MusicXML file
//---------------------------------------------------------

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


//---------------------------------------------------------
// writeAttributes - write the attributes
//---------------------------------------------------------

void MxmlFile::writeAttributes(const int idx)
{
    m_out << "      <attributes>\n";
    m_out << "        <divisions>" << 480 << "</divisions>\n";
    writeKey();
    writeTime();
    writeClef(idx);
    m_out << "      </attributes>\n";
}


//---------------------------------------------------------
// Determine if measure measureNr is the first measure in an alternative ending
//---------------------------------------------------------

static bool isFirstMeasureInAltEnd(const MeasureVec& measures, const size_t measureNr)
{
    const auto repAlt = measures.at(measureNr).m_repeatAlternative;
    if (repAlt == 0)
        return false;

    if (measureNr == 0)
        return true;

    if (measureNr > 0 && measures.at(measureNr - 1).m_repeatAlternative != repAlt)
        return true;

    return false;
}


//---------------------------------------------------------
// Determine if measure measureNr is the last measure in an alternative ending
//---------------------------------------------------------

static bool isLastMeasureInAltEnd(const MeasureVec& measures, const size_t measureNr)
{
    const auto repAlt = measures.at(measureNr).m_repeatAlternative;
    if (repAlt == 0)
        return false;

    if (measureNr == measures.size() - 1)
        return true;

    if (measureNr < measures.size() - 1 && measures.at(measureNr + 1).m_repeatAlternative != repAlt)
        return true;

    return false;
}


//---------------------------------------------------------
// repeatAlternative2EndingNumber - convert repeatAlternative (one bit for each pass)
// to MusicXML ending-number (a comma-separated list// of positive integers without leading zeros)
//---------------------------------------------------------

static QString repeatAlternative2EndingNumber(const quint8 repeatAlternative)
{
    const int max = 4;
    int mask = 1;
    QString res;
    for (int i = 1; i <= max; ++i) {
        if (repeatAlternative & mask) {
            if (res.isEmpty())
                res.setNum(i);
            else
                res += QString(", %1").arg(i);
        }
        mask <<= 1;
    }
    return res;
}


//---------------------------------------------------------
// writeBarlineLeft - write left barline
//---------------------------------------------------------

void MxmlFile::writeBarlineLeft(const int partNr, const size_t measureNr)
{
    const auto& m = m_ef.measures().at(measureNr);

    const bool repeatStart = (m.barTypeStart() == barlineType::REPEATSTART);
    const bool barlineDblLeft = (m.barTypeStart() == barlineType::DOUBLEL);
    const bool endingStart = isFirstMeasureInAltEnd(m_ef.measures(), measureNr) && partNr == 0;
    if (repeatStart || endingStart || barlineDblLeft) {
        m_out << "      <barline location=\"left\">\n";
        if (repeatStart) {
            m_out << "        <bar-style>heavy-light</bar-style>\n";
            m_out << "        <repeat direction=\"forward\"/>\n";
        }
        if (barlineDblLeft) {
            m_out << "          <bar-style>light-light</bar-style>\n";
        }
        if (endingStart) {
            m_out << "        <ending number=\"" << repeatAlternative2EndingNumber(m.m_repeatAlternative) << "\" type=\"start\"/>\n";
        }
        m_out << "      </barline>\n";
    }
}


//---------------------------------------------------------
// writeBarlineRight - write right barline
//---------------------------------------------------------

void MxmlFile::writeBarlineRight(const int partNr, const size_t measureNr)
{
    const auto& m = m_ef.measures().at(measureNr);

    const bool repeatEnd = (m.barTypeEnd() == barlineType::REPEATEND);
    const bool barlineEnd = (m.barTypeEnd() == barlineType::FINAL);
    const bool barlineDbl = (m.barTypeEnd() == barlineType::DOUBLER);
    const bool endingStop = isLastMeasureInAltEnd(m_ef.measures(), measureNr) && partNr == 0;
    if (repeatEnd || endingStop || barlineEnd || barlineDbl) {
        m_out << "      <barline location=\"right\">\n";
        if (repeatEnd || barlineEnd) {
            m_out << "        <bar-style>light-heavy</bar-style>\n";
        }
        if (barlineDbl) {
            m_out << "        <bar-style>light-light</bar-style>\n";
        }
        if (endingStop) {
            QString type = m.m_repeatAlternative == 1 ? "stop" : "discontinue";
            m_out << "        <ending number=\"" << m.m_repeatAlternative << "\" type=\"" << type << "\"/>\n";
        }
        if (repeatEnd || m.m_repeatAlternative == 1) {
            m_out << "        <repeat direction=\"backward\"/>\n";
        }
        m_out << "      </barline>\n";
    }
}


//---------------------------------------------------------
// writeClef - write the clef for staff idx
//---------------------------------------------------------

void MxmlFile::writeClef(const int idx)
{
    // TBD (too) simple implementation: use clef of first measure only
    const bool hasMeasures = m_ef.measures().size() > 0;
    if (hasMeasures) {
        const auto& encline = m_ef.lines().at(0);   // first system
        const auto& data = encline.lineStaffData().at(idx);
        const auto ct = data.m_clef;
        QString sign;
        int line { 0 };
        int octCh { 0 };
        if (encClef2xml(ct, sign, line, octCh)) {
            m_out << "        <clef>\n";
            m_out << QString("          <sign>%1</sign>\n").arg(sign);
            m_out << QString("          <line>%1</line>\n").arg(line);
            if (octCh) {
                m_out << QString("          <clef-octave-change>%1</clef-octave-change>\n").arg(octCh);
            }
            m_out << "        </clef>\n";
        }
    }
}


//---------------------------------------------------------
// createMultiLineString - convert a vector of strings
// to a single newline separated string
//---------------------------------------------------------

static QString createMultiLineString(const std::vector<QString> strVec)
{
    QString res;
    if (strVec.size() > 0) {
        res += strVec.at(0);
        for (size_t i = 1; i < strVec.size(); ++i) {
            if (!strVec.at(i).isEmpty()) {
                res += "\n";
                res += strVec.at(i);
            }
        }
    }
    return res;
}


//---------------------------------------------------------
// writeIdentification - write the identification
//---------------------------------------------------------

void MxmlFile::writeIdentification()
{
    m_out << "  <identification>\n";
    const EncTitle& ttl = m_ef.title();
    const auto author = createMultiLineString(ttl.m_author);
    if (!author.isEmpty()) {
        m_out << "    <creator type=\"composer\">" << author << "</creator>\n";
    }
    const auto lyricist = createMultiLineString(ttl.m_instruction);
    if (!lyricist.isEmpty()) {
        m_out << "    <creator type=\"lyricist\">" << lyricist << "</creator>\n";
    }
    const auto rights = createMultiLineString(ttl.m_copyright);
    if (!rights.isEmpty()) {
        m_out << "    <rights>" << rights << "</rights>\n";
    }
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


//---------------------------------------------------------
// writeKey - write the key
// TBD (too) simple implementation: use keysig of first staff of first measure only
// TODO: remove duplicate code
//---------------------------------------------------------

void MxmlFile::writeKey()
{
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


//---------------------------------------------------------
// writeKeyChange - write a key change
//---------------------------------------------------------

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


static void dump_note_timing_measure_elems(const EncMeasure& m)
{
    for (const auto& elem : m.measureElems()) {
        if (const EncMeasureElemNote* const note = dynamic_cast<const EncMeasureElemNote* const>(elem)) {
            qDebug() << "xxx_note_timing"
                     << "staff" << note->m_staffIdx
                     << "voice" << note->m_voice
                     << "tick" << note->m_tick
                     << "x_offset" << note->m_xoffset
                     << "note"
                     << "value" << (note->m_faceValue & 0x0F)
                     << "dots" << (note->m_dotControl & 3)
                     << "actual" << note->actualNotes()
                     << "normal" << note->normalNotes()
                        ;
        }
        else if (const EncMeasureElemRest* const rest = dynamic_cast<const EncMeasureElemRest* const>(elem)) {
            qDebug() << "xxx_note_timing"
                     << "staff" << rest->m_staffIdx
                     << "voice" << rest->m_voice
                     << "tick" << rest->m_tick
                     << "x_offset" << rest->m_xoffset
                     << "rest"
                     << "value" << (rest->m_faceValue & 0x0F)
                     << "dots" << (rest->m_dotControl & 3)
                        ;
        }
    }
}


//---------------------------------------------------------
// writeRepeatLeft - write repeats at the left side of a measure
//---------------------------------------------------------

static void writeRepeatLeft(QTextStream& out, const repeatType repeat)
{
    const bool coda = repeat == repeatType::CODA1 || repeat == repeatType::CODA2;
    const bool segno = repeat == repeatType::SEGNO;

    if (coda || segno) {
        out << "      <direction placement=\"above\">\n";
        out << "        <direction-type>\n";
        if (coda)
            out << "          <coda/>\n";
        else if (segno)
            out << "          <segno/>\n";
        out << "        </direction-type>\n";
        out << "      </direction>\n";
    }
}


//---------------------------------------------------------
// writeRepeatRight - write repeats at the right side of a measure
//---------------------------------------------------------

static void writeRepeatRight(QTextStream& out, const repeatType repeat)
{
    QString words;

    if (repeat == repeatType::DS)
        words = "D.S.";
    else if (repeat == repeatType::DCALCODA)
        words = "D.C. al Coda";
    else if (repeat == repeatType::DCALFINE)
        words = "D.C. al Fine";
    else if (repeat == repeatType::DSALCODA)
        words = "D.S. al Coda";
    else if (repeat == repeatType::DSALFINE)
        words = "D.S. al Fine";
    else if (repeat == repeatType::DC)
        words = "D.C.";
    else if (repeat == repeatType::FINE)
        words = "Fine";

    if (!words.isEmpty()) {
        out << "      <direction placement=\"above\">\n";
        out << "        <direction-type>\n";
        out << "          <words>" << words << "</words>\n";
        out << "        </direction-type>\n";
        out << "      </direction>\n";
    }
}


//---------------------------------------------------------
// writeMeasure - write a measure of a part
// note: assumes only single staff parts
//---------------------------------------------------------

void MxmlFile::writeMeasure(const int partNr, const size_t measureNr)
{
    if (measureNr >= m_ef.measures().size())
        return;

    m_out << "    <measure number=\"" << measureNr + 1 << "\">\n";

    const auto& m = m_ef.measures().at(measureNr);
    const auto keyChange = findKeyChange(m);


    qDebug() << "xxx_note_timing"
             << "measureNr" << measureNr
                ;
    dump_note_timing_measure_elems(m);

    writeBarlineLeft(partNr, measureNr);

    if (measureNr == 0)
        writeAttributes(partNr);
    else if (keyChange)
        writeKeyChange(keyChange);

    qDebug() << "xxx_repeat_sym"
             << "measureNr" << measureNr
             << "m_coda" << m.m_coda
             << "m.repeat()" << static_cast<unsigned int>(m.repeat())
                ;

    if (partNr == 0) {
        // write repeat only for first staff
        writeRepeatLeft(m_out, m.repeat());
    }

    TupletHandler th;
    std::multimap<quint8 , const EncMeasureElem* const> mmap;
    for (const auto e : m.measureElems()) {
        if (e->m_staffIdx == partNr)
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

    if (partNr == 0) {
        // write repeat only for first staff
        writeRepeatRight(m_out, m.repeat());
    }

    writeBarlineRight(partNr, measureNr);

    m_out << "    </measure>" << endl;
}


//---------------------------------------------------------
// writeNote - write a note
//---------------------------------------------------------

void MxmlFile::writeNote(const EncMeasureElemNote* const note, TupletHandler& th)
{
    char step = ' ';
    int alter = 0;
    int octave = 0;
    int fifths = 0;
    // TBD (too) simple implementation: use keysig of first staff of first measure only
    // TODO: remove duplicate code
    const bool hasMeasures = m_ef.measures().size() > 0;
    if (hasMeasures) {
        const auto& line = m_ef.lines().at(0);
        const auto& data = line.lineStaffData().at(0);
        quint8 kcType = data.m_key;
        fifths = encKeyToFifths(kcType);
    }
    midipitch2xml(note->m_semiTonePitch, static_cast<accidentalType>(note->m_alterationGlyph), fifths, step, alter, octave);
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


//---------------------------------------------------------
// writePart - write part n
//---------------------------------------------------------

void MxmlFile::writePart(const int n)
{
    m_out << "  <part id=\"P" << n + 1 << "\">\n";
    for (unsigned int i = 0; i < m_ef.measures().size(); ++i)
        writeMeasure(n, i);
    m_out << "  </part>" << endl;
}


//---------------------------------------------------------
// writePartList - write the part list
//---------------------------------------------------------

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


//---------------------------------------------------------
// writeParts - write the parts
//---------------------------------------------------------

void MxmlFile::writeParts()
{
    for (unsigned int count = 0; count < m_ef.staves().size(); ++count) {
        writePart(count);
    }
}


//---------------------------------------------------------
// writeRest - write a rest
//---------------------------------------------------------

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


//---------------------------------------------------------
// writeScorePart - write score part n
//---------------------------------------------------------

void MxmlFile::writeScorePart(const int n, const EncStaff& staff)
{
    m_out << "    <score-part id=\"P" << n << "\">\n";
    m_out << "      <part-name>" << staff.m_name << "</part-name>\n";
    m_out << "    </score-part>\n";
}


//---------------------------------------------------------
// writeTime - write the time signature
//---------------------------------------------------------

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


//---------------------------------------------------------
// writeWork - write the work description
//---------------------------------------------------------

void MxmlFile::writeWork()
{
    m_out << "  <work>\n";
    const EncTitle& ttl = m_ef.title();
    const auto subtitle = createMultiLineString(ttl.m_subtitle);
    if (!subtitle.isEmpty()) {
        m_out << "    <work-number>" << createMultiLineString(ttl.m_subtitle) << "</work-number>\n";
    }
    m_out << "    <work-title>" << ttl.m_title << "</work-title>\n";
    m_out << "  </work>\n";
}
