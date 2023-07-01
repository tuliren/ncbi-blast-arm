@echo off
REM
REM $Id: msvcvars.bat 619587 2020-11-06 17:56:11Z gouriano $
REM

:devenv

for /f "tokens=* USEBACKQ" %%i IN (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere" -version [16.0^,17.0^) -property productPath`) do (
    set DEVENV="%%i"
)

:end
