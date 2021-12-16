## Load record instances
dbLoadDatabase ("mySimulation.db", "", "")
iocInit()

dbpf byteArray01 '[1, 2, 3, 4, 5]'
dbpf floatArray01 '[0.1, 0.2, 0.3, 0.4, 0.5]'
dbpf doubleArray01 '[0.1111, 0.2222, 0.3333, 0.44444, 0.5555]'
dbpf intArray01 '[1234, 2345, 3666, 4777, 5888]'
dbpf shortArray01 '[127, -127, 3, 40, -55,120,33,-120]'
