@echo off

cd .\build\
MSBuild .\sovarian.vcxproj /verbosity:quiet

if %ERRORLEVEL% NEQ 0 (
    echo "error: build failed"
    exit 1
)

cd .\Debug\
.\sovarian.exe

exit %ERRORLEVEL%