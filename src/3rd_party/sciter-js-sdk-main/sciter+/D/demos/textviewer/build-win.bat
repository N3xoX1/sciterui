..\..\..\..\bin\windows\packfolder.exe res resources.bin -binary
dmd textviewer.d win-res/app.res -i -I=.;../.. -J=.;../.. -od="../../../../build/windows/dobj" -of="../../../../bin/windows/x64/textviewer.exe" 
"../../../../bin/windows/x64/textviewer.exe" "%CD%/textviewer.d"
rem -O -release -L/subsystem:windows -L/entry:mainCRTStartup
