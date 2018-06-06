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
  c4 r2 r4  | % kompletite
  c4 r2 r4  | % kompletite
  c4 c2 r4  | % kompletite
  b8 c r a c16 c b8 b4  |
  %05
  a1  |
  r1  \bar "|."
}

liniaroAb =
{
  \time 4/4
  c16 c2 r4 r8 r16  | % kompletite
  c16 c2 r4 r8 r16  | % kompletite
  c16 r2 r4 r8 r16  | % kompletite
  c8 c r c c16 c c8 c4  |
  %05
  c2. c8.. c32  |
  \times 2/3 { c8 c c } c4 c2  \bar "|."
}

\bookpart {
  \score {
    \new StaffGroup {
      \override Score.RehearsalMark #'self-alignment-X = #LEFT
      <<
        \new Staff \with {instrumentName = #"" shortInstrumentName = #" "} \liniaroAa
        \new RhythmicStaff \with {instrumentName = #"" shortInstrumentName = #" "} \liniaroAb
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
        \new Staff \liniaroAa
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
        \new RhythmicStaff \liniaroAb
      >>
    }
    \layout {}
  }
}

