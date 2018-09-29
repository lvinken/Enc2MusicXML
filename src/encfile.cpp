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
// implementation of the classes representing an Encore file
//---------------------------------------------------------

#include <QtDebug>

#include "encfile.h"


//---------------------------------------------------------
// hexString - convert a 64-bit integer into a QString
//---------------------------------------------------------

static QString hexString(const qint64 nr)
{
    return QString("%1").arg(nr, 0, 16);
}


//---------------------------------------------------------
// readMagic - read four bytes from data into magic
//---------------------------------------------------------

static bool readMagic(QDataStream& data, QString& magic)
{
    for (int i = 0; i < 4 && !data.atEnd(); ++i) {
        quint8 ch;
        data >> ch;
        magic.append(ch);
    }
    qDebug()
            << "filepos" << hexString(data.device()->pos() - 4)
            << "magic" << magic;
    return true;
}


//---------------------------------------------------------
// isEncInstrumentMagic - check if magic equals "TK<number><number>"
// (typically TK00) corresponding to an instrument block
//---------------------------------------------------------

static bool isEncInstrumentMagic(const QString& magic)
{
    if (magic.length() < 4)
        return false;

    if (magic.at(0) == 'T' && magic.at(1) == 'K' && magic.at(2).isDigit() && magic.at(3).isDigit())
        return true;

    return false;
}


//---------------------------------------------------------
// isKnownMagic - check if magic corresponds to a block
// we know how to handle
//---------------------------------------------------------

static bool isKnownMagic(const QString& magic)
{
    if (magic.length() < 4)
        return false;

    return  magic == "LINE" || magic == "MEAS" || magic == "TITL" || magic == "TEXT"
            || isEncInstrumentMagic(magic);
}


//---------------------------------------------------------
// findNextKnownMagic - find the start of the next block
// we know how to handle
//---------------------------------------------------------

static QString findNextKnownMagic(QDataStream& data)
{
    QString magic;

    for (int i = 0; i < 4 && !data.atEnd(); ++i) {
        quint8 ch;
        data >> ch;
        magic.append(ch);
    }

    while (!isKnownMagic(magic) && !data.atEnd()) {
        magic.remove(0, 1);
        quint8 ch;
        data >> ch;
        magic.append(ch);
    }

    if (!isKnownMagic(magic))
        magic = "";

    qDebug()
            << "filepos" << hexString(data.device()->pos() - 4)
            << "magic" << magic;

    return magic;
}

//---------------------------------------------------------
// EncHeader
//---------------------------------------------------------

EncHeader::EncHeader()
{

}


bool EncHeader::read(QDataStream& data)
{
    readMagic(data, m_magic);
    qDebug() << "m_magic" << m_magic;
    if (m_magic == "SCOW") {
        data.setByteOrder(QDataStream::LittleEndian);
    }
    else if (m_magic == "SCO5") {
        data.setByteOrder(QDataStream::BigEndian);
    }
    else {
        qDebug() << "m_magic" << m_magic << "is incorrect";
        m_magic = "";
        return false;
    }
    data >> m_chuMagio;
    data.skipRawData(0x28 - 5);
    data >> m_chuVersio;
    data >> m_nekon1;
    data >> m_fiksa1;
    data >> m_lineCount;
    data >> m_pageCount;
    data >> m_instrumentCount;
    data >> m_staffPerSystem;
    data >> m_measureCount;
    data.skipRawData(0xC2 - 0x36);  // skip to end
    return true;
}


QDebug operator<<(QDebug dbg, const EncHeader& header)
{
    dbg
            << "m_magic" << header.m_magic
            << "m_chuMagio" << header.m_chuMagio
            << "m_chuVersio" << header.m_chuVersio
            << "m_nekon1" << header.m_nekon1
            << "m_fiksa1" << header.m_fiksa1
            << "m_lineCount" << header.m_lineCount
            << "m_pageCount" << header.m_pageCount
            << "m_instrumentCount" << header.m_instrumentCount
            << "m_staffPerSystem" << header.m_staffPerSystem
            << "m_measureCount" << header.m_measureCount
               ;
    return dbg;
}

