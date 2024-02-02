@echo off

pushd W:\new-horizons\code
set Wildcard=*.h *.c

echo ------
echo ------

@echo STATICS FOUND:
findstr -s -n -i -l "static" %Wildcard%

echo ------
echo ------

@echo GLOBALS FOUND:
findstr -s -n -i -l "local_persist" %Wildcard%
findstr -s -n -i -l "global" %Wildcard%

echo ------
echo ------
popd