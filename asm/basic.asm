#
# Just a few basic instructions to watch the pipeline going in ModelSim
#

	addi	r1 = r0, 255;  # first instruction not executed

	addi	r1 = r0, 15;

	addi	r2 = r0, 4;
	addi	r3 = r0, 3;
	add	r4 = r2, r3;
	halt;
