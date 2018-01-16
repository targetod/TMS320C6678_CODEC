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


