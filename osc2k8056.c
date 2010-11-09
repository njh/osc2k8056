/*

	osc2k8056.c
	
	OSC to Velleman K8056 relay controller.

	Copyright (C) 2010  Nicholas J. Humfrey
  Copyright (C) 2005  Daniel Clement
	
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <sys/fcntl.h>

#include <lo/lo.h>


#define K8056_BAUDRATE    B2400
#define K8056_ATTEMPTS    (6)



/* Global Variables */
int keep_running = 1;
struct	termios oldtio, newtio;
int serial_port = -1;


static
int open_serial(const char* device)
{
	int fh = open(device, O_WRONLY | O_NOCTTY | O_NDELAY );
	if (fh < 0)
 	{
 		fprintf(stderr, "Error opening device %s.\n", device) ;
		exit(-1);
	}
 	tcgetattr(fh, &oldtio); 		/* save current port settings */

 	newtio.c_cflag = CS8 | CLOCAL | CREAD;
 	newtio.c_oflag = 0;

  cfsetispeed(&newtio, B2400);
  cfsetospeed(&newtio, B2400);

 	tcflush(fh, TCOFLUSH);		/* Flushes written but not transmitted. */
 	tcsetattr(fh, TCSANOW, &newtio);
 	
 	return fh;
}

static
unsigned char k8056_checksum(unsigned char cmd[])
{
    /*  Ex. VB: checksum = ( 255 - ( ( ( (a+b+c+d / 256 ) - Int( (13 + cmd[1] + cmd[2] + cmd[3]) / 256 ) ) * 256 ) ) + 1 ;	*/
    /* ( 255 - ((a+b+c+d) modulo 256) ) + 1		Calcul de checkum (complement ‡ 2 de la somme de 4 octets + 1).  	*/
    return ~((cmd[0] + cmd[1] + cmd[2] + cmd[3]) & 0xFF) + 1;
}

static
unsigned char k8056_relay_code(int relay_num)
{
    if (relay_num < 1 || relay_num > 8) {
        printf("Invalid relay number: %d\n", relay_num);
        return 0;
    } else {
        return relay_num + '0';
    }
}

static
int send_command(int port, unsigned char address, unsigned char instruction, unsigned char value)
{
	unsigned char cmd[5];
	int	i;

	cmd[0] = 13;
	cmd[1] = address;
	cmd[2] = instruction;
	cmd[3] = value;
	cmd[4] = k8056_checksum(cmd);

  printf("Sending: 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x\n",
         cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
	
	
	/* Flushes written but not transmitted. */
	tcflush(port, TCOFLUSH);
	
	for (i=0; i < K8056_ATTEMPTS; i++)
	{
		write(port, cmd, 5) ;
		usleep(5000) ;
	}
	
	// Success
	return 0;
}

static
void close_serial(int port)
{
  /* Close serial port */
	tcsetattr(port, TCSANOW, &oldtio);		/* Backup old port settings   */
  close(port);
}


static
void termination_handler(int signum)
{
    switch(signum) {
      case SIGHUP:  fprintf(stderr, "osc2k8062: Received SIGHUP, exiting.\n"); break;
      case SIGTERM: fprintf(stderr, "osc2k8062: Received SIGTERM, exiting.\n"); break;
      case SIGINT:  fprintf(stderr, "osc2k8062: Received SIGINT, exiting.\n"); break;
    }
  
    keep_running = 0;
}

static
int osc_set(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data)
{
    return send_command(serial_port, argv[0]->i, 'B', argv[1]->i);
}

static
int osc_set_relay(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data)
{
    unsigned char relay = k8056_relay_code(argv[1]->i);
    if (relay)
        send_command(serial_port, argv[0]->i, 'S', argv[1]->i);
    return 1;
}

static
int osc_clear_relay(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data)
{
    unsigned char relay = k8056_relay_code(argv[1]->i);
    if (relay)
        send_command(serial_port, argv[0]->i, 'C', relay);
    return 1;
}

static
int osc_toggle_relay(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data)
{
    unsigned char relay = k8056_relay_code(argv[1]->i);
    if (relay)
        send_command(serial_port, argv[0]->i, 'T', relay);
    return 1;
}

static
int osc_emergency_stop(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data)
{
    send_command(serial_port, 0, 'E', 0);
    return 1;
}

static
int osc_display_address(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data)
{
    send_command(serial_port, 0, 'D', 0);
    return 1;
}

static
int osc_set_address(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data)
{
    send_command(serial_port, argv[0]->i, 'T', argv[1]->i);
    return 1;
}

static
int osc_reset_address(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data)
{
    send_command(serial_port, 0, 'A', argv[0]->i);
    return 1;
}

static
void osc_error(int num, const char *msg, const char *path)
{
    fprintf(stderr, "liblo server error %d in path %s: %s\n", num, path, msg);
}

static
int osc_wildcard(const char *path, const char *types, lo_arg **argv, int argc,
		 lo_message msg, void *user_data)
{
    fprintf(stderr, "Warning: unhandled OSC message: '%s' with args '%s'.\n", path, types);
    return 1;
}


static
lo_server start_server(char* port)
{
    lo_server s = lo_server_new(port, osc_error);
    if (!s) {
        printf("Failed to start server.\n");
        keep_running = 0;
        return NULL;
    }
    
    printf("Started server on port %d.\n", lo_server_get_port(s));
    
    lo_server_add_method(s, "/k8056/set", "ii", osc_set, NULL);
    lo_server_add_method(s, "/k8056/set_relay", "ii", osc_set_relay, NULL);
    lo_server_add_method(s, "/k8056/clear_relay", "ii", osc_clear_relay, NULL);
    lo_server_add_method(s, "/k8056/toggle_relay", "ii", osc_toggle_relay, NULL);
    lo_server_add_method(s, "/k8056/emergency_stop", "", osc_emergency_stop, NULL);
    lo_server_add_method(s, "/k8056/display_address", "", osc_display_address, NULL);
    lo_server_add_method(s, "/k8056/set_address", "i", osc_set_address, NULL);
    lo_server_add_method(s, "/k8056/reset_address", "", osc_reset_address, NULL);
  
    // add method that will match any path and args
    lo_server_add_method(s, NULL, NULL, osc_wildcard, NULL);
    
    return s;
}

int main() {
    lo_server server = NULL;
    const char* device = "/dev/k8056";

    /* Make stdout unbuffered for logging/debugging */
    setvbuf(stdout, 0, _IONBF, 0);

    printf("Opening serial port: %s\n", device);
    serial_port = open_serial(device);
    if (serial_port < 0) {
        fprintf(stderr, "Failed to open serial port.\n");
        // FIXME: restore previous settings
        return -1;
    }

    /* OSC server failed? */
    server = start_server("8056");
    if (server == NULL) {
        fprintf(stderr, "Failed to start OSC server.\n");
        return -1;
    }

    // Setup signal handlers - so we exit cleanly
    signal(SIGTERM, termination_handler);
    signal(SIGINT, termination_handler);
    signal(SIGHUP, termination_handler);

    /* Sit and wait for messages */
    while (keep_running) {
        lo_server_recv_noblock(server, 200);
    }

    /* Clean up */
    close_serial(serial_port);
    lo_server_free(server);
    
    return 0;
}
