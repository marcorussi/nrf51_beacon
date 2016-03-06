/*
 * The MIT License (MIT)
 *
 * Copyright (c) [2015] [Marco Russi]
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/




/* ------------------- Inclusions ------------------- */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "bsp.h"
#include "app_timer.h"




/* ------------------- Local defines ------------------- */

#define USE_UICR_FOR_MAJ_MIN_VALUES


/* Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device */
#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                 

/* Time for which the device must be advertising in non-connectable mode (in seconds). 0 disables timeout */
#define APP_CFG_NON_CONN_ADV_TIMEOUT    0          
                
/* The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100ms to 10.24s) */       
#define NON_CONNECTABLE_ADV_INTERVAL    MSEC_TO_UNITS(100, UNIT_0_625_MS) 

/* Company defines */
#define COMPANY_IDENTIFIER				0x0059				/* Company identifier for Nordic Semiconductor ASA */
#define COMPANY_IDENTIFIER_LENGTH		0x02				/* 2 bytes */

/* Beacon info */
#define BEACON_DATA_LENGTH				0x15                       	/* Beacon info specific data */
#define BEACON_INFO_LENGTH          	(BEACON_DATA_LENGTH + 2)	/* Beacon info total length: adding BEACON_DATA_LENGTH and BEACON_TYPE fields */
#define BEACON_TYPE                 	0x02                        /* Fixed Beacon type value */
#define TX_POWER_MEASURED_RSSI         	0xC3                        /* The Beacon's measured RSSI at 1 meter distance in dBm */                      
#define BEACON_MAJOR_VALUE         		0x0102                    	/* Default Major value used to identify Beacons */ 
#define BEACON_MINOR_VALUE            	0x0304                    	/* Default Minor value used to identify Beacons */ 
/* Beacon UUID higher half */ 
#define BEACON_UUID_HIGH               	(uint64_t)(0x0112233445566778) 
/* Beacon UUID lower half */ 
#define BEACON_UUID_LOW                	(uint64_t)(0x899aabbccddeeff0)
/* Beacon UUID length */ 
#define BEACON_UUID_LENGTH             	16	/* 16 byte - 128 bit */

/* Advertising data length */ 
#define BEACON_ADV_DATA_LENGTH			(BEACON_INFO_LENGTH + COMPANY_IDENTIFIER_LENGTH + 5)	/* Total length of advertising data */

/* Maximum advertising data length */ 
#define MAX_ADV_LENGTH					32

/* Value used as error code on stack dump, can be used to identify stack location on stack unwind */
#define DEAD_BEEF                       0xDEADBEEF                        

#define APP_TIMER_PRESCALER             0                           	/* Value of the RTC1 PRESCALER register */
#define APP_TIMER_MAX_TIMERS            (2+BSP_APP_TIMERS_NUMBER)     	/* Maximum number of simultaneously created timers */
#define APP_TIMER_OP_QUEUE_SIZE         4                             	/* Size of timer operation queues */

#if defined(USE_UICR_FOR_MAJ_MIN_VALUES)
/* Address of the UICR register used by this example. The major and minor versions to be encoded into the advertising data will be picked up from this location */
#define UICR_ADDRESS              		0x10001080                  	
#endif




/* ------------------- Local variables ------------------- */

/* Parameters to be passed to the stack when starting advertising */
static ble_gap_adv_params_t m_adv_params;    

/* Parameters to be passed to the stack when starting advertising */
static uint8_t device_name[] = "nrf51_iBeacon"; 




/* ------------------- Local functions prototypes ------------------- */

static void advertising_init	(void);        
static void advertising_start	(void);  
static void ble_stack_init		(void);
static void power_manage		(void);                  




/* ------------------- Local functions ------------------- */

