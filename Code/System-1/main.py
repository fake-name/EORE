# -*- coding: UTF-8 -*-


# pylint: disable=R0913, R0912, W0603

import multiprocessing as mp
import logSetup
import logging


def go():


	printQueue = mp.Queue()

	logSetup.initLogging(printQ = printQueue)
	log = logging.getLogger("Main.Main")

	ctrlManager = mp.Manager()
	ctrlNs = ctrlManager.Namespace()
	ctrlNs.run = True
	ctrlNs.acqRunning = True
	ctrlNs.apiRunning = True
	ctrlNs.logRunning = True
	ctrlNs.stopped = False



	ctrlManager.shutdown()
	sys.exit()


if __name__ == "__main__":

	go()