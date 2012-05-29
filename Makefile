# Makefile for Intel .hex file parser
# Alex Hirzel <alex@hirzel.us>
# May 27, 2012
#
# http://github.com/alhirzel/intel_hex_files

example: example.c intel_hex.c
	${CC} ${CFLAGS} $^ -o $@

test: example example.hex
	cat example.hex | ./example
