#ifndef _NTRU_H
#define _NTRU_H

#include "hls_vector.h"
#include "hls_stream.h"
#include "ap_int.h"

typedef unsigned short GENERAL;
const int POLY_SIZE = 512;
const int BUSWIDTH = 32;

void NTRU(hls::vector<GENERAL, BUSWIDTH> *h,  hls::vector<GENERAL, BUSWIDTH> *r, hls::vector<GENERAL, BUSWIDTH>  *m, hls::vector<GENERAL, BUSWIDTH>  *c, int size);
#endif
