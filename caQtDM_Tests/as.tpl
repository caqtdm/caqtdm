
# when MY:AS:1 == 1 can     write to MYTEST:1
# when MY:AS:1 != 1 can not write to MYTEST:1

record(ai, "MY:AS:1")
{
        field(VAL,  "0")
        field(PINI, "YES")
	field(ASG,  "ASCONTROL")
	field(HIGH, "0.1")
	field(LOW,  "-0.1")
	field(HSV, "MAJOR")
	field(LSV, "MAJOR")
}



record(ai, "MYTEST:1")
{
        field(VAL,  "123")
        field(PINI, "YES")
	field(ASG,  "DEFAULT")
}


