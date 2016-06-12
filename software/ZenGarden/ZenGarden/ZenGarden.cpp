/**
 * ZenGarden
 * https://github.com/funvill/ZenGarden
 * 
 * Created on: June 4, 2016 
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
 * |   G02 | Circular interpolation, clockwise - Very similar in concept to G01. Again, the control interpolates intermediate points 
 * |       | and commands the servo- or stepper motors to rotate the amount needed for the leadscrew to translate the motion to the 
 * |       | correct tool tip positioning. This process repeated thousands of times per minute generates the desired toolpath. In 
 * |       | the case of G02, the interpolation generates a circle rather than a line. As with G01, the actual toolpath of the 
 * |       | machining takes place with the given feedrate on a path that accurately matches the ideal (in G02's case, a circle) 
 * |       | to within very small limits. 
 * |       |
 * |   G03 | Circular interpolation, Counter-clockwise -Same corollary info as for G02.
 * |       |
 * |   G28 | Return to home position (machine zero, aka machine reference point)
 * |       | Example: G28
 * |       | 
 * |   G90 | Absolute programming - Positioning defined with reference to part zero.
 * |       | Example: G90 
 * |       | 
 * |   G91 | Incremental programming - Positioning defined with reference to previous position. 
 * |       | Example: G91  
 * 
 */


/**
 * The corndent system is fucked for some reason 
 *
 * *-----------------------------------------------------------*
 * | Top left {X=+420, y=-400)        Top right {x=0, y=-400)  |
 * |                 Center {X=210, Y=-200}                    |
 * | Top left {X=+420, y=   0)        Top right {x=0, y=   0)  |  
 * *-----------------------------------------------------------*
 * 
 */


#include "stdafx.h"
#include "Serial.h"
#include <conio.h> // Keybord 
#include <math.h>       /* cos */

#define SETTING_COM_PORT					8
#define SETTING_COM_BAUDRATE				57600
#define SETTING_TABLE_SIZE_X				350 
#define SETTING_TABLE_SIZE_Y				350 
#define SETTING_TABLE_SIZE_X_min			50 
#define SETTING_TABLE_SIZE_Y_min		    50 

#define SETTING_DELAY_COMMAND				10

#define SETTING_MANUAL_MODE_STEP			5

#define GCODE_G01_LINEAR_INTERPOLATION						"G01" 
#define GCODE_G02_CIRCULAR_INTERPOLATION_CLOCKWISE			"G02" 
#define GCODE_G03_CIRCULAR_INTERPOLATION_COUNTER_CLOCKWISE  "G03" 
#define GCODE_G01_GO_HOME									"G28" 
#define GCODE_G90_ABSOLUTE_PROGRAMMING						"G90" 
#define GCODE_G91_POSITION_REFERENCED						"G91" 


#define SEND_BUFFER_MAX_LENGTH				1024 
#define READ_BUFFER_MAX_LENGTH				1024


class CPlotter
{
	private:

		CSerial m_serial;

	public:
		bool Open(int port, int baudrate) {
			// Connect to the serial port 
			if (!this->m_serial.Open(port, baudrate)) {
				printf("Error: Could not open the serial port. port=%d, baudrate=%d\n", port, baudrate);
				return false;
			}

			return SendCommand(GCODE_G90_ABSOLUTE_PROGRAMMING);
		}

		void Close() {
			printf("FYI: Disconnecting from plotter\n");
			this->m_serial.Close(); 
		}

		bool Move(float x, float y) {
			printf("FYI: Move X=[%.3f] Y=[%.3f]\n",x,y);
			char sendBuffer[SEND_BUFFER_MAX_LENGTH];
			sprintf_s(sendBuffer, SEND_BUFFER_MAX_LENGTH, "%s X%.3f Y%.3f", GCODE_G01_LINEAR_INTERPOLATION, x, y);
			return SendCommand(sendBuffer); 
		}
		bool Arc(float x, float y, float i, float j, char * command ) {
			printf("FYI: Arc=[%s] X=[%.3f] Y=[%.3f] i=[%.3f] j=[%.3f]\n", command, x, y, i , j );
			char sendBuffer[SEND_BUFFER_MAX_LENGTH];
			sprintf_s(sendBuffer, SEND_BUFFER_MAX_LENGTH, "%s X%.3f Y%.3f I%.3f J%.3f", command, x, y, i ,j);
			return SendCommand(sendBuffer);
		}

