/*
 * vnf1248f.c
 *
 *  Created on: Apr 16, 2026
 *      Author: AnthonyDaCosta
 */

#include "stdint.h"
#include "vnf1248.h"

#include "main.h"


uint8_t VNF_TransmitReceive(SPI_HandleTypeDef *hspi, GPIO_TypeDef *CS_GPIOx, uint16_t CS_Pin, uint8_t *TxBuf, uint8_t *RxBuf)
{
	uint32_t k=0;
	uint8_t tmp=0;
	uint8_t tmp2=0;
	static uint8_t WDbit=0;

	/*
	if (~(TxBuf[0]&0b11000000)) // if write operation, flip the WD bit -- ugly way to pet WD while TIM2 IRQ is disabled
	{
		if(WDbit)
			TxBuf[3]|=0b10;
		else
		 	TxBuf[3]&=!(0b10);
		WDbit^=0b1;
	}
		*/

	// correct parity bit
	tmp = TxBuf[0]^TxBuf[1]^TxBuf[2]^TxBuf[3];
	tmp2=tmp;
	for (k=0;k<7;k++)
	{
		tmp2>>=1;
		tmp^=tmp2;
	}
	tmp = 0x01&(~tmp);
	TxBuf[3] ^= tmp;

	HAL_NVIC_DisableIRQ(TIM2_IRQn);	// avoid conflicts (this is dirty)

	//HAL_Delay(1);
	// ugly short delay
	k=0;
	while(k<10000)
		k++;
	
	HAL_GPIO_WritePin(CS_GPIOx, CS_Pin, GPIO_PIN_RESET);
	//HAL_Delay(1); // rough -- we could do better, but a delay is needed (especially when the VNF is in stdby)
	// ugly short delay
	k=0;
	while(k<10000)
		k++;
	HAL_SPI_TransmitReceive(hspi, TxBuf, RxBuf, 4, 100);
	//HAL_Delay(1);
	// ugly short delay
	k=0;
	while(k<10000)
		k++;
	HAL_GPIO_WritePin(CS_GPIOx, CS_Pin, GPIO_PIN_SET);
	//HAL_Delay(1);
	// ugly short delay
	k=0;
	while(k<10000)
		k++;

	HAL_NVIC_EnableIRQ(TIM2_IRQn);

	return(0); // TODO: add error handling
}

void Initialize_vnf(SPI_HandleTypeDef *hspi, GPIO_TypeDef *CS_GPIOx, uint16_t CS_Pin)
{
	uint8_t TxBuf[4];
	uint8_t RxBuf[4];
	//uint32_t k=0;
	uint8_t r=0;
	uint32_t cfg_word = 0;

	TxBuf[0] = 0xFF; // 0xFFFF --> device reset
	TxBuf[1] = 0b00000000;
	TxBuf[2] = 0b00000000;
	TxBuf[3] = 0b00000001;
	VNF_TransmitReceive(hspi, CS_GPIOx, CS_Pin, TxBuf, RxBuf);

	TxBuf[0] = 0xBF; // 0xBFFF --> register reset (may be unnecessary)
	TxBuf[1] = 0b00000000;
	TxBuf[2] = 0b00000000;
	TxBuf[3] = 0b00000000;
	VNF_TransmitReceive(hspi, CS_GPIOx, CS_Pin, TxBuf, RxBuf);

	TxBuf[0] = CR3_ADDR; // write cmd to CR3
	TxBuf[1] = 0b00000000;
	TxBuf[2] = 0b00000010; // 0x04 --> set UNLOCK bit to 1  -- MUST do CR1 after this to set EN bit
	TxBuf[3] = 0b00000000;
	VNF_TransmitReceive(hspi, CS_GPIOx, CS_Pin, TxBuf, RxBuf);

	for(r=0;r<4;r++)
	{
		switch(r)
		{
			case 0:
				TxBuf[0] = CR1_ADDR;
				cfg_word = CR1_CONFIG;
				break;
			case 1:
				TxBuf[0] = CR2_ADDR;
				cfg_word = CR2_CONFIG;
				break;
			case 2:
				TxBuf[0] = CR3_ADDR;
				cfg_word = CR3_CONFIG;
				break;
			default:
				TxBuf[0] = CR5_ADDR;
				cfg_word = CR5_CONFIG;
				break;
		}
	
		TxBuf[1] = (cfg_word&0x00FF0000)>>16;
		TxBuf[2] = (cfg_word&0x0000FF00)>>8;
		TxBuf[3] = (cfg_word&0x000000FF);
		VNF_TransmitReceive(hspi, CS_GPIOx, CS_Pin, TxBuf, RxBuf);
		// TODO: add check
	}
	return;
}

