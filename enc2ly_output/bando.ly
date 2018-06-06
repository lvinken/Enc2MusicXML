\version "2.16.2"

\header {
  dedication=""
  title="Concert Band"
  subtitle=""
  subsubtitle=""
  poet=""
  meter=""
  piece=""
  composer="Composer"
  arranger=""
  opus=""
  instrument=""
  copyright="     "
  tagline="  "
}

liniaroAa =
\relative c,,,,
{
  \clef treble
  \key c \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAb =
\relative c,,,,
{
  \clef treble
  \key c \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAc =
\relative c,,,,
{
  \clef bass
  \key c \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAd =
\relative c,,,,
{
  \clef treble
  \key d \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAe =
\relative c,,,,
{
  \clef treble
  \key d \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAf =
\relative c,,,,
{
  \clef treble
  \key d \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAg =
\relative c,,,,
{
  \clef treble
  \key d \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAh =
\relative c,,,,
{
  \clef treble
  \key a \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAi =
\relative c,,,,
{
  \clef treble
  \key d \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAj =
\relative c,,,,
{
  \clef treble
  \key a \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAk =
\relative c,,,,
{
  \clef treble
  \key d \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAl =
\relative c,,,,
{
  \clef treble
  \key d \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAm =
\relative c,,,,
{
  \clef treble
  \key g \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAn =
\relative c,,,,
{
  \clef bass
  \key c \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAo =
\relative c,,,,
{
  \clef bass
  \key c \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAp =
\relative c,,,,
{
  \clef bass
  \key c \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAq =
\relative c,,,,
{
  \clef bass
  \key c \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAr =
\relative c,,,,
{
  \clef treble
  \key c \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAs =
\relative c,,,,
{
  \clef bass
  \key c \major
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAt =
\drummode
{
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

liniaroAu =
\drummode
{
  \time 4/4
  R1  |
  R1  |
  R1  \bar "|."
}

\bookpart {
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \with {instrumentName = #"Flute  " shortInstrumentName = #"F"} \liniaroAa
        \new Staff \with {instrumentName = #"Oboe" shortInstrumentName = #"O"} \liniaroAb
        \new Staff \with {instrumentName = #"Bassoon" shortInstrumentName = #"B"} \liniaroAc
        \new Staff \with {instrumentName = #"Clarinet 1" shortInstrumentName = #"C"} \liniaroAd
        \new Staff \with {instrumentName = #"Clarinet 2" shortInstrumentName = #"C"} \liniaroAe
        \new Staff \with {instrumentName = #"Clarinet 3" shortInstrumentName = #"C"} \liniaroAf
        \new Staff \with {instrumentName = #"Bass Clarinet " shortInstrumentName = #"B"} \liniaroAg
        \new Staff \with {instrumentName = #"Alto Sax" shortInstrumentName = #"A"} \liniaroAh
        \new Staff \with {instrumentName = #"Tenor Sax" shortInstrumentName = #"T"} \liniaroAi
        \new Staff \with {instrumentName = #"Baritone Sax" shortInstrumentName = #"B"} \liniaroAj
        \new Staff \with {instrumentName = #"Trumpet 1" shortInstrumentName = #"T"} \liniaroAk
        \new Staff \with {instrumentName = #"Trumpet 2&3" shortInstrumentName = #"T"} \liniaroAl
        \new Staff \with {instrumentName = #"French Horn 1" shortInstrumentName = #"F"} \liniaroAm
        \new Staff \with {instrumentName = #"Trombone 1" shortInstrumentName = #"T"} \liniaroAn
        \new Staff \with {instrumentName = #"Trombone 2&3    " shortInstrumentName = #"T"} \liniaroAo
        \new Staff \with {instrumentName = #"Baritone Horn" shortInstrumentName = #"B"} \liniaroAp
        \new Staff \with {instrumentName = #"Tuba" shortInstrumentName = #"T"} \liniaroAq
        \new Staff \with {instrumentName = #"Xylophone" shortInstrumentName = #"X"} \liniaroAr
        \new Staff \with {instrumentName = #"Timpani" shortInstrumentName = #"T"} \liniaroAs
        \new DrumStaff \with {instrumentName = #"Percussion 1" shortInstrumentName = #"P"} \liniaroAt
        \new DrumStaff \with {instrumentName = #"Percussion 2" shortInstrumentName = #"P"} \liniaroAu
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Flute  "}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAa
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Oboe"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAb
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Bassoon"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAc
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Clarinet 1"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAd
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Clarinet 2"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAe
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Clarinet 3"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAf
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Bass Clarinet "}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAg
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Alto Sax"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAh
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Tenor Sax"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAi
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Baritone Sax"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAj
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Trumpet 1"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAk
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Trumpet 2&3"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAl
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="French Horn 1"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAm
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Trombone 1"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAn
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Trombone 2&3    "}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAo
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Baritone Horn"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAp
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Tuba"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAq
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Xylophone"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAr
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Timpani"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \liniaroAs
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Percussion 1"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new DrumStaff \liniaroAt
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Percussion 2"}
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new DrumStaff \liniaroAu
      >>
    }
    \layout {}
  }
}

