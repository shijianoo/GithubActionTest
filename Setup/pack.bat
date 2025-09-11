@echo off

IF NOT "%~1"=="" SET SOFT_NAME=%~1
IF NOT "%~2"=="" SET SOFT_VERSION=%~2
IF NOT "%~3"=="" SET SOFT_RELEASE_DIR=%~3

IF "%SOFT_NAME%"=="" (
    ECHO Error: SOFT_NAME cannot be empty!
    GOTO :EOF
)

IF "%SOFT_VERSION%"=="" (
    ECHO Error: SOFT_VERSION cannot be empty!
    GOTO :EOF
)

IF NOT EXIST "%SOFT_RELEASE_DIR%" (
    ECHO Error: SOFT_RELEASE_DIR "%SOFT_RELEASE_DIR%" does not exist!
    GOTO :EOF
)

ECHO SOFT_NAME=%SOFT_NAME%
ECHO SOFT_VERSION=%SOFT_VERSION%
ECHO SOFT_RELEASE_DIR=%SOFT_RELEASE_DIR%

SET UninstallExe=%~dp0installer\res\Uninstaller.exe
IF NOT EXIST "%UninstallExe%" (
    cmake -B "%~dp0build\uninstaller" -S "%~dp0uninstaller" -DSOFT_NAME="%SOFT_NAME%" || EXIT /B 1
    cmake --build "%~dp0build\uninstaller" --config Release || EXIT /B 1
    copy /Y "%~dp0bin\Release\Uninstaller.exe" "%UninstallExe%" || EXIT /B 1
)

cmake -B "%~dp0build\installer" -S "%~dp0installer" -DSOFT_NAME="%SOFT_NAME%" -DSOFT_VERSION="%SOFT_VERSION%" -DSOFT_RELEASE_DIR="%SOFT_RELEASE_DIR%" || EXIT /B 1
cmake --build "%~dp0build\installer" --config Release || EXIT /B 1

EXIT /B 0
