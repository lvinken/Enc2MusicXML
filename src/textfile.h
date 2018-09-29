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

#ifndef TEXTFILE_H
#define TEXTFILE_H

#include "encfile.h"


//---------------------------------------------------------
// the analysis file writer
//---------------------------------------------------------

class TextFile
{
public:
    TextFile(const EncFile& ef);
    void write();
private:
    void writeHeader();
    void writeTitle();
    void writeText();
    void writeInstruments();
    void writeLines();
    void writeLineStaffData(const EncLine& line);
    void writeMeasures();
    void writeMeasureElem(const EncMeasureElem* const elem);
    const EncFile& m_ef;
};

#endif // TEXTFILE_H
