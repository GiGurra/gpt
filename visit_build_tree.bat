@echo off
goto:script_begin

REM  -- -- -- --  -- -- -- -- -- 
REM  -- -- -- FUNCTIONS -- -- -- 
REM  -- -- -- --  -- -- -- -- -- 

:process_subdir
	setlocal
	echo Processing %1
	cd %1
	%~2
	echo.
	endlocal
goto:eof

REM  -- -- -- -- -- -- --
REM  -- -- SCRIPT - -- -- 
REM  -- -- -- -- -- -- --
:script_begin

call:process_subdir gpt-common %1
call:process_subdir gpt-displaysreceiver %1
call:process_subdir gpt-displaystransmitter %1
call:process_subdir gpt-keyreceiver %1
call:process_subdir gpt-keytransmitter %1
call:process_subdir gpt-shmreceiver %1
call:process_subdir gpt-shmtransmitter %1
call:process_subdir gpt-windowsontop %1