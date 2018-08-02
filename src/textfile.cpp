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
#include "textfile.h"


//---------------------------------------------------------
// conversion functions from Encore to LilyPond
// copied from enc2ly 0.6 and sometimes slightly adapted
//---------------------------------------------------------

const char * enc_lily_liniaro (quint8 tipo)
{
    const char *chenoj[] =
    {
        "melodio",
        "tabulaturo",
        "ritmo"
    };
    if (tipo < (sizeof(chenoj) / sizeof(chenoj[0])) )
        return (chenoj[tipo]);
    return ("¿liniaro?");
}

const char * enc_lily_klefo (quint8 num)
{
    const char *kch[] =
    {
        "treble",
        "bass",
        "alto",
        "tenor",
        "\"treble^8\"",
        "\"treble_8\"",
        "\"bass_8\"",
        "percussion",
        "tab"
    };
    if (num < (sizeof(kch) / sizeof(kch[0])) )
        return (kch[num]);
    return ("¿klefo?");
}

const char * enc_lily_dieso ()
{
    const char *chenoj[] = {
        /* nederlands */ "is",
        /* catalan    */ "d",
        /* deutsch    */ "is",
        /* english    */ "s",
        /* espanol    */ "s",
        /* italiano   */ "d",
        /* norsk      */ "iss",
        /* portugues  */ "s",
        /* suomi      */ "is",
        /* svenska    */ "iss",
        /* vlaams     */ "k"
    };
    const int lang = 0;
    return chenoj[lang];
}

const char * enc_lily_bemolo ()
{
    const char *chenoj[] = {
        /* nederlands */ "es",
        /* catalan    */ "b",
        /* deutsch    */ "es",
        /* english    */ "f",
        /* espanol    */ "b",
        /* italiano   */ "b",
        /* norsk      */ "ess",
        /* portugues  */ "b",
        /* suomi      */ "es",
        /* svenska    */ "ess",
        /* vlaams     */ "b"
    };

    const int lang = 0;
    return chenoj[lang];
}

const char * enc_lily_noto (unsigned char noto)
{
    const char *chenoj[][7] = {
        /* "nederlands */ { "c",  "d",  "e",  "f",  "g",   "a",  "b"  },
        /* "catalan    */ { "do", "re", "mi", "fa", "sol", "la", "si" },
        /* "deutsch    */ { "c",  "d",  "e",  "f",  "g",   "a",  "h"  },
        /* "english    */ { "c",  "d",  "e",  "f",  "g",   "a",  "b"  },
        /* "espanol    */ { "do", "re", "mi", "fa", "sol", "la", "si" },
        /* "italiano   */ { "do", "re", "mi", "fa", "sol", "la", "si" },
        /* "norsk      */ { "c",  "d",  "e",  "f",  "g",   "a",  "h"  },
        /* "portugues  */ { "do", "re", "mi", "fa", "sol", "la", "si" },
        /* "suomi      */ { "c",  "d",  "e",  "f",  "g",   "a",  "h"  },
        /* "svenska    */ { "c",  "d",  "e",  "f",  "g",   "a",  "h"  },
        /* "vlaams     */ { "do", "re", "mi", "fa", "sol", "la", "si" }
    };

    const int lang = 3;
    return chenoj[lang][noto];
}

const char * enc_lily_tonalo (quint8 num)
{
    const char * tch[11][15] =
    {
        /* nederlands */ { "c",  "f",  "bes", "ees", "aes", "des", "ges",  "ces", "g",   "d",  "a",  "e",  "b",  "fis", "cis" },
        /* catalan    */ { "do", "fa", "sib", "mib", "lab", "reb", "solb", "dob", "sol", "re", "la", "mi", "si", "fad", "dod" },
        /* deutsch    */ { "c",  "f",  "b",   "ees", "aes", "des", "ges",  "ces", "g",   "d",  "a",  "e",  "h",  "fis", "cis" },
        /* english    */ { "c",  "f",  "bf ", "ef",  "af",  "df",  "gf",   "cf",  "g",   "d",  "a",  "e",  "b",  "fs",  "cs"  },
        /* espanol    */ { "do", "fa", "sib", "mib", "lab", "reb", "solb", "dob", "sol", "re", "la", "mi", "si", "fas", "dos" },
        /* italiano   */ { "do", "fa", "sib", "mib", "lab", "reb", "solb", "dob", "sol", "re", "la", "mi", "si", "fad", "dod" },
        /* norsk      */ { "c",  "f",  "b",   "eess","aess","dess","gess", "cess","g",   "d",  "a",  "e",  "h",  "fiss","ciss"},
        /* portugues  */ { "do", "fa", "sib", "mib", "lab", "reb", "solb", "dob", "sol", "re", "la", "mi", "si", "fas", "dos" },
        /* suomi      */ { "c",  "f",  "b",   "ees", "aes", "des", "ges",  "ces", "g",   "d",  "a",  "e",  "h",  "fis", "cis" },
        /* svenska    */ { "c",  "f",  "b",   "eess","aess","dess","gess", "cess","g",   "d",  "a",  "e",  "h",  "fiss","ciss"},
        /* vlaams     */ { "do", "fa", "sib", "mib", "lab", "reb", "solb", "dob", "sol", "re", "la", "mi", "si", "fak", "dok" },
    };
    const int lang = 0;
    return tch[lang][num];
}

