// aarch64-linux-gnu-as 00_hello.s
	.text
.global _start
_start:
	// syscall 64 = write(unsigned int fd, const char *buf, size_t count)
	mov x0, #1
	ldr x1, =hello

	ldr x2, =hello_len
	movz x8, #64

	svc #0
	// syscall 93 = exit(int error_code)
	mov x0, #0
	movz x1, #1
	mov x8, #93

	svc #0
	nop     //even number of insns

	.data
hello:
	.ascii "Hello, aarch64 world!\n"
hello_len =  .-hello

