
XTAL  = 114322294
# XTAL  = 114285000
HSDIV = 7
N1    = 4
TGT   = 198300000

BIT_SHIFT = 28

def go():
	fDCO = TGT * N1 * HSDIV
	print(fDCO)

	controlVal = fDCO << BIT_SHIFT
	controlVal = controlVal / XTAL
	# controlVal &= 0xFFFFFFF
	# controlVal += int(fDCO / XTAL) << BIT_SHIFT

	controlVal = int(controlVal)

	print("{:02x}".format(controlVal).upper())
	print(controlVal)
	print((fDCO*1.0) / XTAL)



if __name__ == "__main__":
	go()
