/*
 * spi_common2.h
 *
 *  Created on: 29 дек. 2017 г.
 *      Author: Alex
 */

#ifndef INC_SPI_COMMON2_H_
#define INC_SPI_COMMON2_H_



#pragma DATA_SECTION(hModule,".testData");
CSL_Edma3Handle hModule;

#pragma DATA_SECTION(paramHandle0,".testData");
CSL_Edma3ParamHandle paramHandle0;

#pragma DATA_SECTION(paramHandle1,".testData");
CSL_Edma3ParamHandle paramHandle1;

#pragma DATA_SECTION(paramHandle0_reload,".testData");
CSL_Edma3ParamHandle paramHandle0_reload;

#pragma DATA_SECTION(paramHandle1_reload,".testData");
CSL_Edma3ParamHandle paramHandle1_reload;


#pragma DATA_SECTION(chParam,".testData");
CSL_Edma3ChannelAttr  chParam;

#pragma DATA_SECTION(ChObj0,".testData");
#pragma DATA_SECTION(ChObj1,".testData");
CSL_Edma3ChannelObj  ChObj0,ChObj1;

#pragma DATA_SECTION(hChannel0,".testData");
#pragma DATA_SECTION(hChannel1,".testData");
CSL_Edma3ChannelHandle  hChannel0,hChannel1;

#pragma DATA_SECTION(chSetup,".testData");
CSL_Edma3HwDmaChannelSetup chSetup;

#pragma DATA_SECTION(paramSetup,".testData");
CSL_Edma3ParamSetup paramSetup;

#pragma DATA_SECTION(moduleObj,".testData");
CSL_Edma3Obj  moduleObj;

#pragma DATA_SECTION(regionIntr, ".testData");
CSL_Edma3CmdIntr regionIntr,regionIpr;
CSL_Status EdmaStat, handleStatus;

#define CSL_EDMA3_CHA_2  2
#define CSL_EDMA3_CHA_3  3
#define CSL_EDMA3_CHA_30  30
#define CSL_EDMA3_CHA_31  31

#define CSL_EDMA3_CHA_32  32
#define CSL_EDMA3_CHA_33  33

#define EDMA_TEST_PASS 1
#define EDMA_TEST_FAIL 0

/*
void Setup_Edma(Uint32 srcBuf,Uint32 dstBuf, Uint32 TEST_ACNT, Uint32 TEST_BCNT, Uint32 TEST_CCNT);
Bool Verify_Transfer(Uint16, Uint16, Uint16, Int16, Int16, Int16, Int16,Uint32,Uint32, Bool);
void Test_Edma();
void Close_Edma();
void Trigger_Edma_Channels();
*/





#endif /* INC_SPI_COMMON2_H_ */
