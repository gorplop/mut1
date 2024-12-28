PREFIX=aarch64-linux-gnu
AS=$(PREFIX)-as
CC=$(PREFIX)-gcc
LD=$(PREFIX)-ld

BUILD=./build

STUFFER=./nopstuffer/nopstuffer.py
MUTATOR=./mutate1/mut1.elf

#all: 00_hello.elf 01_hello.elf 03_hello_nops.elf 04_ctest.elf
all: test_dir 04_ctest_stuffed.elf 04_ctest_stuffed.elf.mut 04_ctest.elf

test_dir:
	@mkdir -p $(BUILD)

%_stuffed.elf: %.c
	$(CC) $< -nostartfiles -Os -S -o $(BUILD)/$(<:.c=.s)
#	Run patcher
	$(STUFFER) $(BUILD)/$(<:.c=.s) $(BUILD)/$(<:.c=_stuffed.s)
	$(AS) $(BUILD)/$(<:.c=_stuffed.s) -o $(BUILD)/$(<:.c=_stuffed.d)
	$(LD) $(BUILD)/$(<:.c=_stuffed.d) -lc -o $@
	chmod +x $@

%.elf.mut: %.elf
	@echo "mutating $<"
	$(MUTATOR) $< # this version appends .mut itself

%.elf: %.c
	$(CC) -nostartfiles -lc $< -o $@

stuffed_clean:
	rm -f *_stuffed.elf
	rm -f $(BUILD)/*
mut_clean:
	rm -f *.elf.mut

elf_clean:
	rm -f *.elf


00_hello.elf: 00_hello.s
	$(AS) $< -o 00_hello.d
	$(LD) 00_hello.d -o $@

00_hello_stuffed.elf: 00_hello.s
	$(STUFFER) $< $(BUILD)/$(<:.s=_stuffed.s)
	$(AS) $(BUILD)/$(<:.s=_stuffed.s) -o $(BUILD)/$(<:.s=_stuffed.d)
	$(LD) $(BUILD)/$(<:.s=_stuffed.d) -lc -o $@

hello_clean:
	rm -f 00_hello.d 00_hello.elf 01_hello.elf 03_hello_nops.elf 04_ctest.elf

01_hello.elf: 01_hello.c
	$(CC) $< -nostartfiles -Os -S -o $(<:.c=.s)
#	Run patcher
	$(STUFFER) $(<:.c=.s) $(<:.c=_stuffed.s)
	$(AS) $(<:.c=_stuffed.s) -o $(<:.c=_stuffed.d)
	$(LD) -lc $(<:.c=_stuffed.d) -o $@


03_hello_nops.elf: 03_hello_nops.s
	$(AS) $< -o 03_hello_nops.d
	$(LD) 03_hello_nops.d -o $@

# Currently the same as 01_hello.c
04_ctest.elf: 04_ctest.c
	$(CC) -nostartfiles -lc $< -o $@

05_proper_c.elf: 05_proper_c.c
	$(CC) $< -o $@

.PHONY: clean

#clean: hello_clean

clean: mut_clean stuffed_clean elf_clean
