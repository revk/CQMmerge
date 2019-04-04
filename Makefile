all: git cqmmerge

AXL/axl.o: AXL/axl.c
	make -C AXL

cqmmerge: cqmmerge.c AXL/axl.o
	cc -O -o $@ $< ${OPTS} -lpopt ${LIBMQTT} -IAXL AXL/axl.o -lcurl

git:
	git submodule update --init

update:
	git submodule update --remote --merge
