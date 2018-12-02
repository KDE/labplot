$DEPLOYDIR = "DEPLOY"

cd C:\CraftRoot

RemoveItem -Recurse $DEPLOYDIR
windeployqt.exe -xml -texttospeech --release bin\labplot2.exe --dir $DEPLOYDIR
