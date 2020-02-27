# Copyright (c) 2017 MIT License by 6.172 Staff

CC := clang
CFLAGS := -O3
CILKFLAGS := -fcilkplus

LDFLAGS := -flto
TARGET := fib_serial fib_cilk
LDFLAGS_TOOL := -flto
all: null-tool.o func-analysis-tool.o linreg-tool.o func-analysis.o linreg.o fib_serial fib_cilk

null-tool.o: null-tool.c
	$(CC) -o null-tool.o $(LDFLAGS_TOOL) -c null-tool.c $(CFLAGS)

func-analysis-tool.o : null-tool.o func-analysis.c
	$(CC) -o func-analysis-tool.o $(LDFLAGS_TOOL) -c func-analysis.c $(CFLAGS)

linreg-tool.o : null-tool.o linreg.c
	$(CC) -o linreg-tool.o $(LDFLAGS_TOOL) -c linreg.c $(CFLAGS)

func-analysis.o: func-analysis-tool.o
linreg.o: linreg-tool.o

func-analysis.o linreg.o: null-tool.o
	llvm-link -o $@ $(patsubst %.o, %-tool.o, $@) null-tool.o

CSI_FLAG := 
ifeq ($(CSI),1)
	CSI_FLAG += -fcsi
  LDFLAGS += -fuse-ld=gold -lclang_rt.csi-x86_64
endif

fib_serial.o: fib_serial.c
	$(CC) -o fib_serial.o $(LDFLAGS_TOOL) $(CSI_FLAG) -c fib_serial.c $(CFLAGS) -g

fib_cilk.o: fib_cilk.c
	$(CC) -o fib_cilk.o $(LDFLAGS_TOOL) $(CSI_FLAG) $(CILKFLAGS) -c fib_cilk.c $(CFLAGS) -g

TOOL :=
ifeq ($(FUNCANALYSIS),1)
	TOOL += func-analysis.o
endif
ifeq ($(LINREG),1)
	TOOL += linreg.o
endif

fib_serial: fib_serial.o $(TOOL)
	$(CC) -o fib_serial fib_serial.o $(TOOL) $(CFLAGS) $(LDFLAGS)

fib_cilk: fib_cilk.o $(TOOL)
	$(CC) -o fib_cilk fib_cilk.o $(TOOL) $(CFLAGS) $(CILKFLAGS) $(LDFLAGS)

clean::
	rm -f fib_serial fib_cilk .buildmode *.o *.bc *.a
