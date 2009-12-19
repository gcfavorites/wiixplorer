#include <time.h>

bool TimePassed(int limit)
{
	time_t timer1 = 0;
	static time_t timer2;

	if (timer2 == 0) {
			time(&timer1);
		    timer2 = timer1;
    }

    if(timer1 != 0)	// timer läuft
        time(&timer1);

    if(difftime(timer1, timer2) >= limit) {
        timer1 = 0;
        return true;
    }

    return false;
}
