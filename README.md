WebConsole is a simple light weight webserver for low power device.

This version works fine and allow to access file in the same derectory as the exceutable. And SSL support is almost complet.

This version use a thread per client (created at the connection) which is inefficient, so I start working on a ThreadPool and moved to an event based achitecture.
