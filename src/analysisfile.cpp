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

#include <iostream>
#include <iomanip>
#include <map>

#include <QtDebug>

#include "encfile.h"
#include "analysisfile.h"

//---------------------------------------------------------
// midipitch2xml
//---------------------------------------------------------

static int alterTab[12] = { 0,   1,   0,   1,   0,  0,   1,   0,   1,   0,   1,   0 };
static char noteTab[12] = { 'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B' };

// determine pitch spelling based on pitch

static void midipitch2xml(const quint8 pitch, char& step, int& alter, int& octave)
{
    // 60 = C 4
    step   = noteTab[pitch % 12];
    alter  = alterTab[pitch % 12];
    octave = pitch / 12 - 1;
}


//---------------------------------------------------------
// faceValue2string
//---------------------------------------------------------

static const char* faceValue2string(const quint8 faceValue)
{
    switch (faceValue) {
    case 1: return "/1";
    case 2: return "/2";
    case 3: return "/4";
    case 4: return "/8";
    case 5: return "/16";
    case 6: return "/32";
    case 7: return "/64";
    case 8: return "/128";
    }

    return "???";
}

static const char* stafftype2string (const quint8 type)
{
    switch (type)
    {
    case 0: return "normal";
    case 1: return "tab";
    case 2: return "perc";
    default: return "unknown";
    }

    Q_UNREACHABLE();
    return nullptr;
}

static const char* cleftype2string (const qint8 type)
{
    switch (type)
    {
    case 0: return "treble";
    case 1: return "bass";
    case 2: return "alto";
    case 3: return "tenor";
    case 4: return "treb^8";
    case 5: return "treb_8";
    case 6: return "bass_8";
    case 7: return "perc";
    case 8: return "tab";
    default: return "unknown";
    }

    Q_UNREACHABLE();
    return nullptr;
}

//   c   f  bf  ef  af  df  gf  cf  g   d   a   e   b  fs  cs

static const char* keytype2string(const quint8 type)
{
    switch (type)
    {
    case  0: return "c";
    case  1: return "f";
    case  2: return "bf";
    case  3: return "ef";
    case  4: return "af";
    case  5: return "df";
    case  6: return "gf";
    case  7: return "cf";
    case  8: return "g";
    case  9: return "d";
    case 10: return "a";
    case 11: return "e";
    case 12: return "b";
    case 13: return "fs";
    case 14: return "cs";
    default: return "unknown";
    }

    Q_UNREACHABLE();
    return nullptr;
}

static const char* ornament2string(const ornamentType type, const quint8 speguleco)
{
    static char buffer[64];

    switch (type)
    {
    case ornamentType::WEDGESTART: return (speguleco & 0x01 ? "Wedge diminuendo" : "Wedge crescendo"); // ST_DINAMIKO
    case ornamentType::WEDGESTOP: return ("Wedge stop"); // ST_DINAMIKOFINO
    case ornamentType::SLURSTART: return ("Slur start"); // ST_LIGARKO
    case ornamentType::SLURSTOP: return ("Slur stop"); // ST_LIGARKOFINO
    default:
        sprintf (buffer, "Ornament 0x%02X", static_cast<unsigned int>(type));
    }

    return buffer;
}


static const char* beam2string(const EncMeasureElemBeam* const /* beam */)
{
    return "Beam";
}


static const char* clef2string(const EncMeasureElemClef* const /* clef */)
{
    return "Clef";
}


static const char* lyric2string(const EncMeasureElemLyric* const /* lyric */)
{
    return "Lyric";
}


static const char* tie2string(const EncMeasureElemTie* const /* tie */)
{
    return "Tie";
}


static const char* chord2string(const EncMeasureElemChord* const /* chord */)
{
    return "Chord";
}


static const char* key2string(const EncMeasureElemKeyChange* const /* key */)
{
    return "Key";
}


static const char* dots2string(quint8 dots)
{
    switch (dots)
    {
    case 0: return "";
    case 1: return "d";
    case 2: return "dd";
    case 3: return "ddd";
    default: return "dots?";
    }

    Q_UNREACHABLE();
    return nullptr;
}


