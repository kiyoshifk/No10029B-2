#include <xc.h>
#include <sys/attribs.h>
#include <string.h>

//	NTSC.c     initial
//	NTSC-No2.c ram èkè¨
//	NTSC-No3.c interlace(ìØä˙êMçÜÇÃÇ›)
//	NTSC-No4.c êÖïΩâëúìxÇ‡2î{Ç…Ç∑ÇÈ  x=0Å`464, y=0Å`320


//#define PSET1(x,y) {if((x)>=0 && (x)<=232 && (y)>=0 && (y)<=160)video[(y)+56][(x)/32+2] |= (0x80000000>>((x)%32));}
//#define PSET1(x,y) {if((x)>=0 && (x)<=232 && (y)>=0 && (y)<=160)video[(y)][(x)/32+2] |= (0x80000000>>((x)%32));}

uint32_t v_sync1[10];
uint32_t v_sync21[10];
uint32_t v_sync23[10];
uint32_t h_sync[10];

uint32_t video[320][20];				// video RAM
const uint32_t video_0[20];

/********************************************************************************/
/*		Pset																	*/
/*																				*/
/*    line 0      video[0]														*/
/*    line 2      video[2]														*/
/*    line 4      video[3]														*/
/*      .																		*/
/*      .																		*/
/*    line 316    video[316]													*/
/*    line 318    video[318]													*/
/*																				*/
/*    line 320    [ 0V èoóÕ 102+1 line ]										*/
/*    line 322    [ 0V èoóÕ 102+1 line ]										*/
/*      .																		*/
/*      .																		*/
/*    line 522    [ 0V èoóÕ 102+1 line ]										*/
/*    line 524    [ 0V èoóÕ 102+1 line ]										*/
/*																				*/
/*    line 1      video[1]														*/
/*    line 3      video[3]														*/
/*      .																		*/
/*      .																		*/
/*    line 317    video[317]													*/
/*    line 319    video[319]													*/
/*																				*/
/*    line 321    [ 0V èoóÕ 102 line ]											*/
/*    line 323    [ 0V èoóÕ 102 line ]											*/
/*      .																		*/
/*      .																		*/
/*    line 521    [ 0V èoóÕ 102 line ]											*/
/*    line 523    [ 0V èoóÕ 102 line ]											*/
/*																				*/
/*	âëúìxÅF448x320ÉhÉbÉg														*/
/*	uint32_t video[320][20];   Video RAM										*/
/*	video[y][x]: y=0Å`319, x=4Å`17 ... Ç±ÇÃîÕàÕäOÇÕ 0 ÇèëÇ´çûÇÒÇ≈Ç®Ç≠Ç±Ç∆		*/
/********************************************************************************/
void Pset(int x, int y, int c)
{
	uint32_t bit;
	int xx;
	
	if(x<0 || x>=448 || y<0 || y>=320)
		return;							// out of range
	xx = x/32+4;
	bit = 0x80000000 >> x%32;
	if(c)
		video[y][xx] |= bit;
	else
		video[y][xx] &= ~bit;
}

int Pget(int x, int y)
{
	uint32_t bit;
	int xx;
	
	if(x<0 || x>=448 || y<0 || y>=320)
		return 0;						// out of range
	xx = x/32+4;
	bit = 0x80000000 >> x%32;
	return video[y][xx] & bit;
}
/********************************************************************************/
/*		sync_data_set															*/
//
//      IO clock = 20MHz
//      shift clock = 20MHz/2/(1+1) = 5MHz  1bit:0.2É s
//      ìØä˙êMçÜ 10ì]ëó = 32x0.2x10 = 64É s
//      h_sync: 4.8É s(24bit) low, 59.2É s(296bit) high
//      v_sync: 59.2É s(296bit) high, 4.8É s(24bit) low
//      ìØä˙êMçÜÇÕ v_sync:3cycle + h_sync:259cycle
//
//		v_sync1  x3
//		h_sync   x259
//		v_sync21 x1
//		v_sync1  x2
//		v_sync23 x1
//		h_sync   x259
/********************************************************************************/
void sync_data_set()
{
    int i, byte;
    uint32_t bit;

	bit = 0x80000000;
	byte = 0;
	for(i=0; i<10*32; i++){
		if(i<296)
			v_sync1[byte] &= ~bit;
		else
			v_sync1[byte] |= bit;
		
		if(i<24)
			v_sync21[byte] &= ~bit;
		else if(i<160)
			v_sync21[byte] |= bit;
		else if(i<296)
			v_sync21[byte] &= ~bit;
		else
			v_sync21[byte] |= bit;
		
		if(i<160)
			v_sync23[byte] &= ~bit;
		else
			v_sync23[byte] |= bit;
		
		if(i<24)
			h_sync[byte] &= ~bit;
		else
			h_sync[byte] |= bit;
		if((bit>>=1)==0){
			bit = 0x80000000;
			++byte;
		}
	}

#if 0
    bit = 0x80000000;
    byte = 0;
    for(i=0; i<10*32; i++){
		if(i<296)
			v_sync[byte] &= ~bit;
		else
			v_sync[byte] |= bit;
		
		if(i<24)
			h_sync[byte] &= ~bit;
		else
			h_sync[byte] |= bit;
		if((bit>>=1)==0){
			bit = 0x80000000;
			++byte;
		}
	}
#endif
}

