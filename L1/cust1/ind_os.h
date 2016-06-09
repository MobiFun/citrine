/************* Revision Controle System Header *************
 *                  GSM Layer 1 software
 *
 *        Filename ind_os.h
 *        Version  1.2
 *        Date     05/25/00
 * 
 ************* Revision Controle System Header *************/



#define OS_OK   0
        
typedef SYS_WORD16  T_OS_RETURN;

// Prototype for independant os commands
extern T_OS_RETURN  ind_os_sleep (SYS_UWORD32 millisecs);


