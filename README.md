Enc2MusicXML is a Qt tool that converts Encore files to MusicXML.
It was developed using Qt 5.8.0 and Apple LLVM version 7.3.0
(clang-703.0.31) on OS X 10.11.6.

## Status

In development. Enc2MusicXML extracts the same information from Encore files as does enc2ly,
but the MusicXML export is still incomplete.

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

## Credits

Based on
* [enc2ly](http://enc2ly.sourceforge.net) version 0.6, copyright (C) 2012, 2013 Felipe E. F. de Castro

Class and variable names (partly) inspired by
* [go-enc2ly](https://github.com/hanwen/go-enc2ly), copyright (C) 2012 - 2014 Han-Wen Nienhuys

## License

GPL v3 or later.
