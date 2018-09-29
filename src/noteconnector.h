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

#ifndef NOTECONNECTOR_H
#define NOTECONNECTOR_H

#include <map>

#include "encfile.h"

class NoteConnector
{
public:
    NoteConnector(const EncFile& ef);
    const EncMeasureElemOrnament* direction(const EncMeasureElemNote* const note) const;
    const EncMeasureElemOrnament* slurStart(const EncMeasureElemNote* const note) const;
    const EncMeasureElemOrnament* slurStop(const EncMeasureElemNote* const note) const;
    bool tieStart(const EncMeasureElemNote* const note) const;
    bool tieStop(const EncMeasureElemNote* const note) const;
    const EncMeasureElemOrnament* wedgeStart(const EncMeasureElemNote* const note) const;
    const EncMeasureElemOrnament* wedgeStop(const EncMeasureElemNote* const note) const;
private:
    const EncMeasureElemNote* findClosestNote(const quint8 xoffset, const quint8 voice, const quint8 staffIdx, const size_t measureNr) const;
    const EncMeasureElemNote* findFirstNoteAfterXoffset(const quint8 xoffset, const quint8 voice, const quint8 staffIdx, const size_t measureNr) const;
    const EncMeasureElemNote* findLastNote(const EncMeasureElemNote* const note, const size_t measureNr) const;
    const EncMeasureElemNote* findLastNoteBeforeXoffset(const quint8 xoffset, const quint8 voice, const quint8 staffIdx, const size_t measureNr) const;
    const EncMeasureElemNote* findPreviousNote(const EncMeasureElemNote* const note, const size_t measureNr) const;
    void initMeasureNumbers();
    void initSlur(const EncMeasureElemOrnament* const orn, const size_t measureNr);
    void initWedge(const EncMeasureElemOrnament* const orn, const size_t measureNr);
    size_t measureNumber(const EncMeasureElem* const elem) const;
    const EncFile& m_ef;
    std::map<const EncMeasureElem* const, const size_t> m_measureElemToMeasureNr;
    std::map<const EncMeasureElemNote* const, const EncMeasureElemOrnament* const> m_slurStarts;
    std::map<const EncMeasureElemNote* const, const EncMeasureElemOrnament* const> m_slurStops;
    std::map<const EncMeasureElemNote* const, const EncMeasureElemOrnament* const> m_wedgeStarts;
    std::map<const EncMeasureElemNote* const, const EncMeasureElemOrnament* const> m_wedgeStops;
};

#endif // NOTECONNECTOR_H
