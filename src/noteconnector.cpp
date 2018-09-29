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

//---------------------------------------------------------
// find connections between notes in an Encore file
// and ties
//---------------------------------------------------------

#include <climits>
#include <cmath>

#include <QtDebug>

#include "noteconnector.h"


//---------------------------------------------------------
//  -
//---------------------------------------------------------


//---------------------------------------------------------
// NoteConnector
//---------------------------------------------------------

NoteConnector::NoteConnector(const EncFile& ef)
    : m_ef(ef)
{
    initMeasureNumbers();
    for (unsigned int measureNr = 0; measureNr < m_ef.measures().size(); ++measureNr) {
        const auto& m = m_ef.measures().at(measureNr);
        for (const auto elem : m.measureElems()) {
            if (const EncMeasureElemOrnament* const orn = dynamic_cast<const EncMeasureElemOrnament* const>(elem)) {
                if (orn->type() == ornamentType::SLURSTART) {
                    initSlur(orn, measureNr);
                }
                else if (orn->type() == ornamentType::WEDGESTART) {
                    initWedge(orn, measureNr);
                }
            }
        }
    }
}


//---------------------------------------------------------
// initMeasureNumbers - cache measureElem to measure number mapping
//---------------------------------------------------------

void NoteConnector::initMeasureNumbers()
{
    for (size_t measureNr = 0; measureNr < m_ef.measures().size(); ++measureNr) {
        const auto& m = m_ef.measures().at(measureNr);
        for (const auto elem : m.measureElems()) {
            m_measureElemToMeasureNr.emplace(elem, measureNr);
        }
    }
}


//---------------------------------------------------------
// initSlur - initialize a slur's state
//---------------------------------------------------------

/*
 * Slur handling
 *
 * Slurs are not explicitly linked to notes but have the same m_tick and similar m_xoffset
 * as the starting note. As matching tick sometimes produces sub-optimal results,
 * m_xoffset is used instead.
 * The ending note is found by matching the slur's m_xoffset2
 * to the nearest note with the same staffidx and voice.
 */

void NoteConnector::initSlur(const EncMeasureElemOrnament* const orn, const size_t measureNr)
{
    const auto startNote = findClosestNote(orn->m_xoffset, orn->m_voice, orn->m_staffIdx, measureNr);
    const auto stopNote = findClosestNote(orn->m_xoffset2, orn->m_voice, orn->m_staffIdx, measureNr + orn->m_al_mezuro);

    if (startNote && stopNote && startNote != stopNote) {
        if (m_slurStarts.find(startNote) != m_slurStarts.end()) {
            qDebug() << "xxx_slur found slur start note already has a slur";
        }
        else if (m_slurStops.find(stopNote) != m_slurStops.end()) {
            qDebug() << "xxx_slur found slur stop note already has a slur";
        }
        else {
            m_slurStarts.emplace(startNote, orn);
            m_slurStops.emplace(stopNote, orn);
        }
    }
}


//---------------------------------------------------------
// initWedge - initialize a wedge's state
//---------------------------------------------------------

void NoteConnector::initWedge(const EncMeasureElemOrnament* const orn, const size_t measureNr)
{
    const auto startNote = findClosestNote(orn->m_xoffset, orn->m_voice, orn->m_staffIdx, measureNr);
    const auto stopNote = findLastNoteBeforeXoffset(orn->m_xoffset2, orn->m_voice, orn->m_staffIdx, measureNr + orn->m_al_mezuro);

    if (startNote && stopNote) {
        if (m_wedgeStarts.find(startNote) != m_wedgeStarts.end()) {
            qDebug() << "xxx_wedgefound wedge start note already has a wedge";
        }
        else if (m_wedgeStops.find(stopNote) != m_wedgeStops.end()) {
            qDebug() << "xxx_wedge found wedge stop note already has a wedge";
        }
        else {
            m_wedgeStarts.emplace(startNote, orn);
            m_wedgeStops.emplace(stopNote, orn);
        }
    }
}


//---------------------------------------------------------
// measureNumber - return measure number for elem
//---------------------------------------------------------

