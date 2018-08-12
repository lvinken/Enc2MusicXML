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

#ifndef MXMLFILE_H
#define MXMLFILE_H


//---------------------------------------------------------
// the tuplet state handler deduces tuplet start and stop notes
//---------------------------------------------------------

class TupletHandler {
public:
    enum class type : quint8 {
        NONE = 0,
        START,
        MID,
        STOP
    };
    TupletHandler() {}
    TupletHandler::type newNote(const quint8 actualNotes, const quint8 normalNotes, const quint8 faceValue);
private:
    int m_count { 0 };
    int m_value { 0 };
};


//---------------------------------------------------------
// the MusicXML file writer
//---------------------------------------------------------

class MxmlFile
{
public:
    MxmlFile(const EncFile& ef);
    bool hasMultipleVoices(const int partNr) const { return m_voicesPerPart.at(partNr) > 1; }
    void write();
    void writeAttributes(const int partNr);
    void writeBackupForward(const int duration, const int voice);
    void writeBarlineLeft(const int partNr, const size_t measureNr);
    void writeBarlineRight(const int partNr, const size_t measureNr);
    void writeClefs(const int partNr);
    void writeIdentification();
    void writeKey();
    void writeKeyChange(const EncMeasureElemKeyChange* keyChange);
    void writeMeasure(const int partNr, const size_t measureNr);
    void writeNote(const EncMeasureElemNote* const note, const int partNr, TupletHandler &th, const bool chord);
    void writePart(const int n);
    void writePartList();
    void writeParts();
    void writeRest(const EncMeasureElemRest* const rest, const int partNr, TupletHandler &th);
    void writeScorePart(const int n, const EncInstrument& instr);
    void writeTime();
    void writeWork();
private:
    void initVoicesPerPart();
    const EncFile& m_ef;
    QTextStream m_out;
    std::vector<std::size_t> m_voicesPerPart;
};

#endif // MXMLFILE_H
