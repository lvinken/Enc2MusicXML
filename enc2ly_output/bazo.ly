\version "2.16.2"

\header {
  dedication=""
  title=""
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
\relative c,,,,
{
  \clef treble
  \key c \major
  \time 4/4
  R1  |
  R1  |
  R1  |
  R1  |
  %05
  R1  |
  R1  \bar "|."
}

\score {
  \new StaffGroup {
    \override Score.RehearsalMark #'self-alignment-X = #LEFT
    <<
      \new Staff \with {instrumentName = #"" shortInstrumentName = #" "} \liniaroAa
    >>
  }
  \layout {}
}