static const char* tuplet2string(quint8 tuplet)
{
    static char buffer[10];

    if (tuplet == 0) {
        return ("");
    }

    sprintf (buffer, " %u/%u", tuplet & 0x0F, (tuplet & 0xF0) >> 4);
    return (buffer);
}


//---------------------------------------------------------
// AnalysisFile - write analysis result
//---------------------------------------------------------

AnalysisFile::AnalysisFile(const EncFile& ef)
    : m_ef(ef)
{

}


void AnalysisFile::write()
{
    qDebug() << "AnalysisFile::write()";
    const EncHeader& hdr = m_ef.header();
    qDebug() << "magic" << hdr.m_magic;
    writeHeader();
    writeTitle();
    writeText();
    writeInstruments();
    writeLines();
    writeMeasures();
}


void AnalysisFile::writeHeader()
{
    const EncHeader& hdr = m_ef.header();
    std::cout
            << "---- HEADER ----" << "\n"
            << std::hex
            << "File magic:\t" << qPrintable(hdr.m_magic) << "\n"
            << "File format:\t" << std::setw(4) << static_cast<unsigned int>(hdr.m_chuMagio) << "\n"
            << "File version:\t" << std::setw(4) << hdr.m_chuVersio << "\n"
            << "Unknown1:\t" << std::setw(4) << hdr.m_nekon1 << "\n"
            << "Unknown2:\t" << std::setw(4) << hdr.m_fiksa1 << "\n"
            << std::dec
            << "N systems:\t" << std::setw(4) << hdr.m_lineCount << "\n"
            << "N pages:\t" << std::setw(4) << hdr.m_pageCount << "\n"
            << "N instruments:\t" << std::setw(4) << static_cast<int>(hdr.m_instrumentCount) << "\n"
            << "N staves:\t" << std::setw(4) << static_cast<int>(hdr.m_staffPerSystem) << "\n"
            << "N measures:\t" << std::setw(4) << hdr.m_measureCount << "\n"
            << "\n";
}


void AnalysisFile::writeTitle()
{
    const EncTitle& ttl = m_ef.title();
    std::cout
            << "---- TITLES ----" << "\n"
            << "Size:\t\t" << ttl.m_varsize << " bytes" << "\n"
            << "Title:\t\t" << qPrintable(ttl.m_title) << "\n";
    for (int i = 0; i < 2 && ttl.m_subtitle.size(); ++i)
        std::cout << "Subtitle " << i + 1 << ":\t" << qPrintable(ttl.m_subtitle.at(i)) << "\n";
    for (int i = 0; i < 3 && ttl.m_instruction.size(); ++i)
        std::cout << "Instruction " << i + 1 << ":\t" << qPrintable(ttl.m_instruction.at(i)) << "\n";
    for (int i = 0; i < 4 && ttl.m_author.size(); ++i)
        std::cout << "Author " << i + 1 << ":\t" << qPrintable(ttl.m_author.at(i)) << "\n";
    for (int i = 0; i < 2 && ttl.m_header.size(); ++i)
        std::cout << "Header " << i + 1 << ":\t" << qPrintable(ttl.m_header.at(i)) << "\n";
    for (int i = 0; i < 2 && ttl.m_footer.size(); ++i)
        std::cout << "Footer " << i + 1 << ":\t" << qPrintable(ttl.m_footer.at(i)) << "\n";
    for (int i = 0; i < 6 && ttl.m_copyright.size(); ++i)
        std::cout << "Copyright " << i + 1 << ":\t" << qPrintable(ttl.m_copyright.at(i)) << "\n";
    std::cout
            << "\n";
}


void AnalysisFile::writeText()
{
    const EncText& txt = m_ef.text();
    std::cout
            << "---- TEXTS ----" << "\n"
            << "N texts:\t" << txt.m_texts.size() << "\n";
    for (unsigned int i = 0; i < txt.m_texts.size(); ++i)
        std::cout << "Text " << i + 1 << ":\t\t" << qPrintable(txt.m_texts.at(i)) << "\n";
    std::cout
            << "\n";
}


