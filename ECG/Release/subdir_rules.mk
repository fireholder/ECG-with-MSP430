################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
ADS1x9x.obj: ../ADS1x9x.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"D:/TI/ccsv6/tools/compiler/ti-cgt-msp430_15.12.1.LTS/bin/cl430" -vmspx --data_model=restricted -O2 --use_hw_mpy=F5 --include_path="D:/TI/ccsv6/ccs_base/msp430/include" --include_path="D:/TI/ccsv6/tools/compiler/ti-cgt-msp430_15.12.1.LTS/include" --advice:power=all --define=__MSP430F5229__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="ADS1x9x.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"D:/TI/ccsv6/tools/compiler/ti-cgt-msp430_15.12.1.LTS/bin/cl430" -vmspx --data_model=restricted -O2 --use_hw_mpy=F5 --include_path="D:/TI/ccsv6/ccs_base/msp430/include" --include_path="D:/TI/ccsv6/tools/compiler/ti-cgt-msp430_15.12.1.LTS/include" --advice:power=all --define=__MSP430F5229__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="main.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


