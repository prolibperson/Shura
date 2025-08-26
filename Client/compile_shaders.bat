@echo off
setlocal enabledelayedexpansion

cd /d "%~dp0"

set "vertex_glsl=Shaders\vertex.glsl"
set "vertex_spv=Shaders\spv\vertex.spv"

if not exist "%vertex_glsl%" (
    echo %vertex_glsl% not found
    goto check_fragment
)

if not exist "%vertex_spv%" (
    goto compile_vertex
)

for %%i in ("%vertex_glsl%") do set vertex_glsl_date=%%~ti
for %%i in ("%vertex_spv%") do set vertex_spv_date=%%~ti

call :convertdate "%vertex_glsl_date%" vertex_glsl_num
call :convertdate "%vertex_spv_date%" vertex_spv_num

if !vertex_glsl_num! GTR !vertex_spv_num! (
    goto compile_vertex
) else (
    goto check_fragment
)

:compile_vertex
glslc -fshader-stage=vertex "%vertex_glsl%" -o "%vertex_spv%"

:check_fragment
set "fragment_glsl=Shaders\fragment.glsl"
set "fragment_spv=Shaders\spv\fragment.spv"

if not exist "%fragment_glsl%" (
    goto end
)

if not exist "%fragment_spv%" (
    goto compile_fragment
)

for %%i in ("%fragment_glsl%") do set fragment_glsl_date=%%~ti
for %%i in ("%fragment_spv%") do set fragment_spv_date=%%~ti

call :convertdate "%fragment_glsl_date%" fragment_glsl_num
call :convertdate "%fragment_spv_date%" fragment_spv_num

if !fragment_glsl_num! GTR !fragment_spv_num! (
    goto compile_fragment
) else (
    goto end
)

:compile_fragment
glslc -fshader-stage=fragment "%fragment_glsl%" -o "%fragment_spv%"

goto end

:convertdate
set "datetime=%~1"
set "result_var=%~2"

for /f "tokens=1,2,3 delims=/ " %%a in ("%datetime%") do (
    set "month=%%a"
    set "day=%%b" 
    set "year=%%c"
)

for /f "tokens=4,5,6 delims=: " %%d in ("%datetime%") do (
    set "hour=%%d"
    set "minute=%%e"
    set "ampm=%%f"
)

if "!ampm!"=="PM" (
    if not "!hour!"=="12" (
        set /a hour=!hour!+12
    )
) else (
    if "!hour!"=="12" (
        set hour=00
    )
)

if !month! lss 10 set month=0!month!
if !day! lss 10 set day=0!day!
if !hour! lss 10 set hour=0!hour!
if !minute! lss 10 set minute=0!minute!

set "comparable=!year!!month!!day!!hour!!minute!"
set "%result_var%=!comparable!"
goto :eof

:end
