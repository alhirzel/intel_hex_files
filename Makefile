example: example.c intel_hex.c
	${CC} ${CFLAGS} $^ -o $@

test: example example.hex
	cat example.hex | ./example
