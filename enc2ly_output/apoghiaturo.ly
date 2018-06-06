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
\relative a''
{
  \clef treble
  \key c \major
  \time 4/4
  \appoggiatura { a16 } g4 r2 r4  | % kompletite
  \acciaccatura { a16 } g4 r2 r4  | % kompletite
  \appoggiatura { g8 } g4 r2 r4  | % kompletite
  c,4 c8 a g4 r4  | % kompletite
  %05
  c4 \appoggiatura { d8 c16 a } c4 r2  | % kompletite
  c4 \appoggiatura { d8 c16 g } b4 r2  | % kompletite
  d8 e \appoggiatura { d4 } c2 r4  | % kompletite
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