void AnalysisFile::writeInstruments()
{
    std::cout
            << "---- INSTRUMENTS ----" << "\n";
    int count = 0;
    for (const auto& s : m_ef.staves()) {
        ++count;
        std::cout
                << std::setw(2) << std::setfill('0') << count << ":\t\t" << qPrintable(s.m_name) << "\n";
    }
    std::cout
            << "\n";
}


void AnalysisFile::writeLines()
{
    std::cout
            << "---- SYSTEMS ----" << "\n";
    int count = 0;
    for (const auto& l : m_ef.lines()) {
        ++count;
        std::cout
                << "--- System " << std::setw(2) << std::setfill('0') << count << "\n"
                << "type:\t\t0x" << std::hex << std::setw(2) << (l.m_offset & 0xFF) << "\n"
                << "measures:\t" << std::dec << static_cast<int>(l.m_measureCount) << "\n"
                   ;
        writeLineStaffData(l);
        std::cout
                << "\n";
    }
}


void AnalysisFile::writeLineStaffData(const EncLine& line)
{
    int count = 0;
    std::cout
            << "Staff\ttype\tclef\tkey" << "\n";
    for (const auto& d : line.lineStaffData()) {
        std::cout
                << count + 1
                << "\t" << stafftype2string(/* TODO */ static_cast<quint8>(d.m_staffType))
                << "\t" << cleftype2string(/* TODO */ static_cast<qint8>(d.m_clef))
                << "\t" << keytype2string(d.m_key) << "\n"
                   ;
        ++count;
    }
}


void AnalysisFile::writeMeasures()
{
    std::cout
            << "---- MEASURES ----" << "\n";
    int count = 0;
    for (const auto& m : m_ef.measures()) {
        ++count;
        std::cout
                << "--- Measure "   << count << "\n"
                << "size:\t\t" << m.m_varsize << " bytes" << "\n"
                << "tempo:\t\t" << m.m_bpm << " BPM" << "\n"
                << "time signature:\t" << static_cast<int>(m.m_timeSigNum) << "/" << static_cast<int>(m.m_timeSigDen) << "\n"
                   //<< "\tStangoj   : " << enc_lily_stango(m.m_barTypeStart) << enc_lily_stango(m.m_barTypeEnd) << "\n"
                   //<< "\tRipetsalto: 0x" << std::hex << static_cast<int>(m.m_repeatAlternative) << std::dec << "\n"
                   //<< "\tSaltsigno : " << enc_lily_saltsigno((m.m_coda >> 8) & 0xFF) << "\n"
                << "--- Elements (in file order):"<< "\n"
                << "xoffs\ttick\tvoice\tstaff\tmdelta\txoffs2\ttype" << "\n"
                   ;

#if 0
        for (int i = 0; i < m_ef.header().m_staffPerSystem; ++i) {
            /* unsorted (in file order */
            for (const auto e : m.measureElems()) {
                if (i == e->m_staffIdx) {
                    writeMeasureElem(e);
                }
            }
            /* sorted on xoffset
            std::multimap<quint8 , const EncMeasureElem* const> mmap;
            for (const auto e : m.measureElems()) {
                if (i == e->m_staffIdx)
                    mmap.insert({e->m_xoffset, e});
            }
            for (const auto& e : mmap) {
                writeMeasureElem(e.second);
            }
            */
            std::cout << "--------------\n";
        }
#endif
#if 1
        for (const auto e : m.measureElems()) {
            writeMeasureElem(e);
        }
#endif
        std::cout << "\n";
    }
}


