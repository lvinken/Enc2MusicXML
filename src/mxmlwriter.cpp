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
    m_xml.setAutoFormatting(true);
    m_xml.setAutoFormattingIndent(2);
}


//---------------------------------------------------------
// writeBackupForward - write backup or forward
//---------------------------------------------------------

void MxmlWriter::writeBackupForward(const int duration, const int voice)
{
    if (duration < 0) {
        m_xml.writeStartElement("backup");
        m_xml.writeTextElement("duration", QString::number(-duration));
        m_xml.writeEndElement();
    }
    else if (duration > 0) {
        m_xml.writeStartElement("forward");
        m_xml.writeTextElement("duration", QString::number(duration));
        m_xml.writeTextElement("voice", QString::number(voice + 1));
        m_xml.writeEndElement();
    }
}


//---------------------------------------------------------
// writeBarlineLeft - write left barline
//---------------------------------------------------------

void MxmlWriter::writeBarlineLeft(const bool repeatStart, const bool endingStart, const bool barlineDblLeft, const QString& endingNumber)
{
    if (repeatStart || endingStart || barlineDblLeft) {
        m_xml.writeStartElement("barline");
        m_xml.writeAttribute("location", "left");
        if (repeatStart) {
            m_xml.writeTextElement("bar-style", "heavy-light");
            m_xml.writeStartElement("repeat");
            m_xml.writeAttribute("direction", "forward");
            m_xml.writeEndElement();
        }
        if (barlineDblLeft) {
            m_xml.writeTextElement("bar-style", "light-light");
        }
        if (endingStart) {
            m_xml.writeStartElement("ending");
            m_xml.writeAttribute("number", endingNumber);
            m_xml.writeAttribute("type", "start");
            m_xml.writeEndElement();
        }
        m_xml.writeEndElement();
    }
}


//---------------------------------------------------------
// writeBarlineRight - write right barline
//---------------------------------------------------------

void MxmlWriter::writeBarlineRight(const bool repeatEnd, const bool endingStop, const bool barlineEnd, const bool barlineDbl, const int repeatAlternative)
{
    if (repeatEnd || endingStop || barlineEnd || barlineDbl) {
        m_xml.writeStartElement("barline");
        m_xml.writeAttribute("location", "right");
        if (repeatEnd || barlineEnd) {
            m_xml.writeTextElement("bar-style", "light-heavy");
        }
        if (barlineDbl) {
            m_xml.writeTextElement("bar-style", "light-light");
        }
        if (endingStop) {
            QString type = repeatAlternative == 1 ? "stop" : "discontinue";
            m_xml.writeStartElement("ending");
            m_xml.writeAttribute("number", QString::number(repeatAlternative));
            m_xml.writeAttribute("type", type);
            m_xml.writeEndElement();
        }
        if (repeatEnd || repeatAlternative == 1) {
            m_xml.writeStartElement("repeat");
            m_xml.writeAttribute("direction", "backward");
            m_xml.writeEndElement();
        }
        m_xml.writeEndElement();
    }
}


//---------------------------------------------------------
// writeBegin - begin writing: creates the header
//---------------------------------------------------------

void MxmlWriter::writeBegin()
{
    m_xml.writeStartDocument();
    m_xml.writeDTD("<!DOCTYPE score-partwise PUBLIC "
                   "\"-//Recordare//DTD MusicXML 3.1 Partwise//EN\" "
                   "\"http://www.musicxml.org/dtds/partwise.dtd\">");
}


//---------------------------------------------------------
// writeClef - write clef
//---------------------------------------------------------

void MxmlWriter::writeClef(const int number, const QString& sign, const int line, const int octCh)
{
    m_xml.writeStartElement("clef");
    if (number >= 0) {
        m_xml.writeAttribute("number", QString::number(number + 1));
    }
    m_xml.writeTextElement("sign", sign);
    m_xml.writeTextElement("line", QString::number(line));
    if (octCh) {
        m_xml.writeTextElement("clef-octave-change", QString::number(octCh));
    }
    m_xml.writeEndElement();
}


