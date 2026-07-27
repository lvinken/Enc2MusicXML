import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

//import com.myself 1.0

//Item {
ApplicationWindow {
    id: root
    width: 650
    height: 270
    visible: true
    color: "white"

    Connections {
        target: converter
        function onResultChanged(res) { console.log("result changed: " + res) }
    }

    MessageDialog {
        id: aboutDialog
        buttons: MessageDialog.Ok
        text: applicationName + " version " + applicationVersion
    }

    FileDialog {
        id: openFileDialog
        title: "Open Encore File"
        nameFilters: [
            "Encore files (*.enc)"
        ]
        onAccepted: {
            // note: selectedFile is a URL
            // MS Windows "file:///c:\aaa\bbb.txt" should be converted to "c:\aaa\bar.txt" (TODO).
            // Linux "file:///aaa/bbb.txt" should be converted to "aaa/bbb.txt" (done).
            //var name = selectedFile.toString().replace(/^(file:\/{2})/, "") // assume the file dialog always returns a local file
            console.log("Encore file: " + selectedFile.toString())
            sourceText.text = selectedFile.toString()
        }
    }

    FileDialog {
        id: saveFileDialog
        title: "Save MusicXML File"
        fileMode: FileDialog.SaveFile
        nameFilters: [
            "MusicXML files (*.musicxml)"
        ]
        currentFile: sourceText.text.replace(/\.enc$/, "")
        onAccepted: {
            // note: selectedFile is a URL
            console.log("MusicXML file: " + selectedFile.toString())
            destinationText.text = selectedFile
        }
    }

    ColumnLayout {
        id: aColumnLayout
        spacing: 0
        x: 10

        RowLayout {
            spacing: 10
            Layout.alignment: Qt.AlignLeft
            Button { text: "About"; onClicked: aboutDialog.open() }
        }

        RowLayout {
            Button { text: "Select Encore file"; onClicked: openFileDialog.open() }
            Text { id: sourceText; Layout.alignment: Qt.AlignLeft }
        }

        RowLayout {
            Button { text: "Select MusicXML file"; onClicked: saveFileDialog.open() }
            Text { id: destinationText; Layout.alignment: Qt.AlignLeft }
        }

        RowLayout {
            Button {
                text: "Convert"
                enabled: sourceText.text !== "" && destinationText.text !== ""
                onClicked: {
                    console.log("convert '" + sourceText.text + "' to '" + destinationText.text + "'")
                    converter.convert(sourceText.text, destinationText.text)
                    console.log("result: " + converter.getResult())
                    resultText.text = "result: " + converter.getResult()
                }
            }

            Text { id: resultText; Layout.alignment: Qt.AlignLeft }
        }
    }
}
