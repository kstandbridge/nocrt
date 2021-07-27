@echo off

set CommonCompilerFlags=-MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4100 -FC -Z7 -GS- -Gs9999999 -DNOCRT_SLOW=1 -DNOCRT_INTERNAL=1 
set CommonLinkerFlags=-STACK:0x100000,0x100000 -incremental:no -opt:ref kernel32.lib

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

del *.pdb > NUL 2> NUL

echo WAITING FOR PDB > lock.tmp

:: cl %CommonCompilerFlags% ..\nocrt\code\nocrt.cpp -LD /link -incremental:no -opt:ref -PDB:nocrt_%random%.pdb -EXPORT:CreateControls -EXPORT:HandleCommand
cl %CommonCompilerFlags% ..\nocrt\code\nocrt.cpp -LD /link /NODEFAULTLIB /SUBSYSTEM:windows %CommonLinkerFlags% -PDB:nocrt_%random%.pdb -EXPORT:CreateControls -EXPORT:HandleCommand

del lock.tmp

rc -nologo ..\nocrt\code\win32_resource.rc
cl %CommonCompilerFlags% ..\nocrt\code\win32_nocrt.cpp ..\nocrt\code\win32_resource.res /link /NODEFAULTLIB /SUBSYSTEM:windows %CommonLinkerFlags%
del /q ..\nocrt\code\win32_resource.res

popd