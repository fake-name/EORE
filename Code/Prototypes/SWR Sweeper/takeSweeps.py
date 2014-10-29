
import time

from controlApi import EoreController
import SignalHound as sh
import logSetup



ACQ_SPAN               = 27e6
ACQ_REF_LEVEL_DB       = 20
ACQ_ATTENUATION_DB     = 30
ACQ_GAIN_SETTING       = 0
ACQ_RBW                = 2.465e3
ACQ_VBW                = ACQ_RBW
ACQ_SWEEP_TIME_SECONDS = 0.0100
ACQ_WINDOW_TYPE        = "hamming"
ACQ_UNITS              = "power"
ACQ_TYPE               = "sweeping"
ACQ_MODE               = "average"
ACQ_Y_SCALE            = "log-scale"



def setupSignalhound(hound, freq=100e6):
	hound.configureAcquisition(ACQ_MODE, ACQ_Y_SCALE)
	hound.configureCenterSpan(center = freq+5e6, span = ACQ_SPAN)
	hound.configureLevel(ref = ACQ_REF_LEVEL_DB, atten = ACQ_ATTENUATION_DB)
	hound.configureGain(gain = ACQ_GAIN_SETTING)
	hound.configureSweepCoupling(rbw = ACQ_RBW, vbw = ACQ_VBW, sweepTime = ACQ_SWEEP_TIME_SECONDS, rbwType = "native", rejection = "no-spur-reject")
	hound.configureWindow(window = ACQ_WINDOW_TYPE)
	hound.configureProcUnits(units = ACQ_UNITS)
	hound.configureTrigger(trigType = "none", edge = "rising-edge", level = 0, timeout = 5)
	# hound.configureIO("dc", "int-ref-out", "out-logic-low")
	# hound.configureDemod("fm", 102.3e6, 250e3, 12e3, 20, 50)

	# hound.configureRawSweep(100, 8, 2)
	hound.initiate(mode = ACQ_TYPE, flag = "ignored")
	print(hound.getCurrentAcquisitionSettings())

def setAcqFreq(hound, freq):
	hound.configureCenterSpan(center = freq+5e6, span = ACQ_SPAN)
	hound.initiate(mode = ACQ_TYPE, flag = "ignored")


def frange(x, y, jump):
	while x <= y:
		yield x
		x += jump

def go():
	print("Starting")
	# logSetup.initLogging()
	print("Loggers initialized")
	port = EoreController("COM4")
	print("Port opened")
	hound = sh.SignalHound()
	print("SignalHound connected")
	setupSignalhound(hound)
	print("SignalHound configured")


	x = 0
	port.writeAtten(0, 0)
	port.writeAtten(1, 0)
	port.writeAtten(2, 0)

	vals = [0,1,2,3,4,5]

	port.writeSwitch(0, 0)

	port.writeAtten(2, 31.5)

	while 1:

		for x in frange(100e6, 200e6, 1e6):
			setAcqFreq(hound, x)

			ret = port.writeOscillator(0, x)
			if not ret:
				print("Wat?")
			time.sleep(0.1)
			trace = hound.fetchTrace()
			print(x, trace['max'].max())



if __name__ == "__main__":
	go()
