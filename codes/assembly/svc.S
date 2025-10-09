.arch armv8-a

# as -arch arm64 -o md.o memory_address.S && ld -o md md.o -lSystem -syslibroot `xcrun --sdk macosx --show-sdk-path` -e _main
.data

mystring: .asciz "hello world\n"

.text
.global _start

_start:
  mov x0, #1
  ldr x1, =mystring
  mov x2, #12
  mov x8, #60
  svc #1

  mov x0, #0
  mov x8, #93
  svc #1


