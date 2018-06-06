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
  c1  |
  d1  |
  g,1  |
  R1  |
  %05
  R1  |
  R1  \bar "|."
}
harmonioAa =
\chordmode
{
  \time 4/4
  a1:A01234567890123 |
  aes1:13sus4/fis |
  a1:APersonigita é è  |
  a1:APersonigita é è  |
  %05
  a1:APersonigita é è  |
  a1:APersonigita é è  |
}

\score {
  \new StaffGroup {
    \override Score.RehearsalMark #'self-alignment-X = #LEFT
    <<
      \new ChordNames {\set chordChanges = ##t \harmonioAa}
      \new Staff \with {instrumentName = #"" shortInstrumentName = #" "} \liniaroAa
    >>
  }
  \layout {}
}
