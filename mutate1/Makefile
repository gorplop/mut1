#PREFIX=aarch64-linux-gnu-
PREFIX=
AS=$(PREFIX)as
CC=$(PREFIX)gcc
LD=$(PREFIX)ld
CFLAGS += -g -DWITH_DEBUG -Wunused

all: mut1.elf

mut1.elf: mut1.c unilist.c main.c
	$(CC) $(CFLAGS) $^ -o $@

mut1.elf_clean:
	rm -f mut1.elf

.PHONY: clean

clean: mut1.elf_clean
