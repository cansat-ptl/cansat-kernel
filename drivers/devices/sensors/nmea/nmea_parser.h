/*
 * nmea_parser.h
 *
 * Created: 11.06.2019 22:43:33
 *  Author: bear1ake
 */ 

#include "../../../../kernel/types.h"
#include <stdlib.h>

char rmc_str[128];
unsigned char rmc_index;
unsigned char field_index;
unsigned char packet_type; 
 // $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
 
struct GPS_t
  {
	  unsigned char valid;
	  unsigned char hour;
	  unsigned char minute;
	  unsigned char second;
	  unsigned char day;
	  unsigned char month;
	  unsigned char year;
	  unsigned int  millisecond;
	  unsigned int cource;
	  float latitude;
	  unsigned char P;
	  float longitude;
	  float speed;
	  float vspeed;
	  unsigned char J;
	  unsigned char Sats;
  };
 
volatile struct GPS_t GPS;

float convertToDecimal(float lat);
unsigned char number_code(char chr);
unsigned char compare(char * a, char * b,unsigned char  len);
unsigned char nmea_load(char data);