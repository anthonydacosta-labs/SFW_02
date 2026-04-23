/*
 * vnf1248f.h
 *
 *  Created on: Apr 16, 2026
 *      Author: AnthonyDaCosta
 */

#ifndef DRIVERS_VNF1248F_H_
#define DRIVERS_VNF1248F_H_

#endif /* DRIVERS_VNF1248F_H_ */


/* Includes ------------------------------------------------------------------*/
#include "main.h"

//#define VOC_THRS 0b001 		/// NEEDS TO BE ADJUSTED
//#define VOC_ADDR 0x50		/// NEEDS TO BE ADJUSTED

#define CR1_NVM_MASK 0x01F008  // all bits = 0 are not mirrored in NVM
#define CR2_NVM_MASK 0xFFFFFC
#define CR3_NVM_MASK 0xCDCDF8
#define CR5_NVM_MASK 0xFFFFFC

//#define CR1_CONFIG 0x018000		// NVM_DEF_CFG_EN=1, FS_MODE=10 (no change)
//#define CR1_CONFIG 0x008000		// NVM_DEF_CFG_EN=0, FS_MODE=10 (no change)
#define CR1_CONFIG 0x008400		// NVM_DEF_CFG_EN=0, FS_MODE=10 (no change)   -- set EN to 1 (only effective if preceded by UNLOCK)
#define CR2_CONFIG 0x977401		// T_NOM=280s, OVC_THR=23.3mV, HSC_THR=60.6mV, VDS_THR=300mV
#define CR3_CONFIG 0x000901		// UV_THR=12V, NTC_THR=37.5mV (~150°C)
#define CR5_CONFIG 0xC00001		// CCM_TIMEOUT=400ms  -- TO BE ADJUSTED

#define CR1_ADDR 0x01
#define CR2_ADDR 0x02
#define CR3_ADDR 0x03
#define CR4_ADDR 0x04
#define CR5_ADDR 0x05

#define SR1_ADDR 0x11
#define SR2_ADDR 0x12
#define SR3_ADDR 0x13
#define SR4_ADDR 0x14
#define SR5_ADDR 0x15
#define SR6_ADDR 0x16
#define SR7_ADDR 0x17
#define SR8_ADDR 0x18

uint8_t VNF_TransmitReceive(SPI_HandleTypeDef *hspi, GPIO_TypeDef *CS_GPIOx, uint16_t CS_Pin, uint8_t *TxBuf, uint8_t *RxBuf);

void Initialize_vnf(SPI_HandleTypeDef *hspi, GPIO_TypeDef *CS_GPIOx, uint16_t CS_Pin);

uint8_t ReadNVM_vnf(SPI_HandleTypeDef *hspi, GPIO_TypeDef *CS_GPIOx, uint16_t CS_Pin, uint8_t CRx, uint8_t *result);
