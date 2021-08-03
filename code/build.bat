@echo off

ctime -begin nocrt.ctm

set CommonCompilerFlags=-diagnostics:column -WL -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -FC -Z7 -GS- -Gs9999999
set CommonCompilerFlags=-DNOCRT_INTERNAL=1 -DNOCRT_SLOW=1 %CommonCompilerFlags%
set CommonLinkerFlags=-STACK:0x100000,0x100000 -incremental:no -opt:ref kernel32.lib

IF NOT EXIST ..\data mkdir ..\data
IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

del *.pdb > NUL 2> NUL

echo WAITING FOR PDB > lock.tmp

REM App
cl %CommonCompilerFlags% -MTd -I..\iaca-win64\ ..\nocrt\code\nocrt.cpp ..\nocrt\code\nocrt_msvc.c -LD /link -incremental:no -opt:ref -PDB:nocrt_%random%.pdb -EXPORT:CreateControls -EXPORT:HandleCommand
set LastError=%ERRORLEVEL%
del lock.tmp

REM Platform
rc -nologo ..\nocrt\code\win32_resource.rc
cl %CommonCompilerFlags% -Od ..\nocrt\code\win32_nocrt.cpp ..\nocrt\code\nocrt_msvc.c ..\nocrt\code\win32_resource.res /link /SUBSYSTEM:windows %CommonLinkerFlags%
:: cl %CommonCompilerFlags% -Od ..\nocrt\code\win32_nocrt.cpp ..\nocrt\code\nocrt_msvc.c ..\nocrt\code\win32_resource.res /link /NODEFAULTLIB /SUBSYSTEM:windows %CommonLinkerFlags%

REM Cleanup
del *.obj
del ..\nocrt\code\win32_resource.res

popd

ctime -end nocrt.ctm %LastError%
