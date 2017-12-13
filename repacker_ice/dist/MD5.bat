@echo off

if "%~1" == "" (
  echo ※このウィンドウを閉じて、MD5ハッシュを確認したいファイルをドラッグ＆ドロップしてください。
  echo close this window, please drag and drop the files you want to check MD5 hash.
  pause
  exit /b
)

echo:

certutil -hashfile "%~1" MD5
echo:
pause