/* Callback function for asserts in the SoftDevice.
   This function will be called in case of an assert in the SoftDevice.
   This handler is an example only and does not fit a final product. 
   You need to analyze how your product is supposed to react in case of Assert.
   On assert from the SoftDevice, the system can only recover on reset.
   param: line_num   Line number of the failing ASSERT call.
   param: file_name  File name of the failing ASSERT call.
*/
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/* Function for initializing the Advertising functionality.
   Encodes the required advertising data and passes it to the stack.
   Also builds a structure to be passed to the stack when starting advertising.
*/
static void advertising_init(void)
{
    uint8_t adv_data_index = 0;
    uint8_t adv_data[BEACON_ADV_DATA_LENGTH];
    uint8_t scan_resp_data[MAX_ADV_LENGTH];
    uint64_t uuid_half_value;
    ble_gap_conn_sec_mode_t gap_conn_sec_mode;

    /* Set advertising flags */
    adv_data[adv_data_index++] = 2;
    adv_data[adv_data_index++] = BLE_GAP_AD_TYPE_FLAGS;
    adv_data[adv_data_index++] = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;

    /* Set advertising header */
    adv_data[adv_data_index++] = 1 + COMPANY_IDENTIFIER_LENGTH + BEACON_INFO_LENGTH;
    adv_data[adv_data_index++] = BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA;

    /* Set company identifier */
    adv_data[adv_data_index++] = (uint8_t)COMPANY_IDENTIFIER;
    adv_data[adv_data_index++] = (uint8_t)((COMPANY_IDENTIFIER & 0xFF00) >> 8);

    /* Set beacon type and data length */
    adv_data[adv_data_index++] = BEACON_TYPE;
    adv_data[adv_data_index++] = BEACON_DATA_LENGTH;

    /* Set beacon UUID value */
    uuid_half_value = BEACON_UUID_LOW;
    memcpy(&adv_data[adv_data_index], (const uint8_t *)&uuid_half_value, (BEACON_UUID_LENGTH / 2));
    adv_data_index += (BEACON_UUID_LENGTH / 2);
    uuid_half_value = BEACON_UUID_HIGH;
    memcpy(&adv_data[adv_data_index], (const uint8_t *)&uuid_half_value, (BEACON_UUID_LENGTH / 2));
    adv_data_index += (BEACON_UUID_LENGTH / 2);

    /* Set beacon major and minor value */
#if defined(USE_UICR_FOR_MAJ_MIN_VALUES)
    uint16_t major_value;
    uint16_t minor_value;
    // If USE_UICR_FOR_MAJ_MIN_VALUES is defined, the major and minor values will be read from the
    // UICR instead of using the default values. The major and minor values obtained from the UICR
    // are encoded into advertising data in big endian order (MSB First).
    // To set the UICR used by this example to a desired value, write to the address 0x10001080
    // using the nrfjprog tool. The command to be used is as follows.
    // nrfjprog --snr <Segger-chip-Serial-Number> --memwr 0x10001080 --val <your major/minor value>
    // For example, for a major value and minor value of 0xabcd and 0x0102 respectively, the
    // the following command should be used.
    // nrfjprog --snr <Segger-chip-Serial-Number> --memwr 0x10001080 --val 0xabcd0102
    major_value = ((*(uint32_t *)UICR_ADDRESS) & 0xFFFF0000) >> 16;
    minor_value = ((*(uint32_t *)UICR_ADDRESS) & 0x0000FFFF);
    /* Set major value */
    adv_data[adv_data_index++] = MSB(major_value);
    adv_data[adv_data_index++] = LSB(major_value);
    /* Set minor value */
    adv_data[adv_data_index++] = MSB(minor_value);
    adv_data[adv_data_index++] = LSB(minor_value);
#else
    /* Set default beacon major and minor values */
    adv_data[adv_data_index++] = (uint8_t)((BEACON_MAJOR_VALUE & 0xFF00) >> 8);
    adv_data[adv_data_index++] = (uint8_t)BEACON_MAJOR_VALUE;
    adv_data[adv_data_index++] = (uint8_t)((BEACON_MINOR_VALUE & 0xFF00) >> 8);
    adv_data[adv_data_index++] = (uint8_t)BEACON_MINOR_VALUE;
#endif

    /* TX power measured RSSI */
    adv_data[adv_data_index] = TX_POWER_MEASURED_RSSI;

    /* set scan response data with shortened device name */
    scan_resp_data[0] = 1 + strlen((const char*)device_name);
    scan_resp_data[1] = BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME;
    memcpy(&scan_resp_data[2], device_name, (scan_resp_data[0] - 1));

    /* set advertising data */
    sd_ble_gap_adv_data_set(adv_data, BEACON_ADV_DATA_LENGTH, scan_resp_data, (scan_resp_data[0] + 1));

    /* Set device name to BLE gap layer */
    gap_conn_sec_mode.sm = 0;
    gap_conn_sec_mode.lv = 0;
    sd_ble_gap_device_name_set((ble_gap_conn_sec_mode_t const *)&gap_conn_sec_mode, device_name, (scan_resp_data[0] - 1));

    /* Initialize advertising parameters (used when starting advertising) */
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.type        = BLE_GAP_ADV_TYPE_ADV_SCAN_IND;
    m_adv_params.p_peer_addr = NULL;	/* Undirected advertisement */
    m_adv_params.fp          = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval    = NON_CONNECTABLE_ADV_INTERVAL;
    m_adv_params.timeout     = APP_CFG_NON_CONN_ADV_TIMEOUT;
}


/* brief Function for starting advertising */
static void advertising_start(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_start(&m_adv_params);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
    APP_ERROR_CHECK(err_code);
}


/* Function for initializing the BLE stack.
   Initializes the SoftDevice and the BLE event interrupt */
static void ble_stack_init(void)
{
    uint32_t err_code;
    ble_enable_params_t ble_enable_params;

    memset(&ble_enable_params, 0, sizeof(ble_enable_params));

    /* Initialize the SoftDevice handler module */
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_RC_250_PPM_TEMP_4000MS_CALIBRATION, NULL);

    /* Enable BLE stack */
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
}


/* Function for doing power management */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}


/* Function for application main entry */
int main(void)
{
    uint32_t err_code;

    /* Initialize */
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, false);
    err_code = bsp_init(BSP_INIT_LED, APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), NULL);
    APP_ERROR_CHECK(err_code);
    ble_stack_init();
    advertising_init();

    /* Start execution */
    advertising_start();

    /* Enter main loop */
    for (;;)
    {
        power_manage();
    }
}




/* End of file */




