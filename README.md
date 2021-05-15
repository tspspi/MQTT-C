## Fork of MQTT-C

This is just a local fork of [MQTT-C](https://github.com/LiamBindle/MQTT-C) that I'm adjusting to
my own needs to deploy a simple MQTT publisher to be used in an automation project.

Modifications I've made:

* Added header files to sources that are required
* Created a small utility examples/mqttpublish.c that allows me to publish some specific events
  to an unencrypted but authenticated MQTT broker (running on localhost thus not using SSL)
* Stripped tests and most utilities from the Makefile

Maybe I'll clean up a little bit later - this has been done to quickly solve a specific problem.
