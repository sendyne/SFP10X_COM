/*
 
 Copyright 2015-2016 Sendyne Corp., New York, USA
 http://www.sendyne.com
 
 C library for FTDI-based serial communication with Sendyne's SFP products.
 
 Authors:
    Damian Glinojecki (Sendyne Corp.)
    Nicolas Clauvelin (Sendyne Corp.)
 
 File:
    SFP10X_COM.c
 
 THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 
*/


#include "SFP10X_COM.h"
#include <stdint.h>


// Default timeout value.
// Used when initializing the communication.
#define DEFAULT_TIMEOUT 20

// Default baudrate.
// Used when initializing the communication.
#define DEFAULT_BAUDRATE 19200


// CRC function declaration.
byte CRC(int, const byte * const);


// Flag lookup function.
char* FlagLookup(byte flag)
{
    switch (flag)
    {
        case RESERVED:
            // 0x00 - Reserved for future use
            return "RESERVED";
            break;
        case SFP_OK:
            // 0x01 - Success return code
            return "SFP_OK";
            break;
        case PORT_FAIL:
            // 0x02 - Port has failed to open or close
            return "PORT_FAIL";
            break;
        case BAUD_FAIL:
            // 0x03 - Changing baudrate failed
            return "BAUD_FAIL";
            break;
        case DATA_CH_FAIL:
            // 0x04 - Port charactarization
            // stop bits, partity, etc failed to set
            return "DATA_CH_FAIL";
            break;
        case WRITE_FAIL:
            // 0x05 - Write to the SFP module has failed
            return "WRITE_FAIL";
            break;
        case READ_FAIL:
            // 0x06 - Reading the SFP module failed
            return "READ_FAIL";
            break;
        case CRC_ERROR:
            // 0x07	- CRC did not pass
            return "CRC_ERROR";
            break;
        case BYTES_INVALID:
            // 0x08 - Returned number of bytes did not match the expected number
            // of bytes
            return "BYTES_INVALID";
            break;
        case RESPONSE_TIMEOUT:
            // 0x09 - Response not recieved within the allowed time frame
            return "RESPONSE_TIMEOUT";
            break;
        case DEVICE_BUSY:
            // 0x0A - The requested device is taken or in an unknown state
            return "DEVICE_BUSY";
            break;
        case FT_LIST_FAIL:
            // 0x0B - Failed getting the number of devices connected to the
            // system
            return "FT_LIST_FAIL";
            break;
        case MEM_FAIL:
            // 0x0C - Memory allocation error.
            return "MEM_FAIL";
        default:
            return "FLAG NOT FOUND";
    }
}


// Initializes the communication with a specified device number.
byte Initialize(int device_num, SFPDevice * sfp_dev)
{
    
    // Check that the SFPDevice pointer points to an allocated structure.
    if (sfp_dev == NULL) 
		return MEM_FAIL;
    
	// Initialize the struct.
	sfp_dev->sfp_device_num = device_num;
	sfp_dev->sfp_handle = NULL;

	// Open the FTDI id that the user requested.
	FT_STATUS rc = FT_Open(sfp_dev->sfp_device_num, &sfp_dev->sfp_handle);

	// Check status.
	if (rc != FT_OK)
	{
		// Failed - clean up just in case, and close port.
		FT_Close(sfp_dev->sfp_handle);
		sfp_dev->sfp_device_num = -99;
		return PORT_FAIL;
	}
    
    // Successful opening, we proceed with setting up the connection.
	else
	{
		// Set baudrate to 19200 - SFP default.
		rc = FT_SetBaudRate(sfp_dev->sfp_handle, DEFAULT_BAUDRATE);
		if (!rc == FT_OK)
		{
			// Failed to set the baudrate.
			FT_Close(sfp_dev->sfp_handle);
			return BAUD_FAIL;
		}

		// Set parity bits (8 data bits, 1 stop bit and no parity).
		rc = FT_SetDataCharacteristics(sfp_dev->sfp_handle, FT_BITS_8, FT_STOP_BITS_1,
			FT_PARITY_NONE);
		if (rc != FT_OK)
		{
			// Failed to set the parity bits.
			FT_Close(sfp_dev->sfp_handle);
			return DATA_CH_FAIL;
		}

		// Set the timeout for the FTDI read and write.
		FT_SetTimeouts(sfp_dev->sfp_handle, DEFAULT_TIMEOUT, DEFAULT_TIMEOUT);

	}

	// Port has been opened and set up.
	return SFP_OK;

}
#undef DEFAULT_TIMEOUT
#undef DEFAULT_BAUDRATE


