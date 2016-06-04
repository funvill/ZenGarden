/**
 * ZenGarden
 * https://github.com/funvill/ZenGarden
 * 
 * Created on: June 4, 2016 
 * Last updated: June 4, 2016 
 * Created by: Steven Smethurst
 * 
 * Helpful information 
 * GCode description https://en.wikipedia.org/wiki/G-code 
 * 
 * GCodes used 
 * ====================
 * This is a list of GCodes used to do these patterns. 
 * 
 * | GCode | Description 
 * |-------|-------------
 * |   G01 | Linear interpolation - The most common workhorse code for feeding during a cut. The program specs the start and end points, 
 * |       | and the control automatically calculates (interpolates) the intermediate points to pass through that will yield a straight 
 * |       | line (hence "linear"). The control then calculates the angular velocities at which to turn the axis leadscrews via their 
 * |       | servomotors or stepper motors. The computer performs thousands of calculations per second, and the motors react quickly 
 * |       | to each input. Thus the actual toolpath of the machining takes place with the given feedrate on a path that is accurately 
 * |       | linear to within very small limits.
 * |       | Example: G01 X10 Y40 
 * |       | 
 * |   G91 | Incremental programming - Positioning defined with reference to previous position. 
 * |       | Example: G91 
 *
 * 
 */


#include "stdafx.h"
#include "Serial.h"
#include <conio.h> // Keybord 

#define SETTING_COM_PORT					7
#define SETTING_COM_BAUDRATE				57600
#define SETTING_TABLE_SIZE_X				100 
#define SETTING_TABLE_SIZE_Y				100 
#define SETTING_DELAY_COMMAND				10

#define SETTING_MANUAL_MODE_STEP			5

#define GCODE_G01_LINEAR_INTERPOLATION      "G01" 
#define GCODE_G91_POSITION_REFERENCED		"G91" 


#define SEND_BUFFER_MAX_LENGTH				1024 
#define READ_BUFFER_MAX_LENGTH				1024


class CPlotter
{
	private:

		CSerial m_serial;
		float m_x;
		float m_y;

	public:
		bool Open(int port, int baudrate) {
			// Connect to the serial port 
			if (!this->m_serial.Open(port, baudrate)) {
				printf("Error: Could not open the serial port. port=%d, baudrate=%d\n", port, baudrate);
				return false;
			}

			// Set up the system incase it is in a bad state. 
			// Set the mode in to relitive 
			char sendBuffer[SEND_BUFFER_MAX_LENGTH];
			sprintf_s(sendBuffer, SEND_BUFFER_MAX_LENGTH, "%s", GCODE_G91_POSITION_REFERENCED);
			return SendCommand(sendBuffer);
			return true; 
		}

		void Close() {
			printf("FYI: Disconnecting from plotter\n");
			this->m_serial.Close(); 
		}

		bool Move(float x, float y) {
			m_x += x;
			m_y += y;
			printf("FYI: Move X=[%.3f] Y=[%.3f] (%.3f,%.3f)\n",x,y, m_x, m_y);


			char sendBuffer[SEND_BUFFER_MAX_LENGTH];

			// Set up the system incase it is in a bad state. 
			// Set the mode in to relitive 
			sprintf_s(sendBuffer, SEND_BUFFER_MAX_LENGTH, "%s X%.3f Y%.3f", GCODE_G01_LINEAR_INTERPOLATION, x, y);
			return SendCommand(sendBuffer); 
		}

		bool SendCommand(char * command)
		{
			// Wait for last command to finish first
			// This is indecated by reciving a ">" 
			while (this->m_serial.ReadDataWaiting() <= 0) {
				Sleep(0); // Give some time back to the OS 
			}
			// For debug, print out what we recived. 
			char recvBuffer[READ_BUFFER_MAX_LENGTH];
			int recvBufferLength = this->m_serial.ReadData(recvBuffer, READ_BUFFER_MAX_LENGTH);
			if (recvBufferLength > 0) {
				recvBuffer[recvBufferLength] = 0;
				printf("%s\n", recvBuffer);
			}

			printf("FYI: Sending Command: [%s]\n", command);
			int length = strlen(command); 
			if (this->m_serial.SendData(command, length) != length) {
				printf("Error: Could not send message to plotter. length=%d, command=[%s]\n", length, command);
				return false;
			}
			this->m_serial.SendData(";\n", 2);
			Sleep(SETTING_DELAY_COMMAND);
			return true;
		}
};


void ManualMode(CPlotter & plotter) {
	printf("FYI: Entering Manual Mode\n");
	bool done = false; 
	while (!done) {
		if (!_kbhit()) {
			continue; 
		}
		char key = _getch(); 
		if (key < 0) {
			continue; 
		}

		printf("FYI: Key [%d]{%c} was pressed\n", key, key); 
		key = toupper(key);
		switch (key) {
			case 72: { // Up arrow 
				plotter.Move(0, SETTING_MANUAL_MODE_STEP);
				break;
			}
			case 75: { // Left arrow 
				plotter.Move((-1)*SETTING_MANUAL_MODE_STEP, 0);
				break;
			}
			case 77: { // Right arrow 
				plotter.Move(SETTING_MANUAL_MODE_STEP, 0);
				break;
			}
			case 80: { // Down arrow 
				plotter.Move(0, (-1)*SETTING_MANUAL_MODE_STEP);
				break;
			}			
			case 'Q': { // 113=q
				done = true; 
				break;
			}
		}
	}
	printf("FYI: Leaving Manual Mode\n");
}

int main()
{
	CPlotter plotter; 

	if (!plotter.Open(SETTING_COM_PORT, SETTING_COM_BAUDRATE)) {
		printf("Error: Could not connect to the plotter");
		return 1;
	}

	// Testing 
	// =====================
	// Square 	
	for (int offset = 0; offset < 20; offset += 5) {
		plotter.Move(0, -1 * offset);
		plotter.Move(1 * offset, 0);
		plotter.Move(0, 1 * offset);
		plotter.Move(-1 * offset, 0);
	}

	// Enter manual mode
	ManualMode(plotter);

	
	plotter.Close(); 




    return 0;
}

