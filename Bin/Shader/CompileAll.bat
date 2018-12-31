for /d %%i in ("./*.*") do (
    cd ./%%i
    call Compile.bat
    cd ../
)
pause