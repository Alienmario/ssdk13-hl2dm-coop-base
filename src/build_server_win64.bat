@echo off
title SOURCESDK SERVER COMPILE
SETLOCAL
:start
cls

IF "%MSBUILD22%"=="" (
	echo Please set MSBUILD22 environment variable to the MSBuild.exe file.
	echo For example: "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
	pause && exit
)

set config=%1
IF "%config%"=="" (
	set config=Release
)

:: Set current directory to the script directory
cd %~dp0

echo ----------------- VPC START -----------------
echo.

devtools\bin\vpc.exe +dedicated /sdk2013ce /win64 /define:SOURCESDK /mksln dedicated64.sln
IF %ERRORLEVEL% GTR 0 goto retry

echo.
echo ----------------- BUILD START (%config%) -----------------
echo.

"%MSBUILD22%" dedicated64.sln /p:Configuration=%config%
IF %ERRORLEVEL% GTR 0 goto retry

echo.
IF NOT "%sourcesdk_server_dest%"=="" (
	echo Copying server binary to "%sourcesdk_server_dest%\bin\x64"
	copy /b "..\game\mod_coopbase\bin\x64\server.dll" "%sourcesdk_server_dest%\bin\x64\"
) ELSE (
	echo If you want to auto-install the server binary, define "sourcesdk_server_dest" env variable.
)

:retry
IF NOT "%2%"=="noretry" (
	echo.
	echo Retry?
	echo.
	pause
	goto start
)