const char * enc_lily_stango (quint8 st)
{
    switch (st)
    {
    case 0x00: return ("|");
    case 0x01: return ("\"fino\"");
    case 0x02: return ("\\repeat volta 2 {");
    case 0x03: return ("\\bar \"||\"");
    case 0x04: return ("}");
    case 0x05: return ("\\bar \"|.\"");
    case 0x06: return ("\\bar \"||\"");
    case 0x08: return ("\"- - - -\"");
    case 0xff: return ("\"neniu\"");
    default: return ("¿stango?");
    }
}

const char * enc_lily_saltsigno (quint32 num)
{
    switch (num)
    {
    case 0x80: return ("\\mark \\markup {D.C. e Coda}");
    case 0x81: return ("\\mark \\markup {D.S. e Coda}");
    case 0x82: return ("\\mark \"D.C. al Fine\"");
    case 0x83: return ("\\mark \"D.S. al Fine\"");
    case 0x84: return ("\\mark \"D.S.\"");
    case 0x85: return ("\\mark \\markup {\\musicglyph #\"scripts.coda\" ???}");
    case 0x86: return ("\\mark \"Fine\"");
    case 0x87: return ("\\mark \"D.C.\"");
    case 0x88: return ("\\mark \\markup {\\musicglyph #\"scripts.segno\"}");
    case 0x89: return ("\\mark \\markup {\\musicglyph #\"scripts.coda\"}");
    default: return ("\\mark \"¿saltsigno?\"");
    }
}

static QString semiTonePitch2Lily(const quint8 semiTonePitch)
{
    QString res;
    const int note = semiTonePitch % 12;
    const int oct  = semiTonePitch / 12;

    switch (note)
    {
    case  0: res = "c"; break;
    case  1: res = "des"; break;
    case  2: res = "d"; break;
    case  3: res = "es"; break;
    case  4: res = "e"; break;
    case  5: res = "f"; break;
    case  6: res = "ges"; break;
    case  7: res = "g"; break;
    case  8: res = "aes"; break;
    case  9: res = "a"; break;
    case 10: res = "bes"; break;
    case 11: res = "b"; break;
    }

    for (int i = oct; i > 4; i--)
        res += "'";

    for (int i = oct; i < 4; ++i)
        res += ",";

    return res;
}

static QString faceValue2Lily(const quint8 faceValue)
{
    switch (faceValue) {
    case 0: return "0";
    case 1: return "1";
    case 2: return "2";
    case 3: return "4";
    case 4: return "8";
    case 5: return "16";
    case 6: return "32";
    case 7: return "64";
    case 8: return "128";
    }
    return "???";
}

const char * enc_lily_punkto (quint8 punkto)
{
    const char *chenoj[] = {"", ".", "..", "..."};

    if (punkto > 3)
        return ("¿punkto?");
    return (chenoj[punkto]);
}

const char * enc_lily_opeco (quint8 opeco)
{
    static char bufro[10];

    if (opeco == 0)
        return ("");
    sprintf (bufro, "%u/%u", opeco & 0x0F, (opeco & 0xF0) >> 4);  /* Inversite mem! */
    return (bufro);
}

