#ifndef _config_h_
#define _config_h_

//#define DEBUG1
//#define MARKETING

/* Release versions
	remote_hvac_release_v_0_3
*/

#define VERSION_MAJOR 0
#define VERSION_MINOR 3 

#define FILTER_EEPROM_ADDRESS 0x10
#define CONFIG_EEPROM_ADDRESS 0x20

#define MIN_BAT_AD int(1.38f * 1023 / 3.3f) // start charging (was 1.35 * 1023 / 3.3f)
#define MAX_BAT_AD int(1.40f * 1023 / 3.3f) // stop charging 
#define SHOW_ACC_BAT_AD int(1.2f * 1023 / 3.3f) // show ACC
#define TURN_OFF_BAT_AD int(1.0f * 1023 / 3.3f) // turn off radio and LCD

#define MIN_TEMP 16.0f 
#define MAX_TEMP 30.0f
#define PID_TURN_OFF_TEMP -100

#ifdef MARKETING
	#define NUM_OF_COND_STAGES 3 
	#define NUM_OF_BLOWER_SPEEDS 3 
#else
	#define NUM_OF_COND_STAGES 7 
	#define NUM_OF_BLOWER_SPEEDS 7 
#endif

#define NUM_OF_DUMPER_POSITIONS 3
#define NUM_OF_HUMIDIFIER_STAGES 3

#define BREEZER_TIMEOUT 90000 // 60000
#define HUMIDIFIER_TIMEOUT 90000 // 60000
#define COND_TIMEOUT 90000 // 60000

#define MAX_SENDING_ATTEMPTS 2
#define SENDING_INTERVAL 3000

// ms
#define BLINK_INTERVAL 500
#define OFF_INTERVAL 3000
#define SENSOR_UPDATE_INTERVAL 10000
#define STANDBY_INTERVAL 15000 // 10000
#define TEMP_UPDATE_INTERVAL 100 // было 200
#define UPDATE_INTERVAL_BUTTONS 50 // было 200
#define UPDATE_START_INTERVAL 250
#define VIBRATION_TIME 500

#define TIMER_RESEND_PERIOD 1000
#define TIMER_RESEND_TIMES 2

#define TIMER_BUTTONS_DELAY 400

#define SET_DAMPER_TIME 300 //300000

#define PERMIT_SET_REMOTE_STATES_TIME 11000 

#define TIMER_DRAW_SMOOTHLY_TIME 60 // с какой быстротой вырисовывать уставки - 40
#define DS_HUMIDIFIER_DELAY 2 // уставка увлажнителя вырисовывается с меньшей скорость, если поставить больше 0 - 2

#define TIMER_ANIMATION_CHAR_HUB_TIME_FIRST 400  // начальная задержка в анимации CHAR_HUB
#define TIMER_ANIMATION_CHAR_HUB_TIME_SHORT 400  // короткая задержка в анимации CHAR_HUB
#define TIMER_ANIMATION_CHAR_HUB_TIME_LONG  400  // длинная задержка в анимации CHAR_HUB
#define TIMER_ANIMATION_CHAR_HUB_CYCLES 5		 // количество мерцаний в анимации CHAR_HUB. Должно быть нечётным числом

#define TIMER_SHOW_THAT_BREEZER_BECAME_ONLINE_PERIOD 400  // задержка в анимации 
#define TIMER_SHOW_THAT_BREEZER_BECAME_ONLINE_CYCLES 5	  // количество мерцаний в анимации. Должно быть нечётным числом

#define TIMER_SHOW_THAT_HUMIDIFIER_BECAME_ONLINE_PERIOD 400  // задержка в анимации
#define TIMER_SHOW_THAT_HUMIDIFIER_BECAME_ONLINE_CYCLES 5	 // количество мерцаний в анимации. Должно быть нечётным числом

#define TIMER_CONTIOUNE_RESET_FACTORY_DELAY 4000 // 3000 + TIMER_CONTIOUNE_RESET_FACTORY_DELAY will reset factory

#define TIMER_HIDE_FIRMWARE_VERSION_DELAY 10000

//#define DAMPER_AUTO_CONTROL

#define xENABLE_CO2_SENSOR

#define xUSE_RL_CC2530_PA2

/* For E18 */
#define POLL_RATE 4 // set left value in ms * 1000
#define TIMER_SET_POLL_RATE_DELAY 4000

#endif
