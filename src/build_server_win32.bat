@echo off
title SOURCESDK SERVER COMPILE
SETLOCAL
:start
cls

set MSBUILD_CMD=%MSBUILD22%
IF "%MSBUILD_CMD%"=="" (
	where msbuild >nul 2>nul
	IF %ERRORLEVEL% EQU 0 (
		set MSBUILD_CMD=msbuild
	) ELSE (
		echo Please set MSBUILD22 environment variable to the MSBuild.exe file.
		echo For example: "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
		pause && exit
	)
)

set config=%1
IF "%config%"=="" (
	set config=Release
)

:: Set current directory to the script directory
cd %~dp0

echo ----------------- VPC START -----------------
echo.

devtools\bin\vpc.exe +dedicated /sdk2013ce /win32 /define:SOURCESDK /mksln dedicated32.sln
IF %ERRORLEVEL% GTR 0 goto retry

echo.
echo ----------------- BUILD START (%config%) -----------------
echo.

"%MSBUILD_CMD%" dedicated32.sln /p:Configuration=%config%
IF %ERRORLEVEL% GTR 0 goto retry

echo.
IF NOT "%sourcesdk_server_dest%"=="" (
	echo Copying server binary to "%sourcesdk_server_dest%\bin\"
	copy /b "..\game\mod_coopbase\bin\server.dll" "%sourcesdk_server_dest%\bin\"
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

exit /b %ERRORLEVEL%