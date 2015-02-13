
import time
import sys
import eore

def go():
	print("Starting")
	port = eore.EoreController("COM41")
	time.sleep(1)

	x = 0
	port.writeAtten(eore.MAIN_TONE_ATTEN,       0)
	port.writeAtten(eore.AUX_TONE_ATTEN,        0)
	port.writeAtten(eore.NOISE_DIODE_ATTEN,     0)
	port.writeAtten(eore.SWITCH_SWR_TONE_ATTEN, 0)
	port.writeAtten(eore.SWITCH_TONE_ATTEN,     0)
	port.writeAtten(eore.MID_AMP_ATTEN,         0)



	################################################################################################
	# center = 101e6
	# width = 20e6

	# stepBase = 2.465e3

	# start = center - (width/2)
	# stop  = center + (width/2)
	# freq = start
	# while 1:
	# 	if freq > stop:
	# 		freq = start

	# 	port.writeOscillator(0, freq)
	# 	freq += stepBase*10

	# while 1:
	# 	port.chirpVco(5).strip()
	# 	print("Sleeping 5 while on")
	# 	time.sleep(5)
	# 	port.chirpVco(0).strip()
	# 	print("Sleeping 5 while off")
	# 	time.sleep(5)


	# # port.writeAtten(2, 31.5)
	print port.noiseDiodePowerCtl(0)
	print port.disableOscillator()
	print port.powerDownVco()
	print port.getTemperature()
	print port.getTemperatureSetpoint()
	print port.setTemperature(40)
	# print port.chirpVco(1)

if __name__ == "__main__":
	go()
