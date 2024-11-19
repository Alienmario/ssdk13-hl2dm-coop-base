@echo off
title SOURCESDK SERVER COMPILE
SETLOCAL
:start
cls

IF "%MSBUILD22%"=="" (
	echo Install build Tools for Visual Studio 2022 ^(select Desktop programming package^) and define MSBUILD22 env variable first
	echo For example: "D:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
	pause && exit
)

set config=%1
IF "%config%"=="" (
	set config=Release
)

echo ----------------- VPC START -----------------
echo.

devtools\bin\vpc.exe /2013 /sdk2013ce +dedicated
IF %ERRORLEVEL% GTR 0 goto retry

echo.
echo ----------------- BUILD START (%config%) -----------------
echo.

"%MSBUILD22%" mathlib/mathlib.vcxproj -p:Configuration=%config%
IF %ERRORLEVEL% GTR 0 goto retry
"%MSBUILD22%" raytrace/raytrace.vcxproj -p:Configuration=%config%
IF %ERRORLEVEL% GTR 0 goto retry
"%MSBUILD22%" tier1/tier1.vcxproj -p:Configuration=%config%
IF %ERRORLEVEL% GTR 0 goto retry
"%MSBUILD22%" game/server/server_sdk2013ce.vcxproj -p:Configuration=%config%
IF %ERRORLEVEL% GTR 0 goto retry

IF NOT "%sourcesdk_server_dest%"=="" (
	echo.
	echo Copying server binary to "%sourcesdk_server_dest%\bin\"
	copy /b "..\game\mod_sdk2013ce\bin\server.dll" "%sourcesdk_server_dest%\bin\"
)

:retry
IF NOT "%2%"=="noretry" (
	echo:
	echo Retry?
	echo:
	pause
	goto start
)