//---------------------------------------------------------
// EncInstrument
//---------------------------------------------------------

EncInstrument::EncInstrument()
{

}


bool EncInstrument::read(QDataStream& data, const quint32 var_size)
{
    qDebug() << "EncInstrument::read()";
    m_offset = var_size;
    m_offset &= 0xFFFF; // TODO: ritmo.enc fails when m_offset is assumed to be 32 bit
    qDebug()
            << "m_offset" << m_offset
            << "charSize" << static_cast<int>(charSize())
               ;

    int nread = 8; // # bytes read sofar
    // read the name as null-terminated 8 or 16-bit chars
    QChar ch;
    bool ready = false;
    while (!ready) {
        if (charSize() == CharSize::ONE_BYTE) {
            quint8 b;
            data >> b;
            ch = b;
            nread += 1;
        }
        else if (charSize() == CharSize::TWO_BYTES) {
            data >> ch;
            nread += 2;
        }
        else {
            qFatal("unknown CharSize");
        }
        if (ch == '\0') {
            ready = true;
        }
        else
            m_name.append(ch);
    }
    qDebug() << "m_name" << m_name;
    data.skipRawData(m_offset - nread);
    return true;
}


// determine if one or two byte characters are used

CharSize EncInstrument::charSize() const
{
    return (m_offset > 250) ? CharSize::TWO_BYTES : CharSize::ONE_BYTE;
}


QDebug operator<<(QDebug dbg, const EncInstrument& /* instrument */)
{
    // TODO

    return dbg;
}

//---------------------------------------------------------
// EncPage
//---------------------------------------------------------

EncPage::EncPage()
{

}


bool EncPage::read(QDataStream& data)
{
    qDebug() << "EncPage::read()";
    readMagic(data, m_id);
    data >> m_offset;
    qDebug()
            << "m_id" << m_id
            << "m_offset" << m_offset
               ;
    data.skipRawData(34 - 8);   // fixed size of 34
    return true;
}

//---------------------------------------------------------
// EncLineStaffData
//---------------------------------------------------------

EncLineStaffData::EncLineStaffData()
{

}

// note: EncLineStaffData::read() reads 30 bytes

bool EncLineStaffData::read(QDataStream& data)
{
    data.skipRawData(14);
    qint8 ct;
    data >> ct;
    m_clef = static_cast<clefType>(ct);
    data >> m_key;
    data >> m_pageIdx;
    data.skipRawData(3);
    quint8 st;
    data >> st;
    m_staffType = static_cast<staffType>(st);
    data >> m_instrStaffIdx;
    data.skipRawData(8);      // skip to end
    qDebug()
            << "m_clef" << static_cast<int>(m_clef)
            << "m_key" << m_key
            << "m_pageIdx" << m_pageIdx
            << "m_staffType" << static_cast<unsigned int>(m_staffType)
            << "m_instrStaffIdx" << m_instrStaffIdx
               ;
    return true;
}

//---------------------------------------------------------
// EncLine
//---------------------------------------------------------

EncLine::EncLine()
{

}

// LINE size (from start of "LINE" to start of next magic string)
// equals line's m_offset plus 8
// observed m_offset values (decimal): 56, 86, 240, 662

bool EncLine::read(QDataStream& data, const quint32 var_size, const int staffPerSystem)
{
    qDebug() << "EncLine::read()";
    m_offset = var_size;
    data.skipRawData(10);
    data >> m_start;
    data >> m_measureCount; // 21 bytes read so far
    qDebug()
            << "m_id" << m_id
            << "m_offset" << m_offset
            << "staffPerSystem" << staffPerSystem
            << "m_start" << m_start
            << "m_measureCount" << m_measureCount
               ;
    for (int i = 0; i < staffPerSystem; ++i) {
        EncLineStaffData lineStaffData;
        lineStaffData.read(data);
        m_lineStaffData.push_back(lineStaffData);
    }
    const int toSkip = m_offset + 8 - 21 - 30 * staffPerSystem;
    qDebug() << "toSkip" << toSkip;
    data.skipRawData(toSkip);     // skip to end
    return true;
}

