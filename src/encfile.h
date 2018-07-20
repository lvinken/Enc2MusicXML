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

#ifndef ENCFILE_H
#define ENCFILE_H

#include <vector>

#include <QDataStream>


//---------------------------------------------------------
// Type aliases
//---------------------------------------------------------

class EncMeasure;
class EncMeasureElem;
using MeasureVec = std::vector<EncMeasure>;
using MeasureElemVec = std::vector<EncMeasureElem*>;
using MeasureElemVecVec = std::vector<MeasureElemVec>;


//---------------------------------------------------------
// size of characters in strings in Encore files
//---------------------------------------------------------

enum class CharSize : char {
    ONE_BYTE,
    TWO_BYTES
};


//---------------------------------------------------------
// the header ("SCOW") block
//---------------------------------------------------------

class EncHeader
{
public:
    EncHeader();
    bool read(QDataStream& data);
    // TODO private:
    QString m_magic;                     // ENCORE_STRUKTURO::magio
    quint8  m_chuMagio          { 0 };   // ENCORE_STRUKTURO::chu_magio
    quint16 m_chuVersio         { 0 };   // ENCORE_STRUKTURO::chu_versio
    quint16 m_nekon1            { 0 };   // ENCORE_STRUKTURO::nekon1
    quint16 m_fiksa1            { 0 };   // ENCORE_STRUKTURO::fiksa1
    qint16  m_lineCount         { 0 };   // ENCORE_STRUKTURO::nsis
    qint16  m_pageCount         { 0 };   // ENCORE_STRUKTURO::npag
    qint8   m_staffCount        { 0 };   // ENCORE_STRUKTURO::nlnar1
    qint8   m_staffPerSystem    { 0 };   // ENCORE_STRUKTURO::nlnar2
    qint16  m_measureCount      { 0 };   // ENCORE_STRUKTURO::nmez
};

QDebug operator<<(QDebug dbg, const EncHeader& header);


//---------------------------------------------------------
// a staff (or instrument ?) ("TK<number><number>") block (typically TK00)
//---------------------------------------------------------

class EncStaff                          // instrumento
{
public:
    EncStaff();
    bool read(QDataStream& data);
    QString m_id;
    quint32 m_offset            { 0 };
    QString m_name;
    CharSize charSize() const;  // chu_utf8
};

QDebug operator<<(QDebug dbg, const EncStaff& staff);


//---------------------------------------------------------
// a page ("PAGE") block
//---------------------------------------------------------

class EncPage
{
public:
    EncPage();
    bool read(QDataStream& data);
    QString m_id;
    quint32 m_offset            { 0 };
};


enum class staffType : quint8 {
    MELODY = 0, // LT_MELODIO
    TAB,        // LT_TABULATURO
    RHYTHM      // LT_RITMO
};


enum class clefType : qint8 {
    ALIA = -1,  // KT_ALIA
    G,          // KT_G
    F,          // KT_F
    C3L,        // KT_C3L C-klefo, 3-a linio
    C4L,        // KT_C4L C-klefo, 4-a linio
    G8P,        // KT_G8P G-klefo, oktigita pli alte (PLUS)
    G8M,        // KT_G8M G-klefo, oktigita malpli alte (MINUS)
    F8M,        // KT_F8M F-klefo, oktigita malpli alte (MINUS)
    PERC,       // KT_FRAPO Frapinstrumenta klefo
    TAB         // KT_TAB Tabulatura klefo
};


enum class accidentalType : quint8 {
    NONE = 0,
    SHARP,
    FLAT,
    NATURAL
};

//---------------------------------------------------------
// one line (staff) in a system
//---------------------------------------------------------

class EncLineStaffData                  // Encore_Liniaro
{
public:
    EncLineStaffData();
    bool read(QDataStream& data);
    clefType  m_clef            { clefType::G };        // clef
    quint8  m_key               { 0 };                  // tonalo
    quint8  m_pageIdx           { 0 };
    staffType  m_staffType      { staffType::MELODY };  // tipo
    quint8  m_staffIdx          { 0 };
};


//---------------------------------------------------------
// a system ("LINE") block
//---------------------------------------------------------

class EncLine                           // Encore_Linio
{
public:
    EncLine();
    bool read(QDataStream& data, const quint32 var_size, const int staffPerSystem);
    const std::vector<EncLineStaffData>& lineStaffData() const { return m_lineStaffData; }
    QString m_id;
    quint32 m_offset            { 0 };
    quint16 m_start             { 0 };
    quint8  m_measureCount      { 0 };  // nmez
private:
    std::vector<EncLineStaffData> m_lineStaffData;
};


