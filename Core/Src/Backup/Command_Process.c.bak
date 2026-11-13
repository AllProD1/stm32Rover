#include "main.h"
#include "usart.h"
#include "gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Command_Process.h"

#define MAX_CMD_ARGS 10
char *argv[MAX_CMD_ARGS];   // Array of pointers to tokens
int argc = 0;     // Argument count

static uint8_t U2_Cmd[U2_CMD_SIZE];
uint16_t Servo_angle = 90;

void Process_Servo_Angle_Cmd(char *argv[], int argc);
void Process_Distance_Cmd(char *argv[], int argc);


int tokenize(char *input, char *argv[], int maxArgs) {
    int argc = 0;
    while (*input && argc < maxArgs) {
        // Skip leading spaces
        while (*input == ' ' || *input == '\t') input++;

        if (*input == '\0') break;

        argv[argc++] = input;

        // Find end of token
        while (*input && *input != ' ' && *input != '\t' && *input != '\r' && *input != '\n') {
            input++;
        }

        if (*input) {
            *input = '\0';  // Null-terminate token
            input++;
        }
    }
    return argc;
}

void Command_Proc(void) {
	if (Get_U2_Cmd(U2_Cmd) > 0) {
		argc = tokenize((char *)U2_Cmd, argv, MAX_CMD_ARGS);
		print("Cmd: %s argc: %d ", U2_Cmd, argc);
		for (int i=0;i<argc;i++) {
			print("[%d] %s ",i,argv[i]);
		}
		print("\r\n");

		if (strcmp(argv[1],"ANG") == 0) {
			Process_Servo_Angle_Cmd(argv, argc);
		}
		else if (strcmp(argv[1],"DST") == 0) {
			Process_Distance_Cmd(argv, argc);
		}
		else
		{
			print("Cmd unknown\r\n");
		}
	}
}

void Process_Servo_Angle_Cmd(char *argv[], int argc) {
	if (strcmp(argv[0], "SET") == 0) {
		if (argc == 3) {
			Servo_angle = atoi(argv[2]);
			print("Set Servo Angle to %d\r\n", Servo_angle);
		} else {
			print("Cmd missing argument\r\n");
		}
	} else if (strcmp(argv[0], "GET") == 0) {
		if (argc == 2) {
			print("Currnet Servo Angle is %d\r\n", Servo_angle);
		} else {
			print("Cmd unknown\r\n");
		}
	}
}

void Process_Distance_Cmd(char *argv[], int argc) {

}