/********************************************************************************/
/*		spi_init																*/
/********************************************************************************/
void spi_init()
{
//	int i;

	sync_data_set();

	/*** SPI setting	***/
	// SPI2 Video RB2
	RPB2R = 4;								// SDO2
	SPI2CON = 0x1083C;						// 32bit
	SPI2CONSET = 0x8000;					// Enable
	SPI2BRG = 0;							// 20MHz/2/(0+1)=10MHz
	
	// SPI1 sync RB13
	RPB13R = 3;								// SDO1
	SPI1CON = 0x1083C;						// 32bit
	SPI1CONSET = 0x8000;					// Enable
	SPI1BRG = 1;							// 20MHz/2/(1+1)=5MHz
	
	/*** DMA setting	***/
	DMACON = 0x8000;						// DMA Enable
	
	//channel 2  NTSC sync
	DCH2CON = 0;
	DCH2CONbits.CHAEN = 0;					// channel automatic off
	DCH2ECONbits.CHSIRQ = _SPI1_TX_IRQ;
	DCH2ECONbits.SIRQEN = 1;				// channel start IRQ enable bit
	DCH2SSA = ((uint32_t) v_sync1)&0x1FFFFFFF;
	DCH2DSA = ((uint32_t) (&SPI1BUF))&0x1FFFFFFF;
	DCH2SSIZ = sizeof(v_sync1);
	DCH2DSIZ = 4;
	DCH2CSIZ = 4;							// 1âÒÇÃì]ëóÉoÉCÉgêî
	DCH2INTbits.CHBCIE = 1;					// Block transfer complete interrupt
	DCH2INTbits.CHBCIF = 0;					// Block transfer complete interrupt flag clear
	IPC10bits.DMA2IP = 5;
	IFS1bits.DMA2IF = 0;
	IEC1bits.DMA2IE = 1;
	
	//channel 3  NTSC Video
	DCH3CON = 0;
	DCH3CONbits.CHAEN = 0;					// channel automatic off
	DCH3ECONbits.CHSIRQ = _SPI2_TX_IRQ;
	DCH3ECONbits.SIRQEN = 1;				// channel start IRQ enable bit
	DCH3SSA = ((uint32_t) video_0)&0x1FFFFFFF;
	DCH3DSA = ((uint32_t) (&SPI2BUF))&0x1FFFFFFF;
	DCH3SSIZ = sizeof(video_0);
	DCH3DSIZ = 4;
	DCH3CSIZ = 4;							// 1âÒÇÃì]ëóÉoÉCÉgêî
	DCH3INTbits.CHBCIE = 1;					// Block transfer complete interrupt
	DCH3INTbits.CHBCIF = 0;					// Block transfer complete interrupt flag clear
	IPC10bits.DMA3IP = 5;
	IFS1bits.DMA3IF = 0;
	IEC1bits.DMA3IE = 1;

	__builtin_disable_interrupts();
	DCH2CONbits.CHEN = 1;					// DMA ch2 enable
	DCH3CONbits.CHEN = 1;					// DMA ch3 enable
	__builtin_enable_interrupts();
	
	/*** timer2 setting for voice ***/
	T2CON = 0;
	PR2 = 255;
	TMR2 = 0;
    T2CONbits.ON = 1;
//	IPC2bits.T2IP = 4;
//	IFS0bits.T2IF = 0;
//	IEC0bits.T2IE = 1;
    
    /*** timer4 setting for voice ***/
    T4CON = 0;
    PR4 = 1250-1;                 // 16KHz
    TMR4 = 0;
    T4CONbits.ON = 1;
    IPC4bits.T4IP = 4;
    IFS0bits.T4IF = 0;

    /*** OC1 setting for voice ***/
    RPB3Rbits.RPB3R = 5;    // OC1 output
    OC1CON = 0x0000; // Turn off the OC1 when performing the setup
    OC1R = 0x0064; // Initialize primary Compare register
    OC1RS = 0x0064; // Initialize secondary Compare register
    OC1CON = 0x0006; // Configure for PWM mode without Fault pin enabled
    OC1CONSET = 0x8000; // Enable OC1
}
/********************************************************************************/
/*		interrupt																*/
/********************************************************************************/
//	NTSC Sync
void __ISR(_DMA2_VECTOR, IPL5AUTO) _DMA2Interrupt ()
{
	static int cnt;
	
	IFS1bits.DMA2IF = 0;					// Clear DMA ch1 IF
	DCH2INTbits.CHBCIF = 0;
	if(cnt < 263){
		if(cnt < 3+7)
			DCH2SSA = ((uint32_t) v_sync1)&0x1FFFFFFF;
		else if(cnt < 262)
			DCH2SSA = ((uint32_t) h_sync)&0x1FFFFFFF;
		else
			DCH2SSA = ((uint32_t) v_sync21)&0x1FFFFFFF;
	}
	else{
		if(cnt < 265+7)
			DCH2SSA = ((uint32_t) v_sync1)&0x1FFFFFFF;
		else if(cnt < 266+7)
			DCH2SSA = ((uint32_t) v_sync23)&0x1FFFFFFF;
		else
			DCH2SSA = ((uint32_t) h_sync)&0x1FFFFFFF;
	}
	DCH2CONbits.CHEN = 1;
    if(++cnt >= (262*2+1))
        cnt = 0;
}

