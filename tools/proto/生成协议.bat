for %%i in (*.proto) do (  
    protoc.exe  --cpp_out=../../src/lib/proto %%i      
) 
pause