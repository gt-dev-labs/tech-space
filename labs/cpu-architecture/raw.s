.global _start

.section .rodata
msg:
	.ascii "Hello from assembly!\n"
	len = . - msg

.section .text
_start:
	# TODO: write(1, msg, len) — the write() syscall, number 1
	#   rax = 1                (syscall number)
	#   rdi = 1                (fd — stdout)
	#   rsi = address of msg   (hint: lea msg(%rip), %rsi)
	#   rdx = len
	#   then: syscall

	# TODO: exit(0) — the exit() syscall, number 60
	#   rax = 60               (syscall number)
	#   rdi = 0                (exit status)
	#   then: syscall
