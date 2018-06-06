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
\relative g''
{
  \clef treble
  \key c \major
  \time 4/4
  \times 4/7 { g16 g g g g g g } \times 4/5 { e e f f e } r2  |
  \times 4/6 { f16 e c b f e } d'2.  |
  e2. \times 2/3 { d8 e, d' }  \bar "||"
  \key des \major   f,,16 c'2. aes'8 <f' des'>4 des,,16 f f'8 f,16 c r4 r des8. f''4 r r <aes,,, des' des'> r  | % troigo!
  %05
  des''4 r2 r4  \bar "||" % kompletite
  \key g \major   c4 r2 r4  \bar "|." % kompletite
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
