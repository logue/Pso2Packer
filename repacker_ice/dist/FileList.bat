@echo off

if "%~1" == "" (
  echo ※このウィンドウを閉じて、ファイルリストを作成したいファイルをドラッグ＆ドロップしてください。
  echo Close this window and drag and drop the file you want to create the file list.
  pause
  exit /b
)

echo:
"%~dp0ice.exe" -f -o "%~dp0%~n1.txt" %*
echo:
pause