//---------------------------------------------------------
// EncMeasure
//---------------------------------------------------------

bool EncMeasure::read(QDataStream& data, const quint32 var_size)
{
    m_varsize = var_size;
    data >> m_bpm;
    data >> m_timeSigGlyph;
    data.skipRawData(1);
    data >> m_beatTicks;
    data >> m_durTicks;
    data >> m_timeSigNum;
    data >> m_timeSigDen;
    data.skipRawData(2);
    data >> m_barTypeStart;
    data >> m_barTypeEnd;
    data >> m_repeatMarker;
    data >> m_repeatAlternative;
    data.skipRawData(9);
    data >> m_coda;
    qDebug()
            << "m_id" << m_id
            << "m_varsize" << m_varsize
            << "m_bpm" << m_bpm
            << "m_timeSigGlyph" << m_timeSigGlyph
            << "m_beatTicks" << m_beatTicks
            << "m_durTicks" << m_durTicks
            << "m_timeSigNum" << m_timeSigNum
            << "m_timeSigDen" << m_timeSigDen
            << "m_barTypeStart" << m_barTypeStart
            << "m_barTypeEnd" << m_barTypeEnd
            << "m_repeatMarker" << m_repeatMarker
            << "m_repeatAlternative" << m_repeatAlternative
            << "m_coda" << m_coda
               ;
    data.skipRawData(59 - 34);    // skip to end
    quint16 tick;
    data >> tick;
    int elemSize = 0;
    qDebug() << "tick" << tick;
    while (tick != 0xFFFF) {
        quint8 typeVoice;
        data >> typeVoice;
        qDebug() << "typeVoice" << typeVoice;
        // sometimes the measure element size is off by two (e.g. akordo.enc)
        // in which case the end-of-elements marker 0xFFFF is not in tick
        // but in typeVoice and the next byte
        if (typeVoice == 0xFF) {
            quint8 byteToSkip;
            data >> byteToSkip;
            qDebug() << "byteToSkip" << byteToSkip;
            break;
        }
        const quint8 type = typeVoice >> 4;
        const quint8 voice = typeVoice & 0x0F;
        EncMeasureElem* elem = nullptr;
        if (elemType(type) == elemType::NONE) {
            elem = new EncMeasureElemNone(tick, type, voice);
        } else if (elemType(type) == elemType::CLEF) {
            elem = new EncMeasureElemClef(tick, type, voice);
        } else if (elemType(type) == elemType::KEYCHANGE) {
            elem = new EncMeasureElemKeyChange(tick, type, voice);
        } else if (elemType(type) == elemType::TIE) {
            elem = new EncMeasureElemTie(tick, type, voice);
        } else if (elemType(type) == elemType::BEAM) {
            elem = new EncMeasureElemBeam(tick, type, voice);
        } else if (elemType(type) == elemType::ORNAMENT) {
            elem = new EncMeasureElemOrnament(tick, type, voice);
        } else if (elemType(type) == elemType::LYRIC) {
            elem = new EncMeasureElemLyric(tick, type, voice);
        } else if (elemType(type) == elemType::CHORD) {
            elem = new EncMeasureElemChord(tick, type, voice);
        } else if (elemType(type) == elemType::REST) {
            elem = new EncMeasureElemRest(tick, type, voice);
        } else if (elemType(type) == elemType::NOTE) {
            elem = new EncMeasureElemNote(tick, type, voice);
        } else if (elemType(type) == elemType::UNKNOWN1) {
            elem = new EncMeasureElemUnknown(tick, type, voice);
        } else if (elemType(type) == elemType::UNKNOWN2) {
            elem = new EncMeasureElemUnknown(tick, type, voice);
        } else {
            qDebug()
                    << "filepos" << hexString(data.device()->pos() - 1)
                    << "unsupported elemType" << type;
            exit(1);
        }
        //qDebug() << "elem:" << elem;
        elem->read(data);
        if (elemType(type) != elemType::NONE)
            m_measureElems.push_back(elem);
        elemSize += elem->m_size;
        data >> tick;
        qDebug() << "tick" << tick;
    }
    const int remaining = m_varsize - elemSize - 4;
    qDebug()
            << "elemSize" << elemSize
            << "remaining" << remaining;
    if (remaining > 0)
        data.skipRawData(remaining);
    return true;
}

