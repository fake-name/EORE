# -*- coding: UTF-8 -*-

# Wrapper for Test-Equipment-Plus's "SignalHound" series of USB spectrum analysers.
#
# Written By Connor Wolf <wolf@imaginaryindustries.com>
#

#  * ----------------------------------------------------------------------------
#  * "THE BEER-WARE LICENSE":
#  * Connor Wolf <wolf@imaginaryindustries.com> wrote this file. As long as you retain
#  * this notice you can do whatever you want with this stuff. If we meet some day,
#  * and you think this stuff is worth it, you can buy me a beer in return.
#  * (Only I don't drink, so a soda will do). Connor
#  * Also, support the Signal-Hound devs. Their hardware is pretty damn awesome.
#  * ----------------------------------------------------------------------------
#

# pylint: disable=R0913, R0912, W0603

import multiprocessing as mp
import common.logSetup

import logging
import SpectraLogger.spectraAcqThread as spectraAcqThread
import SpectraLogger.internalSweepSpectraAcqThread as internalSweepSpectraAcqThread
import SpectraLogger.spectraLogThread as spectraLogThread

import SpectraLogger.spectraPlotApiThread as spectraPlotApiThread

import SpectraLogger.settings as settings

import common.CooperativeTask


def go(printQueue, plzhalt):

	plotQueue = mp.Queue()
	dataQueue = mp.Queue()
	printQueue = printQueue
	ctrlManager = mp.Manager()

	common.logSetup.initLogging(printQ = printQueue)
	log = logging.getLogger("Main.Main")

	ctrlNs = ctrlManager.Namespace()
	ctrlNs.run = True
	ctrlNs.acqRunning = True
	ctrlNs.apiRunning = True
	ctrlNs.logRunning = True
	ctrlNs.stopped = False

	if settings.ACQ_TYPE == "real-time-sweeping":
		print("Importing real-time-sweeping module!")
		acqProc = mp.Process(target=internalSweepSpectraAcqThread.sweepSource, name="AcqThread", args=((dataQueue, plotQueue), ctrlNs, printQueue))
	else:
		print("Importing real-time module!")
		acqProc = mp.Process(target=spectraAcqThread.sweepSource, name="AcqThread", args=((dataQueue, plotQueue), ctrlNs, printQueue))

	acqProc.start()


	logProc = mp.Process(target=spectraLogThread.logSweeps, name="LogThread", args=(dataQueue, ctrlNs, printQueue))
	logProc.start()

	plotProc = mp.Process(target=spectraPlotApiThread.startApiServer, name="PlotApiThread", args=(plotQueue, ctrlNs, printQueue))
	plotProc.start()


	while 1:
		try:
			if plzhalt.value == 0:
				print("Breaking!")
				break
		except Exception:
			break

		common.CooperativeTask.interruptibleSleep(1)



	log.info("Stopping Processes!")


	ctrlNs.run = False

	# You have to empty the queue for everything to exit properly
	log.info("Emptying Queue")

	# Sometimes the last few queue items take a little while to trickle in.
	# therefore, we both poll the queue for items, and try to join() the thread. That way
	# as soon as the queue is *actually* empty, we exit immediately
	# - - -
	# this was a fucking nightmare to track down.

	log.info("Joining on AcqProc")
	while acqProc.is_alive():
		acqProc.join(0.1)


		# print("acqProc.is_alive()", acqProc.is_alive(), "logProc.is_alive()", logProc.is_alive(), "plotProc.is_alive()", plotProc.is_alive())
	log.info("Joining on LogProc")
	while logProc.is_alive():
		logProc.join(0.1)
		# print("acqProc.is_alive()", acqProc.is_alive(), "logProc.is_alive()", logProc.is_alive(), "plotProc.is_alive()", plotProc.is_alive())

	log.info("Joining on PlotProc")
	while plotProc.is_alive():
		plotProc.join(0.1)
		# print("acqProc.is_alive()", acqProc.is_alive(), "logProc.is_alive()", logProc.is_alive(), "plotProc.is_alive()", plotProc.is_alive())




	print("Threads stopped.")
	print("Stopping Shared Memory Manager.")
	ctrlManager.shutdown()

	print("Shutdown complete. Exiting.")


class AcquireData(common.CooperativeTask.TimedJob):

	runDuration = 30
	haltTimeout = 30


	# self.printQ = printQ
	# self.running = shouldRun

	def go(self):

		go(self.printQ, self.plzhalt)

class AcquisitionScheduler(common.CooperativeTask.BaseScheduler):

	jobs = [AcquireData, common.CooperativeTask.PrintHai]
	jobIndex = 0

	def getNextJob(self):
		while 1:
			yield self.jobs[self.jobIndex]
			self.jobIndex += 1
			self.jobIndex = self.jobIndex % len(self.jobs)