const char * enc_lily_adorno (const quint8 adorno)
{
    static char bufro[8];
    const char *adornoj[] =
    {
        "^\\markup{\\teeny \\flat}",
        "^\\markup{\\teeny \\sharp}",
        "^\\markup{\\teeny \\natural}",
        ":32",
        "\\trill",
        "\\trill^\\markup{\\teeny \\flat}",
        "\\trill^\\markup{\\teeny \\sharp}",
        "\\trill^\\markup{\\teeny \\natural}",
        "\\turn",
        "\\espressivo",
        "\\prall",
        "\\mordent",
        "\\prallprall",
        "-1",
        "-2",
        "-3",
        "-4",				/* 0x10 */
        "-5",
        "->",
        "-^",
        "-.-^",
        "-.-^",
        "-.->",
        "-.->",
        "\\upbow",
        "\\downbow",
        "-^",
        "-+",
        "--",
        "-.",
        "\\flageolet",
        "-\\markup {\\teeny \\triangle ##f}",
        "\\fermata",			/* 0x20 */
        "\\fermata",
        "--->",
        "--->",
        "-.--",
        "-.--",
        "---^",
        "---^",
        "\\staccatissimo",
        "\\staccatissimo",
        "\\staccatissimo\\acent",
        "\\staccatissimo\\acent",
        "\\staccatissimo\\tenuto",
        "\\staccatissimo\\tenuto",
        "\\reverseturn",
        "\\prallmordent",
        "-\\markup {\\bold +}",		/* 0x30 */
        "-\\rightHandFinger #1 ",	/* FIXME: mankas meti interne de < > */
        "-\\rightHandFinger #2 ",
        "-\\rightHandFinger #3 ",
        "-\\rightHandFinger #4 ",
        "-\\rightHandFinger #5 ",
        "-P",
        "-H",
        "-T",
        "\\1",				/* FIXME: mankas meti interne de < > */
        "\\2",
        "\\3",
        "\\4",
        "\\5",
        "\\6",
        "\\7",
        "\\8",				/* 0x40 */
        ":8",
        ":16",
        ":64",
        "\\snappizzicato",
        "\\snappizzicato",
        "\\0"
    };

    if (adorno == 0xff)
        return ("");
    if (adorno >= (sizeof adornoj / sizeof (adornoj[0])) /* TODO && ek_chu_shuti() */)
    {
        sprintf (bufro, "^\"0x%02X\"", adorno);
        return bufro;
    }
    return adornoj[adorno];
}

