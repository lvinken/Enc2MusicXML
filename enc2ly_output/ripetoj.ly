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
\relative g'
{
  \clef treble
  \key c \major
  \time 4/4
  \repeat volta 3 { g1  |
  \mark \markup {\musicglyph #"scripts.segno"} a1  |
  \mark \markup {\musicglyph #"scripts.coda"} b1 }
  \alternative { { \mark "Fine" b1 }
  %05
  { d1 }
  { c1 } }
  \repeat volta 2 { b1  |
  c1  | }
  \mark \markup {D.S. e Coda} c1  |
  %10
  \mark \markup {\musicglyph #"scripts.coda"} g2 a  |
  e'2 a  |
  a,2 g  |
  c'2 \< g \!  |
  g,2 \> e' \!  |
  %15
  \mark "D.C. al Fine" b'2 d,  \bar "|."
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
