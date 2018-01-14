

//	// Receive buffer not empty flag
//		while( __HAL_I2S_GET_FLAG(&hi2s2, I2S_FLAG_RXNE ) != SET) { }
//		
//		// Channel Side flag  check
//    if (__HAL_I2S_GET_FLAG(&hi2s2, I2S_FLAG_CHSIDE) == SET) 
//    {
//			
//			do{
//				 status =	HAL_I2S_Receive(&hi2s2, (uint16_t*) &left_in_sample, 1, CODEC_LONG_TIMEOUT);
//				left_out_sample = 0;
//				//status =	HAL_I2SEx_TransmitReceive(&hi2s2, (uint16_t *) &left_out_sample,  (uint16_t*) &left_in_sample, 1, CODEC_LONG_TIMEOUT);
//				
//				if ( status != HAL_OK) {
//				/* Check error */
//					err=	HAL_I2S_GetError(&hi2s2);
//				}
//			}while(status != HAL_OK);

//      left_out_sample = left_in_sample;
//			
//      //Transmit buffer empty flag
//			while( __HAL_I2S_GET_FLAG(&hi2s2, I2S_FLAG_TXE ) != SET) {}

//				
//			do{
//				status =	HAL_I2S_Transmit (&hi2s2, (uint16_t *) &left_out_sample, 1, CODEC_LONG_TIMEOUT);
//				left_in_sample = 0;
//				//status =	HAL_I2SEx_TransmitReceive (&hi2s2, (uint16_t *) &left_out_sample,(uint16_t*) &left_in_sample, 1, CODEC_LONG_TIMEOUT);
//				if ( status != HAL_OK) {
//				/* Check error */
//					err=	HAL_I2S_GetError(&hi2s2);
//				}
//			}while(status != HAL_OK);
//			

//		}
//    else{
//			
//			do{
//				status =	HAL_I2S_Receive(&hi2s2, (uint16_t*) &right_in_sample, 1, CODEC_LONG_TIMEOUT);
//				right_out_sample = 0;
//				//status = HAL_I2SEx_TransmitReceive(&hi2s2, (uint16_t *) &right_out_sample,  (uint16_t*) &right_in_sample, 1, CODEC_LONG_TIMEOUT);
//				if ( status != HAL_OK) {
//				/* Check error */
//					err=	HAL_I2S_GetError(&hi2s2);
//				}
//			}while(status != HAL_OK);
//			
//		
//			
//      right_out_sample = right_in_sample;
//      
//			while( __HAL_I2S_GET_FLAG(&hi2s2, I2S_FLAG_TXE ) != SET){}

//			do{
//				right_in_sample = 0;
//				status =	HAL_I2S_Transmit (&hi2s2, (uint16_t *) &right_out_sample, 1, CODEC_LONG_TIMEOUT);
//				//status =	HAL_I2SEx_TransmitReceive (&hi2s2, (uint16_t *) &right_out_sample,(uint16_t*) &right_in_sample, 1, CODEC_LONG_TIMEOUT);
//				if ( status != HAL_OK) {
//				/* Check error */
//					err=	HAL_I2S_GetError(&hi2s2);
//				}
//			}while(status != HAL_OK);
//			
//		}