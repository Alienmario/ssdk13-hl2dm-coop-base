@echo off
title REL SOURCESDK SERVER COMPILE

:start
cls

echo ----------------- VPC START -----------------
echo:

IF "%MSBUILD22%"=="" (
	echo Install build Tools for Visual Studio 2022 ^(select Desktop programming package^) and define MSBUILD22 env variable first
	echo For example: "D:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
	pause && exit
)

devtools\bin\vpc.exe /2013 /sdk2013ce +dedicated

echo:
echo ----------------- BUILD START -----------------
echo:

"%MSBUILD22%" game/server/server_sdk2013ce.vcxproj -p:Configuration=Release

@REM echo:
@REM echo Retry?
@REM echo:
@REM pause
@REM goto start