//---------------------------------------------------------
// EncMeasureElem
//---------------------------------------------------------

EncMeasureElem::EncMeasureElem(quint16 tick, quint8  type, quint8 voice)
    : m_tick(tick), m_type(type), m_voice(voice)
{

}


bool EncMeasureElem::read(QDataStream& data)
{
    data >> m_size;
    data >> m_staffIdx;
    m_staffIdx &= 0x3F;

    qDebug()
            << "m_tick" << m_tick
            << "m_type" << m_type
            << "m_voice" << m_voice
            << "m_size" << m_size
            << "m_staffIdx" << m_staffIdx
               ;

    return true;
}

//---------------------------------------------------------
// EncMeasureElemNone
//---------------------------------------------------------

EncMeasureElemNone::EncMeasureElemNone(quint16 tick, quint8  type, quint8 voice)
    : EncMeasureElem(tick, type, voice)
{

}


bool EncMeasureElemNone::read(QDataStream& data)
{
    qDebug() << "EncMeasureElemNone::read()";

    EncMeasureElem::read(data);
    data.skipRawData(m_size - 5);     // skip to end

    return true;
}

//---------------------------------------------------------
// EncMeasureElemClef
//---------------------------------------------------------

EncMeasureElemClef::EncMeasureElemClef(quint16 tick, quint8  type, quint8 voice)
    : EncMeasureElem(tick, type, voice)
{

}


bool EncMeasureElemClef::read(QDataStream& data)
{
    qDebug() << "EncMeasureElemClef::read()";

    EncMeasureElem::read(data);
    data.skipRawData(m_size - 5);     // skip to end

    return true;
}

//---------------------------------------------------------
// EncMeasureElemKeyChange
//---------------------------------------------------------

EncMeasureElemKeyChange::EncMeasureElemKeyChange(quint16 tick, quint8  type, quint8 voice)
    : EncMeasureElem(tick, type, voice)
{

}


bool EncMeasureElemKeyChange::read(QDataStream& data)
{
    qDebug() << "EncMeasureElemKeyChange::read()";

    EncMeasureElem::read(data);
    data >> m_tipo;
    data.skipRawData(m_size - 5 - 1); // skip to end
    m_xoffset = 0;                    // like in enc2ly

    qDebug()
            << "m_tipo" << m_tipo
            << "m_xoffset" << m_xoffset
               ;

    return true;
}

//---------------------------------------------------------
// EncMeasureElemTie
//---------------------------------------------------------

EncMeasureElemTie::EncMeasureElemTie(quint16 tick, quint8  type, quint8 voice)
    : EncMeasureElem(tick, type, voice)
{

}


bool EncMeasureElemTie::read(QDataStream& data)
{
    qDebug() << "EncMeasureElemTie::read()";

    EncMeasureElem::read(data);
    data.skipRawData(5);
    data >> m_xoffset;
    data.skipRawData(m_size - 5 - 6);   // skip to end

    qDebug()
            << "m_tick" << m_tick
            << "m_xoffset" << m_xoffset
            << "m_voice" << m_voice
            << "m_staffIdx" << m_staffIdx
               ;

    return true;
}


//---------------------------------------------------------
// EncMeasureElemBeam
//---------------------------------------------------------

EncMeasureElemBeam::EncMeasureElemBeam(quint16 tick, quint8  type, quint8 voice)
    : EncMeasureElem(tick, type, voice)
{

}


bool EncMeasureElemBeam::read(QDataStream& data)
{
    qDebug() << "EncMeasureElemBeam::read()";

    EncMeasureElem::read(data);
    data.skipRawData(m_size - 5);     // skip to end
    m_xoffset = 255;                  // faked like in enc2ly

    qDebug()
            << "m_xoffset" << m_xoffset
               ;

    return true;
}