const char * enc_lily_simbolo (const quint8 tipo, const quint8 speguleco)
{
    static char bufro[64];

    switch (tipo)
    {
    case 0x10: return ("\\ottava #1");
    case 0x12: return ("\\ottava #-1");
    case 0x11:
    case 0x13: return ("\\ottava #0");
    case 0x19: return ("^\\markup {\\box {Z}}");
    case 0x1a: return ("^\\markup {\\rounded-box {Z}}");
    case 0x1b: return ("^\\markup {\\circle {Z}}");
    case 0x1c: return ("^\\markup {\\draw-line #'(2 . 2)}");
    case 0x1d: return (speguleco & 0x01 ? "\\>" : "\\<"); // ST_DINAMIKO
    case 0x4d: return ("\\!"); // ST_DINAMIKOFINO
        /* TODO
    case 0x1e:
        if (enc->tekstoj[obj->atr.simbolo.tind].ero[0])
        {
            sprintf (bufro, "_\"%s\"", enc->tekstoj[obj->atr.simbolo.tind].ero);
            return (bufro);
        }
        else
            return ("");
            */
    case 0x1f: return ("\\sustainOn"); /* Pedalo (etendita):  */ // ST_PEDALO
    case 0x4f: return ("\\sustainOff"); // ST_PEDALOFINO
    case 0x21: return ("("); // ST_LIGARKO
    case 0x41: return (")"); // ST_LIGARKOFINO
    case 0x23: return ("^\\markup {\\path #0.25 #'( (curveto (-0.5 2 -0.5 0 0 2)) )}"); /* Vertikala ligarko */
    case 0x24: return ("^\\markup {\\bracket {Z}}"); /* Rekta krampo */
    case 0x25: return ("\\sostenutoOn"); /* Duon-pedalo (etendita):  */ // ST_DPEDALO
    case 0x45: return ("\\sostenutoOff"); // ST_DPEDALOFINO
    case 0x2d: // ST_PEPO1
    case 0x35: // ST_PEPO2
    case 0x37: return ("\\startTrillSpan"); // ST_PEPO
    case 0x57: return ("\\stopTrillSpan"); // ST_PEPOFINO
    case 0x31: return ("^\"(Z)\"}"); /* Kurba krampo */
        /* TODO
    case 0x32:
        sprintf (bufro, "\\tempo %s%s = %u",
                 enc_lily_rapido ((obj->atr.simbolo.noto & 0x0F) + 1),
                 (obj->atr.simbolo.noto & 0x80) ? "." : "",
                 obj->atr.simbolo.tempo);
        return (bufro);
        */
        /* case 0x34: Regilo MIDI */
    case 0x22:                         /* Glissando maldika */
    case 0x36: return ("\\glissando"); /* kaj dika */
    case 0x80: return ("\\ppp");
    case 0x81: return ("\\pp");
    case 0x82: return ("\\p");
    case 0x83: return ("\\mp");
    case 0x84: return ("\\mf");
    case 0x85: return ("\\f");
    case 0x86: return ("\\ff");
    case 0x87: return ("\\fff");
    case 0x88: return ("\\sfz");
    case 0x89: return ("\\sff");
    case 0x8a: return ("\\fp");
    case 0xa2: return ("\\segno");
    case 0xa3: return ("\\mark \"%%\""); /* Ripeto de mezuro */
    case 0xa5: return ("\\sustainOff");
    case 0xa6: return ("\\coda");
    case 0xa7: return ("^\"//\"");
    case 0xa8: return ("\\breathe");
    case 0xa9: return ("\\sustainOn");
    case 0xaa: return ("\\rfz");
    case 0xab: return ("\\sf");
    case 0xac: return ("^\\markup{\\teeny \\flat}"); /* Bemolo */
    case 0xad: return ("^\\markup{\\teeny \\sharp}"); /* Dieso */
    case 0xae: return ("^\\markup{\\teeny \\natural}"); /* Naturalo */
    case 0xaf: return (":32"); /* Tremsono 3 */
    case 0xb0: return ("\\trill");
    case 0xb1: return ("\\trill^\\markup{\\teeny \\flat}");
    case 0xb2: return ("\\trill^\\markup{\\teeny \\sharp}");
    case 0xb3: return ("\\trill^\\markup{\\teeny \\natural}");
    case 0xb4: return ("\\turn");
    case 0xb5: return ("\\espressivo"); /* Ondo */
    case 0xb6: return ("\\mordent");
    case 0xb7: return ("\\prallprall");
    case 0xb8: return ("\\prallmordent");
    case 0xb9: return ("-1");
    case 0xba: return ("-2");
    case 0xbb: return ("-3");
    case 0xbc: return ("-4");
    case 0xbd: return ("-5");
    case 0xbe: return ("->");
    case 0xbf: return ("-^--");
    case 0xc0: return ("-^-.");
    case 0xc1: return ("-.-^");
    case 0xc2: return ("-.->");
    case 0xc3: return ("-^");
    case 0xc4: return ("\\upbow");
    case 0xc5: return ("\\downbow");
    case 0xc7: return ("-+"); /* Haltite */
    case 0xc6: return ("-^");
    case 0xc8: return ("--");
    case 0xc9: return ("-.");
    case 0xca: return ("\\flageolet"); /* Harmonio 1 */
    case 0xcb: return ("-\\markup {\\teeny \\triangle ##f}"); /* Harmonio 2 */
    case 0xcc: return ("\\fermata");
    case 0xce: return ("--->");
    case 0xcf: return ("->-.");
    case 0xd0: return ("-.--");
    case 0xd1: return ("---.");
    case 0xd2: return ("---^");
    case 0xd3: return ("->--");
    case 0xd4: return ("\\stacatissimo");
    case 0xd5: return ("\\stacatissimo");
    case 0xd6: return ("\\acent\\stacatissimo");
    case 0xd7: return ("\\stacatissimo\\acent");
    case 0xd8: return ("\\tenuto\\stacatissimo");
    case 0xd9: return ("\\stacatissimo\\tenuto");
    case 0xda: return ("\\reverseturn");
    case 0xdb: return ("\\prall");
    case 0xdc: return ("^\\markup {\\bold +}"); /* Haltite dika */
    case 0xdd: return ("-\\rightHandFinger #1 "); /* Dikfingro, gitaro */ /* FIXME: mankas meti interne de < > */
    case 0xde: return ("-\\rightHandFinger #2 "); /* Montra fingro */
    case 0xdf: return ("-\\rightHandFinger #3 "); /* Meza fingro */
    case 0xe0: return ("-\\rightHandFinger #4 "); /* Ringa fingro */
    case 0xe1: return ("-\\rightHandFinger #5 "); /* Eta fingro */
    case 0xe2: return ("^P"); /* "Pull-off" */
    case 0xe3: return ("^H"); /* "Hammer-on" */
    case 0xe4: return ("^T"); /* "Tapping" */
    case 0xe5: return ("\\1"); /* Kordo 1 */ /* FIXME: mankas meti interne de < > */
    case 0xe6: return ("\\2"); /* Kordo 2 */
    case 0xe7: return ("\\3"); /* Kordo 3 */
    case 0xe8: return ("\\4"); /* Kordo 4 */
    case 0xe9: return ("\\5"); /* Kordo 5 */
    case 0xea: return ("\\6"); /* Kordo 6 */
    case 0xeb: return ("\\7"); /* Kordo 7 */
    case 0xec: return ("\\8"); /* Kordo 8 */
    case 0xed: return (":8"); /* Tremsono 1 */
    case 0xee: return (":16"); /* Tremsono 2 */
    case 0xef: return (":64"); /* Tremsono 4 */
    case 0xf0: return ("^\\snappizzicato"); /* Agordi supren */
    case 0xf1: return ("_\\snappizzicato"); /* Agordi suben */
    case 0xf2: return ("-0");
    default:
        sprintf (bufro, "\\0x%02X", static_cast<unsigned int>(tipo));
        // TODO return (ek_chu_shuti () ? bufro : "");
    }

    /* Mankas en Lilypond:
    case 0x26: Stango de elektrogitaro (etendita)
    case 0x28: "Bend" de gitaro, kun malpremo, kurba kun sago (etendita)
    case 0x29: "Bend" de gitaro, kurba kun sago seta (etendita)
    case 0x2a: "Pre-bend" de gitaro, kun sago (etendita)
    case 0x2b: "Pre-bend" de gitaro, kun sago, kun malpremo (etendita)
    case 0x2c: Tenado de gitaro (etendita)
    case 0x2e: Vibrado de gitaro (etendita)
    case 0x2f: Tremsono de gitaro (etendita)
    case 0x30: "Bend" de gitaro, 2 linioj
    case 0xf3 Frapilo
    case 0xf4 Balailo
    case 0xf5 Malmilda frapilo el felto
    case 0xf6 Milda frapilo el felto
    case 0xf7 Centro de tamburo
    case 0xf8 Rando de tamburo
    case 0xf9 Centro de plataĵo
    case 0xfa Rando de plataĵo
    */

    return bufro;
}

