
import time
import serial

import eore

def go():
	print("Starting")
	port = eore.EoreController("COM40")
	time.sleep(2)
	port.port.write("\x5Dasdasdasd")
	print("Port opened, written to")

	x = 0
	port.writeAtten(0, 0)
	port.writeAtten(1, 0)
	port.writeAtten(2, 0)
	port.writeAtten(3, 0)
	port.writeAtten(4, 0)
	port.writeAtten(5, 0)
	port.writeAtten(eore.MID_AMP_ATTEN, 15)
	port.writeAtten(eore.AUX_TONE_INPUT, 30)

	vals = [0,1,2,3,4,5]

	port.writeSwitch(0, 4)

	# port.writeAtten(2, 31.5)
	port.noiseDiodePowerCtl(1)
	port.oscPowerCtl()

	port.writeOscillator(0, 100e6)
	while 1:


		# try:
		# 	atten = float(x)

		# 	val = port.exhaust()
		# 	# port.readSwitch()
		# 	port.writeAtten(0, atten)
		# 	# port.writeAtten(1, atten)
		# 	port.writeAtten(2, atten)
		# 	# port.writeAtten(0, 31.5-atten)


		# except KeyboardInterrupt:
		# 	raise


		# # port.writeOscillator(0, 100e6)
		# port.writeSwitch(0, NOISE_SOURCE)
		# time.sleep(5)

		# port.writeSwitch(0, EMI_ANTENNA)
		# time.sleep(5)

		# x = 0
		# port.writeOscillator(0, 0)
		while 1:
			# x = x % len(vals)
			port.writeSwitch(0, eore.NOISE_SOURCE)
			# print("Switch set to '%s'" % x)
			# # port.writeOscillator(0, 50e6+(10e6*vals[x]))


			# x += 1

			port.noiseDiodePowerCtl(1)
			print("0 atteenuation, connected to powered on diode")
			time.sleep(1)


			raw_input(0)

			port.writeAtten(eore.NOISE_DIODE_ATTEN, 30)
			print("30 atteenuation, connected to powered on diode")

			raw_input(0)

			port.noiseDiodePowerCtl(0)
			print("30 atteenuation, connected to powered off diode")
			time.sleep(1)

			raw_input(0)

			port.writeAtten(eore.NOISE_DIODE_ATTEN, 0)
			print("0 atteenuation, connected to powered off diode")

			raw_input(0)

			port.writeSwitch(0, eore.SW_50R_TERMINATION)
			print("Termination")
			raw_input(0)

		# port.writeAtten(0, 0)
		# time.sleep(1)

		# port.writeAtten(0, 0)
		# time.sleep(1)


		# # x = raw_input("Enter value (0-6): ")
		# x += 0.5
		# if x > 31.5:
		# 	x = 0
		# x = raw_input("Enter value (0-6): ")

		# try:
		# 	x = int(x)
		# 	port.writeSwitch(0, x)
		# except ValueError:
		# 	print("Wat?")
		# 	pass
		# except TypeError:
		# 	print("Wat?")
		# 	pass

		# x += 1
		# if x > 7:
		# 	x = 0

if __name__ == "__main__":
	go()
