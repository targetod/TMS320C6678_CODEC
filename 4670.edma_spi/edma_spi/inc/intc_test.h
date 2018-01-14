/*
 * intc_test.h
 *
 *  Created on: 4 џэт. 2018 у.
 *      Author: Alex
 */

#ifndef INC_INTC_TEST_H_
#define INC_INTC_TEST_H_


#include <ti/csl/src/intc/csl_intc.h>
#include <ti/csl/src/intc/csl_intcAux.h>

#include <ti/csl/csl_cpIntc.h>
#include <ti/csl/csl_cpIntcAux.h>

#define FIRST_QPEND_CIC0_OUT_EVENT 56


void EDMAInterruptHandler (void *arg);
Int32 intc_init (void);
Int32 set_edma_intc ();
void close_intc();

#endif /* INC_INTC_TEST_H_ */
