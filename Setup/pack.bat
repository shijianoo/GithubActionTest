@echo off

SET SOFT_NAME=%~1
SET SOFT_RELEASE=%~2
SET SOFT_VERSION=%~3

IF "%SOFT_NAME%"=="" (
    ECHO ERROR: SOFT_NAME CANNOT BE EMPTY!
    EXIT /B 1
)

IF NOT EXIST "%SOFT_RELEASE%" (
    ECHO ERROR: SOFT_RELEASE "%SOFT_RELEASE%" DOES NOT EXIST!
    EXIT /B 1
)

IF "%SOFT_VERSION%"=="" (
    ECHO ERROR: SOFT_VERSION CANNOT BE EMPTY!
    EXIT /B 1
)

ECHO SOFT_NAME=%SOFT_NAME%
ECHO SOFT_RELEASE=%SOFT_RELEASE%
ECHO SOFT_VERSION=%SOFT_VERSION%

SET UninstallExe=%~dp0installer\res\Uninstaller.exe
IF NOT EXIST "%UninstallExe%" (
    cmake -B "%~dp0build\uninstaller" -S "%~dp0uninstaller" -DSOFT_NAME="%SOFT_NAME%" || EXIT /B 1
    cmake --build "%~dp0build\uninstaller" --config Release || EXIT /B 1
    copy /Y "%~dp0bin\Release\Uninstaller.exe" "%UninstallExe%" || EXIT /B 1
)

cmake -B "%~dp0build\installer" -S "%~dp0installer" -DSOFT_NAME="%SOFT_NAME%" -DSOFT_RELEASE="%SOFT_RELEASE%" -DSOFT_VERSION="%SOFT_VERSION%" || EXIT /B 1
cmake --build "%~dp0build\installer" --config Release || EXIT /B 1

EXIT /B 0
