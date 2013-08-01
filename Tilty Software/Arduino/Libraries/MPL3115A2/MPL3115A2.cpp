/*
MPL3115A2.cpp - A easy to use interface for the MPL3115A2 Altimeter from Freescale Semiconductor
Copyright (C) 2013-2014 Alex Beattie <alexbeattie at tiltyimu dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the version 2 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//#include "WProgram.h"
//#include "Arduino.h"
#include "MPL3115A2.h"
#include <stdint.h>
//#include "i2c_t3.h"

MPL3115A2::MPL3115A2()
{
	//	Do nothing
}



//	Checks to see that the altimeter is connected and functional
//	and sets up the control registers
boolean MPL3115A2::init()
{	
	if (readByte(0x0C) == 196)
	{  
		// CTRL_REG1 (0x26): enable sensor, oversampling x128, altimeter mode 
  		write(0x26, 0xB9);
  		// CTRL_REG4 (0x29): Data ready interrupt enbabled
 		write(0x29, 0x80);
 		// PT_DATA_CFG (0x13): enable both pressure and temp event flags 
		write(0x13, 0x07);
		
		// Use these to set custom sea level pressures (Pressure in pascals at sea level / 2)
		//write(0x14, 0xC6);// BAR_IN_MSB (0x14):
		//write(0x15, 0x5B);// BAR_IN_LSB (0x15):
		
		return true;
	}
	else
	{	return false;}
}



//  Resets the altimeter via software
void MPL3115A2::reset()
{
	write(0x26, 0x04);
}



//  Checks INT_SOURCE register to see if new data is available
boolean MPL3115A2::checkData()
{
	if(readByte(0x12) & 0x80) 
	{	return true;}
	else 
	{	return false;}
}



//	Reads the current altitude in meters
float MPL3115A2::readAltitudeM()
{
	int u_altitude, m_altitude;
	float l_altitude;
	
	readBytes(0x01, 3, _buffer);
	
	u_altitude = _buffer[0] << 8;//  The upper 8 bits of the altitude
	m_altitude = _buffer[1];//  The middle 8 bits of the altitude
	l_altitude = float(_buffer[2] >> 4) / 16.0;//  The lower 4 bits of the altitude
	
	int16_t _temp = u_altitude | m_altitude;
	
	if (_temp < 0)
	{
		return float(_temp) - l_altitude;
	}
	else
	{
		return float(_temp) + l_altitude;
	}
}



//	Reads the current altitude in feet
float MPL3115A2::readAltitudeF() 
{
	return 3.381 * readAltitudeM();
}



//	Reads the current temperature in degrees C
float MPL3115A2::readTempC()
{
	int8_t u_temp;
	float l_temp;
	
	readBytes(0x04, 2, _buffer);

	u_temp = _buffer[0];//  Upper 8 bits of the temperature, representing the numbers before the decimal
	l_temp = float(_buffer[1] >> 4) / 16.0;//  Lower 4 bits of the temperature, representing the numbers 
	
	return float(u_temp) + l_temp;
}



//	Read the current temperature in degrees F
float MPL3115A2::readTempF() 
{
	return (readTempC() * 9) / 5.0 + 32;
}



//	Reads a byte on the sensor from the given address
byte MPL3115A2::readByte(byte _regAddr)
{
	Wire.beginTransmission(_addr);
	Wire.write(_regAddr);
	Wire.endTransmission(I2C_NOSTOP);
	Wire.requestFrom(_addr, 1, I2C_STOP, _timeout); // Request the data...
	return Wire.read();
}



//	Writes a byte of data to the sensor at the given address
byte MPL3115A2::write(byte _regAddr, byte _value)
{
	Wire.beginTransmission(_addr);
	Wire.write(_regAddr);
	Wire.write(_value);
	return Wire.endTransmission(I2C_STOP);
}



//	Reads consecutive bytes into a given data buffer
void MPL3115A2::readBytes(byte _regAddr, uint8_t _length, uint8_t *_data)
{
	Wire.beginTransmission(_addr);
	Wire.write(_regAddr);
	Wire.endTransmission(I2C_NOSTOP);
	Wire.requestFrom(_addr, _length, I2C_STOP, _timeout); // Request the data...
	
	for (int _i = 0; _i < _length; i++)
	{
		_data[_i] = Wire.read();
	}
}

