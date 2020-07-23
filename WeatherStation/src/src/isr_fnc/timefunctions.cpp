#include "timefunctions.h"
/* Code fore reading milliseconds in isr */
// Code adapted for 64 bits from https://www.hackersdelight.org/divcMore.pdf
int64_t IRAM_ATTR divs10(int64_t n) {
	int64_t q, r;
	n = n + (n >> 63 & 9);
	q = (n >> 1) + (n >> 2);
	q = q + (q >> 4);
	q = q + (q >> 8);
	q = q + (q >> 16);
	q = q + (q >> 32);
	q = q >> 3;
	r = n - q * 10;
	return q + ((r + 6) >> 4);
// return q + (r > 9);
}

int64_t IRAM_ATTR divs1000(int64_t n) {
	return divs10(divs10(divs10(n)));
}

unsigned long IRAM_ATTR isr_millis()
{
	return divs1000(esp_timer_get_time());
}
