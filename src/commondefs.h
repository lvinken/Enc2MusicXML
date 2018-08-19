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

#ifndef COMMONDEFS_H
#define COMMONDEFS_H


//---------------------------------------------------------
// common definitions
//---------------------------------------------------------


//---------------------------------------------------------
// the types of grace notes
//---------------------------------------------------------

enum class GraceType : char {
    NORMALNOTE,
    ACCIACCATURA,
    APPOGGIATURA
};


//---------------------------------------------------------
// the states of a note in a tuplet
//---------------------------------------------------------

enum class TupletState : char {
    NONE,
    START,
    MID,
    STOP
};

#endif // COMMONDEFS_H
