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

#include <set>

#include <QFile>
#include <QtDebug>

#include "encfile.h"
#include "mxmlconverter.h"


//---------------------------------------------------------
// faceValue2duration - convert Encore note type to duration
//---------------------------------------------------------

/*
 * note Encore uses:
 * value 3 -> 240
 * value 4 -> 120
 */

static int faceValue2duration(const quint8 faceValue)
{
    switch (faceValue) {
    //case 0: return "0";
    case 1: return  960;
    case 2: return  480;
    case 3: return  240;
    case 4: return  120;
    case 5: return   60;
    case 6: return   30;
    case 7: return   15;
    case 8: return    7;
    }
    return 0;
}


//---------------------------------------------------------
// calculateDots - calculate number of dots from duration and faceValue
// A dotted note has duration = base * 1.5, double dotted = base * 1.75, etc.
//---------------------------------------------------------

static int calculateDots(const int realDuration, const quint8 faceValue)
{
    int baseDuration = faceValue2duration(faceValue & 0x0F);
    if (baseDuration <= 0 || realDuration <= 0) {
        return 0;
    }

    // Check for dotted durations
    // 1 dot: base * 3/2 = base * 1.5
    // 2 dots: base * 7/4 = base * 1.75
    // 3 dots: base * 15/8 = base * 1.875

    if (realDuration == baseDuration) {
        return 0;
    } else if (realDuration == (baseDuration * 3) / 2) {
        return 1;
    } else if (realDuration == (baseDuration * 7) / 4) {
        return 2;
    } else if (realDuration == (baseDuration * 15) / 8) {
        return 3;
    }

    return 0;
}


//---------------------------------------------------------
// detectTuplet - detect tuplet from duration and faceValue
// Returns actual notes (3 for triplet), 0 if not a tuplet
// Sets normalNotes through output parameter
//---------------------------------------------------------

