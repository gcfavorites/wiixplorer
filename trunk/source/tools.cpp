#include <time.h>
#include <math.h>

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

int ROUND2FOUR(double x)
{
    double tmp = 0.0;
    double rest = modf((double) x/4.0f, &tmp);
    int res = 0;

    if(rest >= 0.5)
    {
        res = ((int) x/4) +1;
    }
    else
    {
        res = (int) x/4;
    }


    return res*4;
}
