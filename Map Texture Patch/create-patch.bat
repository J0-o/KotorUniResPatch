@echo off
set SCRIPT_DIR=%~dp0
call "%SCRIPT_DIR%..\..\tools\create-patch.bat" %*