void PetWD_vnf(SPI_HandleTypeDef *hspi, GPIO_TypeDef *CS_GPIOx, uint16_t CS_Pin)
{
	uint8_t TxBuf[4];
	uint8_t RxBuf[4];
	uint8_t tmp=0;
	uint32_t k=0;

	// read CR3
	TxBuf[0] = 0b01000000|CR3_ADDR; // 0b01 = read command
	TxBuf[1] = 0b00000000;
	TxBuf[2] = 0b00000000;
	TxBuf[3] = 0b00000000;
	VNF_TransmitReceive(hspi, CS_GPIOx, CS_Pin, TxBuf, RxBuf);
	// TODO: add error processing
	tmp = (RxBuf[3]&0b10)>>1;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, tmp?GPIO_PIN_SET:GPIO_PIN_RESET); // WD value --> green
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, (RxBuf[0]&0x1)?GPIO_PIN_SET:GPIO_PIN_RESET); // fail-state --> red
	TxBuf[0] = CR3_ADDR; // 0b00 = write command
	for(k=1;k<4;k++)
	{
		TxBuf[k]=RxBuf[k];
	}
	TxBuf[3]^=0b10; // flip WD bit (tot. 2 bits flipped --> don't flip parity bit)
	VNF_TransmitReceive(hspi, CS_GPIOx, CS_Pin, TxBuf, RxBuf);
	// TODO: add error processing

	/*
	k=0;
	while(k<10000)
		k++;
	
	HAL_GPIO_WritePin(CS_GPIOx, CS_Pin, GPIO_PIN_RESET);
	//HAL_Delay(1); // rough -- we could do better, but a delay is needed (especially when the VNF is in stdby)
	// ugly short delay
	k=0;
	while(k<10000)
		k++;
	HAL_SPI_TransmitReceive(hspi, TxBuf, RxBuf, 4, 100);
	//HAL_Delay(1);
	// ugly short delay
	k=0;
	while(k<10000)
		k++;
	HAL_GPIO_WritePin(CS_GPIOx, CS_Pin, GPIO_PIN_SET);
	//HAL_Delay(1);
	// ugly short delay
	k=0;
	while(k<10000)
		k++;
	*/

}


int8_t RW_NVM_vnf(SPI_HandleTypeDef *hspi, GPIO_TypeDef *CS_GPIOx, uint16_t CS_Pin, uint8_t R_nW)
{
	uint8_t TxBuf[4];
	uint8_t RxBuf[4];
	uint8_t tmp=0;
	uint8_t tmp2=0;
	uint8_t k=0;
	uint8_t error = 0;

	// read CR3
	TxBuf[0] = 0b01000000|CR3_ADDR; // 0b01 = read command
	TxBuf[1] = 0b00000000;
	TxBuf[2] = 0b00000000;
	TxBuf[3] = 0b00000000;
	VNF_TransmitReceive(hspi, CS_GPIOx, CS_Pin, TxBuf, RxBuf);
	// Note: should check GSB ??

	// now flip 'UNLOCK' bit and write back
	TxBuf[0] = 0b00000000|CR3_ADDR; // 0b00 = write command
	TxBuf[1] = RxBuf[1];
	TxBuf[2] = RxBuf[2]|0b00000010;
	TxBuf[3] = RxBuf[3]^0x02; // pet WD
	VNF_TransmitReceive(hspi, CS_GPIOx, CS_Pin, TxBuf, RxBuf);

	// now the VNF is unlocked, send the NVM access key
	// datasheet is unclear about writing 105B96, 416E58, or 416E59
	TxBuf[0] = 0b00000000|CR4_ADDR; // 0b00 = write command
	TxBuf[1] = 0x41;
	TxBuf[2] = 0x6E;
	TxBuf[3] = 0x58|((TxBuf[3]^0x02)&0x02); // pet WD at the same time
	VNF_TransmitReceive(hspi, CS_GPIOx, CS_Pin, TxBuf, RxBuf);
	// CR4 should now be unlocked

	// now write sector address, access type and start command in CR4
	TxBuf[0] = 0b00000000|CR4_ADDR; // 0b00 = write command
	TxBuf[1] = 0x00;
	TxBuf[2] = 0x05;
	TxBuf[3] ^= 0x02; // pet WD
	TxBuf[3] &= 0x02;
	if (R_nW==0)
		TxBuf[3] |= 0x08; // write		/////// 0x0C in one step?
	else
	 	TxBuf[3] |= 0x00; // read		/////// 0x04 in one step?
	//TxBuf[3] |= 0x04;	// start
	VNF_TransmitReceive(hspi, CS_GPIOx, CS_Pin, TxBuf, RxBuf);

	TxBuf[3] ^= 0x02; // pet WD
	TxBuf[3] |= 0x04;	// start
	VNF_TransmitReceive(hspi, CS_GPIOx, CS_Pin, TxBuf, RxBuf);


	// now poll SR2 and check for NVM status
	tmp=1;
	error=0;
	while(tmp)
	{
		TxBuf[0] = 0b01000000|SR2_ADDR; // 0b01 = read command
		TxBuf[1] = 0b00000000;
		TxBuf[2] = 0b00000000;
		TxBuf[3] ^= 0x02; // pet WD
		TxBuf[3] &= 0x02;
		VNF_TransmitReceive(hspi, CS_GPIOx, CS_Pin, TxBuf, RxBuf);

		if (RxBuf[1]&0b01111110)
		{
			tmp=0;	// stop polling
			error=1; // NVM error
		}
		else
		{
			tmp2 = ((RxBuf[1]&0x01)<<1) + ((RxBuf[2]&0x80)>>7);
			switch(tmp2)
			{
				case 0b00:
					tmp=0;
					error=-1;
					break;
				case 0b10:
					tmp=0;
					break;
				case 0b11:
					tmp=0;
					error=1;
					break;
				default:
					break;
			}
		}
	}

	return(error);
}
