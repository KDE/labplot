$DEPLOYDIR = "DEPLOY"

cd C:\CraftRoot

Remove-Item -Recurse $DEPLOYDIR
windeployqt.exe -qml -texttospeech -xml -xmlpatterns --release bin\labplot2.exe --dir $DEPLOYDIR