const char * enc_lily_vpoz (qint8 vpoz)
{
    static char buf[10];
    memset(buf, 0, 10);

    switch (vpoz)
    {
    case -1: return strcat (strcpy (buf, enc_lily_noto(6)), ","); // b,
    case  0: return enc_lily_noto (0); // c
    case  1: return enc_lily_noto (1); // d
    case  2: return enc_lily_noto (2); // e
    case  3: return enc_lily_noto (3); // f
    case  4: return enc_lily_noto (4); // g
    case  5: return enc_lily_noto (5); // a
    case  6: return enc_lily_noto (6); // b
    case  7: return strcat(strcpy (buf, enc_lily_noto (0)), "'"); // c'
    case  8: return strcat(strcpy (buf, enc_lily_noto (1)), "'"); // d'
    case  9: return strcat(strcpy (buf, enc_lily_noto (2)), "'"); // e'
    case 10: return strcat(strcpy (buf, enc_lily_noto (3)), "'"); // f'
    case 11: return strcat(strcpy (buf, enc_lily_noto (4)), "'"); // g'
    case 12: return strcat(strcpy (buf, enc_lily_noto (5)), "'"); // a'
    case 13: return strcat(strcpy (buf, enc_lily_noto (6)), "'"); // b'
    case 14: return strcat(strcpy (buf, enc_lily_noto (0)), "''"); // c''
    default: return ("\\C?");
    }
}

const char * enc_lily_vpoz_frape (qint8 vpoz)
{
    switch (vpoz)
    {
    case -1: return ("hc");
    case  0: return ("ss");
    case  1: return ("hhp");
    case  2: return ("tomfl");
    case  3: return ("bd");
    case  4: return ("tomfh");
    case  5: return ("toml");
    case  6: return ("tomml");
    case  7: return ("sn");
    case  8: return ("tommh");
    case  9: return ("hh");
    case 10: return ("tomh");
    case 11: return ("cymr");
    case 12: return ("cymc");
    case 13: return ("cyms");
    case 14: return ("hc");
    default: return ("chinesecymbal");
    }
}

const char * enc_lily_rapido (unsigned char rapido)
{
    static char bufro[5];

    switch (rapido)
    {
    case 0: return ("0");
    case 1: return ("1");
    case 2: return ("2");
    case 3: return ("4");
    case 4: return ("8");
    case 5: return ("16");
    case 6: return ("32");
    case 7: return ("64");
    case 8: return ("128");
    default:
        sprintf (bufro, "$%02X$", rapido);
        return (bufro);
    }
}