//---------------------------------------------------------
// EncMeasureElemOrnament
//---------------------------------------------------------

EncMeasureElemOrnament::EncMeasureElemOrnament(quint16 tick, quint8  type, quint8 voice)
    : EncMeasureElem(tick, type, voice)
{

}


bool EncMeasureElemOrnament::read(QDataStream& data)
{
    qDebug() << "EncMeasureElemOrnament::read()";

    EncMeasureElem::read(data);
    data >> m_tipo;
    data.skipRawData(4);
    data >> m_xoffset;
    data.skipRawData(7);
    data >> m_al_mezuro;
    data.skipRawData(1);
    data >> m_xoffset2;
    data.skipRawData(5);
    data >> m_speguleco;
    m_speguleco &= 3;
    data.skipRawData(1);
    data >> m_noto;
    data.skipRawData(1);
    data >> m_tempo;
    data.skipRawData(1);
    data >> m_tind;
    data.skipRawData(m_size - 5 - 28);     // skip to end

    qDebug()
            << "m_tipo" << m_tipo
            << "m_xoffset" << m_xoffset
            << "m_al_mezuro" << m_al_mezuro
            << "m_xoffset2" << m_xoffset2
            << "m_speguleco" << m_speguleco
            << "m_noto" << m_noto
            << "m_tempo" << m_tempo
            << "m_tind" << m_tind
               ;

    return true;
}


//---------------------------------------------------------
// EncMeasureElemLyric
//---------------------------------------------------------

EncMeasureElemLyric::EncMeasureElemLyric(quint16 tick, quint8  type, quint8 voice)
    : EncMeasureElem(tick, type, voice)
{

}


bool EncMeasureElemLyric::read(QDataStream& data)
{
    qDebug() << "EncMeasureElemLyric::read()";

    EncMeasureElem::read(data);
    data.skipRawData(m_size - 5);     // skip to end

    return true;
}

//---------------------------------------------------------
// EncMeasureElemChord
//---------------------------------------------------------

EncMeasureElemChord::EncMeasureElemChord(quint16 tick, quint8  type, quint8 voice)
    : EncMeasureElem(tick, type, voice)
{

}


bool EncMeasureElemChord::read(QDataStream& data)
{
    qDebug() << "EncMeasureElemChord::read()";

    EncMeasureElem::read(data);
    data >> m_toniko;
    data >> m_tipo;
    data.skipRawData(3);
    data >> m_xoffset;
    data.skipRawData(1);
    data >> m_radiko;
    data >> m_baso;
    const bool hasText = m_tipo & 1;
    if (hasText) {
        // read chord text, TODO: support UTF-8
        bool done = false;

        for (int j = 0; j < 2 * 18 /* AKORDO_BAJTARO */;) {
            quint8 lower;
            data >> lower;
            ++j;
            quint8 upper;
            data >> upper;
            ++j;
            QChar ch = (upper << 8) + lower;
            if (ch == '\0')
                done = true;
            if (!done)
                m_teksto.append(ch);
        }
        data.skipRawData(m_size - 5 - 9 - 2 * 18); // skip to end
    }
    else
        data.skipRawData(m_size - 5 - 9); // skip to end

    qDebug()
            << "m_toniko" << m_toniko
            << "m_tipo" << m_tipo
            << "m_xoffset" << m_xoffset
            << "m_radiko" << m_radiko
            << "m_baso" << m_baso
            << "m_teksto" << m_teksto
               ;

    return true;
}

//---------------------------------------------------------
// EncMeasureElemNote
//---------------------------------------------------------

EncMeasureElemNote::EncMeasureElemNote(quint16 tick, quint8  type, quint8 voice)
    : EncMeasureElem(tick, type, voice)
{

}


