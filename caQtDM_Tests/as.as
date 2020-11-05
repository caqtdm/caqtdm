
HAG(TEST)          {pc2401, pc2401.psi.ch, mpc2060, mpc2060.psi.ch}

ASG(ASCONTROL) {
	RULE(1,READ)
	RULE(1,WRITE)
}

ASG(DEFAULT) {

	INPA("MY:AS:1")

# anyone can read
	RULE(1,READ)

# IOCs can write to each other
	RULE(1,WRITE) {
		HAG(TEST)
		CALC("A=0")
	}

}

