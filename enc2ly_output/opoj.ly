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
\relative c''
{
  \clef treble
  \key c \major
  \time 4/4
  \times 2/3 { c8 f e } \times 2/3 { b d c } \times 2/3 { e,4 r8 } \times 2/3 { d' r4 }  |
  \times 4/6 { c16 a g g' b, b' } \times 4/6 { f e a, b, c' d, } \times 4/5 { f' f,8 c16 a' } \times 4/5 { c a4 }  \bar "|."
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
