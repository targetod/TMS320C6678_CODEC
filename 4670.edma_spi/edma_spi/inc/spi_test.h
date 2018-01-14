/*
 * spi_test.h
 *
 *  Created on: 4 џэт. 2018 у.
 *      Author: Alex
 */

#ifndef INC_SPI_TEST_H_
#define INC_SPI_TEST_H_



#include <ti/csl/cslr_device.h>
#include <ti/csl/cslr_spi.h>
#include <ti/csl/csl_chip.h>
#include <ti/csl/csl_edma3.h>
#include <ti/csl/csl_edma3Aux.h>
/* ------------------------------------------------------------------------ *
 *  SPI Controller                                                          *
 * ------------------------------------------------------------------------ */
#define SPI_BASE                CSL_SPI_REGS
#define SPI_SPIGCR0             *( volatile Uint32* )( SPI_BASE + 0x0 )
#define SPI_SPIGCR1             *( volatile Uint32* )( SPI_BASE + 0x4 )
#define SPI_SPIINT0             *( volatile Uint32* )( SPI_BASE + 0x8 )
#define SPI_SPILVL              *( volatile Uint32* )( SPI_BASE + 0xc )
#define SPI_SPIFLG              *( volatile Uint32* )( SPI_BASE + 0x10 )
#define SPI_SPIPC0              *( volatile Uint32* )( SPI_BASE + 0x14 )
#define SPI_SPIDAT0             *( volatile Uint32* )( SPI_BASE + 0x38 )
#define SPI_SPIDAT1             *( volatile Uint32* )( SPI_BASE + 0x3c )
#define SPI_SPIBUF              *( volatile Uint32* )( SPI_BASE + 0x40 )
#define SPI_SPIEMU              *( volatile Uint32* )( SPI_BASE + 0x44 )
#define SPI_SPIDELAY            *( volatile Uint32* )( SPI_BASE + 0x48 )
#define SPI_SPIDEF              *( volatile Uint32* )( SPI_BASE + 0x4c )
#define SPI_SPIFMT0             *( volatile Uint32* )( SPI_BASE + 0x50 )
#define SPI_SPIFMT1             *( volatile Uint32* )( SPI_BASE + 0x54 )
#define SPI_SPIFMT2             *( volatile Uint32* )( SPI_BASE + 0x58 )
#define SPI_SPIFMT3             *( volatile Uint32* )( SPI_BASE + 0x5c )
#define SPI_INTVEC0             *( volatile Uint32* )( SPI_BASE + 0x60 )
#define SPI_INTVEC1             *( volatile Uint32* )( SPI_BASE + 0x64 )

#define SPI_CS0              0           /* SPI Chip Select number for NOR*/
#define SPI_CS1             1           /* SPI Chip Select number for FPGA*/
#define SPI_MODULE_CLK          200000000   /* SYSCLK7  = CPU_Clk/6 in HZ */
#define SPI_MAX_FREQ            25000000    /* SPI Max frequency in Hz */
#define SPI_NOR_CHAR_LENTH      8           /* Number of bits per SPI trasfered data element for NOR flash */
#define SPI_FPGA_CHAR_LENTH     16          /* Number of bits per SPI trasfered data element for FPGA */

/* SPI error status */
#define SPI_STATUS        Uint32           /* SPI error status type */
#define SPI_EFAIL         (SPI_STATUS)-1   /* General failure code */
#define SPI_EOK           0                /* General success code */



#if (PLATFORM_SEMLOCK_IN)
#define PLIBSPILOCK() Osal_platformSpiCsEnter();
#define PLIBSPIRELEASE() Osal_platformSpiCsExit ();
#else
#define PLIBSPILOCK()
#define PLIBSPIRELEASE()
#endif


SPI_STATUS my_spi_claim( uint32_t      cs,    uint32_t      freq);
void my_spi_release(    void);
SPI_STATUS my_spi_xfer(
    uint32_t              nbytes,
    uint8_t*              data_out,
    uint8_t*              data_in,
    Bool                terminate
);


#endif /* INC_SPI_TEST_H_ */