const char * enc_lily_akordo (const EncMeasureElemChord* const chord, unsigned char rapido)
{
    static char bufro[64];
    char ero[32];
    const char *tnk[] =
    {
        "",
        "m",
        "+",
        "dim",
        "7",
        "5",
        "6",
        "6/9",
        "(add2)",
        "(add9)",
        "(omit3)",
        "(omit5)",
        "maj7",
        "maj7(b5)",
        "maj7(6/9)",
        "maj7(+5)",
        "",
        "maj9",
        "maj9(b5)",
        "maj9(+5)",
        "",
        "maj13",
        "maj13(b5)",
        "",
        "7",
        "7(b5)",
        "7(b9)",
        "7(#9)",
        "",
        "",
        "",
        "",
        "9",
        "9(b5)",
        "11",
        "13",
        "13(b5)",
        "13(b9)",
        "13(#9)",
        "",
        "+7",
        "+7(b9)",
        "+7(#9)",
        "+9",
        "sus2",
        "sus2,sus4",
        "sus4",
        "7sus4",
        "9sus4",
        "13sus4",
        "m(add2)",
        "m(add9)",
        "m6",
        "m6/9",
        "m7",
        "m(maj7)",
        "m7(b5)",
        "m7(add4)",
        "m7(add11)",
        "m9",
        "m(maj9)",
        "m11",
        "m13"
    };

    if (chord->m_tipo == 0x11)
    {
        bufro[0] = chord->m_teksto.toLower().toLatin1().at(0);
        bufro[1] = 0;
        if (rapido < 9)
            strcat (bufro, enc_lily_rapido (rapido));
        strcat (bufro, ":");
        // TODO strcat (bufro, &akd->teksto[1]);
    }
    else
    {
        sprintf (bufro, "%s%s",
                 enc_lily_vpoz (chord->m_radiko & 0x0F),
                 (chord->m_radiko & 0xF0) == 0 ? "" : ((chord->m_radiko & 0xF0) == 0x10 ? enc_lily_dieso () : enc_lily_bemolo ()));
        if (strcmp (bufro, "hes") == 0 || strcmp (bufro, "hess") == 0)
            strcpy (bufro, "b");
        if (rapido < 9)
            strcat (bufro, enc_lily_rapido (rapido));

        if (chord->m_tipo & 0x01)
            sprintf (ero, ":%s", chord->m_teksto.toLatin1().data());
        else if (chord->m_toniko)
            sprintf (ero, ":%s", tnk[chord->m_toniko]);
        else
            ero[0] = 0;
        strcat (bufro, ero);

        if (chord->m_tipo & 0x02)
        {
            sprintf (ero, "/%s%s",
                     enc_lily_vpoz (chord->m_baso & 0x0F),
                     (chord->m_baso & 0xF0) == 0 ? "" : ((chord->m_baso & 0xF0) == 0x10 ? enc_lily_dieso () : enc_lily_bemolo ()));
            if (strcmp (ero, "/hes") == 0 || strcmp (ero, "/hess") == 0)
                strcpy (ero, "/b");
            strcat (bufro, ero);
        }
    }
    return (bufro);
}


//---------------------------------------------------------
// TextFile - write analysis result
// result is (besides minor formatting differences)
// identical to enc2ly 0.6 output
//---------------------------------------------------------

TextFile::TextFile(const EncFile& ef)
    : m_ef(ef)
{

}


void TextFile::write()
{
    qDebug() << "TextFile::write()";
    const EncHeader& hdr = m_ef.header();
    qDebug() << "magic" << hdr.m_magic;
    writeHeader();
    writeTitle();
    writeText();
    writeInstruments();
    writeLines();
    writeMeasures();
}


void TextFile::writeHeader()
{
    const EncHeader& hdr = m_ef.header();
    std::cout
            << "---- KAPO ----" << "\n"
            << std::hex
            << "Magio      : " << qPrintable(hdr.m_magic) << "\n"
            << "Chu_magio  : " << std::setw(4) << static_cast<unsigned int>(hdr.m_chuMagio) << "\n"
            << "Chu_versio : " << std::setw(4) << hdr.m_chuVersio << "\n"
            << "Nekonata1  : " << std::setw(4) << hdr.m_nekon1 << "\n"
            << "Fiksa1     : " << std::setw(4) << hdr.m_fiksa1 << "\n"
            << std::dec
            << "N sistemoj : " << std::setw(4) << hdr.m_lineCount << "\n"
            << "N paghoj   : " << std::setw(4) << hdr.m_pageCount << "\n"
            << "N1 liniaroj: " << std::setw(4) << static_cast<int>(hdr.m_staffCount) << "\n"
            << "N2 liniaroj: " << std::setw(4) << static_cast<int>(hdr.m_staffPerSystem) << "\n"
            << "N mezuroj  : " << std::setw(4) << hdr.m_measureCount << "\n"
            << "\n";
}


