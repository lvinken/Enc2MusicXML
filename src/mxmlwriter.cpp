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

#include "mxmlwriter.h"


//---------------------------------------------------------
// MxmlWriter - MusicXML file writer constructor
//---------------------------------------------------------

MxmlWriter::MxmlWriter()
{

}


//---------------------------------------------------------
// writeBackupForward - write backup or forward
//---------------------------------------------------------

void MxmlWriter::writeBackupForward(const int duration, const int voice)
{
    if (duration < 0) {
        m_out << "      <backup>\n";
        m_out << "        <duration>" << (-duration) << "</duration>\n";
        m_out << "      </backup>\n";
    }
    else if (duration > 0) {
        m_out << "      <forward>\n";
        m_out << "        <duration>" << duration << "</duration>\n";
        m_out << "        <voice>" << (voice + 1) << "</voice>\n";
        m_out << "      <forward>\n";
    }
}


//---------------------------------------------------------
// writeBarlineLeft - write left barline
//---------------------------------------------------------

void MxmlWriter::writeBarlineLeft(const bool repeatStart, const bool endingStart, const bool barlineDblLeft, const QString& endingNumber)
{
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
            m_out << "        <ending number=\"" << endingNumber << "\" type=\"start\"/>\n";
        }
        m_out << "      </barline>\n";
    }
}


//---------------------------------------------------------
// writeBarlineRight - write right barline
//---------------------------------------------------------

void MxmlWriter::writeBarlineRight(const bool repeatEnd, const bool endingStop, const bool barlineEnd, const bool barlineDbl, const int repeatAlternative)
{
    if (repeatEnd || endingStop || barlineEnd || barlineDbl) {
        m_out << "      <barline location=\"right\">\n";
        if (repeatEnd || barlineEnd) {
            m_out << "        <bar-style>light-heavy</bar-style>\n";
        }
        if (barlineDbl) {
            m_out << "        <bar-style>light-light</bar-style>\n";
        }
        if (endingStop) {
            QString type = repeatAlternative == 1 ? "stop" : "discontinue";
            m_out << "        <ending number=\"" << repeatAlternative << "\" type=\"" << type << "\"/>\n";
        }
        if (repeatEnd || repeatAlternative == 1) {
            m_out << "        <repeat direction=\"backward\"/>\n";
        }
        m_out << "      </barline>\n";
    }
}


//---------------------------------------------------------
// writeClef - write clef
//---------------------------------------------------------

void MxmlWriter::writeClef(const int number, const QString& sign, const int line, const int octCh)
{
    if (number >= 0) {
        m_out << QString("        <clef number=\"%1\">\n").arg(number + 1);
    }
    else {
        m_out << "        <clef>\n";
    }
    m_out << QString("          <sign>%1</sign>\n").arg(sign);
    m_out << QString("          <line>%1</line>\n").arg(line);
    if (octCh) {
        m_out << QString("          <clef-octave-change>%1</clef-octave-change>\n").arg(octCh);
    }
    m_out << "        </clef>\n";
}


//---------------------------------------------------------
// writeDivisions - write divisions
//---------------------------------------------------------

void MxmlWriter::writeDivisions(const int divisions)
{
    m_out << "        <divisions>" << divisions << "</divisions>\n";
}


//---------------------------------------------------------
// writeDots - write dots
//---------------------------------------------------------

void MxmlWriter::writeDots(const int dots)
{
    for (int i = 0; i < dots; ++i) {
        m_out << "        <dot/>\n";
    }
}


//---------------------------------------------------------
// writeElement - write an empty element
//---------------------------------------------------------

void MxmlWriter::writeElement(const QString &indent, const QString& element)
{
    m_out << indent << "<" << element << "/>\n";
}


//---------------------------------------------------------
// writeElement - write an element containing an integer value
//---------------------------------------------------------

void MxmlWriter::writeElement(const QString& indent, const QString& element, const int value)
{
    m_out << indent << "<" << element << ">" << value << "</" << element << ">\n";
}


//---------------------------------------------------------
// writeElement - write an element containing a string value
//---------------------------------------------------------

void MxmlWriter::writeElement(const QString& indent, const QString& element, const QString& value)
{
    m_out << indent << "<" << element << ">" << value << "</" << element << ">\n";
}


//---------------------------------------------------------
// writeElement - write an element start tag
//---------------------------------------------------------

void MxmlWriter::writeElementStart(const QString &indent, const QString& element)
{
    m_out << indent << "<" << element << ">\n";
}


//---------------------------------------------------------
// writeElement - write an element end tag
//---------------------------------------------------------

void MxmlWriter::writeElementEnd(const QString& indent, const QString& element)
{
    m_out << indent << "</" << element << ">\n";
}


//---------------------------------------------------------
// writeGrace - write grace
//---------------------------------------------------------

void MxmlWriter::writeGrace(const GraceType type)
{
    if (type == GraceType::ACCIACCATURA) {
        m_out << "        <grace slash=\"yes\"/>\n";
    }
    else if (type == GraceType::APPOGGIATURA) {
        m_out << "        <grace/>\n";
    }
}


//---------------------------------------------------------
// writeIdentification - write identification
//---------------------------------------------------------

void MxmlWriter::writeIdentification(const QString& author,
                                     const QString& lyricist,
                                     const QString& rights,
                                     const QString& software)
{
    m_out << "  <identification>\n";
    if (!author.isEmpty()) {
        m_out << "    <creator type=\"composer\">" << author << "</creator>\n";
    }
    if (!lyricist.isEmpty()) {
        m_out << "    <creator type=\"lyricist\">" << lyricist << "</creator>\n";
    }
    if (!rights.isEmpty()) {
        m_out << "    <rights>" << rights << "</rights>\n";
    }
    m_out << "    <encoding>\n";
    m_out << "      <software>" << software << "</software>\n";
    // TODO fill in real date
    // m_out << "      <encoding-date>TBD</encoding-date>\n";
    m_out << "    </encoding>\n";
    m_out << "  </identification>\n";
}


