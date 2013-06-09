/*
    Module Name:
    sys_rfrw.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2011-05-02      Initial version
*/

#ifndef SYS_RFRW_H
#define SYS_RFRW_H

/*
 * DEFINITIONS AND MACROS
 */
#define RF_CSR_CFG      0xb0180500
#define RF_CSR_KICK     (1<<17)

/*
 * TYPEDEFS AND STRUCTURES
 */


/*
 * EXPORT FUNCTION
 */



/*
 * EXPORT FUNCTION
 */
int rw_rf_reg(int write, int reg, int *data);

#endif
