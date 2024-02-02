@echo off

pushd W:\new-horizons\code
set Wildcard=*.h *.c

echo ------
echo ------

@echo TODOs FOUND:
findstr -s -n -i -l "TODO" %Wildcard%

echo ------
echo ------
popd