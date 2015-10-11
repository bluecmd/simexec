== simexec

Used to seamlessly move executions into a simulator.

When simexec is registered with binfmt_misc it will send the execution
environment to a remote host (defined compile time).

simexec requires SCTP and IPv6 support.

* Install simexec to /usr/local/bin on your host system.
* Execute ./binfmt.sh to register the executor.
* Install simexecd into your simulator and make it start at boot.

Note that the filesystem needs to mirrored by other means, simexec
only executes binaries.