//---------------------------------------------------------
// the types of musical elements contained in a measure
//---------------------------------------------------------

enum class elemType : quint8 {
    NONE = 0,
    CLEF,       // OT_KLEFO
    KEYCHANGE,  // OT_TONALO
    TIE,        // OT_LIGATURO
    BEAM,       // OT_VOSTLIGO
    ORNAMENT,   // OT_SIMBOLO
    LYRIC,      // OT_KANTO
    CHORD,      // OT_AKORDO
    REST,       // OT_PAUZO
    NOTE        // OT_NOTO
};


//---------------------------------------------------------
// base class of all musical elements contained in a measure
//---------------------------------------------------------

class EncMeasureElem
{
public:
    EncMeasureElem(quint16 tick, quint8  type, quint8 voice);
    virtual bool read(QDataStream& data);
    quint16 m_tick;
    quint8  m_type;
    quint8  m_voice;
    quint8  m_size              { 0 };  // offset  4                ENCORE_OBJEKTO::grando
    quint8  m_staffIdx          { 0 };  // offset  5                ENCORE_OBJEKTO::liniaro
    quint8  m_xoffset           { 0 };  // offset 10                ENCORE_OBJEKTO::kie
};


//---------------------------------------------------------
// the musical elements contained in a measure
//---------------------------------------------------------

class EncMeasureElemNone : public EncMeasureElem
{
public:
    EncMeasureElemNone(quint16 tick, quint8  type, quint8 voice);
    bool read(QDataStream& data);
};


class EncMeasureElemKeyChange : public EncMeasureElem
{
public:
    EncMeasureElemKeyChange(quint16 tick, quint8  type, quint8 voice);
    bool read(QDataStream& data);
    quint8  m_tipo              { 0 };  // offset  5 ??
};


class EncMeasureElemTie : public EncMeasureElem
{
public:
    EncMeasureElemTie(quint16 tick, quint8  type, quint8 voice);
    bool read(QDataStream& data);
};


class EncMeasureElemBeam : public EncMeasureElem
{
public:
    EncMeasureElemBeam(quint16 tick, quint8  type, quint8 voice);
    bool read(QDataStream& data);
};


class EncMeasureElemOrnament : public EncMeasureElem
{
public:
    EncMeasureElemOrnament(quint16 tick, quint8  type, quint8 voice);
    bool read(QDataStream& data);
    quint8  m_tipo              { 0 };  // offset  5 ??
    // check:
    quint8  m_al_mezuro         { 0 };  // offset 18
    quint8  m_xoffset2          { 0 };  // offset 20                ENCORE_OBJEKTO::al_kie
    quint8  m_speguleco         { 0 };  // offset 26
    quint8  m_noto              { 0 };  // offset 28
    quint8  m_tempo             { 0 };  // offset 30
    quint8  m_tindo             { 0 };  // offset 32
};


class EncMeasureElemNote : public EncMeasureElem
{
public:
    EncMeasureElemNote(quint16 tick, quint8  type, quint8 voice);
    bool read(QDataStream& data);
    int actualNotes() const { return m_tuplet >> 4; }
    int normalNotes() const { return m_tuplet & 0x0F; }
    quint8  m_faceValue         { 0 };  // offset  5 (WithDuration) atr.noto.rapido
    quint8  m_grace             { 0 };  // offset  6                atr.noto.rap1
    qint8   m_position          { 0 };  // offset 12                atr.noto.vpoz
    quint8  m_tuplet            { 0 };  // offset 13 (WithDuration) atr.noto.opeco
    quint8  m_dotControl        { 0 };  // offset 14 (WithDuration) indikilo
    quint8  m_semiTonePitch     { 0 };  // offset 15                atr.noto.tono
    quint16 m_playbackDurTicks  { 0 };  // offset 16 (WithDuration)
    quint8  m_velocity          { 0 };  // offset 19
    quint8  m_options           { 0 };  // offset 20
    quint8  m_alterationGlyph   { 0 };  // offset 21
    quint8  m_articulationUp    { 0 };  // offset 24
    quint8  m_articulationDown  { 0 };  // offset 26
};