//---------------------------------------------------------
// writeDivisions - write divisions
//---------------------------------------------------------

void MxmlWriter::writeDivisions(const int divisions)
{
    m_xml.writeTextElement("divisions", QString::number(divisions));
}


//---------------------------------------------------------
// writeDots - write dots
//---------------------------------------------------------

void MxmlWriter::writeDots(const int dots)
{
    for (int i = 0; i < dots; ++i) {
        m_xml.writeEmptyElement("dot");
    }
}


//---------------------------------------------------------
// writeElement - write an empty element
//---------------------------------------------------------

void MxmlWriter::writeElement(const QString& element)
{
    m_xml.writeEmptyElement(element);
}


//---------------------------------------------------------
// writeElement - write an element containing an integer value
//---------------------------------------------------------

void MxmlWriter::writeElement(const QString& element, const int value)
{
    m_xml.writeTextElement(element, QString::number(value));
}


//---------------------------------------------------------
// writeElement - write an element containing a string value
//---------------------------------------------------------

void MxmlWriter::writeElement(const QString& element, const QString& value)
{
    m_xml.writeTextElement(element, value);
}


//---------------------------------------------------------
// writeElement - write an element start tag
//---------------------------------------------------------

void MxmlWriter::writeElementStart(const QString& element)
{
    m_xml.writeStartElement(element);
}


//---------------------------------------------------------
// writeElement - write an element end tag
//---------------------------------------------------------

void MxmlWriter::writeElementEnd()
{
    m_xml.writeEndElement();
}


//---------------------------------------------------------
// writeElement - write an element start tag with an attribute
//---------------------------------------------------------

void MxmlWriter::writeElementStartWithAttribute(const QString& element, const QString& attr, const int value)
{
    m_xml.writeStartElement(element);
    m_xml.writeAttribute(attr, QString::number(value));
}


//---------------------------------------------------------
// writeElement - write an element start tag with an attribute
//---------------------------------------------------------

void MxmlWriter::writeElementStartWithAttribute(const QString& element, const QString& attr, const QString& value)
{
    m_xml.writeStartElement(element);
    m_xml.writeAttribute(attr, value);
}


//---------------------------------------------------------
// writeEnd - end writing: flush
//---------------------------------------------------------

void MxmlWriter::writeEnd()
{
    m_xml.writeEndDocument();
}


//---------------------------------------------------------
// writeGrace - write fermata
//---------------------------------------------------------

void MxmlWriter::writeFermata()
{
    m_xml.writeStartElement("notations");
    m_xml.writeEmptyElement("fermata");
    m_xml.writeEndElement();
}


//---------------------------------------------------------
// writeGrace - write grace
//---------------------------------------------------------