bool EncMeasureElemNote::read(QDataStream& data)
{
    qDebug() << "EncMeasureElemNote::read()";

    EncMeasureElem::read(data);
    data >> m_faceValue;
    data >> m_grace1;
    data >> m_grace2;
    data.skipRawData(2);
    data >> m_xoffset;
    data.skipRawData(1);
    data >> m_position;
    data >> m_tuplet;
    data >> m_dotControl;
    data >> m_semiTonePitch;
    data >> m_playbackDurTicks;
    data.skipRawData(1);
    data >> m_velocity;
    data >> m_options;
    data >> m_alterationGlyph;
    data.skipRawData(2);
    data >> m_articulationUp;
    data.skipRawData(1);
    data >> m_articulationDown;
    data.skipRawData(m_size - 27);     // skip to end

    qDebug()
            << "m_faceValue" << m_faceValue
            << "m_grace1" << m_grace1
            << "m_grace2" << m_grace2
            << "m_xoffset" << m_xoffset
            << "m_position" << m_position
            << "m_tuplet" << m_tuplet
            << "m_dotControl" << m_dotControl
            << "m_semiTonePitch" << m_semiTonePitch
            << "m_playbackDurTicks" << m_playbackDurTicks
            << "m_velocity" << m_velocity
            << "m_options" << m_options
            << "m_alterationGlyph" << m_alterationGlyph
            << "m_articulationUp" << m_articulationUp
            << "m_articulationDown" << m_articulationDown
               ;

    return true;
}


GraceType EncMeasureElemNote::graceType() const
{
    quint8 grace1 = m_grace1 & 0x30;
    quint8 grace2 = m_grace2 & 0x05;

    // algorithm copied from enc2ly
    if (grace1 == 0x20 && grace2 == 0x04) {
        return GraceType::ACCIACCATURA;
    }
    else if (grace1 > 0x10 && grace2 != 0x01) {
        return GraceType::APPOGGIATURA;
    }

    return GraceType::NORMALNOTE;
}


//---------------------------------------------------------
// EncMeasureElemRest
//---------------------------------------------------------

EncMeasureElemRest::EncMeasureElemRest(quint16 tick, quint8  type, quint8 voice)
    : EncMeasureElem(tick, type, voice)
{

}


bool EncMeasureElemRest::read(QDataStream& data)
{
    qDebug() << "EncMeasureElemRest::read()";

    EncMeasureElem::read(data);
    data >> m_faceValue;
    data.skipRawData(4);
    data >> m_xoffset;
    data.skipRawData(2);
    data >> m_tuplet;
    data >> m_dotControl;
    data.skipRawData(m_size - 10 - 5);     // skip to end

    qDebug()
            << "m_faceValue" << m_faceValue
            << "m_xoffset" << m_xoffset
            << "m_tuplet" << m_tuplet
            << "m_dotControl" << m_dotControl
               ;

    return true;
}


//---------------------------------------------------------
// EncMeasureElemUnknown
//---------------------------------------------------------

EncMeasureElemUnknown::EncMeasureElemUnknown(quint16 tick, quint8  type, quint8 voice)
    : EncMeasureElem(tick, type, voice)
{

}


bool EncMeasureElemUnknown::read(QDataStream& data)
{
    qDebug() << "EncMeasureElemUnknown::read()";

    EncMeasureElem::read(data);
    data.skipRawData(m_size - 5);     // skip to end

    return true;
}


//---------------------------------------------------------
// EncText
//---------------------------------------------------------

// read a single text from a TEXT block
// structure is a 16 byte header, starting with a two byte size
// size is the number of bytes remaining in the header plus the string
// return text read

static QString readSingleText(QDataStream& data)
{
    quint16 size;
    data >> size;
    // skip remainder of header
    data.skipRawData(14);

    // read the text
    QString text;
    quint8 b { 0 };
    bool done { false };

    for (unsigned int i = 0; i < (size - 14); ++i) {
        data >> b;
        if (b == 0 || b == 4) {
            done = true;
        }
        if (!done) {
            text += QChar(b);
        }
    }

    return text;
}


