/*
 * imuv3.c
 *
 * Created: 11.06.2019 22:43:33
 *  Author: bear1ake
 */ 
// L3GD20H & LSM303D
// minIMU v3

#ifndef _IMUV3_H
#define _IMUV3_H

//#include "../../drivers/twi.h"

#define LSM_adr		0x3B
#define LSM_A_xyz	0x28
#define LSM_M_xyz	0x08
#define L3GD_adr	0xD7
#define L3GD_xyz	0x28

volatile struct LSM_t
{
	unsigned char XH_A;
	unsigned char XL_A;
	unsigned char YH_A;
	unsigned char YL_A;
	unsigned char ZH_A;
	unsigned char ZL_A;
	unsigned char XH_M;
	unsigned char XL_M;
	unsigned char YH_M;
	unsigned char YL_M;
	unsigned char ZH_M;
	unsigned char ZL_M;
} LSM;

volatile struct L3GD_t
{
	unsigned char XH1;
	unsigned char XL1;
	unsigned char YH1;
	unsigned char YL1;
	unsigned char ZH1;
	unsigned char ZL1;
} L3GD;

void lsm_init();
void l3gd_init();
void l3gd_read();
void lsm_a_read();
void lsm_m_read();
void imu_init();
void imu_read();

#endif