void AnalysisFile::writeMeasureElem(const EncMeasureElem* const elem)
{
    std::cout
            << static_cast<int>(elem->m_xoffset)
            << "\t" << elem->m_tick
            << "\t" << static_cast<int>(elem->m_voice)
            << "\t" << static_cast<int>(elem->m_staffIdx)
               ;

    if (const EncMeasureElemNote* const note = dynamic_cast<const EncMeasureElemNote* const>(elem)) {
        //qDebug() << "successfully converted to note:" << elem;
        // adorno handling may not support chords yet
        //const auto& line = m_ef.lines().at(0);
        //const auto& lsd = line.lineStaffData().at(note->m_staffIdx);
        //const bool isPercClef = lsd.m_clef== clefType::PERC;
        //const bool isRhythmStaff = lsd.m_staffType == staffType::RHYTHM;
        char step { '?' };
        int alter { 0 };
        int octave { 0 };
        midipitch2xml(note->m_semiTonePitch, step, alter, octave);
        auto adorno = articulationType::UNDEFINED;
        if (note->m_dotControl & 0x80)
            adorno = note->articulationUp();
        else if (note->m_dotControl & 0x40)
            adorno = note->articulationDown();
        std::cout
                << "\t"
                << "-\t-\t"
                   //<< (isPercClef ? enc_lily_vpoz_frape (note->m_position)
                   //               : (isRhythmStaff ? "c" : qPrintable(semiTonePitch2Lily(note->m_semiTonePitch))))
                << step << (alter ? "#" : "") << octave
                << faceValue2string(note->m_faceValue & 0x0F)
                << dots2string(note->m_dotControl & 3)
                << tuplet2string(note->m_tuplet)
                << ((note->m_grace1 & 0x30) > 0x10 ? " grace" : "")
                   //<< enc_lily_adorno(adorno)
                << "\n";
    }
    else if (const EncMeasureElemClef* const clef = dynamic_cast<const EncMeasureElemClef* const>(elem)) {
        std::cout
                << "\t"
                << "-\t-\t"
                << clef2string(clef)
                << "\n";
    }
    else if (const EncMeasureElemOrnament* const orna = dynamic_cast<const EncMeasureElemOrnament* const>(elem)) {
        std::cout
                << "\t" << static_cast<int>(orna->m_al_mezuro)
                << "\t" << static_cast<int>(orna->m_xoffset2)
                << "\t"
                << ornament2string(orna->type(), orna->m_speguleco)
                << "\n";
    }
    else if (const EncMeasureElemLyric* const lyric = dynamic_cast<const EncMeasureElemLyric* const>(elem)) {
        std::cout
                << "\t"
                << "-\t-\t"
                << lyric2string(lyric)
                << "\n";
    }
    else if (const EncMeasureElemTie* const tie = dynamic_cast<const EncMeasureElemTie* const>(elem)) {
        std::cout
                << "\t"
                << "-\t-\t"
                << tie2string(tie)
                << "\n";
    }
    else if (const EncMeasureElemBeam* const beam = dynamic_cast<const EncMeasureElemBeam* const>(elem)) {
        std::cout
                << "\t"
                << "-\t-\t"
                << beam2string(beam)
                << "\n";
    }
    else if (const EncMeasureElemRest* const rest = dynamic_cast<const EncMeasureElemRest* const>(elem)) {
        std::cout
                << "\t"
                << "-\t-\t"
                << "R"
                << faceValue2string(rest->m_faceValue & 0x0F)
                << "\n";
    }
    else if (const EncMeasureElemChord* const chord = dynamic_cast<const EncMeasureElemChord* const>(elem)) {
        std::cout
                << "\t"
                << "-\t-\t"
                << chord2string(chord)
                << "\n";
    }
    else if (const EncMeasureElemKeyChange* const key = dynamic_cast<const EncMeasureElemKeyChange* const>(elem)) {
        std::cout
                << "\t"
                << "-\t-\t"
                << key2string(key)
                << "\n";
    }
    else if (const EncMeasureElemUnknown* const unknown = dynamic_cast<const EncMeasureElemUnknown* const>(elem)) {
        std::cout
                << "\t"
                << "-\t-\t"
                << "Unknown (TODO) " << unknown
                << "\n";
    }
    else
        qDebug() << "failed to convert:" << elem;
}
