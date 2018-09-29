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

#ifndef MXMLWRITER_H
#define MXMLWRITER_H

#include <QXmlStreamWriter>

#include "commondefs.h"
//#include "xmlwriter.h"


//---------------------------------------------------------
// the MusicXML file writer
//---------------------------------------------------------

class MxmlWriter
{
public:
    MxmlWriter();
    void setDevice(QIODevice *device) { m_xml.setDevice(device); }
    void writeBackupForward(const int duration, const int voice);
    void writeBarlineLeft(const bool repeatStart, const bool endingStart, const bool barlineDblLeft, const QString& endingNumber);
    void writeBarlineRight(const bool repeatEnd, const bool endingStop, const bool barlineEnd, const bool barlineDbl, const int repeatAlternative);
    void writeBegin();
    void writeClef(const int number, const QString& sign, const int line, const int octCh);
    void writeDivisions(const int divisions);
    void writeDots(const int dots);
    void writeElement(const QString& element);
    void writeElement(const QString& element, const int value);
    void writeElement(const QString& element, const QString& value);
    void writeElementStart(const QString& element);
    void writeElementStartWithAttribute(const QString& element, const QString& attr, const int value);
    void writeElementStartWithAttribute(const QString& element, const QString& attr, const QString& value);
    void writeElementEnd();
    void writeEnd();
    void writeFermata();
    void writeGrace(const GraceType type);
    void writeIdentification(const QString& author, const QString& lyricist, const QString& rights, const QString& software);
    void writeKey(const int fifths);
    void writeKeyChange(const int fifths);
    void writeMetronome(const QString& beatUnit, const int beatUnitDots, const int perMinute);
    void writePitch(const char step, const int alter, const int octave);
    void writeRepeatLeft(const bool coda, const bool segno);
    void writeRepeatRight(const QString& words);
    void writeScorePart(const int n, const QString& instr);
    void writeSlur(const StartStop startstop, const int number = 1);
    void writeStaff(const int nstaves, const int staff);
    void writeStaves(const int nstaves);
    void writeTie(const StartStop startstop);
    void writeTied(const StartStop startstop);
    void writeTime(const unsigned int beats, const unsigned int beattype);
    void writeTimeModification(const int actual, const int normal);
    void writeTuplet(TupletState state);
    void writeVoice(const bool hasMultipleVoices, const int voice);
    void writeWedge(const WedgeType wedgetype, const int number = 1);
    void writeWords(const QString& words);
    void writeWork(const QString& title, const QString& subtitle);
private:
    QXmlStreamWriter m_xml;
};

#endif // MXMLWRITER_H
