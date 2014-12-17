# -*- coding: UTF-8 -*-


# Pylint cannot determine contents of multiprocessing.Manager, so turn off the warning.
# pylint: disable=E1101

import multiprocessing as mp
import common.logSetup
import common.printThread

import common.CooperativeTask
import signal
import sys

import jobs


def go():


	signal.signal(signal.SIGINT, signal.SIG_IGN)

	printQueue = mp.Queue()
	common.logSetup.initLogging(printQ = printQueue)
	# log = logging.getLogger("Main.Main")

	ctrlManager = mp.Manager()
	ctrlNs = ctrlManager.Namespace()

	alive = mp.Value('i')
	alive.value = 1

	printAlive = mp.Value('i')
	printAlive.value = 1

	# A separate process for printing, which allows nice easy non-blocking printing.
	printProc = mp.Process(target=common.printThread.printer, name="PrintArbiter", args=(printQueue, printAlive))

	printProc.start()


	def signal_handler(dummy_signal, dummy_frame):
		printQueue.put("Halting")
		alive.value = 0

	signal.signal(signal.SIGINT, signal_handler)


	sched = jobs.AcquisitionScheduler(printQueue, alive)
	sched.go()

	printAlive.value = 0
	printProc.join()

	print("Halting manager")
	ctrlManager.shutdown()
	sys.exit()


if __name__ == "__main__":

	go()