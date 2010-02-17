#ifndef _GECKO_H_
#define _GECKO_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GEKKO
    bool InitGecko();
    void gprintf(const char * format, ...);
#else
	#define gprintf(...)
	#define InitGecko()      false
#endif /* GEKKO */

#ifdef __cplusplus
}
#endif

#endif /* _GECKO_H_ */

