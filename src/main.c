#include "perft.h"
#include "uci.h"

#include <stdlib.h>

#define PERFT 0

int main(void) {
#if PERFT
	perft_run();
#else
	uci_run("Code Artists", "sramon edribeir mvan-ess");
#endif

	return EXIT_SUCCESS;
}
