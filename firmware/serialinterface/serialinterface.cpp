#include "libmaple.h"
#include "gpio.h"
#include "flashstorage.h"
#include "safecast_config.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "log.h"
#include "oled.h"
#include "display.h"
#include <limits.h>
#include "dac.h"
#include "log_read.h"
#include "rtc.h"
#include "realtime.h"
#include <stdint.h>
#include <inttypes.h>
#include "captouch.h"
#include "power.h"


#define TX1 BOARD_USART1_TX_PIN
#define RX1 BOARD_USART1_RX_PIN

typedef void (*command_process_t)(char *);

command_process_t command_stack[10];
int  command_stack_size = 0;

void serial_process_command(char *line) {
  (*command_stack[command_stack_size-1])(line);
}

#define MAX_COMMAND_LEN 32
/* Make sure to increment when adding new commands */
#define MAX_COMMANDS 32

int command_list_size;
char command_list [MAX_COMMANDS][MAX_COMMAND_LEN];
command_process_t command_funcs[MAX_COMMANDS];

void register_cmd(const char *cmd,command_process_t func)
{

  if (command_list_size > MAX_COMMANDS) return;

  strcpy(command_list[command_list_size], cmd);
  command_funcs[command_list_size] = func;

  command_list_size++;
}

void serial_write_string(const char *str)
{
  for (uint32_t n=0; str[n]!=0; n++)
  {
    usart_putc(USART1, str[n]);
  }
}

void command_stack_pop()
{
  command_stack_size--;
  serial_write_string("\r\n>");
}

void cmd_read_log_csv (char *line)
{

  flashstorage_log_pause();
  log_read_start();

  char buffer[1024];
  int size = log_read_csv(buffer);

  for (; size != 0; )
  {
    if (size != 0)
    	serial_write_string(buffer);
    size = log_read_csv(buffer);
  }

  flashstorage_log_resume();
}

void cmd_help(char *line) {

  serial_write_string("Available commands:\r\n\t");
  for (int n=0; n < command_list_size; n++)
  {
    serial_write_string(command_list[n]);
    if (n != command_list_size - 1)
    	serial_write_string(", ");
    if ( (n%10 == 0) && (n > 0) )
      serial_write_string("\r\n\t");	// put 10 commands per line
  }
  serial_write_string("\r\n");

}

void cmd_get_version(char *line)
{
  serial_write_string("Version: ");
  serial_write_string(OS100VERSION);
  serial_write_string("\r\n");
}

void cmd_do_log_stop(char *line)
{
  flashstorage_log_pause();
}

void cmd_get_log_interval(char *line)
{
  char info [100];
  const char * sloginterval;
 
  sloginterval = flashstorage_keyval_get("LOGINTERVAL");
  sprintf(info, "LOGINTERVAL:\t%s", sloginterval);
  serial_write_string(info);
}

void cmd_set_log_interval(char *line)
{
  char * sloginterval;
  sscanf(line,"%s\r\n", sloginterval);
 
  flashstorage_keyval_set("LOGINTERVAL",sloginterval);
}

void cmd_do_log_start(char *line)
{
  flashstorage_log_resume();
}

void cmd_do_log_clear(char *line)
{
  serial_write_string("Clearing flash log\r\n");
  flashstorage_log_clear();
  serial_write_string("Cleared\r\n");
}

void cmd_set_utc(char *line)
{
  uint32_t unixtime = 0;
  sscanf(line,"%"PRIu32"\r\n",&unixtime);
 
  realtime_set_unixtime(unixtime);
  command_stack_pop();
}

void cmd_get_utc(char *line)
{
  char info[100];
  sprintf(info,"%"PRIu32"\r\n",realtime_get_unixtime());
  serial_write_string(info);
}

void cmd_do_timed_start(char *line)
{
}

void cmd_read_timed(char *line)
{
}

void cmd_get_config_run(char *line)
{
	char info[100];
	char key[100];
	const char * val;

	if (sscanf (line, "%[^ =] =", key) != 1)
		serial_write_string("ERROR PARSING!\r\n");
	else
	{
		serial_write_string("DEBUG:\t");
		serial_write_string("key was \"");
		serial_write_string(key);
		serial_write_string("\" ...\r\n");
		val = flashstorage_keyval_get(key);
	}
	sprintf(info,"%s = %s", key, val);
	serial_write_string(info);
	command_stack_pop();
}

