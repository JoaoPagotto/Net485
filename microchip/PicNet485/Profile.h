#ifndef PROFILE_H
#define	PROFILE_H

#include <xc.h>

#define OUTPUT 0
#define INPUT 1

#define LOW 0
#define HIGH 1

#define FALSE 0
#define TRUE 1

//------------------------------------------------------------------------------
// LOCAL

#define LED_TRIS	(TRISEbits.TRISE2)
#define LED         (PORTEbits.RE2)

//------------------------------------------------------------------------------

#define NET485

#define RS485_RO_TRIS	(TRISCbits.TRISC6) // 25 - RC6/TX
#define RS485_RO_IO     (PORTCbits.RC6)

#define RS485_DE_TRIS	(TRISDbits.TRISD4) // 27 - RD4
#define RS485_DE_IO     (PORTDbits.RD4)

#define RS485_DI_TRIS	(TRISCbits.TRISC7) // 26 - RC7/RX/
#define RS485_DI_IO     (PORTCbits.RC7)

//------------------------------------------------------------------------------

#endif	/* PROFILE_H */

