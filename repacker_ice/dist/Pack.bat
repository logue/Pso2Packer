@echo off

if "%~1" == "" (
  echo �����̃E�B���h�E����āA�A�[�J�C�u�������t�@�C�����h���b�O���h���b�v���Ă��������B
  echo close this window, please drag and drop the files you want to archive.
  pause
  exit /b
)

echo:
"%~dp0ice.exe" -a -g 2 -s "%~dp0GroupList.txt" -o "%~dp0%~n1.ice" %*
certutil -hashfile "%~dp0%~n1.ice" MD5
echo:
pause