bool EncText::read(QDataStream& data, const quint32 var_size)
{
    qDebug() << "EncText::read()";

    m_varsize = var_size;

    quint16 ntexts { 0 };
    data.skipRawData(2);
    data >> ntexts;
    data.skipRawData(4);

    for (int i = 0; i < ntexts; ++i) {
        m_texts.push_back(readSingleText(data));
    }

    QString texts;
    for (const auto& s : m_texts) {
        if (!texts.isEmpty()) {
            texts += " ";
        }
        texts += "'";
        texts += s;
        texts += "'";
    }

    qDebug()
            << "ntexts" << ntexts
            << "m_texts" << texts
               ;

    return true;
}


//---------------------------------------------------------
// EncTitle
//---------------------------------------------------------


// read a single text item in a TEXT block

static QString readTextItem(QDataStream& data, const CharSize charsize)
{
    // skip the item header
    data.skipRawData(30);

    // now at the start of the next text item, read it
    QString item;
    bool done = false;

    if (charsize == CharSize::ONE_BYTE) {
        for (int j = 0; j < 66;) {
            quint8 lower;
            data >> lower;
            ++j;
            QChar ch = lower;
            if (ch == '\0')
                done = true;
            if (!done)
                item.append(ch);
        }
    }
    else if (charsize == CharSize::TWO_BYTES) {
        for (int j = 0; j < 1026;) {
            quint8 lower;
            data >> lower;
            ++j;
            quint8 upper;
            data >> upper;
            ++j;
            QChar ch = (upper << 8) + lower;
            if (ch == '\0')
                done = true;
            if (!done)
                item.append(ch);
        }
    }
    else {
        qFatal("unknown CharSize");
    }

    return item;
}


bool EncTitle::read(QDataStream& data, const quint32 var_size, const CharSize charsize)
{
    m_varsize = var_size;

    data.skipRawData(2);

    m_title = readTextItem(data, charsize);
    for (int i = 0; i < 2; ++i) m_subtitle.push_back(readTextItem(data, charsize));
    for (int i = 0; i < 3; ++i) m_instruction.push_back(readTextItem(data, charsize));
    for (int i = 0; i < 4; ++i) m_author.push_back(readTextItem(data, charsize));
    for (int i = 0; i < 2; ++i) m_header.push_back(readTextItem(data, charsize));
    for (int i = 0; i < 2; ++i) m_footer.push_back(readTextItem(data, charsize));
    for (int i = 0; i < 6; ++i) m_copyright.push_back(readTextItem(data, charsize));

    // skip to end of TITL
    if (charsize == CharSize::ONE_BYTE) {
        data.skipRawData(504);
    }
    else if (charsize == CharSize::TWO_BYTES) {
        data.skipRawData(120);
    }
    else {
        qFatal("unknown CharSize");
    }

    return true;
}


//---------------------------------------------------------
// print_orna - debug: print an ornament
//---------------------------------------------------------

/*
static void print_orna(const EncMeasureElemOrnament* const orna)
{
    qDebug()
            << "orna" << orna
            << "m_tick" << orna->m_tick
            << "m_type" << orna->m_type
            << "m_voice" << orna->m_voice
            << "m_size" << orna->m_size
            << "m_staffIdx" << orna->m_staffIdx
            << "m_tipo" << orna->m_tipo
            << "m_xoffset" << orna->m_xoffset
            << "m_al_mezuro" << orna->m_al_mezuro
            << "m_xoffset2" << orna->m_xoffset2
            << "m_speguleco" << orna->m_speguleco
               ;
}
*/


//---------------------------------------------------------
// addSpannerEnds - create ornament ends for selected ornaments
//---------------------------------------------------------