#if 0
//	NTSC Video  y=0Å`320
void __ISR(_DMA3_VECTOR, IPL5AUTO) _DMA3Interrupt ()
{
	static int cnt= 469;
	
	IFS1bits.DMA3IF = 0;					// Clear DMA ch1 IF
	DCH3INTbits.CHBCIF = 0;

	// cnt=0Å`160: video[cnt] = video[0Å`160]
	if(cnt < 161)               // ãÙêîçs 161
		DCH3SSA = ((uint32_t) video[cnt])&0x1FFFFFFF;
	// cnt=161Å`262: 0
	else if(cnt < 262+1)
		DCH3SSA = ((uint32_t) video_0)&0x1FFFFFFF;
	// cnt=263Å`422: video[cnt-102] = video[161Å`320]
	else if(cnt < 262+1+160)    // äÔêîçs 160
		DCH3SSA = ((uint32_t) video[cnt-102])&0x1FFFFFFF;
	// cnt=423Å`524: 0
	else
		DCH3SSA = ((uint32_t) video_0)&0x1FFFFFFF;
	
	DCH3CONbits.CHEN = 1;
    if(++cnt >= (262*2+1))		// 262*2+1 = 525
        cnt = 0;
}
#endif

//	NTSC Video  y=0Å`319
void __ISR(_DMA3_VECTOR, IPL5AUTO) _DMA3Interrupt ()
{
	static int cnt= 413;
	
	IFS1bits.DMA3IF = 0;					// Clear DMA ch1 IF
	DCH3INTbits.CHBCIF = 0;

	// cnt >= 321Å`524: 0V output
	if(cnt >= 320){
		DCH3SSA = ((uint32_t) video_0)&0x1FFFFFFF;
	}
	else{
		if(cnt & 1){
			// cnt = 1, 3 ... 317, 319
			DCH3SSA = ((uint32_t) video[cnt])&0x1FFFFFFF;
		}
		else{
			// cnt = 0, 2 ... 320, 322
			DCH3SSA = ((uint32_t) video[cnt])&0x1FFFFFFF;
		}
	}
	
	DCH3CONbits.CHEN = 1;
	cnt += 2;
	if(cnt >= 525)
		cnt -= 525;
}

