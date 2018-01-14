################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
intc_test.obj: ../intc_test.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccs720/ti-cgt-c6000_8.1.3/bin/cl6x" -mv6600 --abi=eabi --include_path="C:/ti/ccs720/ti-cgt-c6000_8.1.3/include" --include_path="C:/ti/ccs720/pdk_c667x_2_0_5/packages" --include_path="../inc" -g --define=SOC_C6678 --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="intc_test.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main4my.obj: ../main4my.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccs720/ti-cgt-c6000_8.1.3/bin/cl6x" -mv6600 --abi=eabi --include_path="C:/ti/ccs720/ti-cgt-c6000_8.1.3/include" --include_path="C:/ti/ccs720/pdk_c667x_2_0_5/packages" --include_path="../inc" -g --define=SOC_C6678 --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="main4my.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

spi_test.obj: ../spi_test.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ccs720/ti-cgt-c6000_8.1.3/bin/cl6x" -mv6600 --abi=eabi --include_path="C:/ti/ccs720/ti-cgt-c6000_8.1.3/include" --include_path="C:/ti/ccs720/pdk_c667x_2_0_5/packages" --include_path="../inc" -g --define=SOC_C6678 --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="spi_test.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


