/* Some IDEs complain of the filename clash between mbedTLS and the Azure SDK: The azure file is renamed. */
#if USE_MBED_TLS
#include "../../c-utility/src/sha1.c"
#endif /* USE_MBED_TLS */

