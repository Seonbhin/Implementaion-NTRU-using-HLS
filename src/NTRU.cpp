#include "NTRU.h"


extern "C" {

void read_h(hls::vector<GENERAL, BUSWIDTH> *h,  hls::stream<hls::vector<GENERAL, BUSWIDTH> > & hStream, int vsize)
{
   printf("\n\nh[i]\n");
   for (int i = 0; i < 32; i++) {
//#pragma HLS pipeline II=1
      hStream << h[i];
   }
}

void read_r(hls::vector<GENERAL, BUSWIDTH> *r,  hls::stream<hls::vector<GENERAL, BUSWIDTH> > & rStream, int vsize)
{
   for (int i = 0; i < 32; i++) {
//#pragma HLS pipeline II=1
      rStream << r[i];
   }
}

void read_m(hls::vector<GENERAL, BUSWIDTH> *m,  hls::stream<hls::vector<GENERAL, BUSWIDTH> > & mStream, int vsize)
{
   for (int i = 0; i < 32; i++) {
//#pragma HLS pipeline II=1
      mStream << m[i];
   }
}


void enc(hls::stream<hls::vector<GENERAL, BUSWIDTH> > & hStream, hls::stream<hls::vector<GENERAL, BUSWIDTH> > & rStream, hls::stream<hls::vector<GENERAL, BUSWIDTH> > & mStream, GENERAL *c,   int vsize )
{
   ap_uint<11> rh_block[POLY_SIZE]; // if we define type of rh_block to ap_uint<11>, initialization is wrong.
   ap_uint<11> h_block[POLY_SIZE];
   ap_uint<2> r_block[POLY_SIZE];
   ap_uint<2> m_block[POLY_SIZE];

   hls::vector<GENERAL, BUSWIDTH> hj;
   hls::vector<GENERAL, BUSWIDTH> rj;
   hls::vector<GENERAL, BUSWIDTH> mj;
   hls::vector<ap_uint<11>, BUSWIDTH> rh_temp;
   for (int J = 0; J < 16; J++) {
#pragma HLS pipeline II=17
   hj = hStream.read();
   rj = rStream.read();
   mj = mStream.read();
         for(int k = 0; k < BUSWIDTH; k++) {
//#pragma HLS UNROLL
            rh_block[J*BUSWIDTH + k] = 0;
            h_block[J*BUSWIDTH + k] = hj[k];
            r_block[J*BUSWIDTH + k] = rj[k];
            m_block[J*BUSWIDTH + k] = mj[k];
        }
   }
/*
   for(int k = 0; k < POLY_SIZE-3; k++) {
#pragma HLS pipeline II=1
      for (int i=1; i<POLY_SIZE-3 -k; i++) {
#pragma HLS UNROLL
         rh_block[k] += r_block[k+i] * h_block[POLY_SIZE-3 - i];
      }
      for (int i=0; i<k+1; i++)
#pragma HLS UNROLL
         rh_block[k] += r_block[k-i] * h_block[i];
   }
*/


   for(int j = 0; j < POLY_SIZE-3; j++) {
//#pragma HLS pipeline II=509
		i_loop: for(int i = 0; i < POLY_SIZE-3; i++) {
#pragma HLS pipeline II=2
			if (r_block[(509 -i) % 509] == 2) {
				rh_block[j] -= h_block[(j+i) % 509];
			}
			else if (r_block[(509 -i) % 509] == 0) {
				rh_block[j] += 0;
			}
			else if (r_block[(509 -i) % 509] == 1){
				rh_block[j] += h_block[(j+i) % 509];
			} // NTRU r doesn't have coefficient -1.
		}
	}

   j_loop: for(int j = 0; j < 16; j++) {
	   for (int i = 0; i < 32; i++) {
		   c[16*i+j] = rh_block[16*i+j] + m_block[16*i+j];
	   }

   }
   while(!hStream.empty()) hj = hStream.read();
   while(!mStream.empty()) mj = mStream.read();
   while(!rStream.empty()) rj = rStream.read();
}



void NTRU(hls::vector<GENERAL, BUSWIDTH> *h,  hls::vector<GENERAL, BUSWIDTH> *r, hls::vector<GENERAL, BUSWIDTH>  *m, GENERAL *c, int size)
{
#pragma HLS INTERFACE mode=m_axi bundle=m0 port=h
#pragma HLS INTERFACE mode=m_axi bundle=m1 port=r
#pragma HLS INTERFACE mode=m_axi bundle=m2 port=m
#pragma HLS INTERFACE mode=m_axi bundle=m3 port=c

#pragma HLS DATAFLOW

   int vsize = size;

   hls::stream<hls::vector<GENERAL, BUSWIDTH> > hStream("hStream");
   hls::stream<hls::vector<GENERAL, BUSWIDTH> > rStream("rStream");
   hls::stream<hls::vector<GENERAL, BUSWIDTH> > mStream("mStream");
   read_h(h, hStream, vsize);
   read_r(r, rStream, vsize);
   read_m(m, mStream, vsize);

   enc(hStream, rStream, mStream, c, vsize);
}


}
