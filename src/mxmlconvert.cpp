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
#include "mxmlconvert.h"


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
//---------------------------------------------------------

TupletState TupletHandler::newNote(const quint8 actualNotes, const quint8 normalNotes, const quint8 faceValue)
{
    TupletState res { TupletState::NONE };
    char const * printableRes { nullptr };
    if (actualNotes <= 0 || normalNotes <= 0) {
        printableRes = "none";
        m_count = 0;
    }
    else {
        if (m_count == 0) {
            printableRes = "start";
            res = TupletState::START;
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
                res = TupletState::STOP;
                m_count = 0;
            }
            else {
                res = TupletState::MID;
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
    : m_ef(ef)
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
    m_writer.writeXmlHeader();
    m_writer.writeElementStart("", "score-partwise");
    work();
    identification();
    partList();
    parts();
    m_writer.writeElementEnd("", "score-partwise");
}


//---------------------------------------------------------
// attributes - write the attributes
//---------------------------------------------------------

void MxmlConverter::attributes(const int partNr)
{
    m_writer.writeElementStart("      ", "attributes");
    m_writer.writeDivisions(240);
    key();
    time();
    m_writer.writeStaves(m_ef.staves().at(partNr).m_nstaves);
    clefs(partNr);
    m_writer.writeElementEnd("      ", "attributes");
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
    if (hasMeasures) {
        const int nstaves = m_ef.staves().at(partNr).m_nstaves;
        const auto& encline = m_ef.lines().at(0);   // first system
        for (int i = 0; i < nstaves; ++i) {
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
    return v.at(key);
}


//---------------------------------------------------------
// key - write the key
// TBD (too) simple implementation: use keysig of first staff of first measure only
//---------------------------------------------------------

void MxmlConverter::key()
{
    const bool hasMeasures = m_ef.measures().size() > 0;
    if (hasMeasures) {
        const auto& line = m_ef.lines().at(0);
        const auto& data = line.lineStaffData().at(0);
        quint8 kcType = data.m_key;
        m_writer.writeKey(encKeyToFifths(kcType));
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
    int duration = faceValue2duration(note->m_faceValue & 0x0F);
    for (int i = 0; i < (note->m_dotControl & 3); ++i) {
        duration *= 3;
        duration /= 2;
    }

    if (note->actualNotes() > 0 && note->normalNotes() > 0) {
        duration *= note->normalNotes();
        duration /= note->actualNotes();
    }

    if (isGrace(note)) {
        return 0;
    }

    return duration;
}


//---------------------------------------------------------
// Determine if rest's duration
// TODO: refactor (common code shared by note and rest
//---------------------------------------------------------

static int durationRest(const EncMeasureElemRest* const rest)
{
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
    return note1 && note2
            && note1->m_tick == note2->m_tick
            && note1->m_xoffset == note2->m_xoffset;
}


//---------------------------------------------------------
// measure - write a measure of a part
//---------------------------------------------------------

void MxmlConverter::measure(const int partNr, const size_t measureNr)
{
    if (measureNr >= m_ef.measures().size())
        return;

    const QString measurePlusNumber = QString("measure number=\"%1\"").arg(measureNr + 1);
    m_writer.writeElementStart("    ", measurePlusNumber);

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

    if (measureNr == 0)
        attributes(partNr);
    else if (keyCh)
        keyChange(keyCh);

    qDebug() << "xxx_repeat_sym"
             << "measureNr" << measureNr
             << "m_coda" << m.m_coda
             << "m.repeat()" << static_cast<unsigned int>(m.repeat())
                ;

    if (partNr == 0) {
        // write repeat only for first staff
        repeatLeft(m.repeat());
    }

    TupletHandler th;

    // write notes and rests for all voices. As elem->m_tick sometimes differs slightly
    // from the expected value, simply assume all voices start at tick = 0 and no gaps
    // are present.

    int tick = 0;
    for (const auto v : voices) {
        const EncMeasureElemNote* prevnote = nullptr;
        if (tick > 0) {
            // explicit backup to prevent error message at start of voice
            m_writer.writeBackupForward(-tick, 0);
            tick = 0;
        }
        for (const auto& elem : m.measureElems()) {
            if (elem->m_staffIdx == partNr && elem->m_voice == v) {
                int duration = 0;
                if (const EncMeasureElemNote* const curnote = dynamic_cast<const EncMeasureElemNote* const>(elem)) {
                    /*
                    if (elem->m_tick != tick) {
                        qDebug() << "xxx_voice_timing"
                                 << "tick_delta"
                                 << "measureNr" << measureNr
                                 << "voice" << v
                                 << "tick" << tick
                                 << "elem->m_tick" << elem->m_tick
                                    ;
                        writeBackupForward(elem->m_tick - tick, v);
                        tick = elem->m_tick;
                    }
                    */
                    const bool isChord = notesAreInChord(prevnote, curnote);
                    if (isChord) {
                        qDebug() << "xxx_chord"
                                 << "measureNr" << measureNr
                                 << "voice" << v
                                 << "elem->m_tick" << elem->m_tick
                                    ;
                    }
                    note(curnote, partNr, th, isChord);
                    duration = isChord ? 0 : durationNote(curnote);
                    prevnote = curnote;
                }
                else if (const EncMeasureElemRest* const currest = dynamic_cast<const EncMeasureElemRest* const>(elem)) {
                    /*
                    if (elem->m_tick != tick) {
                        qDebug() << "xxx_voice_timing"
                                 << "measureNr" << measureNr
                                 << "voice" << v
                                 << "tick" << tick
                                 << "elem->m_tick" << elem->m_tick
                                    ;
                        writeBackupForward(elem->m_tick - tick, v);
                        tick = elem->m_tick;
                    }
                    */
                    rest(currest, partNr, th);
                    duration = durationRest(currest);
                    prevnote = nullptr; // can't be part of chord
                }
                tick += duration;
            }
        }
    }

    if (partNr == 0) {
        // write repeat only for first staff
        m_writer.writeRepeatRight(encRepeatToWords(m.repeat()));
    }

    barlineRight(partNr, measureNr);

    m_writer.writeElementEnd("    ", "measure");
}


//---------------------------------------------------------
// note - write a note
//---------------------------------------------------------

void MxmlConverter::note(const EncMeasureElemNote* const note, const int partNr, TupletHandler& th, const bool chord)
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
    m_writer.writeElementStart("      ", "note");

    m_writer.writeGrace(note->graceType());

    if (chord) {
        m_writer.writeElement("        ", "chord");
    }

    m_writer.writePitch(step, alter, octave);

    if (!isGrace(note)) {
        m_writer.writeElement("        ", "duration", durationNote(note));
    }

    m_writer.writeVoice(hasMultipleVoices(partNr), note->m_voice + 1);
    m_writer.writeElement("        ", "type", faceValue2xml(note->m_faceValue & 0x0F));
    m_writer.writeDots(note->m_dotControl & 3);
    m_writer.writeTimeModification(note->actualNotes(), note->normalNotes());
    const int nstaves = m_ef.staves().at(partNr).m_nstaves;
    m_writer.writeStaff(nstaves, (note->m_voice < 4) ? 1 : 2);
    const auto tupletState = th.newNote(note->actualNotes(), note->normalNotes(), note->m_faceValue & 0x0F);
    m_writer.writeTuplet(tupletState);
    m_writer.writeElementEnd("      ", "note");
}


//---------------------------------------------------------
// part - write part n
//---------------------------------------------------------

void MxmlConverter::part(const int n)
{
    const QString partPlusId = QString("part id=\"P%1\"").arg(n + 1);
    m_writer.writeElementStart("  ", partPlusId);
    for (unsigned int i = 0; i < m_ef.measures().size(); ++i)
        measure(n, i);
    m_writer.writeElementEnd("  ", "part");
}


//---------------------------------------------------------
// partList - write the part list
//---------------------------------------------------------

void MxmlConverter::partList()
{
    m_writer.writeElementStart("  ", "part-list");
    int count = 0;
    for (const auto& s : m_ef.staves()) {
        ++count;
        scorePart(count, s);
    }
    m_writer.writeElementEnd("  ", "part-list");
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

void MxmlConverter::rest(const EncMeasureElemRest* const rest, const int partNr, TupletHandler &th)
{
    m_writer.writeElementStart("      ", "note");
    m_writer.writeElement("        ", "rest");
    m_writer.writeElement("        ", "duration", durationRest(rest));
    m_writer.writeVoice(hasMultipleVoices(partNr), rest->m_voice + 1);
    m_writer.writeElement("        ", "type", faceValue2xml(rest->m_faceValue & 0x0F));
    m_writer.writeDots(rest->m_dotControl & 3);
    m_writer.writeTimeModification(rest->actualNotes(), rest->normalNotes());
    const int nstaves = m_ef.staves().at(partNr).m_nstaves;
    m_writer.writeStaff(nstaves, (rest->m_voice < 4) ? 1 : 2);
    const auto tupletState = th.newNote(rest->actualNotes(), rest->normalNotes(), rest->m_faceValue & 0x0F);
    m_writer.writeTuplet(tupletState);
    m_writer.writeElementEnd("      ", "note");
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
