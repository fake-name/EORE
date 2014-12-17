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


from common.CooperativeTask import interruptibleSleep

import logging
import signal


NUM_AVERAGE = 1

def printer(printQueue, mpAlive):


	# We're multi-processing. Catch and ignore signals.
	signal.signal(signal.SIGINT, signal.SIG_IGN)

	log = logging.getLogger("Main.Printer")

	while 1:
		if not printQueue.empty():
			print printQueue.get()



		alive = False
		try:
			alive = mpAlive.value
		except Exception:
			pass
		if alive == False:
			print("Stopping Printing-thread!")
			break

		interruptibleSleep(0.001)


	print("Print-thread exiting!")
	printQueue.close()
	printQueue.join_thread()
	print("Print-thread exited!")
