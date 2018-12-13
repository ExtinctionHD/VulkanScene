for /d %%i in ("./*.*") do (
    cd ./%%i
    call compile.bat
    cd ../
)
pause