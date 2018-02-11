/*
 * spi_test.c
 *
 *  Created on: 4 џэт. 2018 у.
 *      Author: Alex
 */

#include "spi_test.h"


static uint32_t data1_reg_val;

static void  my_spi_delay (    uint32_t delay )
{
    volatile uint32_t i;

    for ( i = 0 ; i < delay ; i++ ){ };
}


/******************************************************************************
 *
 * Function:    spi_claim
 *
 * Description: This function claims the SPI bus in the SPI controller
 *
 * Parameters:  Uint32 cs       - Chip Select number for the slave SPI device
 *              Uint32 freq     - SPI clock frequency
 *
 * Return Value: error status
 *
 ******************************************************************************/
SPI_STATUS my_spi_claim( uint32_t      cs,    uint32_t      freq)
{
    uint32_t scalar;

    PLIBSPILOCK()

    /* Enable the SPI hardware */
    SPI_SPIGCR0 = CSL_SPI_SPIGCR0_RESET_IN_RESET;
    my_spi_delay (2000);
    SPI_SPIGCR0 = CSL_SPI_SPIGCR0_RESET_OUT_OF_RESET;

    /* Set master mode, powered up and not activated */
    SPI_SPIGCR1 =   (CSL_SPI_SPIGCR1_MASTER_MASTER << CSL_SPI_SPIGCR1_MASTER_SHIFT)   |
                    (CSL_SPI_SPIGCR1_CLKMOD_INTERNAL << CSL_SPI_SPIGCR1_CLKMOD_SHIFT);


    /* CS0, CS1, CLK, Slave in and Slave out are functional pins */
    if (cs == 0) {
        SPI_SPIPC0 =    (CSL_SPI_SPIPC0_SCS0FUN0_SPI << CSL_SPI_SPIPC0_SCS0FUN0_SHIFT) |
                        (CSL_SPI_SPIPC0_CLKFUN_SPI << CSL_SPI_SPIPC0_CLKFUN_SHIFT)     |
                        (CSL_SPI_SPIPC0_SIMOFUN_SPI << CSL_SPI_SPIPC0_SIMOFUN_SHIFT)   |
                        (CSL_SPI_SPIPC0_SOMIFUN_SPI << CSL_SPI_SPIPC0_SOMIFUN_SHIFT);
    } else if (cs == 1) {
        SPI_SPIPC0 =    ((CSL_SPI_SPIPC0_SCS0FUN1_SPI << CSL_SPI_SPIPC0_SCS0FUN1_SHIFT) |
                        (CSL_SPI_SPIPC0_CLKFUN_SPI << CSL_SPI_SPIPC0_CLKFUN_SHIFT)     |
                        (CSL_SPI_SPIPC0_SIMOFUN_SPI << CSL_SPI_SPIPC0_SIMOFUN_SHIFT)   |
                        (CSL_SPI_SPIPC0_SOMIFUN_SPI << CSL_SPI_SPIPC0_SOMIFUN_SHIFT)) & 0xFFFF;
    }

    /* setup format */
    scalar = ((SPI_MODULE_CLK / freq) - 1 ) & 0xFF;

    if ( cs == 0) {
        SPI_SPIFMT0 =   (8 << CSL_SPI_SPIFMT_CHARLEN_SHIFT)               |
                        (scalar << CSL_SPI_SPIFMT_PRESCALE_SHIFT)                      |
                        (CSL_SPI_SPIFMT_PHASE_DELAY << CSL_SPI_SPIFMT_PHASE_SHIFT)     |
                        (CSL_SPI_SPIFMT_POLARITY_LOW << CSL_SPI_SPIFMT_POLARITY_SHIFT) |
                        (CSL_SPI_SPIFMT_SHIFTDIR_MSB << CSL_SPI_SPIFMT_SHIFTDIR_SHIFT);
    }else if ( cs == 1) {
        SPI_SPIFMT0 =   (16 << CSL_SPI_SPIFMT_CHARLEN_SHIFT)               |
                        (scalar << CSL_SPI_SPIFMT_PRESCALE_SHIFT)                      |
                        (CSL_SPI_SPIFMT_PHASE_NO_DELAY << CSL_SPI_SPIFMT_PHASE_SHIFT)     |
                        (CSL_SPI_SPIFMT_POLARITY_LOW << CSL_SPI_SPIFMT_POLARITY_SHIFT) |
                        (CSL_SPI_SPIFMT_SHIFTDIR_MSB << CSL_SPI_SPIFMT_SHIFTDIR_SHIFT);
    }

    /* Put SPI in Lpbk mode */
    //  ((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIGCR1 |=
   // SPI_SPIGCR1 |=
    //      CSL_SPI_SPIGCR1_LOOPBACK_ENABLE<<CSL_SPI_SPIGCR1_LOOPBACK_SHIFT;

    /* hold cs active at end of transfer until explicitly de-asserted */
    data1_reg_val = (CSL_SPI_SPIDAT1_CSHOLD_ENABLE << CSL_SPI_SPIDAT1_CSHOLD_SHIFT) |
                    (0x02 << CSL_SPI_SPIDAT1_CSNR_SHIFT);
     if (cs == 0) {
         SPI_SPIDAT1 =   (CSL_SPI_SPIDAT1_CSHOLD_ENABLE << CSL_SPI_SPIDAT1_CSHOLD_SHIFT) |
                         (0x02 << CSL_SPI_SPIDAT1_CSNR_SHIFT);
     }

    /* including a minor delay. No science here. Should be good even with
    * no delay
    */
    if (cs == 0) {
        SPI_SPIDELAY =  (8 << CSL_SPI_SPIDELAY_C2TDELAY_SHIFT) |
                        (8 << CSL_SPI_SPIDELAY_T2CDELAY_SHIFT);
        /* default chip select register */
        SPI_SPIDEF  = CSL_SPI_SPIDEF_RESETVAL;
    } else if (cs == 1) {
        SPI_SPIDELAY =  (6 << CSL_SPI_SPIDELAY_C2TDELAY_SHIFT) |
                        (3 << CSL_SPI_SPIDELAY_T2CDELAY_SHIFT);
    }

    /* no interrupts */
    SPI_SPIINT0 = CSL_SPI_SPIINT0_RESETVAL;
    SPI_SPILVL  = CSL_SPI_SPILVL_RESETVAL;

    /* enable SPI */
    SPI_SPIGCR1 |= ( CSL_SPI_SPIGCR1_ENABLE_ENABLE << CSL_SPI_SPIGCR1_ENABLE_SHIFT );

    if (cs == 1) {
        SPI_SPIDAT0 = 1 << 15;
        my_spi_delay (10000);
        /* Read SPIFLG, wait untill the RX full interrupt */
        if ( (SPI_SPIFLG & (CSL_SPI_SPIFLG_RXINTFLG_FULL<<CSL_SPI_SPIFLG_RXINTFLG_SHIFT)) ) {
            /* Read one byte data */
            scalar = SPI_SPIBUF & 0xFF;
            /* Clear the Data */
            SPI_SPIBUF = 0;
        }
        else {
            /* Read one byte data */
            scalar = SPI_SPIBUF & 0xFF;
            return SPI_EFAIL;
        }
    }
    return SPI_EOK;
}