void TextFile::writeTitle()
{
    const EncTitle& ttl = m_ef.title();
    std::cout
            << "---- TITOLOJ ----" << "\n"
            << "-->  Grando: " << ttl.m_varsize << " B" << "\n"
            << "  Titolo      : " << qPrintable(ttl.m_title) << "\n";
    for (int i = 0; i < 2 && ttl.m_subtitle.size(); ++i)
        std::cout << "  Subtitolo " << i <<" : " << qPrintable(ttl.m_subtitle.at(i)) << "\n";
    for (int i = 0; i < 3 && ttl.m_instruction.size(); ++i)
        std::cout << "  Instrukcio " << i <<": " << qPrintable(ttl.m_instruction.at(i)) << "\n";
    for (int i = 0; i < 4 && ttl.m_author.size(); ++i)
        std::cout << "  Autoro " << i <<"    : " << qPrintable(ttl.m_author.at(i)) << "\n";
    for (int i = 0; i < 2 && ttl.m_header.size(); ++i)
        std::cout << "  Kapo " << i <<"      : " << qPrintable(ttl.m_header.at(i)) << "\n";
    for (int i = 0; i < 2 && ttl.m_footer.size(); ++i)
        std::cout << "  Piedo " << i <<"     : " << qPrintable(ttl.m_footer.at(i)) << "\n";
    for (int i = 0; i < 6 && ttl.m_copyright.size(); ++i)
        std::cout << "  Kopirajto " << i <<" : " << qPrintable(ttl.m_copyright.at(i)) << "\n";
    std::cout
            << "\n";
}


void TextFile::writeText()
{
    std::cout
            << "---- TEKSTOJ ----" << "\n"
            << "  --> Kiom: " << (m_ef.text().m_varsize == 8 ? "0" : "???") << "\n"
            << "\n";
}


void TextFile::writeInstruments()
{
    std::cout
            << "---- INSTRUMENTOJ ----" << "\n";
    int count = 0;
    for (const auto& s : m_ef.staves()) {
        ++count;
        std::cout
                << "\t" << std::setw(2) << std::setfill('0') << count << ": " << qPrintable(s.m_name) << "\n";
    }
    std::cout
            << "\n";
}


void TextFile::writeLines()
{
    std::cout
            << "---- LINIOJ ----" << "\n";
    int count = 0;
    for (const auto& l : m_ef.lines()) {
        ++count;
        std::cout
                << "  ---> Linio " << std::setw(2) << std::setfill('0') << count << "\n"
                << "\tTipo   : 0x" << std::hex << std::setw(2) << (l.m_offset & 0xFF) << "\n"
                << "\tMezuroj: " << std::dec << static_cast<int>(l.m_measureCount) << "\n"
                   ;
        writeLineStaffData(l);
    }
    std::cout
            << "\n";
}


void TextFile::writeLineStaffData(const EncLine& line)
{
    int count = 0;
    for (const auto& d : line.lineStaffData()) {
        std::cout
                << "\t---> Liniaro: "  << count << "\n"
                << "\t\tTipo  : " << enc_lily_liniaro(/* TODO */ static_cast<quint8>(d.m_staffType)) << "\n"
                << "\t\tKlefo : " << enc_lily_klefo(/* TODO */ static_cast<qint8>(d.m_clef)) << "\n"
                << "\t\tTonalo: " << enc_lily_tonalo(d.m_key) << "\n"
                   ;
        ++count;
    }
}


void TextFile::writeMeasures()
{
    std::cout
            << "---- MEZUROJ ----" << "\n";
    int count = 0;
    for (const auto& m : m_ef.measures()) {
        ++count;
        std::cout
                << "---> Mezuro "   << count << "\n"
                << "\tGrando    : " << m.m_varsize << " B" << "\n"
                << "\tRapido    : " << m.m_bpm << " FPM" << "\n"
                << "\tTakto     : " << static_cast<int>(m.m_timeSigNum) << "/" << static_cast<int>(m.m_timeSigDen) << "\n"
                << "\tStangoj   : " << enc_lily_stango(m.m_barTypeStart) << enc_lily_stango(m.m_barTypeEnd) << "\n"
                << "\tRipetsalto: 0x" << std::hex << static_cast<int>(m.m_repeatAlternative) << std::dec << "\n"
                << "\tSaltsigno : " << enc_lily_saltsigno((m.m_coda >> 8) & 0xFF) << "\n"
                << " Objektoj (en liniaroj):"<< "\n"
                   ;
        for (int i = 0; i < m_ef.header().m_staffPerSystem; ++i) {
            std::multimap<quint8 , const EncMeasureElem* const> mmap;
            for (const auto e : m.measureElems()) {
                if (i == e->m_staffIdx)
                    mmap.insert({e->m_xoffset, e});
            }
            for (const auto& e : mmap)
                writeMeasureElem(e.second);
            std::cout << "--------------\n";
        }
    }
}


