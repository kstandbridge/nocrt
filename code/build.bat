@echo off

set CommonCompilerFlags=-MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4100 -FC -Z7 -GS- -Gs9999999 -DNOCRT_SLOW=1 -DNOCRT_INTERNAL=1 
set CommonLinkerFlags=-incremental:no -opt:ref -nodefaultlib -subsystem:windows -stack:0x100000,0x100000

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

del *.pdb > NUL 2> NUL

echo WAITING FOR PDB > lock.tmp

cl %CommonCompilerFlags% ..\nocrt\code\nocrt.cpp -LD /link -incremental:no -opt:ref -nodefaultlib -PDB:nocrt_%random%.pdb -EXPORT:CreateControls -EXPORT:HandleCommand

del lock.tmp

cl %CommonCompilerFlags% ..\nocrt\code\win32_nocrt.cpp /link %CommonLinkerFlags% kernel32.lib

popd