

/*
 * Steven Ji  - good example for the C6670 here:
 * http://e2e.ti.com/support/dsp/c6000_multi-core_dsps/f/639/t/170614.aspx
 *
 * The SPI is configured in internal loopback mode and Channel 2 and Channel 3 of EDMA3_CC1
 * are being used for SPIXEVT and SPIREVT (Table 7-36 in C6670 data manual).
 * The source buffer is located in CorePac0 L2 SRAM and being sent to SPI by EDMA.
 * After loopback, the data is received by SPI and being sent
 * to destination buffer located in CorePac1 L2 SRAM by EDMA as well.
 *  - DMA happen only  once.  To trigger the SPI-EDMA transfer for multiple times
 *  - we need to setup reload paramSet for EDMA channels for the continuous
 *  - peripheral support (refer to section 3.4.3 in EDMA user guide).
 *  - And we can disable/enable the DMAREQEN bit in the SPIINT0 register to re-trigger the TX/RX DMA request.(prog 0410.edma_spi)
 */
//#define SOC_C6678

#include "spi_test.h"
//#include <ti/csl/soc.h>
//#include <ti/csl/tistdtypes.h>
//#include <ti/csl/csl_chip.h>
//#include <ti/csl/csl_edma3.h>
//#include <ti/csl/csl_edma3Aux.h>
//#include <ti/csl/cslr_spi.h>
#include "ti\platform\platform.h"
#include "ti\platform\resource_mgr.h"

#include "spi_common.h"

#include <stdio.h>

#define TEST_ACNT 2
#define TEST_BCNT 64
#define TEST_CCNT 1

#define TEST_SYCTYPE_AB     1
#define TEST_SYCTYPE_A      0

#define BUF_SIZE TEST_BCNT

// #pragma DATA_SECTION(dstBuf, ".ddrData") // is located in DDR SRAM
// in the "example.cmd" file, you will see ".ddrData" is pointed to the DDR3_DATA_MEM.
#pragma DATA_SECTION(dstBuf, ".gem1_data") // is located in the L2SRAM
#pragma DATA_ALIGN(dstBuf, 8)
Uint16 dstBuf[BUF_SIZE];

#pragma DATA_SECTION(srcBuf, ".gem0_data")
#pragma DATA_ALIGN(srcBuf, 8)
Uint16 srcBuf[BUF_SIZE];

typedef volatile  CSL_SpiRegs* CSL_SpiRegsOvly;
//volatile CSL_SpiRegs*  pDebSpi =  (volatile CSL_SpiRegs*) CSL_SPI_REGS;


static void  my_spi_delay (    uint32_t delay )
{
    volatile uint32_t i;

    for ( i = 0 ; i < delay ; i++ ){ };
}

Uint32 global_address (Uint32 addr)
{
	Uint32 corenum;
	corenum = CSL_chipReadReg(CSL_CHIP_DNUM);

	addr = addr + (0x10000000 + corenum*0x1000000);
	return addr;
}

void Trigger_Edma_Channels(void)
{
   	// Trigger channel
   	CSL_edma3HwChannelControl(hChannel0,CSL_EDMA3_CMD_CHANNEL_ENABLE,NULL);

   	// Trigger channel
   	CSL_edma3HwChannelControl(hChannel1,CSL_EDMA3_CMD_CHANNEL_ENABLE,NULL);
}


