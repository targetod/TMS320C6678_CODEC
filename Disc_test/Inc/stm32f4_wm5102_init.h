// stm32_wm5102_init.h

#define ARM_MATH_CM4

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_i2c.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_i2s_ex.h"
#include "arm_math.h"
#include "arm_const_structs.h"

#define WM5102_MIC_IN 0
#define WM5102_LINE_IN 1
#define WM5102_DMIC_IN 2

#define IO_METHOD_INTR 0
#define IO_METHOD_DMA 1
#define IO_METHOD_POLL 2


#define FS_8000_HZ 0x11
//#define FS_11025_HZ 0x09
#define FS_12000_HZ 0x01
#define FS_16000_HZ 0x12
//#define FS_22050_HZ 0x0A
#define FS_24000_HZ 0x02
#define FS_32000_HZ 0x13
//#define FS_44100_HZ 0x0B
#define FS_48000_HZ 0x03


#define PING 1
#define PONG 0

#define BUFSIZE 256

/* I2C clock speed configuration (in Hz)  */
#define I2C_SPEED_WS                        100000

/* I2Sx Communication boards Interface */
#define I2Sx                           SPI2
#define I2Sxext                        I2S2ext
#define I2Sx_CLK                       RCC_APB1Periph_SPI2
#define I2Sx_CLK_INIT                  RCC_APB1PeriphClockCmd

#define I2Sx_WS_PIN                    GPIO_Pin_12
#define I2Sx_WS_GPIO_PORT              GPIOB
#define I2Sx_WS_GPIO_CLK               RCC_AHB1Periph_GPIOB
#define I2Sx_WS_SOURCE                 GPIO_PinSource12
#define I2Sx_WS_AF                     GPIO_AF_SPI2

#define I2Sx_CK_PIN                    GPIO_Pin_13
#define I2Sx_CK_GPIO_PORT              GPIOB
#define I2Sx_CK_GPIO_CLK               RCC_AHB1Periph_GPIOB
#define I2Sx_CK_SOURCE                 GPIO_PinSource13
#define I2Sx_CK_AF                     GPIO_AF_SPI2

#define I2Sx_SD_PIN                    GPIO_Pin_3
#define I2Sx_SD_GPIO_PORT              GPIOC
#define I2Sx_SD_GPIO_CLK               RCC_AHB1Periph_GPIOC
#define I2Sx_SD_SOURCE                 GPIO_PinSource3
#define I2Sx_SD_AF                     GPIO_AF_SPI2

#define I2Sx_MCK_PIN                   GPIO_Pin_6
#define I2Sx_MCK_GPIO_PORT             GPIOC
#define I2Sx_MCK_GPIO_CLK              RCC_AHB1Periph_GPIOC
#define I2Sx_MCK_SOURCE                GPIO_PinSource6
#define I2Sx_MCK_AF                    GPIO_AF_SPI2

#define I2Sxext_SD_PIN                 GPIO_Pin_2
#define I2Sxext_SD_GPIO_PORT           GPIOC
#define I2Sxext_SD_GPIO_CLK            RCC_AHB1Periph_GPIOC
#define I2Sxext_SD_SOURCE              GPIO_PinSource2
#define I2Sxext_SD_AF                  GPIO_AF_SPI3

/* I2C peripheral configuration defines (control interface of the audio codec) */
/* 
#define CODEC_I2C                      I2C1
#define CODEC_I2C_CLK                  RCC_APB1Periph_I2C1
#define CODEC_I2C_GPIO_CLOCK           RCC_AHB1Periph_GPIOB
#define CODEC_I2C_GPIO_AF              GPIO_AF_I2C1
#define CODEC_I2C_GPIO                 GPIOB
#define CODEC_I2C_SCL_PIN              GPIO_Pin_8
#define CODEC_I2C_SDA_PIN              GPIO_Pin_7
#define CODEC_I2C_SCL_PINSRC           GPIO_PinSource8
#define CODEC_I2C_SDA_PINSRC           GPIO_PinSource7
*/

#define CODEC_I2C                      I2C2
#define CODEC_I2C_CLK                  RCC_APB1Periph_I2C2
#define CODEC_I2C_GPIO_CLOCK           RCC_AHB1Periph_GPIOB
#define CODEC_I2C_GPIO_AF              GPIO_AF_I2C2
#define CODEC_I2C_GPIO                 GPIOB
#define CODEC_I2C_SCL_PIN              GPIO_Pin_10
#define CODEC_I2C_SDA_PIN              GPIO_Pin_11
#define CODEC_I2C_SCL_PINSRC           GPIO_PinSource10
#define CODEC_I2C_SDA_PINSRC           GPIO_PinSource11

#define I2C2_ADDRESS      0x33

#define CODEC_FLAG_TIMEOUT             ((uint32_t)0x1000)
#define CODEC_LONG_TIMEOUT             ((uint32_t)(300 * CODEC_FLAG_TIMEOUT))

#define W5102_ADDR_0 0x1a  //  0x1a  << 1 == 0x34 
#define W5102_ADDR_1 0x34  // CIF1ADDR Pull-down Disabled
#define W5102_ADDR_2 0x36  // CIF1ADDR Pull-down enable

#define NOISELEVEL 8000


extern I2C_HandleTypeDef hi2c2;

extern I2S_HandleTypeDef hi2s2;


short prbs(int16_t noise_level);
void DonaldDelay( __IO uint32_t nCount);
uint32_t Codec_TIMEOUT_UserCallback(void);
uint32_t Codec_WriteRegister(uint32_t RegisterAddr, uint16_t RegisterValue);

void stm32_wm5102_init(uint16_t fs, int input_select, int io_method);


static void WM_SystemClock_Config(void);
static void Codec_CtrlInterface_Init(void);
static void WM_I2S2_Init(void);

