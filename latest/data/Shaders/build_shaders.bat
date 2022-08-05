
@ECHO OFF
SETLOCAL
FOR /R %%f IN (run.bat) DO (
	@IF EXIST %%f echo %%f
	@IF EXIST %%f cd %%~pf
	@IF EXIST %%f call %%f
)
echo Compleate
pause