void MxmlWriter::writeGrace(const GraceType type)
{
    if (type == GraceType::ACCIACCATURA) {
        m_xml.writeStartElement("grace");
        m_xml.writeAttribute("slash", "yes");
        m_xml.writeEndElement();
    }
    else if (type == GraceType::APPOGGIATURA) {
        m_xml.writeEmptyElement("grace");
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
    m_xml.writeStartElement("identification");
    if (!author.isEmpty()) {
        m_xml.writeStartElement("creator");
        m_xml.writeAttribute("type", "composer");
        m_xml.writeCharacters(author);
        m_xml.writeEndElement();
    }
    if (!lyricist.isEmpty()) {
        m_xml.writeStartElement("creator");
        m_xml.writeAttribute("type", "lyricist");
        m_xml.writeCharacters(lyricist);
        m_xml.writeEndElement();
    }
    if (!rights.isEmpty()) {
        m_xml.writeTextElement("rights", rights);
    }
    m_xml.writeStartElement("encoding");
    m_xml.writeTextElement("software", software);
    // TODO fill in real date
    // <encoding-date>TBD</encoding-date>
    m_xml.writeEndElement();
    m_xml.writeEndElement();
}


//---------------------------------------------------------
// writeKey - write key
//---------------------------------------------------------

void MxmlWriter::writeKey(const int fifths)
{
    m_xml.writeStartElement("key");
    m_xml.writeTextElement("fifths", QString::number(fifths));
    m_xml.writeEndElement();
}


//---------------------------------------------------------
// writeKey - write key change
//---------------------------------------------------------

void MxmlWriter::writeKeyChange(const int fifths)
{
    m_xml.writeStartElement("attributes");
    writeKey(fifths);
    m_xml.writeEndElement();
}


//---------------------------------------------------------
// writeMetronome - write metronome
//---------------------------------------------------------

void MxmlWriter::writeMetronome(const QString& beatUnit, const int beatUnitDots, const int perMinute)
{
    m_xml.writeStartElement("direction");
    m_xml.writeAttribute("placement", "above");
    m_xml.writeStartElement("direction-type");
    m_xml.writeStartElement("metronome");
    m_xml.writeTextElement("beat-unit", beatUnit);
    for (int i = 0; i < beatUnitDots; ++i) {
        m_xml.writeEmptyElement("beat-unit-dot");
    }
    m_xml.writeTextElement("per-minute", QString::number(perMinute));
    m_xml.writeEndElement();
    m_xml.writeEndElement();
    m_xml.writeEndElement();
}


//---------------------------------------------------------
// writeRepeatLeft - write repeats at the left side of a measure
//---------------------------------------------------------

void MxmlWriter::writeRepeatLeft(const bool coda, const bool segno)
{
    if (coda || segno) {
        m_xml.writeStartElement("direction");
        m_xml.writeAttribute("placement", "above");
        m_xml.writeStartElement("direction-type");
        if (coda) {
            m_xml.writeEmptyElement("coda");
        }
        else if (segno) {
            m_xml.writeEmptyElement("segno");
        }
        m_xml.writeEndElement();
        m_xml.writeEndElement();
    }
}

//---------------------------------------------------------
// writeRepeatRight - write repeats at the right side of a measure
//---------------------------------------------------------

void MxmlWriter::writeRepeatRight(const QString& words)
{
    if (!words.isEmpty()) {
        m_xml.writeStartElement("direction");
        m_xml.writeAttribute("placement", "above");
        m_xml.writeStartElement("direction-type");
        m_xml.writeTextElement("words", words);
        m_xml.writeEndElement();
        m_xml.writeEndElement();
    }
}


//---------------------------------------------------------
// writeScorePart - write score part ncontaining instrument instr
//---------------------------------------------------------

void MxmlWriter::writeScorePart(const int n, const QString& instr)
{
    m_xml.writeStartElement("score-part");
    m_xml.writeAttribute("id", QString("P%1").arg(n));
    m_xml.writeTextElement("part-name", instr);
    m_xml.writeEndElement();
}


//---------------------------------------------------------
// writeStaff - write staff
//---------------------------------------------------------

void MxmlWriter::writeStaff(const int nstaves, const int staff)
{
    if (nstaves > 1) {
        m_xml.writeTextElement("staff", QString::number(staff));
    }
}


//---------------------------------------------------------
// writeStaves - write staves
//---------------------------------------------------------

void MxmlWriter::writeStaves(const int nstaves)
{
    if (nstaves > 1) {
        m_xml.writeTextElement("staves", QString::number(nstaves));
    }
}


//---------------------------------------------------------
// writePitch - write pitch
//---------------------------------------------------------

void MxmlWriter::writePitch(const char step, const int alter, const int octave)
{
    m_xml.writeStartElement("pitch");
    m_xml.writeTextElement("step", QChar(step));
    if (alter) {
        m_xml.writeTextElement("alter", QString::number(alter));
    }
    m_xml.writeTextElement("octave", QString::number(octave));
    m_xml.writeEndElement();
}


//---------------------------------------------------------
// writeSlur - write slur
//---------------------------------------------------------

void MxmlWriter::writeSlur(const StartStop startstop, const int number)
{
    m_xml.writeStartElement("notations");
    m_xml.writeStartElement("slur");
    m_xml.writeAttribute("type", startstop == StartStop::START ? "start" : "stop");
    m_xml.writeAttribute("number", QString::number(number));
    m_xml.writeEndElement();
    m_xml.writeEndElement();
}

//---------------------------------------------------------
// writeTie - write tie
//---------------------------------------------------------

void MxmlWriter::writeTie(const StartStop startstop)
{
    m_xml.writeStartElement("tie");
    m_xml.writeAttribute("type", startstop == StartStop::START ? "start" : "stop");
    m_xml.writeEndElement();
}


//---------------------------------------------------------
// writeTied - write tied
//---------------------------------------------------------

void MxmlWriter::writeTied(const StartStop startstop)
{
    m_xml.writeStartElement("notations");
    m_xml.writeStartElement("tied");
    m_xml.writeAttribute("type", startstop == StartStop::START ? "start" : "stop");
    m_xml.writeEndElement();
    m_xml.writeEndElement();
}


//---------------------------------------------------------
// writeTime - write time signature
//---------------------------------------------------------

void MxmlWriter::writeTime(const unsigned int beats, const unsigned int beattype)
{
    m_xml.writeStartElement("time");
    m_xml.writeTextElement("beats", QString::number(beats));
    m_xml.writeTextElement("beat-type", QString::number(beattype));
    m_xml.writeEndElement();
}


//---------------------------------------------------------
// writeTimeModification - write time modification
//---------------------------------------------------------

void MxmlWriter::writeTimeModification(const int actual, const int normal)
{
    if (actual > 0 && normal > 0) {
        m_xml.writeStartElement("time-modification");
        m_xml.writeTextElement("actual-notes", QString::number(actual));
        m_xml.writeTextElement("normal-notes", QString::number(normal));
        m_xml.writeEndElement();
    }
}


//---------------------------------------------------------
// writeTuplet - write tuplet start or stop
//---------------------------------------------------------

void MxmlWriter::writeTuplet(TupletState state)
{
    if (state == TupletState::START || state == TupletState::STOP) {
        m_xml.writeStartElement("notations");
        if (state == TupletState::START) {
            m_xml.writeStartElement("tuplet");
            m_xml.writeAttribute("type", "start");
            m_xml.writeEndElement();
        }
        if (state == TupletState::STOP) {
            m_xml.writeStartElement("tuplet");
            m_xml.writeAttribute("type", "stop");
            m_xml.writeEndElement();
        }
        m_xml.writeEndElement();
    }
}


//---------------------------------------------------------
// writeVoice - write voice
//---------------------------------------------------------

void MxmlWriter::writeVoice(const bool hasMultipleVoices, const int voice)
{
    if (hasMultipleVoices) {
        m_xml.writeTextElement("voice", QString::number(voice));
    }
}


//---------------------------------------------------------
// writeWedge - write wedge
//---------------------------------------------------------

void MxmlWriter::writeWedge(const WedgeType wedgetype, const int number)
{
    QString type;
    switch (wedgetype) {
    case WedgeType::CRESCENDO: type = "crescendo"; break;
    case WedgeType::DIMINUENDO: type = "diminuendo"; break;
    case WedgeType::STOP: type = "stop"; break;
    default: /* TBD */; break;
    }

    m_xml.writeStartElement("direction");
    m_xml.writeStartElement("direction-type");
    m_xml.writeStartElement("wedge");
    m_xml.writeAttribute("type", type);
    m_xml.writeAttribute("number", QString::number(number));
    m_xml.writeEndElement();
    m_xml.writeEndElement();
    m_xml.writeEndElement();
}


//---------------------------------------------------------
// writeWords - write words
//---------------------------------------------------------

void MxmlWriter::writeWords(const QString& words)
{
    m_xml.writeStartElement("direction");
    m_xml.writeStartElement("direction-type");
    m_xml.writeTextElement("words", words);
    m_xml.writeEndElement();
    m_xml.writeEndElement();
}


//---------------------------------------------------------
// writeWork - write work
//---------------------------------------------------------

void MxmlWriter::writeWork(const QString& title, const QString& subtitle)
{
    m_xml.writeStartElement("work");
    if (!subtitle.isEmpty()) {
        m_xml.writeTextElement("work-number", subtitle);
    }
    m_xml.writeTextElement("work-title", title);
    m_xml.writeEndElement();
}
