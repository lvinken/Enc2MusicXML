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
\relative b
{
  \clef treble
  \key c \major
  \time 4/4
  b8 c d e f g a b  |
  c8 d e f g a b c  |
  R1  |
  R1  |
  %05
  R1  |
  R1  \bar "|."
}

liniaroAb =
\drummode
{
  \time 4/4
  hc8 ss hhp tomfl bd tomfh toml tomml  |
  sn8 tommh hh tomh cymr cymc cyms hc  |
  \times 2/3 { sn8 sn sn } sn8. sn8.. r4 r16 r32  | % kompletite
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
        \new Staff \with {instrumentName = #"" shortInstrumentName = #" "} \liniaroAa
        \new DrumStaff \with {instrumentName = #"" shortInstrumentName = #" "} \liniaroAb
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
        \new DrumStaff \liniaroAb
      >>
    }
    \layout {}
  }
}

