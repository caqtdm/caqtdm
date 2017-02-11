find . -name '*.cpp' | xargs wc -l
find . -name '*.h' | xargs wc -l
find . -name '*.c' | xargs wc -l
cloc --exclude-lang=DTD,Lua,make,Python,Perl,HTML,IDL,CSS,XML,ASP.NET,'MSBuild script','Bourne Shell','Bourne Again Shell',ASP.Net,'WiX source','DOS Batch',JSON,'Windows Resource File' .