class EncMeasureElemChord : public EncMeasureElem
{
public:
    EncMeasureElemChord(quint16 tick, quint8  type, quint8 voice);
    bool read(QDataStream& data);
    quint8  m_toniko            { 0 };  // offset  5
    quint8  m_tipo              { 0 };  // offset  6
    quint8  m_radiko            { 0 };  // offset 12
    quint8  m_baso              { 0 };  // offset 13
    QString m_teksto;
};


class EncMeasureElemRest : public EncMeasureElem
{
public:
    EncMeasureElemRest(quint16 tick, quint8  type, quint8 voice);
    bool read(QDataStream& data);
    int actualNotes() const { return m_tuplet >> 4; }
    int normalNotes() const { return m_tuplet & 0x0F; }
    quint8  m_faceValue         { 0 };  // offset  5 (WithDuration) atr.pauzo.rapido
    quint8  m_tuplet            { 0 };  // offset 13 (WithDuration) atr.pauzo.opeco
    quint8  m_dotControl        { 0 };  // offset 14 (WithDuration) indikilo
};


//---------------------------------------------------------
// a measure ("MEAS") block
//---------------------------------------------------------


enum class barlineType : quint8 {
    NORMAL      = 0,
    REPEATSTART = 2,
    DOUBLEL     = 3,
    REPEATEND   = 4,
    FINAL       = 5,
    DOUBLER     = 6
};


enum class repeatType : quint8 {
    NONE        = 0,
    DCALCODA    = 0x80,
    DSALCODA    = 0x81,
    DCALFINE    = 0x82,
    DSALFINE    = 0x83,
    DS          = 0x84,
    CODA1       = 0x85,
    FINE        = 0x86,
    DC          = 0x87,
    SEGNO       = 0x88,
    CODA2       = 0x89
};


class EncMeasure                        // ENCORE_MEZURO
{
public:
    EncMeasure() = default;
    bool read(QDataStream& data, const quint32 var_size);
    const MeasureElemVec& measureElems() const { return m_measureElems; }
    void push_back(EncMeasureElem* elem) { m_measureElems.push_back(elem); }
    barlineType barTypeStart() const { return static_cast<barlineType>(m_barTypeStart); }
    barlineType barTypeEnd() const { return static_cast<barlineType>(m_barTypeEnd); }
    repeatType repeat() const { return static_cast<repeatType>((m_coda >> 8) & 0xFF); }
    QString m_id;
    qint32  m_varsize           { 0 };
    quint16 m_bpm               { 0 };
    quint8  m_timeSigGlyph      { 0 };
    quint16 m_beatTicks         { 0 };
    quint16 m_durTicks          { 0 };
    quint8  m_timeSigNum        { 0 };
    quint8  m_timeSigDen        { 0 };
    quint8  m_barTypeStart      { 0 };
    quint8  m_barTypeEnd        { 0 };
    quint8  m_repeatMarker      { 0 };
    quint8  m_repeatAlternative { 0 };
    quint32 m_coda              { 0 };  // second least significant byte is enc2ly's saltsigno
private:
    MeasureElemVec m_measureElems;
};


//---------------------------------------------------------
// a text ("TEXT") block
//---------------------------------------------------------

class EncText
{
public:
    EncText() = default;
    bool read(QDataStream& data, const quint32 var_size);
    quint32  m_varsize          { 0 };
};


//---------------------------------------------------------
// a title ("TITL") block
//---------------------------------------------------------

class EncTitle
{
public:
    EncTitle() = default;
    bool read(QDataStream& data, const quint32 var_size, const CharSize charsize);
    quint32  m_varsize          { 0 };
    QString m_title;
    std::vector<QString> m_subtitle;
    std::vector<QString> m_instruction;
    std::vector<QString> m_author;
    std::vector<QString> m_header;
    std::vector<QString> m_footer;
    std::vector<QString> m_copyright;
};


//---------------------------------------------------------
// an Encore file
//---------------------------------------------------------

class EncFile
{
public:
    EncFile();
    bool read(QDataStream& data);
    const EncHeader& header() const { return m_header; }
    const std::vector<EncStaff>& staves() const { return m_staves; }
    const std::vector<EncLine>& lines() const { return m_lines; }
    const MeasureVec& measures() const { return m_measures; }
    const EncText& text() const { return m_text; }
    const EncTitle& title() const { return m_title; }
private:
    EncHeader m_header;
    std::vector<EncStaff> m_staves;     // Encore_Strukturo.instrumentoj
    std::vector<EncLine> m_lines;
    MeasureVec m_measures;
    EncText m_text;
    EncTitle m_title;
};

#endif // ENCFILE_H
