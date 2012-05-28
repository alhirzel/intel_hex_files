/* Intel .hex file parser
 * Alex Hirzel <alex@hirzel.us>
 * May 27, 2012
 *
 * http://github.com/alhirzel/intel_hex
 */

#ifndef _INTEL_HEX_H_
#define _INTEL_HEX_H_

#include <stdint.h>

/* Different possible records for Intel .hex files (from Wikipedia). */
enum intel_hex_record_type {
	DATA_RECORD = 0x00,
	EOF_RECORD  = 0x01, /* End Of File */
	ESA_RECORD  = 0x02, /* Extended Segment Address */
	SSA_RECORD  = 0x03, /* Start Segment Address */
	ELA_RECORD  = 0x04, /* Extended Linear Address */
	SLA_RECORD  = 0x05  /* Start Linear Address */
};

struct intel_hex_record {
	uint8_t byte_count;
	uint16_t address;
	uint8_t record_type;
	uint8_t data[UINT8_MAX + 1];
	uint8_t checksum;
};

/* For the purposes of this implementation, an "error" is an exception to state
 * machine execution (including healthy, normal termination). */
enum intel_hex_slurp_error {
	SLURP_ERROR_NONE,
	SLURP_ERROR_DONE,
	SLURP_ERROR_PARSING,
	SLURP_ERROR_NON_HEX_CHARACTER,
	SLURP_ERROR_INVALID_CHECKSUM,
	SLURP_ERROR_ESA_ADDRESS_NOT_ZERO,
	SLURP_ERROR_ESA_BYTE_COUNT_NOT_TWO,
	SLURP_ERROR_ESA_DATA_FORMAT_INVALID,
	SLURP_ERROR_SSA_ADDRESS_NOT_ZERO,
	SLURP_ERROR_SSA_BYTE_COUNT_NOT_FOUR,
	SLURP_ERROR_ELA_ADDRESS_NOT_ZERO,
	SLURP_ERROR_ELA_BYTE_COUNT_NOT_TWO,
	SLURP_ERROR_SLA_ADDRESS_NOT_ZERO,
	SLURP_ERROR_SLA_BYTE_COUNT_NOT_FOUR
};

/* Returns valid, populated 'intel_hex_record' structure. Fails on any error
 * including invalid checksum. */
enum intel_hex_slurp_error slurp_next_intel_hex_record(char (*)(void), struct intel_hex_record *);

#endif /* _INTEL_HEX_H_ */