//---------------------------------------------------------
// writeKey - write key
//---------------------------------------------------------

void MxmlWriter::writeKey(const int fifths)
{
    m_out << "        <key>" << endl;
    m_out << "          <fifths>" << fifths << "</fifths>" << endl;
    m_out << "        </key>" << endl;
}


//---------------------------------------------------------
// writeKey - write key change
//---------------------------------------------------------

void MxmlWriter::writeKeyChange(const int fifths)
{
    m_out << "      <attributes>" << endl;
    writeKey(fifths);
    m_out << "      </attributes>" << endl;
}


//---------------------------------------------------------
// writeRepeatLeft - write repeats at the left side of a measure
//---------------------------------------------------------

void MxmlWriter::writeRepeatLeft(const bool coda, const bool segno)
{
    if (coda || segno) {
        m_out << "      <direction placement=\"above\">\n";
        m_out << "        <direction-type>\n";
        if (coda)
            m_out << "          <coda/>\n";
        else if (segno)
            m_out << "          <segno/>\n";
        m_out << "        </direction-type>\n";
        m_out << "      </direction>\n";
    }
}

//---------------------------------------------------------
// writeRepeatRight - write repeats at the right side of a measure
//---------------------------------------------------------

void MxmlWriter::writeRepeatRight(const QString& words)
{
    if (!words.isEmpty()) {
        m_out << "      <direction placement=\"above\">\n";
        m_out << "        <direction-type>\n";
        m_out << "          <words>" << words << "</words>\n";
        m_out << "        </direction-type>\n";
        m_out << "      </direction>\n";
    }
}


//---------------------------------------------------------
// writeScorePart - write score part ncontaining instrument instr
//---------------------------------------------------------

void MxmlWriter::writeScorePart(const int n, const QString& instr)
{
    m_out << "    <score-part id=\"P" << n << "\">\n";
    m_out << "      <part-name>" << instr << "</part-name>\n";
    m_out << "    </score-part>\n";
}


//---------------------------------------------------------
// writeStaff - write staff
//---------------------------------------------------------

void MxmlWriter::writeStaff(const int nstaves, const int staff)
{
    if (nstaves > 1) {
        m_out << "        <staff>" << staff << "</staff>\n";
    }
}


//---------------------------------------------------------
// writeStaves - write staves
//---------------------------------------------------------

void MxmlWriter::writeStaves(const int nstaves)
{
    if (nstaves > 1) {
        m_out << "        <staves>" << nstaves << "</staves>\n";
    }
}


//---------------------------------------------------------
// writePitch - write pitch
//---------------------------------------------------------

void MxmlWriter::writePitch(const char step, const int alter, const int octave)
{
    m_out << "        <pitch>\n";
    m_out << "          <step>" << step << "</step>\n";
    if (alter) {
        m_out << "          <alter>" << alter << "</alter>\n";
    }
    m_out << "          <octave>" << octave << "</octave>\n";
    m_out << "        </pitch>\n";
}


//---------------------------------------------------------
// writeTime - write time signature
//---------------------------------------------------------

void MxmlWriter::writeTime(const unsigned int beats, const unsigned int beattype)
{
    m_out << "        <time>\n";
    m_out << "          <beats>" << beats << "</beats>\n";
    m_out << "          <beat-type>" << beattype << "</beat-type>\n";
    m_out << "        </time>\n";
}


//---------------------------------------------------------
// writeTime - write time modification
//---------------------------------------------------------

void MxmlWriter::writeTimeModification(const int actual, const int normal)
{
    if (actual > 0 && normal > 0) {
        m_out << "        <time-modification>\n";
        m_out << "          <actual-notes>" << actual << "</actual-notes>\n";
        m_out << "          <normal-notes>" << normal << "</normal-notes>\n";
        m_out << "        </time-modification>\n";
    }
}


//---------------------------------------------------------
// writeClef - write tuplet start or stop
//---------------------------------------------------------

void MxmlWriter::writeTuplet(TupletState state)
{
    if (state == TupletState::START || state == TupletState::STOP) {
        m_out << "        <notations>\n";
        if (state == TupletState::START) {
            m_out << "          <tuplet type=\"start\"/>\n";
        }
        if (state == TupletState::STOP) {
            m_out << "          <tuplet type=\"stop\"/>\n";
        }
        m_out << "        </notations>\n";
    }
}


//---------------------------------------------------------
// writeVoice - write voice
//---------------------------------------------------------

void MxmlWriter::writeVoice(const bool hasMultipleVoices, const int voice)
{
    if (hasMultipleVoices) {
        m_out << "        <voice>" << voice << "</voice>\n";
    }
}


//---------------------------------------------------------
// writeWork - write work
//---------------------------------------------------------

void MxmlWriter::writeWork(const QString& title, const QString& subtitle)
{
    m_out << "  <work>\n";
    if (!subtitle.isEmpty()) {
        m_out << "    <work-number>" << subtitle << "</work-number>\n";
    }
    m_out << "    <work-title>" << title << "</work-title>\n";
    m_out << "  </work>\n";
}


//---------------------------------------------------------
// writeXmlHeader - write xml header
//---------------------------------------------------------

void MxmlWriter::writeXmlHeader()
{
    m_out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          << "<!DOCTYPE score-partwise PUBLIC"
          << " \"-//Recordare//DTD MusicXML 3.1 Partwise//EN\""
          << " \"http://www.musicxml.org/dtds/partwise.dtd\">\n";
}

