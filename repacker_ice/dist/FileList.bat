@echo off

if "%~1" == "" (
  echo �����̃E�B���h�E����āA�t�@�C�����X�g���쐬�������t�@�C�����h���b�O���h���b�v���Ă��������B
  echo Close this window and drag and drop the file you want to create the file list.
  pause
  exit /b
)

echo:
"%~dp0ice.exe" -f -o "%~dp0%~n1.txt" %*
echo:
pause
