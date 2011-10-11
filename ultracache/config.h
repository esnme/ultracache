#pragma once

#define CONFIG_KEY_LENGTH 0xff
#define CONFIG_VALUE_LENGTH 0xfffff

#define CONFIG_ALIGNMENT_BYTES 8
#define CONFIG_ALIGNMENT_SHIFT 3
#define CONFIG_COMPRESSPTR_BITS 32

#define CONFIG_MAX_KEY_LENGTH 256
#define CONFIG_UINT64_STRING_LENGTH 20  /* "18446744073709551615" */

#define CONFIG_PACKET_SIZE 1472
#define CONFIG_MAX_REQUEST_SIZE (CONFIG_VALUE_LENGTH + 512)

#define CONFIG_EXPTIME_OFFSET (60*60*24*30)

#define CONFIG_VERSION_STRING "UltraCache"