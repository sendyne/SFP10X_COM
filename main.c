/*
 
 Copyright 2015-2016 Sendyne Corp., New York, USA
 http://www.sendyne.com
 
 SFP10X_COM library example.
 
 Authors:
 Damian Glinojecki (Sendyne Corp.)
 Nicolas Clauvelin (Sendyne Corp.)
 
 File:
    main.c
 
 Abstract:
    A linear example demonstrating reading and writing to the SFP module as well
    as reading the status flags and changing the baudrate.
 
    The SFP register information are described in the relevant datasheets.
 
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
#include <stdio.h>
#include <stdlib.h>


#ifdef _WIN32
#define SCANF_MACRO scanf_s
#else
#define SCANF_MACRO scanf
#endif


int main()
{
    
	// Fetch the number of FTDI devices available.
	const int device_count = GetFTDIDeviceCount();

	// Display the number of FTDI devices available.
	printf("Number of available devices: %d \n\n", device_count);

	// Display information about each device.
	printf("Device information: \n");
	for (int i = 0; i < device_count; i++)
	{
		char buf[256];
		GetFTDIDeviceInfo(i, buf);
		printf("device[%d] = ", i);
		printf("%s\n", buf);
	}
	printf("\n");

	// Prompt user which device to open.
	printf("Enter the device number you would like to open: ");
	int user_input = -99;
	
	// Get user input.
	SCANF_MACRO("%d", &user_input);

    // Device structure.
    SFPDevice sfp_device;

	// Open the device number that the user requested.
    // We return if any error occurs during initialization.
	const byte rc = Initialize(user_input, &sfp_device);
    if (rc != SFP_OK) 
	{
        printf("Failed to open the device number %d \n", user_input);
        printf("Error during port initialization, status flag = %02x %s\n",
               rc,
               FlagLookup(rc));
        return -1;
    }
	else
	{
		printf("\n");
	}
    
    // Example procedure illustrating the various SFP10X_COM functions.
	if (sfp_device.sfp_device_num != -99)
	{
        
        // Status flag variable.
        byte status_flag = PORT_FAIL;
        
		// Create a data buffer to hold the incoming data from the SFP module.
        byte data_buffer[10] = { 0 };

		// Read register 0x1E - Serial number, 3 bytes (see datasheet for more details).
		status_flag = ReadRegister(&sfp_device, 0x1E, BYTES_3,
                                   (char*)data_buffer);
		if (status_flag == SFP_OK)
		{
            // Got the data successfully, display it.
            // Expecting 8 bytes, 6 data and 2 overhead.
			printf("Raw data from the SFP module (hex): ");
			for (int i = 0; i < 5; i++)
				printf("%02x", (unsigned char)data_buffer[i]);

			// Data explanation
			printf("\nStatus register: %02x", (unsigned char)data_buffer[0]);
			printf("\nRegister 0x20: \t %02x", (unsigned char)data_buffer[1]);
			printf("\nRegister 0x1F: \t %02x", (unsigned char)data_buffer[2]);
			printf("\nRegister 0x1E: \t %02x", (unsigned char)data_buffer[3]);
			printf("\nCRC checksum:  \t %02x", (unsigned char)data_buffer[4]);

			// Decoded data to represent the Serial number
			printf("\nDecoded serial number (hex): ");
			printf("%02x", (unsigned char)data_buffer[3]);
			printf("%02x", (unsigned char)data_buffer[2]);
			printf("%02x", (unsigned char)data_buffer[1]);
		}
        else {
            // Error while reading the register.
			printf("Error reading register, status flag = %02x %s",
                   status_flag,
                   FlagLookup(status_flag));
        }

		printf("\n\n");

		// Read register 0x32 - Current, 3 bytes (see datasheet for more details).
		status_flag = ReadRegister(&sfp_device, 0x32, BYTES_3,
                                   (char*)data_buffer);
		if (status_flag == SFP_OK)
		{
            // Got the data successfully, display it.
            // Expecting 5 bytes, 3 data and 2 overhead.
			printf("Raw data from the SFP module (hex): ");
			for (int i = 0; i < 5; i++)
				printf("%02x", (unsigned char)data_buffer[i]);

			// Concatinate the data to a single number
			unsigned int unsigned_register_data = (unsigned int)
												(((unsigned int)data_buffer[3] << 16)
												+ ((unsigned int)data_buffer[2] << 8)
												+ data_buffer[1]);

			// Sign extend
			int signed_register_data = (unsigned_register_data << 8);

			// Shift back
			signed_register_data = (signed_register_data >> 8);

			// Print the signed and converted to Amps data
			printf("\nSigned data, assuming SFP101, 100uOhm shunt: %f Amps\n", 
											(signed_register_data * 0.00006119));
			
			
		}
        else 
		{
            // Error while reading the register.
			printf("Error reading register, status flag = %02x %s\n",
                   status_flag,
                   FlagLookup(status_flag));
        }


		// Read signed register 0x52 - voltage
        long long signed_register_data = 0;
        status_flag = ReadSignedRegister(&sfp_device, 0x52, BYTES_3,
                                         &signed_register_data);
        if (status_flag != SFP_OK)
        {
            // Error while reading the register.
            printf("Error reading register, status flag = %02x %s\n",
                   status_flag,
                   FlagLookup(status_flag));
        }
		printf("\nSigned data, assuming SFP101, 28.7uV per count: %f V\n\n",
			(signed_register_data  * 0.0000287));

		// Change baudrate on the host and the sfp module.
		status_flag = ChangeBaudRate(&sfp_device, SFP_BAUD_9600);
		if (status_flag == SFP_OK)
		{	

			printf("Changed baud rate to 9600 successfully \n");

			//****NOTE: The reason you see diviation between the two current reads
			// is because of internal averaging. 
			// If the current does not change, the more time passes, the more 
			// precise the current reading will be. 

			// Read signed register - 0x32, current.
            long long signed_register_data = 0;
            status_flag = ReadSignedRegister(&sfp_device, 0x32, BYTES_3,
                                             &signed_register_data);
            if (status_flag != SFP_OK)
            {
                // Error while reading the register.
                printf("Error reading register, status flag = %02x %s\n",
                       status_flag,
                       FlagLookup(status_flag));
            }
			printf("\nSigned data, assuming SFP101, 100uOhm shunt: %f Amps\n\n",
				(signed_register_data * 0.00006119));
			
			// Restart the module.
			printf("Restarting the SFP module \n");

			// Create the data packet.
			char data_send[1];
			data_send[0] = 0x01;    // Reset bit toggled

			// Write the reset register (0x10).
			if (WriteRegister(&sfp_device, 0x10, 1, data_send) == SFP_OK)
				printf("Reset Successfull \n");
			else
				printf("Reset Failed \n");

			printf("\n");

			// This should throw an error as the host is on a different baudrate
            // than the SFP.
			status_flag = ReadRegister(&sfp_device, 0x32, BYTES_3,
                                       (char*)data_buffer);
			if (status_flag == SFP_OK)
			{
                // Got the data successfully, display it.
                // Expecting 8 bytes, 6 data and 2 overhead.
				printf("Data coming from the SFP (5 bytes): ");
				for (int i = 0; i < 5; i++)
					printf("%02x", (unsigned char)data_buffer[i]);
			}
            else 
			{
                // Expected error.
				printf("(Expected error) Error reading register, \n \
                       status flag = %02x %s\n",
                       status_flag,
                       FlagLookup(status_flag));
            }

		}
        else 
		{
            // Error while trying to change baudrate.
			printf("Failed to change baudrate - %s \n",
                   FlagLookup(status_flag));
        }
			
	}

	// Close the port.
	const byte close_rc = ClosePort(&sfp_device);
    if (close_rc != SFP_OK) 
	{
        printf("Failed to close the port - %s \n",
               FlagLookup(close_rc));
        return -1;
    }
    
	printf("\n\n");

	return 0;
    
}
