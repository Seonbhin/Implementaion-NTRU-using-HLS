/***********************************************코드********************************************/

1. src 폴더에는 host.cpp, NTRU.cpp와 NTRU.h 총 3개의 코드가 있습니다.
2. Run 폴더에는 Makefile, input_r.dat, input_h.dat, input_r.dat 총 4개의 코드가 있습니다.
3. Run 폴더에는 output_gold.dat이 있습니다. 이는 결과확인을 위한 추가 파일이며 실행에는 필요하지 않습니다.
4. Vitis HLS를 이용한 C synthesis 시 4ns가 아닌 4.5ns로 진행해야 slack 발생하지 않습니다.

/***********************************************************************************************/

/***********************************************실행********************************************/

1. Run 폴더에 있는 Makefile에 들어가서 56번째 줄에 있는 Makefile이 있는 디렉토리로 경로를 변경해줍니다.
2. run을 실행할 디렉토리에 input_r.dat, input_h.dat, input_m.dat, Makefile을 넣어줍니다.
3. run을 실행할 디렉토리에 src 디렉토리를 만들어서 host.cpp, NTRU.cpp, NTRU.h를 만들어 넣어줍니다.(혹은 경로는 Makefile 96번째 줄에서 변경가능)
4. 그 후 'make cleanall'을 입력해 emulation 관련 파일들을 삭제해줍니다.
5. 'make run TARGET=sw_eum DEVICE=$AWS_PLATFORM' 을 입력하여 sw_emu를 실행합니다.
6. 'make run TARGET=hw_eum DEVICE=$AWS_PLATFORM' 을 입력하여 hw_emu를 실행합니다.
7. 'make all TARGET=hw DEVICE=$AWS_PLATFORM' 을 입력하여 bit file을 생성하여 FPGA test를 진행합니다.

/***********************************************************************************************/
