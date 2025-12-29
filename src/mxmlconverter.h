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

#ifndef MXMLCONVERTER_H
#define MXMLCONVERTER_H

#include "commondefs.h"
#include "mxmlwriter.h"
#include "noteconnector.h"

//---------------------------------------------------------
// the tuplet state handler deduces tuplet start and stop notes
// Groups tuplets by counting notes until actualNotes is reached
//---------------------------------------------------------

class TupletHandler {
public:
    TupletHandler() {}
    TupletState newNote(const quint8 actualNotes, const quint8 normalNotes, const int tick, const int duration);
    bool needsClose() const { return m_inTuplet; }
    void close() { m_inTuplet = false; m_groupStartTick = -1; m_groupDuration = 0; }
private:
    bool m_inTuplet { false };
    int m_groupStartTick { -1 };
    int m_groupDuration { 0 };
};


//---------------------------------------------------------
// the MusicXML converter class converts data representing an Encore file
// into MusicXML elements and attributes and writes it to standard output
//---------------------------------------------------------

class MxmlConverter
{
public:
    MxmlConverter(const EncFile& ef);
    void convertEncToMxml();
private:
    bool hasMultipleVoices(const int partNr) const { return (partNr < static_cast<int>(m_voicesPerPart.size())) && (m_voicesPerPart.at(partNr) > 1); }
    int nstaves(const int partNr) const { return (partNr < static_cast<int>(m_ef.staves().size())) ? m_ef.staves().at(partNr).m_nstaves : 1; }
    void attributes(const int partNr);
    void barlineLeft(const int partNr, const size_t measureNr);
    void barlineRight(const int partNr, const size_t measureNr);
    void clefs(const int partNr);
    void identification();
    void initVoicesPerPart();
    void key();
    void keyChange(const EncMeasureElemKeyChange* keyCh);
    void measure(const int partNr, const size_t measureNr);
    void note(const EncMeasureElemNote* const note, const int partNr, TupletHandler &th, const bool chord, const bool forceCloseTuplet, const int calculatedTick);
    void part(const int n);
    void partList();
    void parts();
    void repeatLeft(const repeatType repeat);
    void rest(const EncMeasureElemRest* const rest, const int partNr, TupletHandler &th, const bool forceCloseTuplet, const int calculatedTick);
    void scorePart(const int n, const EncInstrument& instr);
    void time();
    void work();
    const EncFile& m_ef;
    const NoteConnector m_nc;
    MxmlWriter m_writer;
    std::vector<std::size_t> m_voicesPerPart;
    int m_currentFifths { 0 };  // Current key signature for pitch spelling
};

#endif // MXMLCONVERTER_H
