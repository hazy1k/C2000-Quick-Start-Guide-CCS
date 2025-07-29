################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
device/%.obj: ../device/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"E:/TI/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla2 --float_support=fpu32 --tmu_support=tmu1 --vcu_support=vcrc -Ooff --include_path="E:/StudyNote/study_C2000/2.code/9.SPI" --include_path="E:/TI/sdk/C2000Ware_5_05_00_00" --include_path="E:/StudyNote/study_C2000/2.code/9.SPI/device" --include_path="E:/TI/sdk/C2000Ware_5_05_00_00/driverlib/f28p55x/driverlib/" --include_path="E:/TI/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS/include" --include_path="E:/StudyNote/study_C2000/2.code/9.SPI/BSP" --define=DEBUG --define=_FLASH --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="device/$(basename $(<F)).d_raw" --include_path="E:/StudyNote/study_C2000/2.code/9.SPI/CPU1_FLASH/syscfg" --obj_directory="device" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

device/%.obj: ../device/%.asm $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"E:/TI/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla2 --float_support=fpu32 --tmu_support=tmu1 --vcu_support=vcrc -Ooff --include_path="E:/StudyNote/study_C2000/2.code/9.SPI" --include_path="E:/TI/sdk/C2000Ware_5_05_00_00" --include_path="E:/StudyNote/study_C2000/2.code/9.SPI/device" --include_path="E:/TI/sdk/C2000Ware_5_05_00_00/driverlib/f28p55x/driverlib/" --include_path="E:/TI/ccs/tools/compiler/ti-cgt-c2000_22.6.2.LTS/include" --include_path="E:/StudyNote/study_C2000/2.code/9.SPI/BSP" --define=DEBUG --define=_FLASH --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="device/$(basename $(<F)).d_raw" --include_path="E:/StudyNote/study_C2000/2.code/9.SPI/CPU1_FLASH/syscfg" --obj_directory="device" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


