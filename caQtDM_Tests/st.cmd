#ld < /home/mezger/base-3.14.8.2/src/softIoc/O.linux-x86/devGaussWaveform.o
## Load record instances
require "sysmon"
dbLoadTemplate("SF-SGUIDE_main.subs")
iocInit()
