@echo off

if "%~1" == "" (
  echo �����̃E�B���h�E����āAMD5�n�b�V�����m�F�������t�@�C�����h���b�O���h���b�v���Ă��������B
  echo close this window, please drag and drop the files you want to check MD5 hash.
  pause
  exit /b
)

echo:

certutil -hashfile "%~1" MD5
echo:
pause