// Changes the timeout time for write and read to and from the device.
byte ChangeTimeout(SFPDevice device, int time_ms)
{
	// Change the timeout in ms.
	FT_STATUS rc = FT_SetTimeouts(device.sfp_handle, time_ms, time_ms);
    
    // Check the status and return accordingly.
	if (rc == FT_OK)
		return SFP_OK;
	else
		return PORT_FAIL;
    
}


// Reads data from a specific register on the SFP module.
byte ReadRegister(SFPDevice device,
                  byte SFP_reg_address,
                  byte number_of_bytes,
                  char * const data)
{
    
    // Check that the data array is properly allocated.
    if (data == NULL) 
        return MEM_FAIL;

	// Construct the message for sending.
    char packet[10] = { 0 };

	// Mode - read orred with the number of bytes requested.
	packet[0] = 0x80 | number_of_bytes;

	// Register address.
	packet[1] = SFP_reg_address;

	// Number of bytes written to the wire.
	DWORD bytes_written = 0;

    // Determine how many bytes we are expecting back.
	int bytes_expected;
	switch (number_of_bytes)
	{
	case BYTES_1:
		bytes_expected = 3;     // 1 data byte, 1 status, and 1 crc
		break;
	case BYTES_2:
		bytes_expected = 4;     // 2 data byte, 1 status, and 1 crc
		break;
	case BYTES_3:
		bytes_expected = 5;     // 3 data byte, 1 status, and 1 crc
		break;
	case BYTES_6:
		bytes_expected = 8;     // 6 data byte, 1 status, and 1 crc
		break;
	default:
        return BYTES_INVALID;   // error
	}

	// Recieve buffer.
    byte m_rx_buffer[10] = { 0 };
    
	// Number of bytes recieved.
    DWORD m_bytes_received = 0;

	// Write the request on the line.
	FT_STATUS rc = FT_Write(device.sfp_handle, &packet, 2, &bytes_written);
	if (rc == FT_OK)
	{
        
		// Clean the rx_buffer before writing to it.
		for (int i = 0; i < 10; i++)
			m_rx_buffer[i] = '\0';

		// Wait for the answer from the SFP module.
		rc = FT_Read(device.sfp_handle, m_rx_buffer, bytes_expected,
                     &m_bytes_received);
		if (rc == FT_OK)
		{
			// Save the data into the users buffer.
			for (int i = 0; i < bytes_expected; i++)
				data[i] = m_rx_buffer[i];

			// Check to make sure we have the expected number of bytes.
			if (m_bytes_received != bytes_expected)
				return RESPONSE_TIMEOUT;
		}
		else
			// Read was unsucessful.
			return READ_FAIL;
	}
	else
		// Failed writing to the wire.
		return WRITE_FAIL;

	// Put the data in a packet array.
	for (int i = 2; i < bytes_expected + 2; i++)
		packet[i] = m_rx_buffer[i - 2];

	// Run a crc on the data.
	if (CRC(bytes_expected + 2, (byte *)packet) == 0x00)
	{
		// Save the data into the users buffer.
		for (int i = 0; i < bytes_expected; i++)
			data[i] = m_rx_buffer[i];
		return SFP_OK;
	}
	else
	{
		// Something went wrong, clear the buffers.
        // Purge both Rx and Tx buffers.
		rc = FT_Purge(device.sfp_handle, FT_PURGE_RX | FT_PURGE_TX);

		// Return crc error.
		return CRC_ERROR;
	}
    
    // We should not arrive here.
    return SFP_OK;

}


