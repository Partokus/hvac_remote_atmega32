#ifndef _remote_h_
#define _remote_h_

#include "../core/wiring.h"
#include "config.h"
#include "lcd_hvac.h"
#include "pins.h"
#include "shared.h"

#include "../customRF_hvac.h"
#include "../framework.h"
#include "../SHT3x.h"

static uint8_t last_damper_position = 0xFF; 
static uint8_t set_damper_timer_flag = 0;
static bool save_settings_2 = 0;
static bool damper_flag = 0;
static bool timer_go = 0; 
static bool fillwater = false; 
static bool permit_set_remote_states = 0; // 1 - permit, 0 - allow
// Draw smoothly variables
static uint8_t ds_cooler_flag = 0;
static uint8_t ds_breezer_flag = 0;
static uint8_t ds_humidifier_flag = 0;
static uint8_t ds_cooler_value = 0;
static uint8_t ds_breezer_value = 0;
static uint8_t ds_humidifier_value = 0;
static uint8_t ds_humidifier_delay_counter = 0;
static uint8_t ds_close_simultaneously = 0;
//------------------------------------------------
static bool remoteStateWasGot = 0;
static bool breezerStateWasGot = 0;
// Animation for char HUB
static bool ach_flag = 0; // 1 - высвечивается, 2 - гаснет
static uint8_t ach_value = 0;
static bool ach_polarity = 0; 
//------------------------------------------------

// Animation for breezer's icon
static uint8_t abi_value = 0;
static bool abi_polarity = 0;
//------------------------------------------------

// Animation for himidifier's icon
static uint8_t ahi_value = 0;
static bool ahi_polarity = 0;
//------------------------------------------------

static uint8_t resend_times_counter = 0;

// Anti DDOS
static bool can_send_button_power = 1;
static bool can_send_button_damper = 1;
static bool can_send_button_flow = 1;
static bool can_send_button_mode = 1;
static bool can_send_button_breezer = 1;
static bool can_send_button_humidifier = 1;
static bool can_send_button_conditioner = 1;
static bool can_send_button_temperature = 1;


static bool can_send_button_power_delayed = 0;
static bool can_send_button_damper_delayed = 0;
static bool can_send_button_flow_delayed = 0;
static bool can_send_button_mode_delayed = 0;
static bool can_send_button_breezer_delayed = 0;
static bool can_send_button_humidifier_delayed = 0;
static bool can_send_button_conditioner_delayed = 0;
static bool can_send_button_temperature_delayed = 0;

//-----------------------------------------------------

static bool is_binding_device = 0;

static bool was_breezer_speed_zero = false;


//#define DEBUG1

enum
{
	INPUT_DC_IN,
	INPUT_WIND_SPEED_PLUS,
	INPUT_T_PLUS,
	INPUT_WIND_SPEED_MINUS,
	INPUT_T_MINUS,
	INPUT_H_PLUS,
	INPUT_H_MINUS,
	INPUT_COND_PLUS,
	INPUT_POWER,
	INPUT_IN_EX,
	INPUT_DUCT,
	INPUT_MODE,
	INPUT_COND_MINUS,
	
	NUM_OF_INPUTS
};

const uint8_t PROGMEM input_pins_PGM[NUM_OF_INPUTS * 2] = {
	REMOTE_DC_IN, INPUT,
	REMOTE_WIND_SPEED_PLUS, INPUT_PULLUP,
	REMOTE_T_PLUS, INPUT_PULLUP,
	REMOTE_WIND_SPEED_MINUS, INPUT_PULLUP,
	REMOTE_T_MINUS, INPUT_PULLUP,
	REMOTE_H_PLUS, INPUT_PULLUP,
	REMOTE_H_MINUS, INPUT_PULLUP,
	REMOTE_COND_PLUS, INPUT_PULLUP,
	REMOTE_POWER, INPUT_PULLUP,
	REMOTE_IN_EX, INPUT_PULLUP,
	REMOTE_DUCT, INPUT_PULLUP,
	REMOTE_MODE, INPUT_PULLUP,
	REMOTE_COND_MINUS, INPUT_PULLUP
};

enum
{
	OUTPUT_LCD_BL,
	OUTPUT_CHARGE,
	OUTPUT_SHTVDD,
	OUTPUT_S3_3V_ON,
	
	NUM_OF_OUTPUTS
};

