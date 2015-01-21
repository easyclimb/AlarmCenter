REM usage:
REM "$(ProjectDir)AfterBuild.bat" $(ProjectDir) $(TargetDir) 
echo -----------------------------------------------------
echo Congrats! You've built your solution successfully! :)
echo %1
echo %2
AddVersion.exe "%1VersionNo.h" "%1VersionNo.ini" "b" "1"
copy "%1VersionNo.ini" "%2VersionNo.ini"
echo Add file's version OK!
echo -----------------------------------------------------