void cmd_set_config_run(char *line)
{
	char info[100];
	char key[30];
	char val[30];

	if (sscanf (line, "%[^=] = %[^\r\n]", key, val) != 2)
		serial_write_string("ERROR PARSING!\r\n");
	else
	{
		char skey[50]; sprintf(skey,"%s",key);
		char sval[50]; sprintf(sval,"%s",val);
		serial_write_string("DEBUG:\t");
		serial_write_string("key was \"");
		serial_write_string(skey);
		serial_write_string("\", val was \"");
		serial_write_string(sval);
		serial_write_string("\" ...\r\n");
		flashstorage_keyval_set(skey, sval);
	}
	// command_stack_pop();		NOTE: Do NOT call that, cmd_get_config_run() will do

	sprintf(info,"%s =\r\n", key);
	cmd_get_config_run(info);
}

void cmd_get_config(char *line)
{
	serial_write_string("Enter KEY =\r\n");
	serial_write_string("#>");
	command_stack[command_stack_size] = cmd_get_config_run;
	command_stack_size++;
}

void cmd_set_config(char *line)
{
	serial_write_string("Enter KEY = VALUE\r\n");
	serial_write_string("#>");
	command_stack[command_stack_size] = cmd_set_config_run;
	command_stack_size++;
}

void cmd_get_utc_offset(char *line)
{
  int16_t mins;
  char info[100];

  mins = realtime_getutcoffset_mins();
  sprintf(info,"UTC_OFFSET=%d\r\n",mins);
  serial_write_string(info);
}

void cmd_set_utc_offset(char *line)
{
  int mins;
  char * buf;
  sscanf(line,"%d\r\n", &mins);
  sprintf(buf,"%d",mins);
  flashstorage_keyval_set("UTC_OFFSET",buf);
  realtime_setutcoffset_mins(mins);
}


void register_cmds()
{
  register_cmd("help"			,cmd_help);
  register_cmd("get config"		,cmd_get_config);
  register_cmd("set config"		,cmd_set_config);

  register_cmd("read log csv"		,cmd_read_log_csv);
  register_cmd("get log interval"	,cmd_set_log_interval);
  register_cmd("set log interval"	,cmd_set_log_interval);
  register_cmd("do log stop"		,cmd_do_log_stop);
  register_cmd("do log start"		,cmd_do_log_start);
  register_cmd("do log clear"		,cmd_do_log_clear);

  register_cmd("get verion"		,cmd_get_version);
  register_cmd("get UTC"		,cmd_get_utc);
  register_cmd("set UTC"		,cmd_set_utc);
  register_cmd("get UTC offset"		,cmd_get_utc_offset);
  register_cmd("set UTC offset"		,cmd_set_utc_offset);

  register_cmd("do timed start"		,cmd_do_timed_start);
  register_cmd("read timed"		,cmd_read_timed);

}

void cmd_main_menu(char *line) {

  for(int n=0;n<command_list_size;n++) {
    if(strcmp(line,command_list[n]) == 0) {
      (*command_funcs[n])(line);
    }
  }
 
  if (command_stack_size == 1)
    serial_write_string("\r\n>");
}

void serial_initialise() {
  const stm32_pin_info *txi = &PIN_MAP[TX1];
  const stm32_pin_info *rxi = &PIN_MAP[RX1];

  gpio_set_mode(txi->gpio_device, txi->gpio_bit, GPIO_AF_OUTPUT_OD); 
  gpio_set_mode(rxi->gpio_device, rxi->gpio_bit, GPIO_INPUT_FLOATING);

  if (txi->timer_device != NULL) {
      /* Turn off any PWM if there's a conflict on this GPIO bit. */
      timer_set_mode(txi->timer_device, txi->timer_channel, TIMER_DISABLED);
  }

  register_cmds();
  command_stack[0] = cmd_main_menu;
  command_stack_size = 1;

  usart_init(USART1);
  usart_set_baud_rate(USART1, STM32_PCLK2, ERROR_USART_BAUD); 
  usart_enable(USART1);
}




char currentline[1024];
uint32_t currentline_position=0;

void serial_eventloop() {
  char buf[1024];

  uint32_t read_size = usart_rx(USART1,(uint8 *) buf,1024);
  if(read_size == 0) return;

  if(read_size > 1024) return; // something went wrong

  for(uint32_t n=0;n<read_size;n++) {
    
    // echo
    usart_putc(USART1, buf[n]);

    if((buf[n] == 13) || (buf[n] == 10)) {
      serial_write_string("\r\n");
      currentline[currentline_position]=0;
      serial_process_command(currentline);
      currentline_position=0;
    } else {
      currentline[currentline_position] = buf[n];
      currentline_position++;
    }
  }
}
