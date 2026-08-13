..\..\..\..\bin\windows\packfolder.exe res resources.bin -binary
dmd dsciter.d win-res/app.res -i -I=.;../.. -J=.;../.. -od="../../../../build/windows/dobj" -of="../../../../bin/windows/x64/dsciter.exe" -O -release 
rem To disable console add these: -L/subsystem:windows -L/entry:mainCRTStartup
"../../../../bin/windows/x64/dsciter.exe"