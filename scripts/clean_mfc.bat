@echo off
setlocal

if /I "%~1"=="/?" goto usage
if /I "%~1"=="-h" goto usage
if /I "%~1"=="--help" goto usage

pushd "%~dp0\.."

if not exist build mkdir build

if exist build\obj rd /s /q build\obj
del /q build\PmbusGuiTool.lib >nul 2>nul
del /q build\PmbusGuiTool.exp >nul 2>nul
del /q build\*.pdb >nul 2>nul
del /q build\*.ilk >nul 2>nul
del /q build\*.idb >nul 2>nul
del /q build\*.iobj >nul 2>nul
del /q build\*.ipdb >nul 2>nul

echo Clean complete. Preserved build\PmbusGuiTool.exe and build\pmbus_gui_tool.ini.

popd
endlocal
goto eof

:usage
echo Usage: scripts\clean_mfc.bat
echo Removes PC build intermediates and debug artifacts.
echo Preserves build\PmbusGuiTool.exe and build\pmbus_gui_tool.ini.
endlocal

:eof
