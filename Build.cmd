@echo off
cd "%~dp0/src"

echo Clearing binaries...
rd /q /s "bin"
md "bin"

echo Clearing objects...
rd /q /s "obj"
md "obj"

echo Building objects...
windres.exe -i "Resources\Library.rc" -o "obj\Library.o"

echo Building artifacts...
gcc.exe -Oz -s -Wl,--gc-sections,--exclude-all-symbols,--wrap=memcpy,--wrap=memset -static -shared -municode -nostdlib -e DllMain -DINITGUID "Library.c" "obj\Library.o" -lminhook -lkernel32 -luser32 -ldxgi -o "bin\vcruntime140_1.dll"

echo %~dp0/src/bin/vcruntime140_1.dll