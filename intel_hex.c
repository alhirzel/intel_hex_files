/* Intel .hex file parser
 * Alex Hirzel <alex@hirzel.us>
 * May 27, 2012
 *
 * http://github.com/alhirzel/intel_hex
 */

#include "intel_hex.h"

/* For the simple state machine used in 'slurp_next_intel_hex_record'. */
enum slurp_state {
	SLURP_INIT,
	SLURP_READ_COLON_OR_LINEBREAK,
	SLURP_READ_BYTE_COUNT,
	SLURP_READ_ADDRESS,
	SLURP_READ_RECORD_TYPE,
	SLURP_VERIFY_ESA_ADDRESS_ZERO,
	SLURP_VERIFY_ESA_BYTE_COUNT_TWO,
	SLURP_READ_ESA_DATA,
	SLURP_VERIFY_ESA_DATA_FORMAT_IS_PROPER_ADDRESS,
	SLURP_VERIFY_SSA_ADDRESS_ZERO,
	SLURP_VERIFY_SSA_BYTE_COUNT_FOUR,
	SLURP_VERIFY_ELA_ADDRESS_ZERO,
	SLURP_VERIFY_ELA_BYTE_COUNT_TWO,
	SLURP_VERIFY_SLA_ADDRESS_ZERO,
	SLURP_VERIFY_SLA_BYTE_COUNT_FOUR,
	SLURP_READ_DATA,
	SLURP_READ_CHECKSUM,
	SLURP_VERIFY_CHECKSUM
};

/* Local utility functions. */
static enum intel_hex_slurp_error slurp8bits(char (*)(void), uint8_t *, uint16_t *);
static enum intel_hex_slurp_error slurp16bits(char (*)(void), uint16_t *, uint16_t *);
static enum intel_hex_slurp_error slurp_bytes(int, char (*)(void), uint8_t (*)[], uint16_t *);

enum intel_hex_slurp_error slurp_next_intel_hex_record(char (*slurp_char)(void), struct intel_hex_record *r) {
	enum slurp_state state = SLURP_INIT;
	enum intel_hex_slurp_error err = SLURP_ERROR_NONE;
	uint16_t checksum = 0;
	uint8_t checksum_read;
	char colon;

	while (err == SLURP_ERROR_NONE) {
		switch (state) {
			case SLURP_INIT:
				state = SLURP_READ_COLON_OR_LINEBREAK;
				break;

			case SLURP_READ_COLON_OR_LINEBREAK:
				colon = (*slurp_char)();
				if (':' == colon) {
					state = SLURP_READ_BYTE_COUNT;
				} else if (('\r' == colon) || ('\n' == colon)) {
					state = SLURP_READ_COLON_OR_LINEBREAK;
				} else {
					err = SLURP_ERROR_PARSING;
				}
				break;

			case SLURP_READ_BYTE_COUNT:
				err = slurp8bits(slurp_char, &(r->byte_count), &checksum);
				state = SLURP_READ_ADDRESS;
				break;

			case SLURP_READ_ADDRESS:
				err = slurp16bits(slurp_char, &(r->address), &checksum);
				state = SLURP_READ_RECORD_TYPE;
				break;

			case SLURP_READ_RECORD_TYPE:
				err = slurp8bits(slurp_char, &(r->record_type), &checksum);
				switch (r->record_type) {
					case DATA_RECORD:
					case EOF_RECORD:  state = SLURP_READ_DATA;               break;
					case ESA_RECORD:  state = SLURP_VERIFY_ESA_ADDRESS_ZERO; break;
					case SSA_RECORD:  state = SLURP_VERIFY_SSA_ADDRESS_ZERO; break;
					case ELA_RECORD:  state = SLURP_VERIFY_ELA_ADDRESS_ZERO; break;
					case SLA_RECORD:  state = SLURP_VERIFY_SLA_ADDRESS_ZERO; break;

				}
				break;

			case SLURP_VERIFY_ESA_ADDRESS_ZERO:
				if (0 == r->address) {
					state = SLURP_VERIFY_ESA_BYTE_COUNT_TWO;
				} else {
					err = SLURP_ERROR_ESA_ADDRESS_NOT_ZERO;
				}
				break;

			case SLURP_VERIFY_ESA_BYTE_COUNT_TWO:
				if (2 == r->byte_count) {
					state = SLURP_READ_ESA_DATA;
				} else {
					err = SLURP_ERROR_ESA_BYTE_COUNT_NOT_TWO;
				}
				break;

			case SLURP_READ_ESA_DATA:
				err = slurp_bytes(2, slurp_char, &(r->data), &checksum);
				state = SLURP_VERIFY_ESA_DATA_FORMAT_IS_PROPER_ADDRESS;
				break;

			case SLURP_VERIFY_ESA_DATA_FORMAT_IS_PROPER_ADDRESS:
				if (0 == (r->data[1] & 0x0F)) {
					state = SLURP_READ_CHECKSUM;
				} else {
					err = SLURP_ERROR_ESA_DATA_FORMAT_INVALID;
				}
				break;

			case SLURP_VERIFY_SSA_ADDRESS_ZERO:
				if (0 == r->address) {
					state = SLURP_VERIFY_SSA_BYTE_COUNT_FOUR;
				} else {
					err = SLURP_ERROR_SSA_ADDRESS_NOT_ZERO;
				}
				break;

			case SLURP_VERIFY_SSA_BYTE_COUNT_FOUR:
				if (4 == r->byte_count) {
					state = SLURP_READ_DATA;
				} else {
					err = SLURP_ERROR_SSA_BYTE_COUNT_NOT_FOUR;
				}
				break;

			case SLURP_VERIFY_ELA_ADDRESS_ZERO:
				if (0 == r->address) {
					state = SLURP_VERIFY_ELA_BYTE_COUNT_TWO;
				} else {
					err = SLURP_ERROR_ELA_ADDRESS_NOT_ZERO;
				}
				break;

			case SLURP_VERIFY_ELA_BYTE_COUNT_TWO:
				if (2 == r->byte_count) {
					state = SLURP_READ_DATA;
				} else {
					err = SLURP_ERROR_ELA_BYTE_COUNT_NOT_TWO;
				}
				break;

			case SLURP_VERIFY_SLA_ADDRESS_ZERO:
				if (0 == r->address) {
					state = SLURP_VERIFY_SLA_BYTE_COUNT_FOUR;
				} else {
					err = SLURP_ERROR_SLA_ADDRESS_NOT_ZERO;
				}
				break;

			case SLURP_VERIFY_SLA_BYTE_COUNT_FOUR:
				if (4 == r->byte_count) {
					state = SLURP_READ_DATA;
				} else {
					err = SLURP_ERROR_SLA_BYTE_COUNT_NOT_FOUR;
				}
				break;

			case SLURP_READ_DATA:
				err = slurp_bytes(r->byte_count, slurp_char, &(r->data), &checksum);
				state = SLURP_READ_CHECKSUM;
				break;

			case SLURP_READ_CHECKSUM:
				err = slurp8bits(slurp_char, &checksum_read, &checksum);
				state = SLURP_VERIFY_CHECKSUM;
				break;

			case SLURP_VERIFY_CHECKSUM:
				checksum &= 0xFF;
				if (0 == checksum) {
					err = SLURP_ERROR_DONE;
				} else {
					err = SLURP_ERROR_INVALID_CHECKSUM;
				}
				break;
		}
	}

