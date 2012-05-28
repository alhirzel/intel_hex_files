/* Intel .hex file parser (exmaple program)
 * Alex Hirzel <alex@hirzel.us>
 * May 27, 2012
 *
 * http://github.com/alhirzel/intel_hex
 */

#include <stdio.h>
#include <stdlib.h>
#include "intel_hex.h"

struct intel_hex_record r;

char my_slurp_char(void) {
	return getc(stdin);
}

int main(int argc, char *argv[]) {
	enum intel_hex_slurp_error err;

	do {
		err = slurp_next_intel_hex_record(&my_slurp_char, &r);

		if (SLURP_ERROR_NONE != err) {
			printf("Got error 0x%02X, aborting due to invalid record!", err);
			exit(1);
		}

		switch (r.record_type) {
			case DATA_RECORD:
				printf("Got data record with length %u\n", r.byte_count);
				break;
			case EOF_RECORD:
				puts("Got EOF record");
				break;
			case ESA_RECORD:
				puts("Got ESA record");
				break;
			case SSA_RECORD:
				puts("Got SSA record");
				break;
			case ELA_RECORD:
				puts("Got ELA record");
				break;
			case SLA_RECORD:
				puts("Got SLA record");
				break;
		}
	} while (r.record_type != EOF_RECORD);

	return EXIT_SUCCESS;
}