void TextFile::writeMeasureElem(const EncMeasureElem* const elem)
{
    if (const EncMeasureElemNote* const note = dynamic_cast<const EncMeasureElemNote* const>(elem)) {
        //qDebug() << "successfully converted to note:" << elem;
        // adorno handling may not support chords yet
        const auto& line = m_ef.lines().at(0);
        const auto& lsd = line.lineStaffData().at(note->m_staffIdx);
        const bool isPercClef = lsd.m_clef== clefType::PERC;
        const bool isRhythmStaff = lsd.m_staffType == staffType::RHYTHM;
        quint8 adorno = 0xFF;
        if (note->m_dotControl & 0x80)
            adorno = note->m_articulationUp;
        else if (note->m_dotControl & 0x40)
            adorno = note->m_articulationDown;
        std::cout
                << " "
                << (isPercClef ? enc_lily_vpoz_frape (note->m_position)
                               : (isRhythmStaff ? "c" : qPrintable(semiTonePitch2Lily(note->m_semiTonePitch))))
                << qPrintable(faceValue2Lily(note->m_faceValue & 0x0F))
                << ((note->m_grace1 & 0x30) > 0x10 ? "!" : "")
                << enc_lily_punkto(note->m_dotControl & 3)
                << enc_lily_opeco(note->m_tuplet)
                << enc_lily_adorno(adorno)
                << " [" << static_cast<int>(note->m_xoffset) << "]"
                << " (vocho: " << static_cast<int>(note->m_voice) << ")"
                << "\n";
    }
    else if (const EncMeasureElemOrnament* const orna = dynamic_cast<const EncMeasureElemOrnament* const>(elem)) {
        std::cout
                << " "
                << enc_lily_simbolo(orna->m_tipo, orna->m_speguleco)
                << " [0;" << static_cast<int>(orna->m_xoffset)
                << " -> " << static_cast<int>(orna->m_al_mezuro)
                << ";" << static_cast<int>(orna->m_xoffset2)
                << "]"
                << " (vocho: " << static_cast<int>(orna->m_voice) << ")"
                << "\n";
    }
    else if (const EncMeasureElemTie* const tie = dynamic_cast<const EncMeasureElemTie* const>(elem)) {
        std::cout
                << " "
                << "~"
                << " [" << static_cast<int>(tie->m_xoffset) << "]"
                << " (vocho: " << static_cast<int>(tie->m_voice) << ")"
                << "\n";
    }
    else if (const EncMeasureElemBeam* const beam = dynamic_cast<const EncMeasureElemBeam* const>(elem)) {
        std::cout
                << " "
                << "Vostligo"
                << " [" << static_cast<int>(beam->m_xoffset) << "]"
                << " (vocho: " << static_cast<int>(beam->m_voice) << ")"
                << "\n";
    }
    else if (const EncMeasureElemRest* const rest = dynamic_cast<const EncMeasureElemRest* const>(elem)) {
        std::cout
                << " "
                << "r"
                << qPrintable(faceValue2Lily(rest->m_faceValue & 0x0F))
                << " [" << static_cast<int>(rest->m_xoffset) << "]"
                << " (vocho: " << static_cast<int>(rest->m_voice) << ")"
                << "\n";
    }
    else if (const EncMeasureElemChord* const chord = dynamic_cast<const EncMeasureElemChord* const>(elem)) {
        std::cout
                << " "
                << "^\""
                << enc_lily_akordo(chord, -1)
                << "\""
                << " [" << static_cast<int>(chord->m_xoffset) << "]"
                << " (vocho: " << static_cast<int>(chord->m_voice) << ")"
                << "\n";
    }
    else if (const EncMeasureElemKeyChange* const key = dynamic_cast<const EncMeasureElemKeyChange* const>(elem)) {
        std::cout
                << " "
                << "\\key "
                << enc_lily_tonalo(key->m_tipo)
                << " [" << static_cast<int>(key->m_xoffset) << "]"
                << " (vocho: " << static_cast<int>(key->m_voice) << ")"
                << "\n";
    }
    else
        qDebug() << "failed to convert:" << elem;
}
