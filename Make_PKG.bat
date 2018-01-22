@echo off
set PS3SDK=/c/PSDK3v2
set WIN_PS3SDK=C:/PSDK3v2
set PATH=%WIN_PS3SDK%/mingw/msys/1.0/bin;%WIN_PS3SDK%/mingw/bin;%WIN_PS3SDK%/ps3dev/bin;%WIN_PS3SDK%/ps3dev/ppu/bin;%WIN_PS3SDK%/ps3dev/spu/bin;%WIN_PS3SDK%/mingw/Python27;%PATH%;
set PSL1GHT=%PS3SDK%/psl1ght
set PS3DEV=%PS3SDK%/ps3dev

if exist EP0001-DBREBUILD_00-0000000000000000.pkg del EP0001-DBREBUILD_00-0000000000000000.pkg>nul
if exist DB_Rebuilder_by_Red-J_Installer.pkg del DB_Rebuilder_by_Red-J_Installer.pkg>nul

if exist DB_Rebuilder_by_Red-J_Installer.elf del DB_Rebuilder_by_Red-J_Installer.elf>nul
if exist DB_Rebuilder_by_Red-J_Installer.self del DB_Rebuilder_by_Red-J_Installer.self>nul
if exist build del /s/q build\*.*>nul

make pkg

ren build\pkg EP0001-DBREBUILD_00-0000000000000000
param_sfo_editor.exe build\EP0001-DBREBUILD_00-0000000000000000\PARAM.SFO "ATTRIBUTE" 133

if exist DB_Rebuilder_by_Red-J_Installer.elf del DB_Rebuilder_by_Red-J_Installer.elf>nul
if exist DB_Rebuilder_by_Red-J_Installer.self del DB_Rebuilder_by_Red-J_Installer.self>nul
if exist DB_Rebuilder_by_Red-J_Installer.pkg del DB_Rebuilder_by_Red-J_Installer.pkg>nul
if exist build del /q build\*.*>nul
if not exist build goto end

echo ContentID = EP0001-DBREBUILD_00-0000000000000000>package.conf
echo Klicensee = 000000000000000000000000000000000000>>package.conf
echo PackageVersion = 01.00>>package.conf
echo DRMType = Free>>package.conf
echo ContentType = GameExec>>package.conf

psn_package_npdrm.exe -n package.conf build\EP0001-DBREBUILD_00-0000000000000000

del package.conf

if exist DB_Rebuilder_by_Red-J_Installer.pkg del DB_Rebuilder_by_Red-J_Installer.pkg>nul
move /y EP0001-DBREBUILD_00-0000000000000000.pkg DB_Rebuilder_by_Red-J_Installer.pkg
pause

del /s/q build>nul
rd /q/s build

:end
