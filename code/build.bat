@echo off

ctime -begin nocrt.ctm

set CommonCompilerFlags=-diagnostics:column -WL -O2 -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -FC -Z7 -GS- -Gs9999999
set CommonCompilerFlags=-DNOCRT_INTERNAL=1 -DNOCRT_SLOW=1 -DNOCRT_WIN32=1 %CommonCompilerFlags%
set CommonLinkerFlags=-STACK:0x100000,0x100000 -incremental:no -opt:ref /NODEFAULTLIB /SUBSYSTEM:windows

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

del *.pdb > NUL 2> NUL

echo WAITING FOR PDB > lock.tmp

cl %CommonCompilerFlags% -MTd ..\nocrt\code\nocrt.cpp ..\nocrt\code\nocrt_msvc.c -LD /link %CommonLinkerFlags% -PDB:nocrt_%random%.pdb -EXPORT:CreateControls -EXPORT:HandleCommand

del lock.tmp

rc -nologo ..\nocrt\code\win32_resource.rc

cl %CommonCompilerFlags% ..\nocrt\code\win32_nocrt.cpp ..\nocrt\code\nocrt_msvc.c ..\nocrt\code\win32_resource.res /link %CommonLinkerFlags% kernel32.lib
set LastError=%ERRORLEVEL%

del /q ..\nocrt\code\win32_resource.res

popd

ctime -end nocrt.ctm %LastError%