
@echo This batch file installs inadyn as a service. 
@echo The 'instsrv.exe' and 'srvany.exe' are needed. These are free programs, part of Windows Res Kit CD ,  but cannot be re-distributed. So one has to download them from Microsoft or other sites: Example: http://www.electrasoft.com/srvany/srvany.htm. See documentation for more details.

 copy srvany.exe %SystemRoot%\srvany.exe
 INSTSRV inadyn %SystemRoot%\srvany.exe