const uint8_t PROGMEM output_pins_PGM[NUM_OF_OUTPUTS] = {
	REMOTE_LCD_BL,
	REMOTE_CHARGE,
	REMOTE_SHTVDD,
	REMOTE_S3_3V_ON
};

const TimedInput PROGMEM timedInputs_PGM[] = {
	{ INPUT_MODE,  3000 },
	{ INPUT_POWER, 3000 },
	{ INPUT_DUCT,  3000 }
};

enum
{
	TIMER_BLINK,
	TIMER_SENSORS,
	TIMER_STANDBY,
	TIMER_TEMP,
	
	TIMER_BREEZER,
	TIMER_HUMIDIFIER,
	TIMER_COND,
	
	TIMER_COND_BUTTONS,
	TIMER_BREEZER_BUTTONS,
	TIMER_HUMIDIFER_BUTTONS,
	TIMER_SET_DAMPER,
	TIMER_PERMIT_SET_REMOTE_STATES,
	
	TIMER_DRAW_SMOOTHLY,
	TIMER_ANIMATION_CHAR_HUB,
	
	TIMER_RESEND,
	
	// antiflood of buttons
	TIMER_BUTTON_POWER,
	TIMER_BUTTON_DAMPER,
	TIMER_BUTTON_FLOW,
	TIMER_BUTTON_MODE,
	TIMER_BUTTON_BREEZER,
	TIMER_BUTTON_HUMIDIFIER,
	TIMER_BUTTON_CONDITIONER,
	TIMER_BUTTON_TEMPERATURE,
	// -----------------------

	TIMER_SHOW_THAT_BREEZER_BECAME_ONLINE,
	TIMER_SHOW_THAT_HUMIDIFIER_BECAME_ONLINE,
	
	TIMER_CONTIOUNE_RESET_FACTORY,
	TIMER_HIDE_FIRMWARE_VERSION,
	
	TIMER_SET_POLL_RATE,
	
	NUM_OF_TIMERS
};

class CRemote : public CFramework
{
public:
	CRemote();
	
	void setup();
	void loop();
	void setCO2ppm(int CO2ppm1);
	void showTextUpdate();
protected:
	void checkBat_ad();
	void controlCharge();
	void incrCmd();
	void onBlink();
	void redraw();
	void sendState();
	void updateBinding();
	void updateTemp();
	void updateCond();
	void updateBreezer();
	void updateHumidifier();
	void ProcessParameters();
	void permitSetRemoteStates();
	void drawSmoothly();
	void drawSmoothly_cooler(bool up_or_down);
	void drawSmoothly_breezer(bool up_or_down);
	void drawSmoothly_humidifier(bool up_or_down);
	void drawSmoothly_all(bool up_or_down);
	void startAnimation_charHub(bool visible);
	void Animation_charHub();
	void startAnimation_breezerIcon();
	void Animation_breezerIcon();
	void Animation_humidifierIcon();
	void startAnimation_humidifierIcon();
	void printErrorCode();
	
	void loadFirmwareVersion();
	void showFirmwareVersion();
	void resend_packet_start();
	
protected:
	void s3_3Off();
	void s3_3On();
protected:
	void inputEventRun(int input, InputState* inputState, unsigned long duration);
	bool timedInputEventRun(int input);
	bool timerEventRun(int timerIdx);

protected:
	bool _s3_3On, _saveSettings, _showACC;
	bool _breezerOnline, _humidifierOnline, _condOnline;
	int _bat_ad;
	float _deltaTemp, _savedUserTemperature;
	int8_t _deltaCond;
	int8_t _deltaBreezer;
	int8_t _deltaHumidifier;
	uint8_t _savedDamperPosition;
protected:
	BreezerState _breezerState;
	RemoteState _remoteState;
	RemoteState _remoteStateFromMaster;
	ErrorState _es;
	ErrorState _errorState;
	CondState _condState;
	HumidifierState _humidifierState;
	uint8_t _displayedErrorCode, _errorCode;
protected:
	CLCD_HVAC _lcd;
	InputState _remoteInputs[NUM_OF_INPUTS];
	CCountdownTimer _timers[NUM_OF_TIMERS];
	CSHT3X _sensor;
	CCustomRF_HVAC _ZigBee;
	
private:
	void onIncomingData(CBuffer* buf);
	void onLoadState();
};




#endif