size_t NoteConnector::measureNumber(const EncMeasureElem* const elem) const
{
    const auto resultPair = m_measureElemToMeasureNr.find(elem);
    Q_ASSERT(resultPair != m_measureElemToMeasureNr.end());
    return resultPair->second;
}


//---------------------------------------------------------
// findClosestNote - find closest note to a given xpos
//---------------------------------------------------------

const EncMeasureElemNote* NoteConnector::findClosestNote(const quint8 xoffset, const quint8 voice, const quint8 staffIdx, const size_t measureNr) const
{
    const EncMeasureElemNote* closestNote { nullptr };
    int minimum { INT_MAX };
    const auto& m = m_ef.measures().at(measureNr);

    for (const auto elem : m.measureElems()) {
        if (const EncMeasureElemNote* const note = dynamic_cast<const EncMeasureElemNote* const>(elem)) {
            if (note->m_voice == voice && note->m_staffIdx == staffIdx) {
                const auto minimumCandidate = abs(xoffset - note->m_xoffset);
                if (minimumCandidate < minimum) {
                    minimum = minimumCandidate;
                    closestNote = note;
                }
            }
        }
    }

    return closestNote;
}


//---------------------------------------------------------
// findFirstNoteAfterXoffset - find first note after a given xpos
//---------------------------------------------------------

const EncMeasureElemNote* NoteConnector::findFirstNoteAfterXoffset(const quint8 xoffset, const quint8 voice, const quint8 staffIdx, const size_t measureNr) const
{
    const auto& m = m_ef.measures().at(measureNr);

    for (const auto elem : m.measureElems()) {
        if (const EncMeasureElemNote* const note = dynamic_cast<const EncMeasureElemNote* const>(elem)) {
            if (note->m_voice == voice && note->m_staffIdx == staffIdx && note->m_xoffset > xoffset) {
                return note;
            }
        }
    }

    return nullptr;
}


//---------------------------------------------------------
// findLastNote - find last note in measure in same voice
//---------------------------------------------------------

const EncMeasureElemNote* NoteConnector::findLastNote(const EncMeasureElemNote* const note, const size_t measureNr) const
{
    const EncMeasureElemNote* previousNote { nullptr };
    const auto& m = m_ef.measures().at(measureNr);

    for (const auto elem : m.measureElems()) {
        if (const EncMeasureElemNote* const prev = dynamic_cast<const EncMeasureElemNote* const>(elem)) {
            if (prev->m_voice == note->m_voice
                    && prev->m_staffIdx == note->m_staffIdx) {
                previousNote = prev;
            }
        }
    }

    return previousNote;
}


//---------------------------------------------------------
// findLastNoteBeforeXoffset - find last note before a given xpos
//---------------------------------------------------------

// last note before a given xpos
const EncMeasureElemNote* NoteConnector::findLastNoteBeforeXoffset(const quint8 xoffset, const quint8 voice, const quint8 staffIdx, const size_t measureNr) const
{
    const EncMeasureElemNote* lastNote { nullptr };
    const auto& m = m_ef.measures().at(measureNr);

    for (const auto elem : m.measureElems()) {
        if (const EncMeasureElemNote* const note = dynamic_cast<const EncMeasureElemNote* const>(elem)) {
            if (note->m_voice == voice && note->m_staffIdx == staffIdx && note->m_xoffset < xoffset) {
                lastNote = note;
            }
        }
    }

    return lastNote;
}


//---------------------------------------------------------
// findPreviousNote - find previous note in measure in same voice
//---------------------------------------------------------

const EncMeasureElemNote* NoteConnector::findPreviousNote(const EncMeasureElemNote* const note, const size_t measureNr) const
{
    const EncMeasureElemNote* previousNote { nullptr };

    if (note->m_tick > 0) {
        const auto& m = m_ef.measures().at(measureNr);
        for (const auto elem : m.measureElems()) {
            if (const EncMeasureElemNote* const prev = dynamic_cast<const EncMeasureElemNote* const>(elem)) {
                if (prev->m_tick < note->m_tick
                        && prev->m_voice == note->m_voice
                        && prev->m_staffIdx == note->m_staffIdx) {
                    previousNote = prev;
                }
            }
        }
    }

    return previousNote;
}


