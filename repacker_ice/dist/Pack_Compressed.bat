@echo off

if "%~1" == "" (
  echo ※このウィンドウを閉じて、アーカイブしたいファイルをドラッグ＆ドロップしてください。
  echo close this window, please drag and drop the files you want to archive.
  pause
  exit /b
)

echo:
"%~dp0ice.exe" -a -c -g 2 -s "%~dp0GroupList.txt" -o "%~dp0%~n1.ice" %*
echo:
certutil -hashfile "%~dp0%~n1.ice" MD5
pause
