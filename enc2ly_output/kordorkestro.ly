\version "2.16.2"

\header {
  dedication=""
  title="S"
  subtitle=""
  subsubtitle=""
  poet=""
  meter=""
  piece=""
  composer=""
  arranger=""
  opus=""
  instrument=""
  copyright="     "
  tagline="  "
}

liniaroAa =
\relative a'
{
  \clef treble
  \key c \major
  \time 4/4
  a2 r2  | % kompletite
  R1  |
  R1  |
  R1  |
  %05
  R1  |
  R1  \bar "|."
}

liniaroAb =
\relative d''
{
  \clef treble
  \key c \major
  \time 4/4
  d2 r2  | % kompletite
  R1  |
  R1  |
  R1  |
  %05
  R1  |
  R1  \bar "|."
}

liniaroAc =
\relative d'
{
  \clef alto
  \key c \major
  \time 4/4
  d2 r2  | % kompletite
  R1  |
  R1  |
  R1  |
  %05
  R1  |
  R1  \bar "|."
}

liniaroAd =
\relative c'
{
  \clef bass
  \key c \major
  \time 4/4
  c2 r2  | % kompletite
  R1  |
  R1  |
  R1  |
  %05
  R1  |
  R1  \bar "|."
}

liniaroAe =
\relative b
{
  \clef bass
  \key c \major
  \time 4/4
  b2 r2  | % kompletite
  R1  |
  R1  |
  R1  |
  %05
  R1  |
  R1  \bar "|."
}

liniaroAf =
\relative d''
{
  \clef treble
  \key c \major
  \time 4/4
  d2 r2  | % kompletite
  R1  |
  R1  |
  R1  |
  %05
  R1  |
  R1  \bar "|."
}

liniaroAg =
\relative b,
{
  \clef bass
  \key c \major
  \time 4/4
  b2 r2  | % kompletite
  R1  |
  R1  |
  R1  |
  %05
  R1  |
  R1  \bar "|."
}

\bookpart {
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \with {instrumentName = #"Violin I" shortInstrumentName = #"V"} \liniaroAa
        \new Staff \with {instrumentName = #"Violin II" shortInstrumentName = #"V"} \liniaroAb
        \new Staff \with {instrumentName = #"Viola" shortInstrumentName = #"V"} \liniaroAc
        \new Staff \with {instrumentName = #"Cello" shortInstrumentName = #"C"} \liniaroAd
        \new Staff \with {instrumentName = #"Double Bass  " shortInstrumentName = #"D"} \liniaroAe
        \new Staff \with {instrumentName = #"Piano" shortInstrumentName = #"P"} \liniaroAf
        \new Staff \with {instrumentName = #"" shortInstrumentName = #" "} \liniaroAg
      >>
    }
    \layout {}
  }
}

\bookpart {
  \header {instrument="Violin I"}
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
  \header {instrument="Violin II"}
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
  \header {instrument="Viola"}
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
  \header {instrument="Cello"}
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
  \header {instrument="Double Bass  "}
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
  \header {instrument="Piano"}
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
  \header {instrument=""}
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