//---------------------------------------------------------
// direction - return any direction associated with note
//---------------------------------------------------------

const EncMeasureElemOrnament* NoteConnector::direction(const EncMeasureElemNote* const note) const
{
    // ornaments are not handled correctly for SCO5 files (many values incorrectly set to 0)
    // -> temporarily disabled
    if (m_ef.header().m_magic == "SCO5") {
        return nullptr;
    }

    const auto measureNr = measureNumber(note);
    const auto& m = m_ef.measures().at(measureNr);

    for (const auto elem : m.measureElems()) {
        if (const EncMeasureElemOrnament* const orn = dynamic_cast<const EncMeasureElemOrnament* const>(elem)) {
            if (note->m_tick == orn->m_tick
                    && note->m_voice == orn->m_voice
                    && note->m_staffIdx == orn->m_staffIdx
                    && (orn->type() == ornamentType::STAFFTEXT
                        || orn->type() == ornamentType::TEMPO)) {
                return orn;
            }
        }
    }

    return nullptr;
}


//---------------------------------------------------------
// slurStart - return any slur starting at note
//---------------------------------------------------------

const EncMeasureElemOrnament* NoteConnector::slurStart(const EncMeasureElemNote* const note) const
{
    const auto resultPair = m_slurStarts.find(note);

    if (resultPair != m_slurStarts.end()) {
        return resultPair->second;
    }

    return nullptr;
}


//---------------------------------------------------------
// slurStop - return any slur stopping at note
//---------------------------------------------------------

const EncMeasureElemOrnament* NoteConnector::slurStop(const EncMeasureElemNote* const note) const
{
    const auto resultPair = m_slurStops.find(note);

    if (resultPair != m_slurStops.end()) {
        return resultPair->second;
    }

    return nullptr;
}


//---------------------------------------------------------
// tieStart - return true iff a tie starts at note
//---------------------------------------------------------

/*
 * Tie handling
 *
 * Ties are not explicitly linked to notes but have the same m_tick and m_xoffset
 * as the starting note. The ending note is implicit (first note after the note/chord
 * starting the tie with the same staffidx and voice).
 */

bool NoteConnector::tieStart(const EncMeasureElemNote* const note) const
{
    const auto measureNr = measureNumber(note);
    const auto& m = m_ef.measures().at(measureNr);

    for (const auto elem : m.measureElems()) {
        if (const EncMeasureElemTie* const tie = dynamic_cast<const EncMeasureElemTie* const>(elem)) {
            if (note->m_tick == tie->m_tick
                    && note->m_voice == tie->m_voice
                    && note->m_staffIdx == tie->m_staffIdx
                    && note->m_xoffset == tie->m_xoffset) {
                return true;
            }
        }
    }

    return false;
}


//---------------------------------------------------------
// tieStop - return true iff a tie stops at note
//---------------------------------------------------------

bool NoteConnector::tieStop(const EncMeasureElemNote* const note) const
{
    const EncMeasureElemNote* previousNote { nullptr };
    const auto measureNr = measureNumber(note);
    bool res { false };

    if (note->m_tick > 0) {
        previousNote = findPreviousNote(note, measureNr);
        if (previousNote) {
            res = tieStart(previousNote);
        }
    }
    else if (measureNr > 0) {
        previousNote = findLastNote(note, measureNr - 1);
        if (previousNote) {
            res = tieStart(previousNote);
        }
    }

    return res;
}


//---------------------------------------------------------
// wedgeStart - return any wedge starting at note
//---------------------------------------------------------

const EncMeasureElemOrnament* NoteConnector::wedgeStart(const EncMeasureElemNote* const note) const
{
    const auto resultPair = m_wedgeStarts.find(note);
    if (resultPair != m_wedgeStarts.end()) {
        return resultPair->second;
    }

    return nullptr;
}


//---------------------------------------------------------
// wedgeStop - return any wedge stopping at note
//---------------------------------------------------------

const EncMeasureElemOrnament* NoteConnector::wedgeStop(const EncMeasureElemNote* const note) const
{
    const auto resultPair = m_wedgeStops.find(note);
    if (resultPair != m_wedgeStops.end()) {
        return resultPair->second;
    }

    return nullptr;
}
