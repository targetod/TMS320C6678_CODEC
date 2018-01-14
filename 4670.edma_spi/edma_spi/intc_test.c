/*
 * intc_test.c
 *
 *  Created on: 4 янв. 2018 г.
 *      Author: Alex
 */


#include "intc_test.h"
#include <ti/csl/csl_edma3.h>
#include <stdio.h>



/* INTC Objects */
#pragma DATA_SECTION(edmaIntcObj,".testData");
CSL_IntcObj                  edmaIntcObj;

#pragma DATA_SECTION(intcContext,".testData");
CSL_IntcContext              intcContext;

#pragma DATA_SECTION(EventHandler,".testData");
CSL_IntcEventHandlerRecord   EventHandler[2];

#pragma DATA_SECTION(edmaIntcHandle,".testData");
CSL_IntcHandle              edmaIntcHandle; // дескриптор переривання для edma

#pragma DATA_SECTION(cphnd,".testData");
CSL_CPINTC_Handle           cphnd;
#pragma DATA_SECTION(sys_event,".testData");
CSL_CPINTCSystemInterrupt   sys_event = CSL_INTC0_CPU_3_1_EDMACC_GINT; //6 EDMA3CC1 CC_GINT EDMA3CC1 GINT

#pragma DATA_SECTION(host_event,".testData");
CSL_CPINTCHostInterrupt     host_event ; //


Int32 set_edma_intc (){
    CSL_Status                  status; // статус

    /*  Channel. */
    /* Відмітимо, що channel = 32 + (11 * CoreNumber) = 43 канал для Core0 */
    CSL_CPINTCChannel           channel = 0;

    CSL_IntcEventHandlerRecord  EventRecord;// запис дескриптора події
    CSL_IntcParam               vectId; // айді вектора переривання
    CSL_IntcEventId             eventId = CSL_GEM_INTC0_OUT0_OR_INTC1_OUT0;


/*
 *  The CPINTC is the interrupt controller which handles the system interrupts
 *  for the host, processes & prioritizes them and then is responsible for
 *  delivering these to the host.
 */
    /* Initialize the chip level CIC CSL handle */
    cphnd = CSL_CPINTC_open(0);
    if (cphnd == 0){
        printf("Error: Unable to initialize CPINTC-0\n");
        return -1;
    }

    // Map the CIC input event to the CorePac input event
    // Step 2: Map the CIC input event to the CorePac input event
     // (which are
    // Disable all host interrupts.
    CSL_CPINTC_disableAllHostInterrupt(cphnd);

    CSL_CPINTC_setNestingMode(cphnd, CPINTC_NO_NESTING);


    // Map the input system event to a channel. Note, the
    // mapping from channel to Host event is fixed.
    // from table 7-38 CIC0    6 --- EDMA3CC1 CC_GINT --- EDMA3CC1 GINT
    CSL_CPINTC_mapSystemIntrToChannel(cphnd, sys_event, channel);

    /* Очищаємо системне переривання EDMA3CC1 GINT system interrupt номер 6 */
    /* Номер взятий з Table 7-38 data manual 6678 */
    CSL_CPINTC_clearSysInterrupt (cphnd, sys_event);

    /* Включаємо системне переривання EDMA3CC1 GINT system interrupt номер 6 на CIC0 */
    CSL_CPINTC_enableSysInterrupt (cphnd, sys_event);

    host_event = channel;
    /* Host interrupt mapping is fixed  43 канал - 43 хост */
    //CSL_CPINTC_mapChannelToHostInterrupt (cphnd, channel, host_event);
    // Enable the channel (output).
    CSL_CPINTC_enableHostInterrupt(cphnd, host_event);

    // Enable all host interrupts.
    CSL_CPINTC_enableAllHostInterrupt(cphnd);

    //  Hook an ISR to the CorePac input event
    // Open INTC
    vectId = CSL_INTC_VECTID_5; // встановлюємо 5 вектор переривання

    // відкриваємо переривання для події
        edmaIntcHandle = CSL_intcOpen(&edmaIntcObj, eventId, &vectId, &status);
    if (edmaIntcHandle == NULL)
        return -1;

    /* Bind ISR to Interrupt */
    // записуємо функцію, яка буде оброблювати переривання по таймеру
    EventRecord.handler = (CSL_IntcEventHandler) EDMAInterruptHandler;
    // передаєм додатковий аргумент
    EventRecord.arg     = (void *)eventId;
    // зв*язуємо функцію оброблення з перериванням
    CSL_intcPlugEventHandler(edmaIntcHandle, &EventRecord);

    /* Clear the Interrupt */
    CSL_intcHwControl(edmaIntcHandle,CSL_INTC_CMD_EVTCLEAR,NULL);

    // Event Enable  дозволяємо подію
    CSL_intcHwControl(edmaIntcHandle, CSL_INTC_CMD_EVTENABLE, NULL);
//////////////////////////////


    return 0;
}


