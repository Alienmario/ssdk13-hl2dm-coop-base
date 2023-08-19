@echo off
title REL SOURCESDK SERVER COMPILE
SETLOCAL
:start
cls

echo ----------------- VPC START -----------------
echo:

IF "%MSBUILD22%"=="" (
	echo Install build Tools for Visual Studio 2022 ^(select Desktop programming package^) and define MSBUILD22 env variable first
	echo For example: "D:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
	pause && exit
)

set config=%1
IF "%config%"=="" (
	set config=Release
)

devtools\bin\vpc.exe /2013 /sdk2013ce +dedicated

echo:
echo ----------------- BUILD START (%config%) -----------------
echo:

"%MSBUILD22%" game/server/server_sdk2013ce.vcxproj -p:Configuration=%config%

IF NOT "%2%"=="noretry" (
	echo:
	echo Retry?
	echo:
	pause
	goto start
)