		bool SendCommand(char * command)
		{
			ReadIncomingBuffer(); 

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

		void ReadIncomingBuffer() {
			// Wait for last command to finish first
			// This is indecated by reciving a ">" 
			while (this->m_serial.ReadDataWaiting() <= 0) {
				Sleep(0); // Give some time back to the OS 
			}
			// For debug, print out what we recived. 
			char recvBuffer[READ_BUFFER_MAX_LENGTH];
			do {
				int recvBufferLength = this->m_serial.ReadData(recvBuffer, READ_BUFFER_MAX_LENGTH);
				if (recvBufferLength > 0) {
					recvBuffer[recvBufferLength - 1] = 0;
					printf("%s\n", recvBuffer);
				}
			} while (this->m_serial.ReadDataWaiting() > 0);

		}
};
CPlotter plotter;


/*
void PatternOffSidedBox() {
	printf("FYI: PatternOffSidedBox\n");
	
	// Move to bottom left conner 
	plotter.SendCommand(GCODE_G90_ABSOLUTE_PROGRAMMING);
	plotter.Move(SETTING_TABLE_SIZE_X / 2, SETTING_TABLE_SIZE_Y / 2);

	// Start drawing 
	plotter.SendCommand(GCODE_G91_POSITION_REFERENCED);
	for (int offset = 0; offset < 400; offset += 5) {
		plotter.Move(0, -1 * offset);
		plotter.Move(1 * offset, 0);
		plotter.Move(0, 1 * offset);
		plotter.Move(-1 * offset, 0);
	}
}
*/

void PatternGoToCenter() {
	// Go to center
	plotter.SendCommand(GCODE_G90_ABSOLUTE_PROGRAMMING);
	plotter.Move(SETTING_TABLE_SIZE_X / 2, SETTING_TABLE_SIZE_Y / 2);
}
void PatternBorder() {
	// Go to center
	plotter.SendCommand(GCODE_G90_ABSOLUTE_PROGRAMMING);
	plotter.Move(10, 10); // Bottom right 
	plotter.Move(SETTING_TABLE_SIZE_X , 10); // Bottom left 
	plotter.Move(SETTING_TABLE_SIZE_X, SETTING_TABLE_SIZE_Y); // Top left 
	plotter.Move(10, SETTING_TABLE_SIZE_Y); // Top right 
	plotter.Move(10, 10); // Bottom right 
}


void PatternBoxToCenter() {
	printf("FYI: PatternBoxToCenter\n");

	plotter.SendCommand(GCODE_G90_ABSOLUTE_PROGRAMMING);

	int setting_step = 5; 
	int box_size = ((SETTING_TABLE_SIZE_X > SETTING_TABLE_SIZE_Y) ? SETTING_TABLE_SIZE_Y : SETTING_TABLE_SIZE_X);
	for (int offset = setting_step; offset < box_size - setting_step; offset += setting_step) {
		plotter.Move(offset, offset); // Bottom right 
		plotter.Move(SETTING_TABLE_SIZE_X - offset, offset); // Bottom left 
		plotter.Move(SETTING_TABLE_SIZE_X - offset, SETTING_TABLE_SIZE_Y - offset); // Top left 
		plotter.Move(offset, SETTING_TABLE_SIZE_Y - offset); // Top right 
		plotter.Move(offset, offset); // Bottom right 

		printf("FYI: %d of %d Loops \n", (box_size - offset) / setting_step, box_size / setting_step);
	}	
}

void PatternCircleOutFromCenter() {
	printf("FYI: PatternCircleOutFromCenter\n");

	plotter.SendCommand(GCODE_G90_ABSOLUTE_PROGRAMMING);
	// plotter.Move(SETTING_TABLE_SIZE_X_min+SETTING_TABLE_SIZE_X / 2, SETTING_TABLE_SIZE_Y_min+SETTING_TABLE_SIZE_Y / 2);



	
	for (int radius = 10; radius < 150; radius += 10) {
		for (int i = 0; i < 360; i += 20)
		{
			float angle = i * 2 * 3.14 / 360;
			float Xpos =  (cos(angle) * radius);
			float Ypos =  (sin(angle) * radius);

			printf("i=%d, Xpos=%f, Ypos=%f\n", i, Xpos, Ypos);
			plotter.Move(Xpos, Ypos);
		}
	}

	
	// plotter.Arc(200, 0, 10, 01, GCODE_G02_CIRCULAR_INTERPOLATION_CLOCKWISE);

	

	printf("Done\n");
}

void PatternRandomeLines() {
	printf("FYI: PatternRandomeLines\n");

	int borderOffset = 10; 

	plotter.SendCommand(GCODE_G90_ABSOLUTE_PROGRAMMING);
	bool done = false;
	while (!done) {
		if (_kbhit()) {
			done = true; 
			break; 
		}
		float x = (rand() % (SETTING_TABLE_SIZE_X - borderOffset)) + borderOffset;
		float y = (rand() % (SETTING_TABLE_SIZE_X - borderOffset)) + borderOffset;
		// Move some where randomly 
		plotter.Move(x, y);

		printf("FYI: Press any key to stop\n");
	}
}

void PatternStar() {
	printf("FYI: PatternStar\n");
	plotter.SendCommand(GCODE_G90_ABSOLUTE_PROGRAMMING);

	int setting_border_offset = 10;
	int setting_step = 50;
	int box_size = ((SETTING_TABLE_SIZE_X > SETTING_TABLE_SIZE_Y) ? SETTING_TABLE_SIZE_Y : SETTING_TABLE_SIZE_X);

	// X Pattern 		
	for (int offset = setting_step; offset < box_size; offset += setting_step) {
		plotter.Move(offset, setting_border_offset);
		plotter.Move(offset+ setting_border_offset, setting_border_offset);
		plotter.Move(SETTING_TABLE_SIZE_X - offset, SETTING_TABLE_SIZE_Y- setting_border_offset);
		plotter.Move(SETTING_TABLE_SIZE_X - offset - setting_border_offset, SETTING_TABLE_SIZE_Y - setting_border_offset);

		printf("FYI: %d of %d Loops \n", (box_size - offset) / setting_step, box_size / setting_step);
	}

	// Y Pattern 
	for (int offset = box_size; offset > 0; offset -= setting_step) {
		plotter.Move(setting_border_offset, offset);
		plotter.Move(setting_border_offset, offset + setting_border_offset);
		plotter.Move(SETTING_TABLE_SIZE_X - setting_border_offset, SETTING_TABLE_SIZE_Y - offset);
		plotter.Move(SETTING_TABLE_SIZE_X - setting_border_offset, SETTING_TABLE_SIZE_Y - offset - setting_border_offset);

		printf("FYI: %d of %d Loops \n", (box_size - offset) / setting_step, box_size / setting_step);
	}
	printf("FYI: Done");
}


void PrintHelp() {
	printf("Help:\n");
	printf("Version: 0.01, Last updated: June 9th, 2016\n");
	printf("\n");

	printf("Utilties: \n");
	printf("1 = Go Home\n");
	printf("2 = Go To Center\n");
	printf("3 = Outline the working area\n");
	printf("7 = Random Lines\n");

	printf("Modes: \n");
	printf("4 = PatternBoxToCenter\n");
	printf("5 = PatternStar\n");
	printf("6 = PatternCircleOutFromCenter\n");
	
	
	
	printf("\n");
}
void ManualMode() {
	printf("FYI: Entering Manual Mode\n");
	plotter.SendCommand(GCODE_G91_POSITION_REFERENCED);

	bool done = false; 
	while (!done) {

		// Clear that damn infernal buffer. 
		plotter.ReadIncomingBuffer();

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
				plotter.Move(SETTING_MANUAL_MODE_STEP, 0);
				break;
			}
			case 77: { // Right arrow 
				plotter.Move((-1)*SETTING_MANUAL_MODE_STEP, 0);
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
			
			case '1': { // 
				printf("goHome\n");
				plotter.SendCommand(GCODE_G01_GO_HOME);				
				break;
			}
			
			case '2': { // 
				PatternGoToCenter();
				break;
			}
			case '3': { // 
				PatternBorder();
				break;
			}
			case '4': { // 
				PatternBoxToCenter();
				break;
			}
			case '5': { // 
				PatternStar();
				break;
			}
			case '6': { // 
				PatternCircleOutFromCenter();
				break;
			}
			case '7': { // 
				PatternRandomeLines();
				break;
			}		  
					  
			default: { // Print help 
				PrintHelp(); 
				break; 
			}
		}
	}
	printf("FYI: Leaving Manual Mode\n");
}