Int32 intc_init (void){

    // Global Interrupt enable state
    CSL_IntcGlobalEnableState   state;

    /* INTC module initialization */
    intcContext.eventhandlerRecord = EventHandler;
    intcContext.numEvtEntries      = 2;
    if (CSL_intcInit(&intcContext) != CSL_SOK)
       return -1;

    /* Enable NMIs */
    if (CSL_intcGlobalNmiEnable() != CSL_SOK)
       return -1;

    /* Enable global interrupts */
    if (CSL_intcGlobalEnable(&state) != CSL_SOK)
       return -1;

    /* INTC has been initialized successfully. */
    return 0;
}


void close_intc(){
    /* Disable the events. */
       CSL_intcHwControl(edmaIntcHandle, CSL_INTC_CMD_EVTDISABLE, NULL);

       CSL_intcClose(edmaIntcHandle);
}

extern Uint16 bCon;

// ISR function
void EDMAInterruptHandler (void *arg){
/*
   extern far CSL_CPINTCSystemInterrupt   sys_event;
   extern far CSL_CPINTCHostInterrupt     host_event;
   extern far CSL_CPINTC_Handle           cphnd;
   extern far CSL_IntcHandle              edmaIntcHandle;
   */
    // need far
    extern far CSL_Edma3Handle hModule;
    extern far CSL_Edma3CmdIntr regionIpr;
    /* Disable the CIC0 host interrupt output */
    CSL_CPINTC_disableHostInterrupt(cphnd, host_event);
    /* Clear the CIC0 system interrupt */
    CSL_CPINTC_clearSysInterrupt(cphnd, sys_event);

/*
 * When an interrupt transfer completion code with TCC = n is detected by the EDMA3CC, then the
 * corresponding bit is set in the interrupt pending register (IPR.I n, if n = 0 to 31; IPRH.I n, if n = 32 to 63).
 * Note that once a bit is set in the interrupt pending registers, it remains set; it is your responsibility to clear
 * these bits. The bits set in IPR/IPRH are cleared by writing a 1 to the corresponding bits in the interrupt
 * clear registers (ICR/ICRH).
 *
 */
    regionIpr.region  = CSL_EDMA3_REGION_GLOBAL;
    regionIpr.intr    = 0;
    regionIpr.intrh   = 0;
    CSL_edma3GetHwStatus(hModule,CSL_EDMA3_QUERY_INTRPEND,&regionIpr);

    //  Read the interrupt pending register (IPR/IPRH).

     if((regionIpr.intr & 0x08) != 0x08){ //channel_3 - 0x08 .
             printf("channel_3 intc\n");
     }

     if((regionIpr.intr & 0x40) != 0x40){   //channel_2 - 0x04 . channel_6 - 0x40
         printf("channel_2|6 intc\n");
     }


     /* Clear pending interrupt */
        CSL_edma3HwControl(hModule,CSL_EDMA3_CMD_INTRPEND_CLEAR, &regionIpr);

        /* Clear the event ID. */
      CSL_intcEventClear((CSL_IntcEventId)arg);

      /* Clear the CorePac interrupt */
      CSL_intcHwControl(edmaIntcHandle,CSL_INTC_CMD_EVTCLEAR,NULL);

      /* Enable the CIC0 host interrupt output */
      CSL_CPINTC_enableHostInterrupt(cphnd, host_event);


     // bCon = 0;
      bCon ++;

     /*
       * // Disable DMA Request
          ((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIINT0 &=\
              ~(CSL_SPI_SPIINT0_DMAREQEN_ENABLE<<CSL_SPI_SPIINT0_DMAREQEN_SHIFT);

       // Enable DMA Request
         ((CSL_SpiRegsOvly) CSL_SPI_REGS)->SPIINT0 |=\
              CSL_SPI_SPIINT0_DMAREQEN_ENABLE<<CSL_SPI_SPIINT0_DMAREQEN_SHIFT;
*/

}

