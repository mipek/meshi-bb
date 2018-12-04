#ifndef _include_i2cmeshiprotocol_h_
#define _include_i2cmeshiprotocol_h_

#include <stdint.h>

typedef enum
{
	kPacketID_Enumerate=0,
	kPacketID_GetValue,
	kPacketID_UnknownPacket,
} PacketID;

/**
 * Known sensor types
 */
typedef enum
{
	kSensorType_Temperature = 1,
	kSensorType_Gas = 2
} SensorType;

#endif //_include_i2cmeshiprotocol_h_