void Setup_Edma (Uint32 srcBuf,Uint32 dstBuf)
{

    // EDMA Module Initialization
    EdmaStat = CSL_edma3Init(&context);

 	// EDMA Module Open
    hModule = CSL_edma3Open(&moduleObj, CSL_EDMA3CC_1, NULL, &EdmaStat);

	// SPI Tx Channel Open - Channel 2 for Tx (SPIXEVT)
	chParam.regionNum  = CSL_EDMA3_REGION_GLOBAL;
	chSetup.que        = CSL_EDMA3_QUE_0;
	chParam.chaNum     = CSL_EDMA3_CHA_2;

	hChannel0 = CSL_edma3ChannelOpen(&ChObj0, CSL_EDMA3CC_1, &chParam, &EdmaStat);
	chSetup.paramNum   = chParam.chaNum; //CSL_EDMA3_CHA_2;
    CSL_edma3HwChannelSetupParam(hChannel0,chSetup.paramNum);

	// SPI Rx Channel Open - Channel 3 for Rx (SPIREVT)
	chParam.regionNum  = CSL_EDMA3_REGION_GLOBAL;
	chSetup.que        = CSL_EDMA3_QUE_0;
	chParam.chaNum     = CSL_EDMA3_CHA_3;

	hChannel1 = CSL_edma3ChannelOpen(&ChObj1, CSL_EDMA3CC_1, &chParam, &EdmaStat);
	chSetup.paramNum = chParam.chaNum; //CSL_EDMA3_CHA_3;
	// �*������ �����  (#hChannel1) � PSET (#CSL_EDMA3_CHA_3)
    CSL_edma3HwChannelSetupParam(hChannel1,chSetup.paramNum);

	// Parameter Handle Open
	// Open all the handles and keep them ready
    // ����� ������ ������ PaRAM  � �������� handle �� PSET (e.g. #2)
	paramHandle0            = CSL_edma3GetParamHandle(hChannel0,CSL_EDMA3_CHA_2,&EdmaStat);
  	paramHandle1            = CSL_edma3GetParamHandle(hChannel1,CSL_EDMA3_CHA_3,&EdmaStat);
// TX
    paramSetup.aCntbCnt     = CSL_EDMA3_CNT_MAKE(TEST_ACNT,(TEST_BCNT));
	paramSetup.srcDstBidx   = CSL_EDMA3_BIDX_MAKE(TEST_ACNT,0 );
	paramSetup.srcDstCidx   = CSL_EDMA3_CIDX_MAKE(0,0);
	paramSetup.cCnt         = TEST_CCNT;
	paramSetup.option       = CSL_EDMA3_OPT_MAKE(CSL_EDMA3_ITCCH_DIS,
	                                             CSL_EDMA3_TCCH_DIS,
	                                             CSL_EDMA3_ITCINT_DIS,
	                                             CSL_EDMA3_TCINT_EN,      /* or CSL_EDMA3_TCINT_DIS*/
	                                             CSL_EDMA3_CHA_2,// 0 ????
	                                             CSL_EDMA3_TCC_NORMAL,
                                                  CSL_EDMA3_FIFOWIDTH_NONE,
                                                  CSL_EDMA3_STATIC_DIS,
                                                  CSL_EDMA3_SYNC_A,
                                                  CSL_EDMA3_ADDRMODE_INCR,
                                                  CSL_EDMA3_ADDRMODE_INCR);

	if( ((Uint32)srcBuf & 0xFFF00000) == 0x00800000)
		paramSetup.srcAddr      = (Uint32)(global_address((Uint32)srcBuf));
	else
		paramSetup.srcAddr      = (Uint32)(srcBuf);

#ifdef _BIG_ENDIAN
    paramSetup.dstAddr      = ((Uint32)&(((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIDAT0)) + (4-TEST_ACNT);
#else
 //   paramSetup.dstAddr      = (Uint32)&(((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIDAT0); //SPIDAT0 was
    paramSetup.dstAddr      =  (Uint32)& (SPI_SPIDAT0);
#endif

	paramSetup.linkBcntrld  = CSL_EDMA3_LINKBCNTRLD_MAKE(CSL_EDMA3_LINK_NULL,0);
// set options
	CSL_edma3ParamSetup(paramHandle0,&paramSetup);
// RX

    paramSetup.aCntbCnt     = CSL_EDMA3_CNT_MAKE(TEST_ACNT,TEST_BCNT);
	paramSetup.srcDstBidx   = CSL_EDMA3_BIDX_MAKE(0,TEST_ACNT );
	paramSetup.srcDstCidx   = CSL_EDMA3_CIDX_MAKE(0,0);
	paramSetup.cCnt         = TEST_CCNT;
	paramSetup.option       = CSL_EDMA3_OPT_MAKE(CSL_EDMA3_ITCCH_DIS,
                                                 CSL_EDMA3_TCCH_DIS,
                                                 CSL_EDMA3_ITCINT_DIS,
                                                 CSL_EDMA3_TCINT_EN,      /* or CSL_EDMA3_TCINT_DIS*/
	                                             CSL_EDMA3_CHA_3,
	                                             CSL_EDMA3_TCC_NORMAL,
                                                  CSL_EDMA3_FIFOWIDTH_NONE,
                                                  CSL_EDMA3_STATIC_DIS,
                                                  CSL_EDMA3_SYNC_A,
                                                  CSL_EDMA3_ADDRMODE_INCR,
                                                  CSL_EDMA3_ADDRMODE_INCR);


    #ifdef _BIG_ENDIAN
        paramSetup.srcAddr      = ((Uint32)&(((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIBUF)) + (4 -TEST_ACNT);
    #else
     //   paramSetup.srcAddr      = (Uint32)&(((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIBUF);
        paramSetup.srcAddr      =(Uint32)& ( SPI_SPIBUF);
    #endif

	if( ((Uint32)dstBuf & 0xFFF00000) == 0x00800000)
		paramSetup.dstAddr      = (Uint32)(global_address((Uint32)dstBuf));
	else
		paramSetup.dstAddr      = (Uint32)dstBuf;

	paramSetup.linkBcntrld  = CSL_EDMA3_LINKBCNTRLD_MAKE(CSL_EDMA3_LINK_NULL,0);
// set options
	CSL_edma3ParamSetup(paramHandle1,&paramSetup);

	// map the channel (#4) to a queue
	CSL_edma3HwChannelSetupQue(hChannel0, CSL_EDMA3_QUE_0);
	CSL_edma3HwChannelSetupQue(hChannel1, CSL_EDMA3_QUE_0);

	Trigger_Edma_Channels();
}

void Test_Edma(void)
{
	// Wait for interrupt
    regionIpr.region  = CSL_EDMA3_REGION_GLOBAL;
	regionIpr.intr    = 0;
	regionIpr.intrh   = 0;
	do{
		CSL_edma3GetHwStatus(hModule,CSL_EDMA3_QUERY_INTRPEND,&regionIpr);
	}while ((regionIpr.intr & 0x08) != 0x08);	//channel_3


	do{
		CSL_edma3GetHwStatus(hModule,CSL_EDMA3_QUERY_INTRPEND,&regionIpr);
	}while ((regionIpr.intr & 0x04) != 0x04);	//channel_2

	 /* Clear pending interrupt */
	//   CSL_edma3HwControl(hModule,CSL_EDMA3_CMD_INTRPEND_CLEAR, &regionIpr);
}


void Close_Edma()
{
    CSL_FINST(hModule->regs->TPCC_SECR,TPCC_TPCC_SECR_SECR2,RESETVAL);
  	CSL_FINST(hModule->regs->TPCC_SECR,TPCC_TPCC_SECR_SECR3,RESETVAL);

 	CSL_edma3ChannelClose(hChannel0);
 	CSL_edma3ChannelClose(hChannel1);
 	CSL_edma3Close(hModule);
}

void Setup_SPI (  uint32_t      freq)
{
    uint32_t scalar;

    //PLIBSPILOCK()
    /* Reset SPI */
  //  ((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIGCR0=
    SPI_SPIGCR0 =
        CSL_SPI_SPIGCR0_RESET_IN_RESET<<CSL_SPI_SPIGCR0_RESET_SHIFT;

    my_spi_delay (2000);
    /* Take SPI out of reset */
    //((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIGCR0=
    SPI_SPIGCR0 =
        CSL_SPI_SPIGCR0_RESET_OUT_OF_RESET<<CSL_SPI_SPIGCR0_RESET_SHIFT;

    /* Configure SPI as master */
   // ((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIGCR1=
    SPI_SPIGCR1 =
        CSL_SPI_SPIGCR1_CLKMOD_INTERNAL<<CSL_SPI_SPIGCR1_CLKMOD_SHIFT|
        CSL_SPI_SPIGCR1_MASTER_MASTER<<CSL_SPI_SPIGCR1_MASTER_SHIFT;

    /* Configure SPI in 4-pin SCS mode */
   // ((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIPC0=
    SPI_SPIPC0 =
        CSL_SPI_SPIPC0_SOMIFUN_SPI<<CSL_SPI_SPIPC0_SOMIFUN_SHIFT|
        CSL_SPI_SPIPC0_SIMOFUN_SPI<<CSL_SPI_SPIPC0_SIMOFUN_SHIFT|
        CSL_SPI_SPIPC0_CLKFUN_SPI<<CSL_SPI_SPIPC0_CLKFUN_SHIFT|
        CSL_SPI_SPIPC0_SCS0FUN1_SPI<<CSL_SPI_SPIPC0_SCS0FUN1_SHIFT;//  CSL_SPI_SPIPC0_SCS0FUN0_SHIFT was


	/* Put SPI in Lpbk mode */
	//((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIGCR1 |=
    SPI_SPIGCR1 |=
		CSL_SPI_SPIGCR1_LOOPBACK_ENABLE<<CSL_SPI_SPIGCR1_LOOPBACK_SHIFT;

    /* Chose SPIFMT0  Choose the SPI data format register n (SPIFMTn)
     *  to be used by configuring the DFSEL bit in the SPI transmit data register (SPIDAT1)
     */ //?
   // ((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIDAT1=
    SPI_SPIDAT1 =
        CSL_SPI_SPIDAT1_DFSEL_FORMAT0<<CSL_SPI_SPIDAT1_DFSEL_SHIFT;

    /* setup format */
       scalar = ((SPI_MODULE_CLK / freq) - 1 ) & 0xFF;
    /* Configure for WAITEN=YES,SHIFTDIR=MSB,POLARITY=HIGH,PHASE=IN,CHARLEN=16*/
   // ((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIFMT[0]=
    SPI_SPIFMT0 =
        CSL_SPI_SPIFMT_WAITENA_DISABLE<<CSL_SPI_SPIFMT_WAITENA_SHIFT|
        CSL_SPI_SPIFMT_SHIFTDIR_MSB<<CSL_SPI_SPIFMT_SHIFTDIR_SHIFT|
        CSL_SPI_SPIFMT_POLARITY_LOW<<CSL_SPI_SPIFMT_POLARITY_SHIFT|
        CSL_SPI_SPIFMT_PHASE_DELAY<<CSL_SPI_SPIFMT_PHASE_SHIFT|
        (scalar << CSL_SPI_SPIFMT_PRESCALE_SHIFT)                |
        0x10<<CSL_SPI_SPIFMT_CHARLEN_SHIFT;   // 16 bits


    /*
     * delay in master mode
     * C2TDELAY is used in master mode only. It defines a setup time for the slave device that
     * delays the data transmission from the chip select active edge by a multiple of SPI module clock cycles.
     * C2TDELAY can be configured between 2 and 257 SPI module clock cycles.
     *
     * T2CDELAY is used in master mode only. It defines a hold time for the slave device that
     * delays the chip select deactivation by a multiple of SPI module clock cycles after the last bit is transferred.
     */
   // ((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIDELAY =
    SPI_SPIDELAY =
           (6 << CSL_SPI_SPIDELAY_C2TDELAY_SHIFT) |
             (3 << CSL_SPI_SPIDELAY_T2CDELAY_SHIFT);

	//((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIINT0 =
    SPI_SPIINT0 =
	        /*Reserved Reads return 0 and writes have no effect. */
	CSL_SPI_SPIINT0_ENABLEHIGHZ_ENABLE<<CSL_SPI_SPIINT0_ENABLEHIGHZ_SHIFT|
	/*Overrun interrupt enable. An interrupt is to be generated when the SPIFLG.OVRNINTFLG bit is set. */
	CSL_SPI_SPIINT0_OVRNINTENA_ENABLE<<CSL_SPI_SPIINT0_OVRNINTENA_SHIFT|
	/*Enables interrupt on bit error. An interrupt is to be generated when the SPIFLG.BITERRFLG is set.*/
	CSL_SPI_SPIINT0_BITERRENA_ENABLE<<CSL_SPI_SPIINT0_BITERRENA_SHIFT|
	/*Reserved Reads return 0 and writes have no effect. */
	CSL_SPI_SPIINT0_DESYNCENA_ENABLE<<CSL_SPI_SPIINT0_DESYNCENA_SHIFT|
	CSL_SPI_SPIINT0_PARERRENA_ENABLE<<CSL_SPI_SPIINT0_PARERRENA_SHIFT|
	CSL_SPI_SPIINT0_TIMEOUTENA_ENABLE<<CSL_SPI_SPIINT0_TIMEOUTENA_SHIFT|
	CSL_SPI_SPIINT0_DLENERRENA_ENABLE<<CSL_SPI_SPIINT0_DLENERRENA_SHIFT;


	//((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPILVL =
  //  SPI_SPILVL =
	//        CSL_SPI_SPILVL_RESETVAL;


    /* Enable communication */
   // ((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIGCR1|=
    SPI_SPIGCR1 |=
        CSL_SPI_SPIGCR1_ENABLE_ENABLE<<CSL_SPI_SPIGCR1_ENABLE_SHIFT;

    /*DMA request enable. Enables the DMA request signal to be generated for both receive and transmit
     * channels. Set DMAREQEN only after setting the SPIGCR1.ENABLE bit to 1.*/
	//((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIINT0 |=
    SPI_SPIINT0 |=
		CSL_SPI_SPIINT0_DMAREQEN_ENABLE<<CSL_SPI_SPIINT0_DMAREQEN_SHIFT;

	/* Transmitter empty interrupt flag. Serves as an interrupt flag indicating that
	 * the transmit buffer (TXBUF) is empty and a new data can be written to it.
	 *  This flag is set when a data is copied to the shift register either directly
	 *   or from the TXBUF register
	 *   1 = Transmit buffer is empty. An interrupt is pending to fill the transmitter.
	 *   0 = Transmit buffer is now full. No interrupt pending for transmitter empty.
	 *
	 *    This bit is cleared by one of following ways:
            - Writing a new data to either SPIDAT0 or SPIDAT1
            - Writing a 0 to SPIGCR1.ENABLE
	 *
	 *   1 - � ������� ������ ��� (����������� ���������������)
	 *   */
	//while(! ( ( (CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIFLG & 0x00000200)) {}
	while(! ( SPI_SPIFLG & 0x00000200) ) {}

	/*
	 * Receiver full interrupt flag. This flag is set when a word is received
	 * and copied into the buffer register (SPIBUF).
	 * This bit is cleared under the following ways:
	 * - Reading the SPIBUF register.
	 * - Reading INTVEC0 or INTVEC1 register when there is a receive buffer full interrupt
	 * - Writing a 1 to this bit
	 * - Writing a 0 to SPIGCR1.ENABLE
	 * - System reset
        0 = No new received data pending. Receive buffer is empty.
        1 = A newly received data is ready to be read. Receive buffer is full.
	 *
	 *  0 - ������ ������
	 */
	//while(!(((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIFLG & 0x00000100)) {}
	while(! ( SPI_SPIFLG & 0x00000100) ) {}

	//pDebSpi;
}


Bool Verify_Transfer(Uint16 aCnt, Uint16 bCnt, Uint16 cCnt, Int16 srcBIdx, Int16 dstBIdx, Int16 srcCIdx, Int16 dstCIdx,Uint32 srcBuff,Uint32 dstBuff, Bool abSync)
{
	Uint8* srcArrayPtr = (Uint8*)srcBuff;
	Uint8* dstArrayPtr = (Uint8*)dstBuff;
	Uint8* srcFramePtr = (Uint8*)srcBuff;
	Uint8* dstFramePtr = (Uint8*)dstBuff;

	int i,j,k;
	for (i = 0; i < cCnt; i++)
	{
		for (j = 0; j < bCnt ;j++)
		{
			for (k = 0;k < aCnt;k++)
			{
				if (srcArrayPtr[k] != dstArrayPtr[k])
				{
					return FALSE;
				}
			}
			srcArrayPtr = srcArrayPtr + srcBIdx;
			dstArrayPtr = dstArrayPtr + dstBIdx;
		}
		if (abSync) {
			srcFramePtr = srcFramePtr + srcCIdx;
			srcArrayPtr = srcFramePtr;
			dstFramePtr = dstFramePtr + dstCIdx;
			dstArrayPtr = dstFramePtr;
		} else
		{
		    srcFramePtr = srcArrayPtr + srcCIdx - srcBIdx;
			srcArrayPtr = srcFramePtr;
			dstFramePtr = dstArrayPtr + dstCIdx - dstBIdx;
			dstArrayPtr = dstFramePtr;
		}
	}
	return TRUE;
}

void Initiate_Buffers(Uint16 aCnt, Uint16 bCnt, Uint16 cCnt, Int16 srcBIdx, Int16 dstBIdx, Int16 srcCIdx, Int16 dstCIdx,Uint32 srcBuff,Uint32 dstBuff, Bool abSync)
{
	Uint8* srcArrayPtr = (Uint8*)srcBuff;
	Uint8* dstArrayPtr = (Uint8*)dstBuff;
	Uint8* srcFramePtr = (Uint8*)srcBuff;
	Uint8* dstFramePtr = (Uint8*)dstBuff;

	Uint8 cnt = 0;
	int i,j,k;
	for (i = 0; i < cCnt; i++)
	{
		for (j = 0; j < bCnt ;j++)
		{
			for (k = 0;k < aCnt;k++)
			{
				srcArrayPtr[k] = cnt++;
				dstArrayPtr[k] = 0;
			}
			srcArrayPtr = srcArrayPtr + srcBIdx;
			dstArrayPtr = dstArrayPtr + dstBIdx;
		}
		if (abSync) {
			srcFramePtr = srcFramePtr + srcCIdx;
			srcArrayPtr = srcFramePtr;
			dstFramePtr = dstFramePtr + dstCIdx;
			dstArrayPtr = dstFramePtr;
		} else
		{
		    srcFramePtr = srcArrayPtr + srcCIdx - srcBIdx;
			srcArrayPtr = srcFramePtr;
			dstFramePtr = dstArrayPtr + dstCIdx - dstBIdx;
			dstArrayPtr = dstFramePtr;
		}
	}
}

void main (void)
{
	volatile Uint32 failFlag = FALSE;
	Uint16  out = 5606;
	Uint16  in ;
	Uint16 i, delay = 2000 ;
	//////////////////////////////////////platform init start////////////////////////////////////////
    platform_init_flags init_flags;
    platform_init_config init_config;

    /* Initialize platform with default values */
    memset(&init_flags, 0x01, sizeof(platform_init_flags));
    memset(&init_config, 0, sizeof(platform_init_config));
    if (platform_init(&init_flags, &init_config) != Platform_EOK) {
       return;
    }
   // platform_get_info(&p_info);
	/////////////////////////////////////////////////////////////////platform init end

	/* test spi polling - working
	 while(1) {
	         my_spi_claim(SPI_CS1, 1000000);
	         my_spi_xfer(2, ( Uint8*) &out,( Uint8*) &in,TRUE);
	         my_spi_release();
	         for ( i = 0 ; i < delay ; i++ ){ };
	         out++;
	}
	*/


	//Initiate src/dst buffers
	Initiate_Buffers(TEST_ACNT,TEST_BCNT, TEST_CCNT,TEST_ACNT ,TEST_ACNT, TEST_ACNT*TEST_BCNT, TEST_ACNT*TEST_BCNT,(Uint32)srcBuf,(Uint32)dstBuf, CSL_EDMA3_SYNC_A);

   	//Setup EDMA for SPI transfer
   	Setup_Edma((Uint32)srcBuf,(Uint32)dstBuf);

	//Configure SPI in loopback mode and enable DMA interrupt support
	Setup_SPI( 5000000);

	//Check EDMA transfer completion status
    Test_Edma();

	//Close EDMA channels/module                                        */
	Close_Edma();
	my_spi_release();
	//Verify src/dst buffers after transfer
	failFlag = Verify_Transfer(TEST_ACNT,TEST_BCNT, TEST_CCNT,TEST_ACNT ,TEST_ACNT, TEST_ACNT*TEST_BCNT, TEST_ACNT*TEST_BCNT,(Uint32)srcBuf,(Uint32)dstBuf, CSL_EDMA3_SYNC_A);

	if (failFlag == TRUE)
		printf("data verification passed\n");
	else
		printf("data verification failed\n");

	printf("end of test\n");

}


///////////////////////////////////////////////////////////////////
/* OSAL functions for Platform Library */
uint8_t *Osal_platformMalloc (uint32_t num_bytes, uint32_t alignment)
{
    return malloc(num_bytes);
}

void Osal_platformFree (uint8_t *dataPtr, uint32_t num_bytes)
{
    /* Free up the memory */
    if (dataPtr)
    {
        free(dataPtr);
    }
}

void Osal_platformSpiCsEnter(void)
{
    /* Get the hardware semaphore.
     *
     * Acquire Multi core CPPI synchronization lock
     */
    while ((CSL_semAcquireDirect (PLATFORM_SPI_HW_SEM)) == 0);

    return;
}

void Osal_platformSpiCsExit (void)
{
    /* Release the hardware semaphore
     *
     * Release multi-core lock.
     */
    CSL_semReleaseSemaphore (PLATFORM_SPI_HW_SEM);

    return;
}

/////////////////////////////////////////////////////////////

