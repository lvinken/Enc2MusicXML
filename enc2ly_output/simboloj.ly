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
\relative b'
{
  \clef treble
  \key c \major
  \time 4/4
  b8-\rightHandFinger #1  d-\rightHandFinger #2  d-\rightHandFinger #3  c-\rightHandFinger #4  d-\rightHandFinger #5  d-P f-H d-T  |
  f8\1 e\2 e\3 g\4 f\5 f\6 f\7 f\8  \bar "|."
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