int main()
{
	PrintHelp();	





	if (!plotter.Open(SETTING_COM_PORT, SETTING_COM_BAUDRATE)) {
		printf("Error: Could not connect to the plotter");
		return 1;
	}

	// Find home. 
	// plotter.SendCommand(GCODE_G01_GO_HOME);
	while (true)
	{
		PatternCircleOutFromCenter();
	}
	
	
	// Enter manual mode
	// Wait on use key.
	//ManualMode();


	// Testing 
	// =====================
	/*
	for (int offset = 0; offset < 100; offset+= 10 ) {
		plotter.Move(offset, -20 - offset);
		plotter.Move(-20 - offset, offset);
		plotter.Move(offset, +20 + offset);
		plotter.Move(+20 + (2*offset), offset);

		// plotter.Move(10, 10);
	}
	*/

	/*
	// other 	
	for (int offset = 0; offset < 100; offset += 5) {
		for (int direction = 0; direction < 4; direction++) {
			switch (direction) {
			case 0: // North 
				plotter.Move(0, -1 * offset);
				break;
			case 1: // West 
				plotter.Move(-1 * offset, 0);
				break;
			case 2: // South 
				plotter.Move(0, 1 * offset);
				break;
			case 3: // East 
				plotter.Move(1 * offset, 0);
				break;
			}
		}		
	}
	*/

	
	plotter.Close(); 




    return 0;
}