static void addSpannerEnds(MeasureVec& mv)
{
    MeasureElemVecVec mevv(mv.size());

    //qDebug() << "addSpannerEnds 1";
    int i = 0;
    for (const auto& meas : mv) {
        //qDebug() << "i" << i;
        for (const auto elem : meas.measureElems()) {
            if (const EncMeasureElemOrnament* const orna = dynamic_cast<const EncMeasureElemOrnament* const>(elem)) {
                //qDebug() << "orna type" << static_cast<unsigned int>(orna->m_tipo);
                if (orna->type() == ornamentType::SLURSTART) { // ST_LIGARKO
                    EncMeasureElemOrnament* end_orna = new EncMeasureElemOrnament(0, 0, 0);
                    *end_orna = *orna;
                    end_orna->setType(ornamentType::SLURSTOP); // ST_LIGARKOFINO
                    end_orna->m_xoffset = orna->m_xoffset2;
                    //print_orna(orna);
                    //print_orna(end_orna);
                    const int endMeas = i + orna->m_al_mezuro;
                    //qDebug() << "endMeas" << endMeas;
                    mevv.at(endMeas).push_back(end_orna);
                }
                else if (orna->type() == ornamentType::WEDGESTART) { // ST_DINAMIKO
                    EncMeasureElemOrnament* end_orna = new EncMeasureElemOrnament(0, 0, 0);
                    *end_orna = *orna;
                    end_orna->setType(ornamentType::WEDGESTOP); // ST_DINAMIKOFINO
                    end_orna->m_xoffset = orna->m_xoffset2;
                    //print_orna(orna);
                    //print_orna(end_orna);
                    const int endMeas = i + orna->m_al_mezuro;
                    //qDebug() << "endMeas" << endMeas;
                    mevv.at(endMeas).push_back(end_orna);
                }
            }
        }
        ++i;
    }

    //qDebug() << "addSpannerEnds 2";
    int j = 0;
    for (const auto& mev : mevv) {
        //qDebug() << "j" << j;
        for (const auto e : mev) {
            //qDebug() << "e" << e;
            mv.at(j).push_back(e);
        }
        ++j;
    }
}


//---------------------------------------------------------
// fixupInstruments - create instrument descriptions if not found
// which occurs when files do not contain TKxx blocks
//---------------------------------------------------------

static void fixupInstruments(std::vector<EncInstrument>& instruments, const int count)
{
    if (instruments.size() == 0) {
        for (int i = 0; i < count; ++i) {
            EncInstrument instrument;
            instrument.m_name = QString("Part %1").arg(i + 1);
            instruments.push_back(instrument);
        }
    }
}


//---------------------------------------------------------
// countStaves - count the number of staves for each instrument
//---------------------------------------------------------

static void countStaves(std::vector<EncInstrument>& instruments, const std::vector<EncLineStaffData>& lineStaffData)
{
    for (unsigned int i = 0; i < instruments.size(); ++i) {
        int staves = 0;
        for (const auto& staff : lineStaffData) {
            if ((staff.instrumentIndex()) == i) {
                ++staves;
            }
        }
        instruments[i].m_nstaves = staves;
    }
}


//---------------------------------------------------------
// EncFile
//---------------------------------------------------------

EncFile::EncFile()
{

}


bool EncFile::read(QDataStream& data)
{
    m_header.read(data);
    qDebug() << "header" << m_header;
    CharSize charsize = CharSize::ONE_BYTE;

    while (!data.atEnd()) {
        auto next_id = findNextKnownMagic(data);
        quint32 var_size;
        data >> var_size;
        qDebug() << "next id" << next_id << "var_size" << var_size;
        if (next_id == "LINE") {
            EncLine line;
            line.read(data, var_size, m_header.m_staffPerSystem);
            m_lines.push_back(line);
        }
        else if (next_id == "MEAS") {
            EncMeasure measure;
            measure.read(data, var_size);
            m_measures.push_back(measure);
        }
        else if (next_id == "TEXT") {
            m_text.read(data, var_size);
        }
        else if (next_id == "TITL") {
            m_title.read(data, var_size, charsize);
        }
        else if (isEncInstrumentMagic(next_id)) {
            EncInstrument instrument;
            instrument.read(data, var_size);
            m_instruments.push_back(instrument);
            charsize = instrument.charSize();
        }
        else
            data.skipRawData(var_size);
    }

    fixupInstruments(m_instruments, m_header.m_instrumentCount);
    countStaves(m_instruments, m_lines.at(0).lineStaffData());
    addSpannerEnds(m_measures);

    return true;
}
