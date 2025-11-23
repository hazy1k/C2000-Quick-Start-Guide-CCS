/*
 * Copyright (c) 2020 Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "board.h"

//*****************************************************************************
//
// Board Configurations
// Initializes the rest of the modules. 
// Call this function in your application if you wish to do all module 
// initialization.
// If you wish to not use some of the initializations, instead of the 
// Board_init use the individual Module_inits
//
//*****************************************************************************
void Board_init()
{
	EALLOW;

	PinMux_init();
	INPUTXBAR_init();
	GPIO_init();
	XINT_init();
	INTERRUPT_init();

	EDIS;
}

//*****************************************************************************
//
// PINMUX Configurations
//
//*****************************************************************************
void PinMux_init()
{
	//
	// PinMux for modules assigned to CPU1
	//
	
	// GPIO27 -> User_KEY Pinmux
	GPIO_setPinConfig(GPIO_27_GPIO27);
	// A17, B17, C17, GPIO20 -> User_LED Pinmux
	GPIO_setPinConfig(GPIO_20_GPIO20);
	// AGPIO -> GPIO mode selected
	GPIO_setAnalogMode(20, GPIO_ANALOG_DISABLED);

}

//*****************************************************************************
//
// GPIO Configurations
//
//*****************************************************************************
void GPIO_init(){
	User_KEY_init();
	User_LED_init();
}

void User_KEY_init(){
	GPIO_setPadConfig(User_KEY, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(User_KEY, GPIO_QUAL_6SAMPLE);
	GPIO_setDirectionMode(User_KEY, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(User_KEY, GPIO_CORE_CPU1);
}
void User_LED_init(){
	GPIO_setPadConfig(User_LED, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(User_LED, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(User_LED, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(User_LED, GPIO_CORE_CPU1);
}

//*****************************************************************************
//
// INPUTXBAR Configurations
//
//*****************************************************************************
void INPUTXBAR_init(){
	myINPUTXBARINPUT0_init();
}

void myINPUTXBARINPUT0_init(){
	XBAR_setInputPin(INPUTXBAR_BASE, myINPUTXBARINPUT0_INPUT, myINPUTXBARINPUT0_SOURCE);
}

//*****************************************************************************
//
// INTERRUPT Configurations
//
//*****************************************************************************
void INTERRUPT_init(){
	
	// Interrupt Settings for INT_User_KEY_XINT
	// ISR need to be defined for the registered interrupts
	Interrupt_register(INT_User_KEY_XINT, &INT_User_KEY_XINT_ISR);
	Interrupt_enable(INT_User_KEY_XINT);
}
//*****************************************************************************
//
// XINT Configurations
//
//*****************************************************************************
void XINT_init(){
	User_KEY_XINT_init();
}

void User_KEY_XINT_init(){
	GPIO_setInterruptType(User_KEY_XINT, GPIO_INT_TYPE_FALLING_EDGE);
	GPIO_setInterruptPin(User_KEY, User_KEY_XINT);
	GPIO_enableInterrupt(User_KEY_XINT);
}

