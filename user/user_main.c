/*
* The MIT License (MIT)
* 
* Copyright (c) 2015 David Ogilvy (MetalPhreak)
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/



#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "ets_sys.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"
#include "espconn.h"
#include "driver/spi.h"
#include "driver/spi_register.h"
#include "driver/mcp23s17.h"

#define TopTaskPrio        2
#define TopTaskQueueLen    1

#define ActionTaskPrio        1
#define ActionTaskQueueLen    8

#define IdleTaskPrio        0
#define IdleTaskQueueLen    1

#define SSID "Your_AP_SSID"
#define SSID_PASSWORD "Your_AP_Password"

#define AP_SSID "GPIO16_" //SoftAP SSID
#define AP_SSID_PASSWORD "ESP8266WIFI" //SoftAP SSID
#define AP_CHANNEL 1 //SoftAP Channel
#define AP_AUTH AUTH_WPA_WPA2_PSK //SoftAP Encryption

static void TopTask(os_event_t *events);
static void ActionTask(os_event_t *events);
static void IdleTask(os_event_t *events);
static void apList(void *arg, STATUS status);
static void tcp_recv(void *arg, char *pdata, unsigned short len);
static void station_init(void);
static void softap_init(uint8 channel);
static void wifi_event_cb(System_Event_t *evt);

os_event_t    TopTaskQueue[TopTaskQueueLen];
os_event_t    ActionTaskQueue[ActionTaskQueueLen];
os_event_t    IdleTaskQueue[IdleTaskQueueLen];

uint16 portval = 0;

static struct espconn *TCP_Server;

#define TCP_PORT 33333
#define SERVER_TIMEOUT 60
#define MAX_CONNS 5



static void ICACHE_FLASH_ATTR TopTask(os_event_t *events){


}

static void ICACHE_FLASH_ATTR ActionTask(os_event_t *events){


}

static void ICACHE_FLASH_ATTR IdleTask(os_event_t *events){


//Add task to add IdleTask back to the queue
	system_os_post(IdleTaskPrio, 0, 0);
}


void ICACHE_FLASH_ATTR tcp_recv(void *arg, char *pdata, unsigned short len){
	if(len == 2){
	uint16 gpiodata = pdata[0]<<8|pdata[1];
	sGPIO_SET(PORT0, gpiodata);
	}
}

void ICACHE_FLASH_ATTR task_init(void)
{
	system_os_task(TopTask, TopTaskPrio, TopTaskQueue, TopTaskQueueLen);
	system_os_task(ActionTask, ActionTaskPrio, ActionTaskQueue, ActionTaskQueueLen);
	system_os_task(IdleTask, IdleTaskPrio, IdleTaskQueue, IdleTaskQueueLen);

	system_os_post(IdleTaskPrio, 0, 0);
}

void ICACHE_FLASH_ATTR station_init(void)
{
	uint8 ssid[32] = SSID;
    uint8 password[64] = SSID_PASSWORD;
    struct station_config stationConf;
    os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 64);
	wifi_station_set_config(&stationConf);
	wifi_set_event_handler_cb(wifi_event_cb);
}

void ICACHE_FLASH_ATTR wifi_event_cb(System_Event_t *evt)
{
	uint8 newchannel = 0;
	switch (evt->event) {
		case EVENT_STAMODE_CONNECTED: //When connected to AP, change SoftAP to same channel to keep DHCPS working
		newchannel = evt->event_info.connected.channel;
		softap_init(newchannel);
		break;
	}

}

void ICACHE_FLASH_ATTR softap_init(uint8 channel)
{
	uint8 ap_ssid[32];
    uint8 ap_password[64];
    uint8 ap_channel = channel;
    uint8 ap_auth = AP_AUTH;
    uint8 ap_mac[6];
    wifi_get_macaddr(SOFTAP_IF, ap_mac);
    os_sprintf(ap_ssid, "%s%02X%02X%02X%02X%02X%02X\n", AP_SSID, ap_mac[0], ap_mac[1], ap_mac[2], ap_mac[3], ap_mac[4], ap_mac[5]);
    struct softap_config softapConf;
    os_memcpy(&softapConf.ssid, ap_ssid, strlen(ap_ssid));
    os_sprintf(ap_password, "%s", AP_SSID_PASSWORD);
    os_memcpy(&softapConf.password, ap_password, strlen(ap_password));
    //os_printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", ap_mac[0],ap_mac[1],ap_mac[2],ap_mac[3],ap_mac[4],ap_mac[5]);
    softapConf.channel = ap_channel;
    softapConf.authmode = ap_auth;
    softapConf.max_connection = 4;
    softapConf.beacon_interval = 100;
	wifi_softap_set_config(&softapConf);

	wifi_softap_dhcps_stop();
	struct dhcps_lease dhcp_lease;
	IP4_ADDR(&dhcp_lease.start_ip, 192, 168, 5, 2);
	IP4_ADDR(&dhcp_lease.end_ip, 192, 168, 5, 100);
	wifi_softap_set_dhcps_lease(&dhcp_lease);
    struct ip_info info;
    IP4_ADDR(&info.ip, 192, 168, 5, 1); 
    IP4_ADDR(&info.gw, 192, 168, 5, 1);
    IP4_ADDR(&info.netmask, 255, 255, 255, 0);
    wifi_set_ip_info(SOFTAP_IF, &info);
	wifi_softap_dhcps_start();
}

void user_init(void)
{
	wifi_set_opmode(0x03); //0x01 Station mode, 0x02 Soft-AP, 0x03 Combined Mode
	
	softap_init(AP_CHANNEL);
	station_init();

	mcp23s17_init();
	mcp23s17_REG_SET(IODIR_CTRL, PORT0, 0x0000); //setup all GPIO pins as outputs

	TCP_Server = (struct espconn *)os_zalloc(sizeof(struct espconn));
	TCP_Server->type = ESPCONN_TCP;
	TCP_Server->state = ESPCONN_NONE;
	TCP_Server->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
	TCP_Server->proto.tcp->local_port = TCP_PORT;
	espconn_regist_recvcb(TCP_Server, tcp_recv);
	espconn_accept(TCP_Server);
	espconn_regist_time(TCP_Server, SERVER_TIMEOUT, 0);
	
	task_init();
}