/******************************************************************************
 *
 * Function:    spi_release
 *
 * Description: This function releases the bus in SPI controller
 *
 * Parameters:  None
 *
 * Return Value: None
 *
 ******************************************************************************/
void my_spi_release (    void)
{
    /* Disable the SPI hardware */
    SPI_SPIGCR1 = CSL_SPI_SPIGCR1_RESETVAL;

    PLIBSPIRELEASE()
}

/******************************************************************************
 *
 * Function:    spi_xfer
 *
 * Description: This function sends and receives 8-bit data serially
 *
 * Parameters:  uint32_t nbytes   - Number of bytes of the TX data
 *              uint8_t* data_out - Pointer to the TX data
 *              uint8_t* data_in  - Pointer to the RX data
 *              Bool terminate  - TRUE: terminate the transfer, release the CS
 *                                FALSE: hold the CS
 *
 * Return Value: error status
 *
 ******************************************************************************/
SPI_STATUS my_spi_xfer
(
    uint32_t              nbytes,
    uint8_t*              data_out,
    uint8_t*              data_in,
    Bool                terminate
)
{
    uint32_t          i, buf_reg;
    uint8_t*          tx_ptr = data_out;
    uint8_t*          rx_ptr = data_in;


    /* Clear out any pending read data */
    SPI_SPIBUF;

    for (i = 0; i < nbytes; i++)
    {
        /* Wait untill TX buffer is not full */
        while( SPI_SPIBUF & CSL_SPI_SPIBUF_TXFULL_MASK );

        /* Set the TX data to SPIDAT1 */
        data1_reg_val &= ~0xFFFF;
        if(tx_ptr)
        {
            data1_reg_val |= *tx_ptr & 0xFF;
            tx_ptr++;
        }

        /* Write to SPIDAT1 */
        if((i == (nbytes -1)) && (terminate))
        {
            /* Release the CS at the end of the transfer when terminate flag is TRUE */
            SPI_SPIDAT1 = data1_reg_val & ~(CSL_SPI_SPIDAT1_CSHOLD_ENABLE << CSL_SPI_SPIDAT1_CSHOLD_SHIFT);
        } else
        {
            SPI_SPIDAT1 = data1_reg_val;
        }


        /* Read SPIBUF, wait untill the RX buffer is not empty */
        while ( SPI_SPIBUF & ( CSL_SPI_SPIBUF_RXEMPTY_MASK ) );

        /* Read one byte data */
        buf_reg = SPI_SPIBUF;
        if(rx_ptr)
        {
            *rx_ptr = buf_reg & 0xFF;
            rx_ptr++;
        }
    }

    return SPI_EOK;

}
