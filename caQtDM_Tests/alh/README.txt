alh test material
=================

alh_test.db           softIoc database: jba:Example1..10 (ai, HIGH 70 MINOR, HIHI 90 MAJOR),
                      testjbaai (force pv), ALH:ACK (bo), ALH:SEVR, ALH:HB, ALH:CONFIG (char waveform 16000)
test.alhConfig        upstream example of the alh distribution
alh_runtime.alhConfig small config using $SEVRPV, $HEARTBEATPV, $ACKPV, $FORCEPV, masks T/D
alh_configpv.ui       panel generated with alh2ui from alh_runtime.alhConfig plus configPV = ALH:CONFIG

  softIoc -S -d alh_test.db &
  caQtDM alh_runtime.alhConfig          (or caQtDM alh_configpv.ui)
  caput jba:Example1 95                 -> MAJOR, latched until acknowledged
  caput testjbaai 1                     -> group SUB disabled by $FORCEPV, 0 restores
  caput -S ALH:CONFIG '{"alh_runtime.alhConfig":{"nodes":{"SMOKE/jba:Example2":{"disable":true}}}}'
  CAQTDM_LOGGING_CONSOLE_VERBOSE=1      shows the JSON alarm events (category caqtdm.alarm.events)