	if (SLURP_ERROR_DONE == err) {
		err = SLURP_ERROR_NONE;
	}

	return err;
}

static enum intel_hex_slurp_error slurp8bits(char (*slurp_char)(void), uint8_t *dest, uint16_t *checksum) {
	char temp;
	int i;

	*((uint8_t *) dest) = 0;

	for (i = 4; i >= 0; i -= 4) {
		switch ((*slurp_char)()) {
			case '0': temp = 0x0; break;
			case '1': temp = 0x1; break;
			case '2': temp = 0x2; break;
			case '3': temp = 0x3; break;
			case '4': temp = 0x4; break;
			case '5': temp = 0x5; break;
			case '6': temp = 0x6; break;
			case '7': temp = 0x7; break;
			case '8': temp = 0x8; break;
			case '9': temp = 0x9; break;
			case 'A': temp = 0xA; break;
			case 'a': temp = 0xA; break;
			case 'B': temp = 0xB; break;
			case 'b': temp = 0xB; break;
			case 'C': temp = 0xC; break;
			case 'c': temp = 0xC; break;
			case 'D': temp = 0xD; break;
			case 'd': temp = 0xD; break;
			case 'E': temp = 0xE; break;
			case 'e': temp = 0xE; break;
			case 'F': temp = 0xF; break;
			case 'f': temp = 0xF; break;
			default:              return SLURP_ERROR_NON_HEX_CHARACTER;
		}

		*((uint8_t *) dest) |= temp << i;
	}
	*checksum += *dest;

	return SLURP_ERROR_NONE;
}

static enum intel_hex_slurp_error slurp16bits(char (*slurp_char)(void), uint16_t *dest, uint16_t *checksum) {
	uint8_t b1, b2;
	enum intel_hex_slurp_error err;

	err = slurp8bits(slurp_char, &b1, checksum);
	if (SLURP_ERROR_NONE == err) {
		err = slurp8bits(slurp_char, &b2, checksum);
		if (SLURP_ERROR_NONE == err) {
			*dest = b1;
			*dest <<= 8;
			*dest |= b2;
		}
	}

	return SLURP_ERROR_NONE;
}

static enum intel_hex_slurp_error slurp_bytes(int nbytes, char (*slurp_char)(void), uint8_t (*dest)[], uint16_t *checksum) {
	enum intel_hex_slurp_error err = SLURP_ERROR_NONE;
	int i;

	for (i = 0; i < nbytes; i++) {
		err = slurp8bits(slurp_char, &((*dest)[i]), checksum);
		if (SLURP_ERROR_NONE != err) {
			break;
		}
	}

	return err;
}

