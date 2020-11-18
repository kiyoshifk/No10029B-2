/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdio.h>
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes


void spi_init();
void display(int x, int y, const char * str);
void char_disp(int x, int y, int c, int rev_flag);
void circle1(int x0, int y0, int r, int color);
void Pset(int x, int y, int c);
int Pget(int x, int y);

/*
 *                       Main application
 *
 *    wait 200ms at 40MHz clock
 *    volatile int i;
 *    for(i=0; i<1000000; i++) ;
 */

int main ( void )
{
	int i, j;
	char buf[100];

    /* Initialize all modules */
    SYS_Initialize ( NULL );

    spi_init();
    LATBbits.LATB4 = 1;					// LED On

	Pset(300,200,1);
	sprintf(buf, "[299,200]=%x [300,200]=%x [301,200]=%x", Pget(299,200), Pget(300,200), Pget(301,200));
    display(0,0,buf);
	for(;;) ;


	display(0, 0, "=== Char/Graphic Test ===");
	for(i=0; i<4; i++){
		for(j=0; j<64; j++){
			char_disp(j*6, i*10+20, j+(i&1)*64, i>1);
		}
	}
    for(i=10; i<=100; i+=10){
        circle1(200, 180, i, 1);
    }


    while ( true )
    {
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

