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
    if (rc != SFP_OK) {
        printf("Failed to open the device number %d \n", user_input);
        printf("Error during port initialization, status flag = %02x %s\n",
               rc,
               FlagLookup(rc));
        return -1;
    }
    
    // Example procedure illustrating the various SFP10X_COM functions.
	if (sfp_device.sfp_device_num != -99)
	{
        
        // Status flag variable.
        byte status_flag = PORT_FAIL;
        
		// Create a data buffer to hold the incoming data from the SFP module.
        char data_buffer[10] = { 0 };

		// Read register (see datasheet for more details).
		status_flag = ReadRegister(sfp_device, 0x26, BYTES_2, data_buffer);
		if (status_flag == SFP_OK)
		{
            // Got the data successfully, display it.
            // Expecting 8 bytes, 6 data and 2 overhead.
			printf("Data coming from the SFP (8 bytes): ");
			for (int i = 0; i < 8; i++)
				printf("%02x", (unsigned char)data_buffer[i]);
		}
        else {
            // Error while reading the register.
			printf("Error reading register, status flag = %02x %s\n",
                   status_flag,
                   FlagLookup(status_flag));
        }

		printf("\n");

		// Query the device for data.
		status_flag = ReadRegister(sfp_device, 0x32, BYTES_2, data_buffer);
		if (status_flag == SFP_OK)
		{
            // Got the data successfully, display it.
            // Expecting 8 bytes, 6 data and 2 overhead.
			printf("Data coming from the SFP (8 bytes): ");
			for (int i = 0; i < 8; i++)
				printf("%02x", (unsigned char)data_buffer[i]);
		}
        else 
		{
            // Error while reading the register.
			printf("Error reading register, status flag = %02x %s\n",
                   status_flag,
                   FlagLookup(status_flag));
        }

		printf("\n");

		// Change baudrate on the host and the sfp module.
		status_flag = ChangeBaudRate(sfp_device, SFP_BAUD_9600);
		if (status_flag == SFP_OK)
		{
            
			printf("Changed baud rate to 9600 successfully \n");

            // Query the device for data.
			status_flag = ReadRegister(sfp_device, 0x32, BYTES_6, data_buffer);
			if (status_flag == SFP_OK)
			{
                // Got the data successfully, display it.
                // Expecting 8 bytes, 6 data and 2 overhead.
				printf("Data coming from the SFP (8 bytes): ");
				for (int i = 0; i < 8; i++)
					printf("%02x", (unsigned char)data_buffer[i]);
			}
            else 
			{
                // Error while reading the register.
				printf("Error reading register, status flag = %02x %s\n",
                       status_flag,
                       FlagLookup(status_flag));
            }

			printf("\n");

			// Restart the module.
			printf("Restarting the SFP module \n");

			// Create the data packet.
			char data_send[1];
			data_send[0] = 0x01;    // Reset bit toggled

			// Write the reset register (0x10).
			if (WriteRegister(sfp_device, 0x10, 1, data_send) == SFP_OK)
				printf("Reset Successfull \n");
			else
				printf("Reset Failed \n");

			printf("\n");

			// This should throw an error as the host is on a different baudrate
            // than the SFP.
			status_flag = ReadRegister(sfp_device, 0x32, BYTES_6, data_buffer);
			if (status_flag == SFP_OK)
			{
                // Got the data successfully, display it.
                // Expecting 8 bytes, 6 data and 2 overhead.
				printf("Data coming from the SFP (8 bytes): ");
				for (int i = 0; i < 8; i++)
					printf("%02x", (unsigned char)data_buffer[i]);
			}
            else 
			{
                // Expected error.
				printf("(Expected error) Error reading register, \
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
	const byte close_rc = ClosePort(sfp_device);
    if (close_rc != SFP_OK) 
	{
        printf("Failed to close the port - %s \n",
               FlagLookup(close_rc));
        return -1;
    }
    
	printf("\n\n");

	return 0;
    
}