static int detectTuplet(const int realDuration, const quint8 faceValue, int& normalNotes)
{
    int baseDuration = faceValue2duration(faceValue & 0x0F);
    if (baseDuration <= 0 || realDuration <= 0) {
        normalNotes = 0;
        return 0;
    }

    // Triplet (3:2): duration = base * 2/3
    // e.g., eighth note base=120, triplet eighth=80
    if (realDuration == (baseDuration * 2) / 3) {
        normalNotes = 2;
        return 3;
    }

    // Quintuplet (5:4): duration = base * 4/5
    if (realDuration == (baseDuration * 4) / 5) {
        normalNotes = 4;
        return 5;
    }

    // Sextuplet (6:4): duration = base * 4/6 = base * 2/3 (same as triplet)
    // Already covered by triplet case

    normalNotes = 0;
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
// correctNoteType - correct note type to match actual duration
// Common Encore bug: last note in measure has wrong duration
// because m_durTicks is incorrect. Instead of changing duration
// (which would overflow the measure), change the note type to match.
//---------------------------------------------------------

static QString correctNoteType(const int realDuration, const quint8 faceValue)
{
    // Get the type that faceValue would give
    QString originalType = faceValue2xml(faceValue & 0x0F);

    if (realDuration <= 0) {
        return originalType;
    }

    // Find what note type matches the actual duration
    // Check common durations: whole=960, half=480, quarter=240, eighth=120, 16th=60, 32nd=30
    QString correctType = originalType;

    if (realDuration == 960) correctType = "whole";
    else if (realDuration == 480) correctType = "half";
    else if (realDuration == 240) correctType = "quarter";
    else if (realDuration == 120) correctType = "eighth";
    else if (realDuration == 60) correctType = "16th";
    else if (realDuration == 30) correctType = "32nd";
    else if (realDuration == 15) correctType = "64th";
    // Also check dotted values
    else if (realDuration == 720) correctType = "half";      // dotted half
    else if (realDuration == 360) correctType = "quarter";   // dotted quarter
    else if (realDuration == 180) correctType = "eighth";    // dotted eighth
    else if (realDuration == 90) correctType = "16th";       // dotted 16th
    else if (realDuration == 45) correctType = "32nd";       // dotted 32nd
    // Triplet durations (3:2 ratio, so 2/3 of normal duration)
    else if (realDuration == 640) correctType = "whole";     // whole triplet
    else if (realDuration == 320) correctType = "half";      // half triplet
    else if (realDuration == 160) correctType = "quarter";   // quarter triplet
    else if (realDuration == 80) correctType = "eighth";     // eighth triplet
    else if (realDuration == 40) correctType = "16th";       // 16th triplet
    else if (realDuration == 20) correctType = "32nd";       // 32nd triplet (fusa)
    else if (realDuration == 10) correctType = "64th";       // 64th triplet

    if (correctType != originalType) {
        qDebug() << "xxx_type_fix: correcting type from" << originalType
                 << "to" << correctType << "for duration" << realDuration;
    }

    return correctType;
}


//---------------------------------------------------------
// encClef2xml - convert Encore to MusicXML clef type
//---------------------------------------------------------

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
// Groups tuplets by their natural duration - all tuplet notes within the same
// tuplet group belong together. The group duration depends on note value:
// - Eighth triplet (80 ticks each): group = 240 ticks (one quarter beat)
// - Quarter triplet (160 ticks each): group = 480 ticks (one half beat)
// - Sixteenth triplet (40 ticks each): group = 120 ticks (one eighth beat)
//---------------------------------------------------------

TupletState TupletHandler::newNote(const quint8 actualNotes, const quint8 normalNotes, const int tick, const int duration)
{
    TupletState res { TupletState::NONE };

    if (actualNotes <= 0 || normalNotes <= 0) {
        // Not a tuplet note
        if (m_inTuplet) {
            // End previous tuplet group
            res = TupletState::STOP;
            m_inTuplet = false;
            m_groupStartTick = -1;
            m_groupDuration = 0;
        }
        return res;
    }

    // Calculate the total duration of this tuplet group
    // For 3:2, three notes replace two normal notes, so group = duration * actualNotes
    int groupDuration = duration * actualNotes;

    if (!m_inTuplet) {
        // Start a new tuplet group
        res = TupletState::START;
        m_inTuplet = true;
        m_groupStartTick = tick;
        m_groupDuration = groupDuration;
    }
    else {
        // Check if this note belongs to a different tuplet group
        // A new group starts only if we've exceeded the current group's duration
        // Don't start new group just because note value changed (allows mixed eighths/sixteenths)
        if (tick >= m_groupStartTick + m_groupDuration) {
            // Close previous tuplet and start new one
            res = TupletState::STOPSTART;
            m_groupStartTick = tick;
            m_groupDuration = groupDuration;
        }
    }
    // else: continue in same tuplet group (no action needed)

    return res;
}


//---------------------------------------------------------
// nrOfVoicesInPart - count the voices in a part
//---------------------------------------------------------

static int nrOfVoicesInPart(const EncFile& ef, const int firstStaff, const int nstaves)
{
    std::set<quint8> voices;
    for (const auto& m : ef.measures()) {
        for (const auto e : m.measureElems()) {
            if (firstStaff <= e->m_staffIdx && e->m_staffIdx < (firstStaff + nstaves))
                voices.insert(e->m_voice);
        }
    }

    return voices.size();
}


//---------------------------------------------------------
// initVoicesPerPart - count the voices in all parts
//---------------------------------------------------------

void MxmlConverter::initVoicesPerPart()
{
    int count = 0;
    for (size_t i = 0; i < m_ef.staves().size(); ++i) {
        m_voicesPerPart.push_back(nrOfVoicesInPart(m_ef, count, 1));
        qDebug()
                << "initVoicesPerPart"
                << "part" << count + 1
                << "voices" << m_voicesPerPart.at(count)
                   ;
        ++count;
    }

}


//---------------------------------------------------------
// MxmlFile - MusicXML converter constructor
//---------------------------------------------------------

MxmlConverter::MxmlConverter(const EncFile& ef)
    : m_ef(ef), m_nc(ef)
{
    initVoicesPerPart();
}


//---------------------------------------------------------
// convertEncToMxml - convert Encore to MusicXML
//---------------------------------------------------------

void MxmlConverter::convertEncToMxml()
{
    qDebug() << "MxmlConverter::convertEncToMxml()";
    QFile outFile;
    outFile.open(stdout, QFile::WriteOnly);
    m_writer.setDevice(&outFile);
    m_writer.writeBegin();
    m_writer.writeElementStart("score-partwise");
    work();
    identification();
    partList();
    parts();
    m_writer.writeElementEnd();
    m_writer.writeEnd();
}


//---------------------------------------------------------
// attributes - write the attributes
//---------------------------------------------------------

void MxmlConverter::attributes(const int partNr)
{
    m_writer.writeElementStart("attributes");
    m_writer.writeDivisions(240);
    key();
    time();
    const int nstaves = (partNr < static_cast<int>(m_ef.staves().size())) ? m_ef.staves().at(partNr).m_nstaves : 1;
    m_writer.writeStaves(nstaves);
    clefs(partNr);
    m_writer.writeElementEnd();
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
// barlineLeft - write left barline
//---------------------------------------------------------

void MxmlConverter::barlineLeft(const int partNr, const size_t measureNr)
{
    const auto& m = m_ef.measures().at(measureNr);

    const bool repeatStart = (m.barTypeStart() == barlineType::REPEATSTART);
    const bool barlineDblLeft = (m.barTypeStart() == barlineType::DOUBLEL);
    const bool endingStart = isFirstMeasureInAltEnd(m_ef.measures(), measureNr) && partNr == 0;
    const QString endingNumber = repeatAlternative2EndingNumber(m.m_repeatAlternative);

    m_writer.writeBarlineLeft(repeatStart, endingStart, barlineDblLeft, endingNumber);
}


//---------------------------------------------------------
// barlineRight - write right barline
//---------------------------------------------------------

void MxmlConverter::barlineRight(const int partNr, const size_t measureNr)
{
    const auto& m = m_ef.measures().at(measureNr);

    const bool repeatEnd = (m.barTypeEnd() == barlineType::REPEATEND);
    const bool barlineEnd = (m.barTypeEnd() == barlineType::FINAL);
    const bool barlineDbl = (m.barTypeEnd() == barlineType::DOUBLER);
    const bool endingStop = isLastMeasureInAltEnd(m_ef.measures(), measureNr) && partNr == 0;

    m_writer.writeBarlineRight(repeatEnd, endingStop, barlineEnd, barlineDbl, m.m_repeatAlternative);
}


//---------------------------------------------------------
// clefs - write the clef(s) for part partNr
//---------------------------------------------------------

void MxmlConverter::clefs(const int partNr)
{
    // TBD (too) simple implementation: use clef of first measure only
    const bool hasMeasures = m_ef.measures().size() > 0;
    if (hasMeasures && m_ef.lines().size() > 0) {
        const int nstaves = (partNr < static_cast<int>(m_ef.staves().size())) ? m_ef.staves().at(partNr).m_nstaves : 1;
        const auto& encline = m_ef.lines().at(0);   // first system
        for (int i = 0; i < nstaves; ++i) {
            if (static_cast<size_t>(partNr + i) >= encline.lineStaffData().size()) {
                break;
            }
            const auto& data = encline.lineStaffData().at(partNr + i);
            const auto ct = data.m_clef;
            QString sign;
            int line { 0 };
            int octCh { 0 };
            if (encClef2xml(ct, sign, line, octCh)) {
                m_writer.writeClef((nstaves > 1) ? i : -1, sign, line, octCh);
            }
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
// identification - write the identification
//---------------------------------------------------------

void MxmlConverter::identification()
{
    const EncTitle& ttl = m_ef.title();
    const auto author = createMultiLineString(ttl.m_author);
    const auto lyricist = createMultiLineString(ttl.m_instruction);
    const auto rights = createMultiLineString(ttl.m_copyright);
    const QString software = "Enc2MusicXML";

    m_writer.writeIdentification(author, lyricist, rights, software);
}


//---------------------------------------------------------
// encClef2xml - convert Encore key to MusicXML fifths
//---------------------------------------------------------

int encKeyToFifths(unsigned int key)
{
    std::vector<int> v =
    {
        //   c   f  bf  ef  af  df  gf  cf  g   d   a   e   b  fs  cs
        /**/ 0, -1, -2, -3, -4, -5, -6, -7, 1,  2,  3,  4,  5,  6,  7
    };
    if (key >= v.size()) {
        qDebug() << "encKeyToFifths: key out of range:" << key;
        return 0;
    }
    return v.at(key);
}


//---------------------------------------------------------
// key - write the key
// TBD (too) simple implementation: use keysig of first staff of first measure only
//---------------------------------------------------------

void MxmlConverter::key()
{
    const bool hasMeasures = m_ef.measures().size() > 0;
    if (hasMeasures && m_ef.lines().size() > 0) {
        const auto& line = m_ef.lines().at(0);
        if (line.lineStaffData().size() > 0) {
            const auto& data = line.lineStaffData().at(0);
            quint8 kcType = data.m_key;
            m_currentFifths = encKeyToFifths(kcType);
            m_writer.writeKey(m_currentFifths);
        }
    }
}


//---------------------------------------------------------
// findKeyChange - write a key change in measure m
//---------------------------------------------------------

const EncMeasureElemKeyChange* findKeyChange(const EncMeasure& m)
{
    for (const auto elem : m.measureElems()) {
        if (const EncMeasureElemKeyChange* const key = dynamic_cast<const EncMeasureElemKeyChange* const>(elem))
            return key;
    }
    return nullptr;
}


//---------------------------------------------------------
// keyChange - write a key change
//---------------------------------------------------------

void MxmlConverter::keyChange(const EncMeasureElemKeyChange* keyCh)
{
    quint8 kcType = keyCh->m_tipo;
    const auto fifths = encKeyToFifths(kcType);
    m_writer.writeKeyChange(fifths);
    qDebug()
            << "writeKeyChange"
            << "kcType" << kcType
            << "fifths" << fifths
               ;
}


//---------------------------------------------------------
// Determine if note is a grace note
//---------------------------------------------------------

static bool isGrace(const EncMeasureElemNote* const note)
{
    return note->graceType() != GraceType::NORMALNOTE;
}


//---------------------------------------------------------
// Determine if note's duration
// TODO: refactor (common code shared by note and rest
//---------------------------------------------------------

static int durationNote(const EncMeasureElemNote* const note)
{
    if (isGrace(note)) {
        return 0;
    }

    // Calculate expected duration from faceValue
    int expectedDuration = faceValue2duration(note->m_faceValue & 0x0F);
    for (int i = 0; i < (note->m_dotControl & 3); ++i) {
        expectedDuration *= 3;
        expectedDuration /= 2;
    }

    // Apply time modification (tuplet) if present
    if (note->actualNotes() > 0 && note->normalNotes() > 0) {
        expectedDuration *= note->normalNotes();
        expectedDuration /= note->actualNotes();
    }

    // Use real duration if available
    // Note: Even if duration seems wrong for a tuplet, trust the Encore file
    // as it may be a tied note split across measure boundaries
    if (note->m_realDuration > 0) {
        return note->m_realDuration;
    }

    return expectedDuration;
}


//---------------------------------------------------------
// Determine if rest's duration
// TODO: refactor (common code shared by note and rest
//---------------------------------------------------------

static int durationRest(const EncMeasureElemRest* const rest)
{
    // Use real duration calculated from ticks if available
    if (rest->m_realDuration > 0) {
        return rest->m_realDuration;
    }

    // Fall back to calculated duration from faceValue
    int duration = faceValue2duration(rest->m_faceValue & 0x0F);
    for (int i = 0; i < (rest->m_dotControl & 3); ++i) {
        duration *= 3;
        duration /= 2;
    }

    if (rest->actualNotes() > 0 && rest->normalNotes() > 0) {
        duration *= rest->normalNotes();
        duration /= rest->actualNotes();
    }

    return duration;
}


//---------------------------------------------------------
// debug
//---------------------------------------------------------

static void dump_note_timing_measure_elem(const EncMeasureElem* const elem, const QString& id)
{
    if (const EncMeasureElemNote* const note = dynamic_cast<const EncMeasureElemNote* const>(elem)) {
        qDebug() << id
                 << "staff" << note->m_staffIdx
                 << "voice" << note->m_voice
                 << "tick" << note->m_tick
                 << "x_offset" << note->m_xoffset
                 << "note"
                 << "value" << (note->m_faceValue & 0x0F)
                 << "dots" << (note->m_dotControl & 3)
                 << "actual" << note->actualNotes()
                 << "normal" << note->normalNotes()
                 << "calcdur" << durationNote(note)
                 << (isGrace(note) ? "grc" : "nrm")
                 << ((note->m_grace1 & 0x30) >> 4)
                 << (note->m_grace2 & 0x05)
                 << "pbdur" << note->m_playbackDurTicks
                 << "nexttick" << (note->m_tick + durationNote(note))
                    ;
    }
    else if (const EncMeasureElemRest* const rest = dynamic_cast<const EncMeasureElemRest* const>(elem)) {
        qDebug() << id
                 << "staff" << rest->m_staffIdx
                 << "voice" << rest->m_voice
                 << "tick" << rest->m_tick
                 << "x_offset" << rest->m_xoffset
                 << "rest"
                 << "value" << (rest->m_faceValue & 0x0F)
                 << "dots" << (rest->m_dotControl & 3)
                 << "calcdur" << durationRest(rest)
                 << "nexttick" << (rest->m_tick + durationRest(rest))
                    ;
    }
}


//---------------------------------------------------------
// debug
//---------------------------------------------------------

static void dump_note_timing_measure_elems(const EncMeasure& m, const QString& id)
{
    for (const auto& elem : m.measureElems()) {
        dump_note_timing_measure_elem(elem, id);
    }
}


//---------------------------------------------------------
// encRepeatToWords - convert Encore repeats to MusicXML words
//---------------------------------------------------------

static QString encRepeatToWords(const repeatType repeat)
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

    return words;
}


//---------------------------------------------------------
// notesAreInChord - determine if note1 and note2 are in a chord together
//---------------------------------------------------------

static bool notesAreInChord(const EncMeasureElemNote* const note1,
                            const EncMeasureElemNote* const note2)
{
    // Notes are in a chord if they have the same tick
    // x_offset is just visual positioning and should not affect chord detection
    return note1 && note2
            && note1->m_tick == note2->m_tick;
}


//---------------------------------------------------------
// measure - write a measure of a part
//---------------------------------------------------------

void MxmlConverter::measure(const int partNr, const size_t measureNr)
{
    if (measureNr >= m_ef.measures().size())
        return;

    m_writer.writeElementStartWithAttribute("measure", "number", measureNr + 1);

    const auto& m = m_ef.measures().at(measureNr);
    const auto keyCh = findKeyChange(m);


    qDebug() << "xxx_note_timing"
             << "measureNr" << measureNr
                ;
    dump_note_timing_measure_elems(m, "xxx_note_timing");

    std::set<quint8> voices;
    for (const auto e : m.measureElems()) {
        if (e->m_staffIdx == partNr)
            voices.insert(e->m_voice);
    }

    qDebug() << "xxx_voice_timing"
             << "measureNr" << measureNr
                ;
    for (const auto v : voices) {
        qDebug() << "xxx_voice_timing"
                 << "voice" << v
                    ;
        for (const auto& elem : m.measureElems()) {
            if (elem->m_staffIdx == partNr && elem->m_voice == v) {
                dump_note_timing_measure_elem(elem, "xxx_voice_timing");
            }
        }
    }

    barlineLeft(partNr, measureNr);

    // Check for time signature change
    bool timeSigChanged = false;
    if (measureNr > 0) {
        const auto& prevM = m_ef.measures().at(measureNr - 1);
        if (m.m_timeSigNum != prevM.m_timeSigNum || m.m_timeSigDen != prevM.m_timeSigDen) {
            timeSigChanged = true;
        }
    }

    if (measureNr == 0)
        attributes(partNr);
    else if (keyCh || timeSigChanged) {
        // Write attributes with key change and/or time signature change
        m_writer.writeElementStart("attributes");
        if (keyCh) {
            quint8 kcType = keyCh->m_tipo;
            m_currentFifths = encKeyToFifths(kcType);
            m_writer.writeKey(m_currentFifths);
            qDebug() << "writeKeyChange" << "kcType" << kcType << "fifths" << m_currentFifths;
        }
        if (timeSigChanged) {
            m_writer.writeTime(m.m_timeSigNum, m.m_timeSigDen);
            qDebug() << "writeTimeChange" << m.m_timeSigNum << "/" << m.m_timeSigDen;
        }
        m_writer.writeElementEnd();
    }

    qDebug() << "xxx_repeat_sym"
             << "measureNr" << measureNr
             << "m_coda" << m.m_coda
             << "m.repeat()" << static_cast<unsigned int>(m.repeat())
                ;

    if (partNr == 0) {
        // write repeat only for first staff
        repeatLeft(m.repeat());
    }

    // write notes and rests for all voices. As elem->m_tick sometimes differs slightly
    // from the expected value, simply assume all voices start at tick = 0 and no gaps
    // are present.

    int tick = 0;
    for (const auto v : voices) {
        TupletHandler th;  // Each voice has its own tuplet handler
        const EncMeasureElemNote* prevnote = nullptr;
        if (tick > 0) {
            // explicit backup to prevent error message at start of voice
            m_writer.writeBackupForward(-tick, 0);
            tick = 0;
        }

        // First pass: collect valid elements for this voice
        std::vector<EncMeasureElem*> voiceElems;
        for (const auto& elem : m.measureElems()) {
            if (elem->m_staffIdx == partNr && elem->m_voice == v) {
                if (elem->m_tick > m.m_durTicks) continue;  // Skip garbage
                if (const auto* note = dynamic_cast<const EncMeasureElemNote*>(elem)) {
                    const bool isChord = notesAreInChord(prevnote, note);
                    if (!isChord && tick >= m.m_durTicks) continue;  // Skip extra notes
                    voiceElems.push_back(elem);
                    tick += isChord ? 0 : durationNote(note);
                    prevnote = note;
                }
                else if (const auto* rest = dynamic_cast<const EncMeasureElemRest*>(elem)) {
                    int restDur = durationRest(rest);
                    if (restDur <= 0) continue;  // Skip invalid rests
                    voiceElems.push_back(elem);
                    tick += restDur;
                    prevnote = nullptr;
                }
            }
        }

        // Reset for second pass
        tick = 0;
        prevnote = nullptr;

        // Second pass: write elements with proper tuplet handling
        for (size_t i = 0; i < voiceElems.size(); ++i) {
            const auto& elem = voiceElems[i];

            // Check if next NON-CHORD element is a tuplet note (for closing current tuplet)
            // We need to skip chord notes to find the actual next musical event
            bool nextNonChordIsTuplet = false;
            bool isLastNonChordInMeasure = true;
            const EncMeasureElemNote* curAsNote = dynamic_cast<const EncMeasureElemNote*>(elem);
            for (size_t j = i + 1; j < voiceElems.size(); ++j) {
                if (const auto* nextNote = dynamic_cast<const EncMeasureElemNote*>(voiceElems[j])) {
                    // Check if this is a chord note (same tick as current)
                    if (curAsNote && notesAreInChord(curAsNote, nextNote)) {
                        continue;  // Skip chord notes
                    }
                    // Found next non-chord note
                    nextNonChordIsTuplet = (nextNote->actualNotes() > 0 && nextNote->normalNotes() > 0);
                    isLastNonChordInMeasure = false;
                    break;
                }
                else if (const auto* nextRest = dynamic_cast<const EncMeasureElemRest*>(voiceElems[j])) {
                    nextNonChordIsTuplet = (nextRest->actualNotes() > 0 && nextRest->normalNotes() > 0);
                    isLastNonChordInMeasure = false;
                    break;
                }
            }

            int duration = 0;
            if (const EncMeasureElemNote* const curnote = dynamic_cast<const EncMeasureElemNote* const>(elem)) {
                const bool isChord = notesAreInChord(prevnote, curnote);

                // Generate forward or backup if note doesn't start at current tick position
                if (!isChord && elem->m_tick != tick) {
                    m_writer.writeBackupForward(elem->m_tick - tick, v);
                    tick = elem->m_tick;
                }

                const auto direction = m_nc.direction(curnote);
                if (direction
                        && direction->type() == ornamentType::STAFFTEXT
                        && direction->m_tind < m_ef.text().m_texts.size()) {
                    m_writer.writeWords(m_ef.text().m_texts.at(direction->m_tind));
                }
                else if (direction
                         && direction->type() == ornamentType::TEMPO) {
                    m_writer.writeMetronome(faceValue2xml((direction->m_noto & 0x0F) + 1),
                                            (direction->m_noto & 0x80) ? 1 : 0,
                                            direction->m_tempo);
                }

                const auto wedgeStart = m_nc.wedgeStart(curnote);
                const auto wedgeStop = m_nc.wedgeStop(curnote);

                if (wedgeStart) {
                    m_writer.writeWedge((wedgeStart->m_speguleco & 0x01)
                                        ? WedgeType::DIMINUENDO
                                        : WedgeType::CRESCENDO);
                }

                // Pass info about whether to force close tuplet
                bool forceCloseTuplet = th.needsClose() && (!nextNonChordIsTuplet || isLastNonChordInMeasure);
                // Pass calculated tick for tuplet grouping (m_tick from Encore can be inconsistent)
                note(curnote, partNr, th, isChord, forceCloseTuplet, tick);
                duration = isChord ? 0 : durationNote(curnote);
                prevnote = curnote;

                if (wedgeStop) {
                    m_writer.writeWedge(WedgeType::STOP);
                }
            }
            else if (const EncMeasureElemRest* const currest = dynamic_cast<const EncMeasureElemRest* const>(elem)) {
                if (elem->m_tick != tick) {
                    m_writer.writeBackupForward(elem->m_tick - tick, v);
                    tick = elem->m_tick;
                }
                bool forceCloseTuplet = th.needsClose() && (!nextNonChordIsTuplet || isLastNonChordInMeasure);
                // Pass calculated tick for tuplet grouping
                rest(currest, partNr, th, forceCloseTuplet, tick);
                duration = durationRest(currest);
                prevnote = nullptr;
            }
            tick += duration;
        }

        // Reset tuplet handler state for next voice
        if (th.needsClose()) {
            th.close();
        }
    }

    if (partNr == 0) {
        // write repeat only for first staff
        m_writer.writeRepeatRight(encRepeatToWords(m.repeat()));
    }

    barlineRight(partNr, measureNr);

    m_writer.writeElementEnd();
}


//---------------------------------------------------------
// note - write a note
//---------------------------------------------------------

void MxmlConverter::note(const EncMeasureElemNote* const note, const int partNr, TupletHandler& th, const bool chord, const bool forceCloseTuplet, const int calculatedTick)
{
    char step = ' ';
    int alter = 0;
    int octave = 0;
    const int noteDur = durationNote(note);  // Calculate duration once for consistency

    // Use current key signature (tracked through key changes) for pitch spelling
    midipitch2xml(note->m_semiTonePitch, static_cast<accidentalType>(note->m_alterationGlyph), m_currentFifths, step, alter, octave);
    m_writer.writeElementStart("note");

    m_writer.writeGrace(note->graceType());

    if (chord) {
        m_writer.writeElement("chord");
    }

    m_writer.writePitch(step, alter, octave);

    if (!isGrace(note)) {
        m_writer.writeElement("duration", noteDur);
    }

    const bool tieStart = m_nc.tieStart(note);
    const bool tieStop = m_nc.tieStop(note);

    if (tieStop) {
        m_writer.writeTie(StartStop::STOP);
    }
    if (tieStart) {
        m_writer.writeTie(StartStop::START);
    }

    m_writer.writeVoice(hasMultipleVoices(partNr), note->m_voice + 1);
    // Use correctNoteType to fix type when duration doesn't match faceValue (common Encore bug)
    m_writer.writeElement("type", correctNoteType(noteDur, note->m_faceValue));
    // Always calculate dots from actual duration (m_dotControl is unreliable in old format files)
    const int noteDots = calculateDots(noteDur, note->m_faceValue);
    m_writer.writeDots(noteDots);

    // Detect tuplet from duration if not set in file
    int noteActual = note->actualNotes();
    int noteNormal = note->normalNotes();
    if (noteActual == 0 && noteDur > 0) {
        noteActual = detectTuplet(noteDur, note->m_faceValue, noteNormal);
    }
    m_writer.writeTimeModification(noteActual, noteNormal);
    const int nstaves = (partNr < static_cast<int>(m_ef.staves().size())) ? m_ef.staves().at(partNr).m_nstaves : 1;
    m_writer.writeStaff(nstaves, (note->m_voice < 4) ? 1 : 2);
    // Don't count chord notes for tuplet state - they're simultaneous with the previous note
    // Use calculated tick for proper grouping (m_tick from Encore can be inconsistent)
    auto tupletState = chord ? TupletState::NONE : th.newNote(noteActual, noteNormal, calculatedTick, noteDur);
    // Force close tuplet if this is the last tuplet note before a non-tuplet or end of measure
    // But NOT for chord notes - they shouldn't carry tuplet brackets
    if (forceCloseTuplet && !chord && tupletState == TupletState::NONE && th.needsClose()) {
        tupletState = TupletState::STOP;
        th.close();
    }
    m_writer.writeTuplet(tupletState);


    const auto slurstart = m_nc.slurStart(note);
    const auto slurstop = m_nc.slurStop(note);

    // ignore overlapping slurs for now
    if (slurstop) {
        m_writer.writeSlur(StartStop::STOP);
    }
    if (slurstart) {
        m_writer.writeSlur(StartStop::START);
    }

    if (tieStop) {
        m_writer.writeTied(StartStop::STOP);
    }
    if (tieStart) {
        m_writer.writeTied(StartStop::START);
    }

    // articulations (for the time being only fermata)
    if (note->articulationUp() == articulationType::FERMATA) {
        m_writer.writeFermata();
    }

    m_writer.writeElementEnd();
}


//---------------------------------------------------------
// part - write part n
//---------------------------------------------------------

void MxmlConverter::part(const int n)
{
    const QString partId = QString("P%1").arg(n + 1);
    m_writer.writeElementStartWithAttribute("part", "id", partId);
    for (unsigned int i = 0; i < m_ef.measures().size(); ++i)
        measure(n, i);
    m_writer.writeElementEnd();
}


//---------------------------------------------------------
// partList - write the part list
//---------------------------------------------------------

void MxmlConverter::partList()
{
    m_writer.writeElementStart("part-list");
    int count = 0;
    for (const auto& s : m_ef.staves()) {
        ++count;
        scorePart(count, s);
    }
    m_writer.writeElementEnd();
}


//---------------------------------------------------------
// parts - write the parts
//---------------------------------------------------------

void MxmlConverter::parts()
{
    for (unsigned int count = 0; count < m_ef.staves().size(); ++count) {
        part(count);
    }
}


//---------------------------------------------------------
// repeatLeft - write repeats at the left side of a measure
//---------------------------------------------------------

void MxmlConverter::repeatLeft(const repeatType repeat)
{
    const bool coda = repeat == repeatType::CODA1 || repeat == repeatType::CODA2;
    const bool segno = repeat == repeatType::SEGNO;

    m_writer.writeRepeatLeft(coda, segno);
}


//---------------------------------------------------------
// rest - write a rest
//---------------------------------------------------------

void MxmlConverter::rest(const EncMeasureElemRest* const rest, const int partNr, TupletHandler &th, const bool forceCloseTuplet, const int calculatedTick)
{
    const int restDur = durationRest(rest);  // Calculate duration once for consistency

    m_writer.writeElementStart("note");
    m_writer.writeElement("rest");
    m_writer.writeElement("duration", restDur);
    m_writer.writeVoice(hasMultipleVoices(partNr), rest->m_voice + 1);
    // Use correctNoteType to fix type when duration doesn't match faceValue (common Encore bug)
    m_writer.writeElement("type", correctNoteType(restDur, rest->m_faceValue));
    // Always calculate dots from actual duration (m_dotControl is unreliable in old format files)
    const int restDots = calculateDots(restDur, rest->m_faceValue);
    m_writer.writeDots(restDots);

    // Use explicit tuplet info from Encore (don't auto-detect for rests)
    const int restActual = rest->actualNotes();
    const int restNormal = rest->normalNotes();
    m_writer.writeTimeModification(restActual, restNormal);
    const int nstaves = (partNr < static_cast<int>(m_ef.staves().size())) ? m_ef.staves().at(partNr).m_nstaves : 1;
    m_writer.writeStaff(nstaves, (rest->m_voice < 4) ? 1 : 2);
    // Use calculated tick for proper grouping (m_tick from Encore can be inconsistent)
    auto tupletState = th.newNote(restActual, restNormal, calculatedTick, restDur);
    // Force close tuplet if this is the last tuplet note before a non-tuplet or end of measure
    if (forceCloseTuplet && tupletState == TupletState::NONE && th.needsClose()) {
        tupletState = TupletState::STOP;
        th.close();
    }
    m_writer.writeTuplet(tupletState);
    m_writer.writeElementEnd();
}


//---------------------------------------------------------
// scorePart - write score part n
//---------------------------------------------------------

void MxmlConverter::scorePart(const int n, const EncInstrument &instr)
{
    m_writer.writeScorePart(n, instr.m_name);
}


//---------------------------------------------------------
// time - write the time signature
//---------------------------------------------------------

void MxmlConverter::time()
{
    // (too) simple implementation: use timesig of first measure only
    const bool hasMeasures = m_ef.measures().size() > 0;
    if (hasMeasures) {
        m_writer.writeTime(m_ef.measures().at(0).m_timeSigNum, m_ef.measures().at(0).m_timeSigDen);
    }
}


//---------------------------------------------------------
// work - write the work description
//---------------------------------------------------------

void MxmlConverter::work()
{
    const EncTitle& enctitle = m_ef.title();
    const auto subtitle = createMultiLineString(enctitle.m_subtitle);
    m_writer.writeWork(enctitle.m_title, subtitle);
}
