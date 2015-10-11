
.PHONEY: libsctp-dev

simexec: simexec.c
	gcc simexec.c -o simexec -lsctp -lpthread

simexecd: simexecd.c
	or1k-linux-gnu-gcc simexecd.c -o simexecd -lsctp -static

libsctp-dev:
	(cd lksctp-tools; ./bootstrap; ./configure --host=or1k-linux-gnu \
		--build=x86_64-linux-gnu && make && \
		make DESTDIR=/srv/compilers/openrisc-devel/or1k-linux-gnu/sys-root/ install)