// Writes to a specific register on the SFP module.
byte WriteRegister(SFPDevice device, byte SFP_reg_address, byte number_of_bytes,
                   char * const data)
{
    
    // Check that the data array is properly allocated.
    if (data == NULL) 
        return MEM_FAIL;

	// Create a new packet array.
	// This holds the entire message that is to be written to the line.
    char packet[10] = { 0 };

	// Number of bytes written to the wire.
	DWORD bytes_written = 0;

    // Determine how many bytes we are expecting back.
	int bytes_to_write;
	switch (number_of_bytes)
	{
	case BYTES_1:
		bytes_to_write = 1;     // 1 data byte
		break;
	case BYTES_2:
		bytes_to_write = 2;     // 2 data bytes
		break;
	case BYTES_3:
		bytes_to_write = 3;     // 3 data bytes
		break;
	case BYTES_6:
		bytes_to_write = 6;     // 6 data bytes
		break;
	default:
		return BYTES_INVALID;   // error
	}

	// Save the mode packet.
	packet[0] = 0x0 | number_of_bytes;
	packet[1] = SFP_reg_address;

	// Load the data packet with the data passed in by the user.
	for (int i = 0; i < bytes_to_write; i++)
		packet[i + 2] = data[i];

	// Save the CRC on the whole packet.
	packet[bytes_to_write + 2] = CRC(bytes_to_write + 2, (byte *)packet);

	// Write the packet onto the wire.
	FT_STATUS rc = FT_Write(device.sfp_handle, &packet, bytes_to_write + 3,
                            &bytes_written);
	if (rc != FT_OK)
		// Failed writing to the wire.
		return WRITE_FAIL;

	// Normal return.
	return SFP_OK;
    
}


// Changes the baudrate on the host and on the SFP module.
byte ChangeBaudRate(SFPDevice device, byte baud_rate)
{
    
	// Status holder.
    FT_STATUS rc = FT_OTHER_ERROR;

	// Handles for checking how many bytes were written and recieved.
    DWORD bytes_written = 0;
	DWORD m_bytes_received = 0;

	// Receive buffer.
	char m_rx_buffer[10] = { 0 };

	// Create a packet to be sent over the wire.
    char packet[10] = { 0 };

	// Write the mode packet.
	packet[0] = 0x00;

	// Write the address.
	packet[1] = 0x01;

	// Write the baud rate.
	packet[2] = baud_rate;

	// Write the crc.
	packet[3] = CRC(3, (byte *)packet);

	// Write the packet on the wire.
	rc = FT_Write(device.sfp_handle, &packet, 4, &bytes_written);
	if (rc != FT_OK)
		// Something went wrong when writng.
		return WRITE_FAIL;

    // Below we verify that the baud rate is changed.

	// Write the mode packet.
	packet[0] = 0x80;

	// Write the address.
	packet[1] = 0x01;

	// Write the packet on the wire.
	rc = FT_Write(device.sfp_handle, &packet, 2, &bytes_written);
	if (rc != FT_OK)
		// Something went wrong when writng.
		return WRITE_FAIL;

	// Clean the m_rx_buffer before writing to it.
	for (int i = 0; i < 10; i++)
		m_rx_buffer[i] = '\0';

	// Wait for the response and if everything verifies, change the baudrate.
	rc = FT_Read(device.sfp_handle, m_rx_buffer, 3, &m_bytes_received);
	if (rc == FT_OK)
	{
		// Check to make sure we have the 3 bytes expected.
		if (m_bytes_received != 3)
			return RESPONSE_TIMEOUT;
	}
	else
		return READ_FAIL;

	// Save the rx message into the packet buffer for validation.
	for (int i = 0; i < 3; i++)
		packet[i + 2] = m_rx_buffer[i];

	// Make sure that the message that we got back is valid.
	if (!CRC(5, (byte *)packet) == 0x00)
		return CRC_ERROR;

	// Make sure that the baud rate was switched on the SFP module.
	if (packet[3] != baud_rate)
		return BAUD_FAIL;

    // Switch the host baud rate and return status 
	return ChangeOnlyHostBaudRate(device, baud_rate);

}


