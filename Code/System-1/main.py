# -*- coding: UTF-8 -*-


# Pylint cannot determine contents of multiprocessing.Manager, so turn off the warning.
# pylint: disable=E1101

import multiprocessing as mp
import logSetup
import logging

import time
import CooperativeTask
import signal
import sys


def go():




	signal.signal(signal.SIGINT, signal.SIG_IGN)

	printQueue = mp.Queue()
	# logSetup.initLogging(printQ = printQueue)
	# log = logging.getLogger("Main.Main")

	ctrlManager = mp.Manager()
	ctrlNs = ctrlManager.Namespace()
	ctrlNs.alive = True


	def signal_handler(dummy_signal, dummy_frame):
		print("Halting")
		ctrlNs.alive = False

	signal.signal(signal.SIGINT, signal_handler)

	CooperativeTask.runScheduler(printQueue, ctrlNs)

	print("Halting manager")
	ctrlManager.shutdown()
	sys.exit()


if __name__ == "__main__":

	go()