Enc2MusicXML is a Qt tool that converts Encore files to MusicXML.
It was developed using Qt 5.8.0 and Apple LLVM version 7.3.0
(clang-703.0.31) on OS X 10.11.6.

## Status

In development. Enc2MusicXML extracts the same information from Encore files as does enc2ly,
but the MusicXML export is still incomplete.

Supported features:
- Single and multi-staff parts
- Multiple voices per staff
- Notes, rests, chords
- Dotted and double-dotted values
- Triplets and quintuplets
- Grace notes
- Ties, slurs
- Dynamic wedges (crescendo/decrescendo)
- Tempo markings
- Staff text
- Repeats (including alternative endings, segno, coda)
- Clef changes
- Key signature changes (with correct pitch spelling)
- Time signature changes
- Staff visibility
- Instrument names (including v0xC4 formula-position encoding)
- MIDI-recorded files: note durations are inferred from the written face value and
  actual MIDI timing, with tuplet groups detected automatically.

Known limitations:
- Lyrics not supported
- Nested tuplets not supported
- Some non-standard articulations are ignored

## Building

* Install the [Qt](https://www.qt.io) toolkit.
* Open Enc2MusicXML.pro in Qt Creator and build the project.

Alternatively, build on the command line using:

 qmake
 make

## Running

Enc2MusicXML writes output to stdout and (lots of) debug info
to stderr. To convert an Encore file to MusicXML on Unix, use:

 Enc2MusicXML -m file.enc >file.musicxml 2>/dev/null

## Testing

Test data and an autotester (iotest) are provided in the testdata directory.

Run all tests with:

 cd testdata && bash iotest ../src/Enc2MusicXML

Each test consists of an `.enc` input file, a `.ref.txt` reference for the text dump
(`-d` flag), and optionally a `.ref.xml` reference for the MusicXML output (`-m` flag).

The test suite covers notated scores, MIDI-recorded files, multi-part/multi-voice scores,
chord clusters, tie/slur handling, and MIDI artifact filtering. Large MIDI-recorded
orchestral scores (multi-staff, complex tuplet timing) are validated manually against
MuseScore Studio import.

## Credits

Based on
* [enc2ly](http://enc2ly.sourceforge.net) version 0.6, copyright (C) 2012, 2013 Felipe E. F. de Castro

Class and variable names (partly) inspired by
* [go-enc2ly](https://github.com/hanwen/go-enc2ly), copyright (C) 2012 - 2014 Han-Wen Nienhuys

## License

GPL v3 or later.