// Changes only the baud rate of the host.
byte ChangeOnlyHostBaudRate(SFPDevice device, byte baud_rate)
{
    
	// Status holder.
    FT_STATUS rc = FT_OTHER_ERROR;

	// Determine which baudrate to set.
    int new_rate = 0;
	switch (baud_rate)
	{
	case SFP_BAUD_9600:
		new_rate = 9600;
		break;
	case SFP_BAUD_19200:
		new_rate = 19200;
		break;
	case SFP_BAUD_115200:
		new_rate = 115200;
	}

	// Close port.
	FT_Close(device.sfp_handle);

	// Open the FTDI id that the user wants.
	rc = FT_Open(device.sfp_device_num, &device.sfp_handle);
    
	// Check return code to make sure all is good.
	if (rc != FT_OK)
	{
        // Failed - clean up just in case.
		// Close port.
		FT_Close(device.sfp_handle);
		return PORT_FAIL;
	}
	else
	{
		// Set baudrate to specified baudrate.
		rc = FT_SetBaudRate(device.sfp_handle, new_rate);
		if (!rc == FT_OK)
		{
			// Failed to set the baudrate.
			// Close port.
			FT_Close(device.sfp_handle);
			return BAUD_FAIL;
		}

		// Set parity bits (8 data bits, 1 stop bit and no parity).
		rc = FT_SetDataCharacteristics(device.sfp_handle, FT_BITS_8, FT_STOP_BITS_1,
			FT_PARITY_NONE);
		if (rc != FT_OK)
		{
			// Failed to set the parity bits.
			FT_Close(device.sfp_handle);
			return DATA_CH_FAIL;
		}

	}

	// everything ok 
	return SFP_OK;
    
}


// Closes the open port.
byte ClosePort(SFPDevice device)
{
	// Close the port .
	if (FT_Close(device.sfp_handle) == FT_OK)
		return SFP_OK;
	else
		return PORT_FAIL;
}


// Gets the number of FTDI devices available on the host.
int GetFTDIDeviceCount()
{
    // Check and return how many devices are connected.
    DWORD num_of_devices = 0;
    FT_STATUS ftStatus_static = FT_OTHER_ERROR;
	ftStatus_static = FT_ListDevices(&num_of_devices,
                                     NULL,
                                     FT_LIST_NUMBER_ONLY);
	if (ftStatus_static == FT_OK)
		// FT_ListDevices OK, number of devices connected is in num_devs.
		return num_of_devices;
	else
		return FT_LIST_FAIL;
}


// Gets the device status/serial.
byte GetFTDIDeviceInfo(int device_num, char *  buffer)
{
    // Check that the buffer array is properly allocated.
	if (buffer == NULL) 
	{
		return MEM_FAIL;
	}
	
	// Querry for the device info.
    FT_STATUS ftStatus = FT_OTHER_ERROR;
	ftStatus = FT_ListDevices((PVOID)((intptr_t)device_num),
                               buffer,
                               FT_LIST_BY_INDEX
                               | FT_OPEN_BY_SERIAL_NUMBER);
	if (ftStatus == FT_OK)
		return SFP_OK;
	else
	{
		buffer = "Busy";
		return DEVICE_BUSY;
	}

}


// CRC function.
//
// This functions computes the CRC8 of the byte array data.
byte CRC(int length, const byte * const data)
{
	byte rem = 0;
	for (int i = 0; i < length; ++i)
	{

		rem = (byte)(rem ^ (data[i]));

		for (int j = 0; j < 8; ++j)
		{
			if ((rem & 0x80) > 0)
			{
				rem = (byte)((rem << 1) ^ 0x07);
			}
			else
			{
				rem = (byte)(rem << 1);
			}
		}
	}
	return rem;
}
