EORE
====

Era of Reionization Experiment

This is the hardware designs for the RF and backend components of a radio-telescope intended to look at the [21 CM hydrogen line](http://en.wikipedia.org/wiki/Reionization#21-cm_line).  

Current topology is to use a SignalHound [BB60C](https://www.signalhound.com/products/bb60c/) spectrum analyser for the backend receiver (though my [API wrapper](https://github.com/fake-name/pySignalHound)), while the boards in this repository are front-end amplifiers and various components for tracking local oscillator drift, antenna characterization and so forth.
