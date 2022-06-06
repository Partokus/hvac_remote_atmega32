#include "remote.h"

#include "../core/mpu.h"

#include <avr/wdt.h>
#include <string.h>

#define EEPROM_ADDRESS 0

CRemote::CRemote()
: CFramework(_remoteInputs, input_pins_PGM, NUM_OF_INPUTS, output_pins_PGM, NUM_OF_OUTPUTS, timedInputs_PGM, NUM_OF_ELEMENTS(timedInputs_PGM), _timers, NUM_OF_TIMERS),
_s3_3On(false), _saveSettings(false), _showACC(false),
_breezerOnline(false), _humidifierOnline(false), _condOnline(false),
_bat_ad(0),
_deltaTemp(0.0f), _savedUserTemperature(0.0f), _deltaCond(0), _deltaBreezer(0), _deltaHumidifier(0),
_savedDamperPosition(0),
_lcd(REMOTE_DATA, REMOTE_WR, REMOTE_RD, REMOTE_CS),
_ZigBee(HEADER_REMOTE)
{
	memset(&_breezerState, 0, sizeof(_breezerState));
	memset(&_remoteState, 0, sizeof(_remoteState));
	memset(&_errorState, 0, sizeof(_errorState));
	memset(&_condState, 0, sizeof(_condState));
	memset(&_humidifierState, 0, sizeof(_humidifierState));
	
	_breezerState.header = HEADER_BREEZER;
	_remoteState.header = HEADER_REMOTE;
	_errorState.header = HEADER_BREEZER_ERR;
	_condState.header = HEADER_COND;
	_humidifierState.header = HEADER_HUMIDIFIER;
}

void CRemote::setup()
{
	CFramework::setup();
	digitalWrite(REMOTE_SHTVDD, HIGH);

	s3_3On();
	
	
	if (!load(CONFIG_EEPROM_ADDRESS, (uint8_t*)&_remoteState, sizeof(_remoteState)))
	{
		_remoteState.userBlowerSpeed = 0;
		_remoteState.userTemperature = 22.0f;
	}
	
	loadFirmwareVersion();
	
	onLoadState();
	
	startTimer(TIMER_BLINK, BLINK_INTERVAL);
	startTimer(TIMER_SENSORS, SENSOR_UPDATE_INTERVAL);
	
	startTimer(TIMER_TEMP, UPDATE_START_INTERVAL);	
	startTimer(TIMER_COND_BUTTONS, UPDATE_START_INTERVAL);
	startTimer(TIMER_BREEZER_BUTTONS, UPDATE_START_INTERVAL);
	startTimer(TIMER_HUMIDIFER_BUTTONS, UPDATE_START_INTERVAL);
	startTimer(TIMER_SET_DAMPER, SET_DAMPER_TIME);
	
	#ifdef DEBUG1
	//_lcd.setAll();
	_condOnline = 1;
	_breezerOnline = 1;
	_humidifierOnline = 1;
	#endif

	digitalWrite(REMOTE_LCD_BL, HIGH);
	startTimer(TIMER_STANDBY, STANDBY_INTERVAL);
	startTimer(TIMER_SET_POLL_RATE, TIMER_SET_POLL_RATE_DELAY);
}

void CRemote::loop()
{
	CFramework::loop();
	
	controlCharge();
	updateBinding();
	
	_ZigBee.autoconnect(); 
	
	CBuffer* buffer = _ZigBee.lockBuffer();
		
	if (buffer)
	{
		onIncomingData(buffer);
		_ZigBee.unlockBuffer();
	}
	
}

void CRemote::checkBat_ad()
{
	int curBat_ad = analogRead(REMOTE_BAT_AD);
	
	if (_bat_ad > TURN_OFF_BAT_AD && curBat_ad <= TURN_OFF_BAT_AD)
	{
		delay(300);
		curBat_ad = analogRead(REMOTE_BAT_AD);
		
		if (curBat_ad <= TURN_OFF_BAT_AD)
		{
			s3_3Off();
		}
	}
	else if (curBat_ad > SHOW_ACC_BAT_AD)
	{
		s3_3On();
	}
	
	_bat_ad = curBat_ad;
}

void CRemote::controlCharge()
{
	if (_remoteInputs[INPUT_DC_IN].state == LOW)
	{
		return;
	}
	
	_bat_ad = analogRead(REMOTE_BAT_AD);
	
	if (_bat_ad < MIN_BAT_AD)
	{
		digitalWrite(REMOTE_CHARGE, HIGH);
	}
	else if (_bat_ad > MAX_BAT_AD)
	{
		digitalWrite(REMOTE_CHARGE, LOW);
	}
}

void CRemote::incrCmd()
{
	_remoteState.cmd = 1;
	/*
	++_remoteState.cmd;
	
	if (_remoteState.cmd < NUM_OF_CMDS)
	{
		_remoteState.cmd = NUM_OF_CMDS;
	}
	*/
}

void CRemote::onBlink()
{
	
}



void CRemote::redraw()
{
	_lcd.setDecoration(true);
	
	//_lcd.setSensorCO2ppm(-1);
	_lcd.setSensorHumidity(_sensor.humidity);
	_lcd.setSensorTemperature(_sensor.cTemp);
	
	//if (_condOnline) _lcd.setChar(CHAR_HUB, HIGH);
	//else _lcd.setChar(CHAR_HUB, LOW);
		
	if (!_remoteState.powerOn) return;
	
	
	
	#ifdef DEBUG1
	_breezerOnline = 1;
	_condOnline = 1;
	_humidifierOnline = 1;
	#endif
	
	
	
	if (_breezerOnline || _condOnline)
	{
		
		//if (_remoteState.userTemperature > MIN_TEMP)
		//{
		_lcd.setUserTemperature(_remoteState.userTemperature, _remoteState.nightOn);
		//} 
		
#ifdef DEBUG1
		//_lcd.setSensorCO2ppm(set_damper_timer_flag);
#endif

		if (_breezerOnline)
		{
			if (remoteStateWasGot) _lcd.setUserBlowerSpeed(_remoteState.userBlowerSpeed, _remoteState.nightOn);
			//if (ds_breezer_flag == 0) _lcd.setUserBlowerSpeed_optimized(_remoteState.userBlowerSpeed, _remoteState.nightOn);
			
			
			
			#ifdef DAMPER_AUTO_CONTROL
			if (_remoteState.userBlowerSpeed != 0) 
			{
				if (set_damper_timer_flag == 2) _lcd.setUserDamperPos(_remoteState.userDamperPosition);
				else if (set_damper_timer_flag == 1) _lcd.setUserDamperPos(last_damper_position);
			} else if (_remoteState.userBlowerSpeed == 0) // заслонка закрыта если скорость приточки равна 0
			{
				_lcd.setUserDamperPos(THE_LIMIT);
			}
			#else
			
			if (_remoteState.userBlowerSpeed != 0)
			{
				_lcd.setUserDamperPos(_remoteState.userDamperPosition);
			} else if (_remoteState.userBlowerSpeed == 0) // заслонка закрыта если скорость приточки равна 0
			{
				_lcd.setUserDamperPos(THE_LIMIT);
			}
			
			#endif
			
			
			
			
		}
	
		if (_condOnline)
		{
			if (remoteStateWasGot) _lcd.setUserCoolerIntensity(_remoteState.REFfanSpeed, _remoteState.nightOn);
			_lcd.setChar(CHAR_NIGHT, _remoteState.nightOn);
			_lcd.setChar(CHAR_AUTO, _remoteState.autoOn);	
		}
		
		
	} 
	
	
	if (_humidifierOnline)
	{
		if (remoteStateWasGot) _lcd.setUserHumidity(_remoteState.userHumidity, _remoteState.nightOn);
	}

}

void CRemote::sendState()
{
	
	_ZigBee.send((unsigned char*)&_remoteState, (unsigned char)sizeof(_remoteState));
}

void CRemote::updateBinding()
{
	if (is_binding_device)
	{
		digitalWrite(REMOTE_LCD_BL, HIGH);
				
		if (_remoteInputs[INPUT_MODE].state == HIGH)
		{
			_lcd.setChar(CHAR_SEARCH, false);
			//_ZigBee.bindDevice(false);
			is_binding_device = 0;
			startTimer(TIMER_STANDBY, STANDBY_INTERVAL);
		}
	}
}



void CRemote::s3_3Off()
{
	if (_s3_3On)
	{
		_lcd.clear();
		delay(100);
		_s3_3On = false;
		digitalWrite(REMOTE_S3_3V_ON, LOW);	
	}
}

void CRemote::s3_3On()
{
	if (!_s3_3On)
	{
		_s3_3On = true;
		
		digitalWrite(REMOTE_S3_3V_ON, HIGH);
		delay(300);
		
		_lcd.init();
		_ZigBee.init();
		
		
		_condOnline = 0;
		_breezerOnline = 0;
		_humidifierOnline = 0;
		stopTimer(TIMER_COND);
		stopTimer(TIMER_BREEZER);
		stopTimer(TIMER_HUMIDIFIER);
		
		_sensor.get();
		redraw();
	}
}

void CRemote::inputEventRun(int input, InputState* inputState, unsigned long duration)
{
	bool can_send_buttons = 0;
	
	if (inputState->state == HIGH)
	{
		switch (input)
		{
			case INPUT_T_PLUS:
			case INPUT_T_MINUS:
			case INPUT_COND_MINUS:
			case INPUT_COND_PLUS:
			case INPUT_H_MINUS:
			case INPUT_H_PLUS:
			case INPUT_WIND_SPEED_MINUS:
			case INPUT_WIND_SPEED_PLUS:
			case INPUT_MODE:
			case INPUT_DUCT:
			case INPUT_DC_IN:
				break;
			default:
				return;
		}
	}
	else
	{
		if (input == INPUT_MODE)
			return;
	}
	
	digitalWrite(REMOTE_LCD_BL, HIGH);
	startTimer(TIMER_STANDBY, STANDBY_INTERVAL);
	permitSetRemoteStates();
	
	switch (input)
	{			
		case INPUT_DUCT:
			if (_remoteState.powerOn == true && _breezerOnline)
			{
				_remoteState.powerOn = false;
				
				last_damper_position = _remoteState.userDamperPosition;
				_remoteState.userDamperPosition = ZERO1;
				if (_remoteState.userBlowerSpeed == 0) {
					_remoteState.userBlowerSpeed = 1;
					was_breezer_speed_zero = true;
				}
				_lcd.setChar(CHAR_DUCT, HIGH);
				drawSmoothly_all(LOW);
			} else break;

			_remoteState.REFmode = _remoteState.powerOn ? 1 : 0;
			_saveSettings = true;
			
			if (can_send_button_power) {
				can_send_buttons = 1;
				can_send_button_power = 0;
				} else {
				can_send_button_power_delayed = 1;
			}
			startTimer(TIMER_BUTTON_FLOW, TIMER_BUTTONS_DELAY);
			break;
			
		case INPUT_IN_EX:
			if (_remoteState.powerOn == 1 && _remoteState.userBlowerSpeed != 0 && _breezerOnline == 1)
			{
				
#ifdef DAMPER_AUTO_CONTROL
			if (set_damper_timer_flag == 2)
			{
				
			
			_remoteState.userDamperPosition = (_remoteState.userDamperPosition + 1) % NUM_OF_DUMPER_POSITIONS;
			
			if (_breezerState.tempIn < 0)
			{
				if (_remoteState.userDamperPosition == MIDDLE)
				{
					++_remoteState.userDamperPosition;
				}
			}
			
			_remoteState.autoOn = _remoteState.nightOn = false;
			_savedDamperPosition = _remoteState.userDamperPosition;
			_saveSettings = true;
			
			} else if (set_damper_timer_flag == 1) {
				
				
				last_damper_position = (last_damper_position + 1) % NUM_OF_DUMPER_POSITIONS;
				_lcd.setUserDamperPos(last_damper_position);
				if (_breezerState.tempIn < 0)
				{
					if (last_damper_position == MIDDLE)
					{
						++last_damper_position;
					}
				}
				
				_remoteState.autoOn = _remoteState.nightOn = false;
				_savedDamperPosition = _remoteState.userDamperPosition;
				_saveSettings = true;
				
			}
#else
_remoteState.userDamperPosition = (_remoteState.userDamperPosition + 1) % NUM_OF_DUMPER_POSITIONS;

if (_breezerState.tempIn < 0)
{
	if (_remoteState.userDamperPosition == MIDDLE)
	{
		++_remoteState.userDamperPosition;
	}
}

_remoteState.autoOn = _remoteState.nightOn = false;
_savedDamperPosition = _remoteState.userDamperPosition;
_saveSettings = true;

if (can_send_button_damper) {
	can_send_buttons = 1;
	can_send_button_damper = 0;
	} else {
	can_send_button_damper_delayed = 1;
}
startTimer(TIMER_BUTTON_DAMPER, TIMER_BUTTONS_DELAY);

#endif /* DAMPER_AUTO_CONTROL */
			}
			break;
			
			
			
		case INPUT_COND_PLUS:
			/*
			if (_remoteState.REFfanSpeed < NUM_OF_COND_STAGES)
			{
				++_remoteState.REFfanSpeed;
				_saveSettings = true;


			}
			*/
			if (_remoteState.powerOn == 1)
			{
				
			if (inputState->state == LOW)
			{
				_deltaCond = +1;
				//cond_plus_cnt++;
				//if (cond_plus_cnt > 2) cond_plus_cnt = 2;
				updateCond();
			}
			else
			{
				
				_deltaCond = 0;
				//cond_plus_cnt = 0;
				_saveSettings = true;
				
				if (can_send_button_conditioner) {
					can_send_buttons = 1;
					can_send_button_conditioner = 0;
				} else {
					can_send_button_conditioner_delayed = 1;
				}
				startTimer(TIMER_BUTTON_CONDITIONER, TIMER_BUTTONS_DELAY);
			}
			
			startTimer(TIMER_COND_BUTTONS, UPDATE_START_INTERVAL);
			
			
			}
			break;
			
			
		case INPUT_COND_MINUS:
		/*
			if (_remoteState.REFfanSpeed > 0)
			{
				--_remoteState.REFfanSpeed;
				_saveSettings = true;


			}
			*/
		    if (_remoteState.powerOn == 1)
		    {
			
			if (inputState->state == LOW)
			{
				_deltaCond = -1;
				updateCond();
			}
			else
			{
				
				_deltaCond = 0;
				_saveSettings = true;
				
				if (can_send_button_conditioner) {
					can_send_buttons = 1;
					can_send_button_conditioner = 0;
					} else {
					can_send_button_conditioner_delayed = 1;
				}
				startTimer(TIMER_BUTTON_CONDITIONER, TIMER_BUTTONS_DELAY);
			}
			startTimer(TIMER_COND_BUTTONS, UPDATE_START_INTERVAL);
			
			}
			break;
		
		
		
		case INPUT_H_MINUS:
		/*
			if (_remoteState.userHumidity > 0)
			{
				--_remoteState.userHumidity;
				_saveSettings = true;
				

			}
			*/
			if (_remoteState.powerOn == 1 && _humidifierState.fillWater != 1) // если нет ошибки "Fill Water" 
			{
		
			if (inputState->state == LOW)
			{
				_deltaHumidifier = -1;
				updateHumidifier();
			}
			else
			{
				_deltaHumidifier = 0;
				_saveSettings = true;
				
				if (can_send_button_humidifier) {
					can_send_buttons = 1;
					can_send_button_humidifier = 0;
					} else {
					can_send_button_humidifier_delayed = 1;
				}
				startTimer(TIMER_BUTTON_HUMIDIFIER, TIMER_BUTTONS_DELAY);
			}
			startTimer(TIMER_HUMIDIFER_BUTTONS, UPDATE_START_INTERVAL);
			
			}
			break;
		
		case INPUT_H_PLUS:
		/*
			if (_remoteState.userHumidity < NUM_OF_HUMIDIFIER_STAGES)
			{
				++_remoteState.userHumidity;
				_saveSettings = true;

			}
			*/
			if (_remoteState.powerOn == 1 && _humidifierState.fillWater != 1) // если нет ошибки "Fill Water" 
			{
				
			if (inputState->state == LOW)
			{
				_deltaHumidifier = +1;
				updateHumidifier();
			}
			else
			{
				_deltaHumidifier = 0;
				_saveSettings = true;
				
				if (can_send_button_humidifier) {
					can_send_buttons = 1;
					can_send_button_humidifier = 0;
					} else {
					can_send_button_humidifier_delayed = 1;
				}
				startTimer(TIMER_BUTTON_HUMIDIFIER, TIMER_BUTTONS_DELAY);
			}
			startTimer(TIMER_HUMIDIFER_BUTTONS, UPDATE_START_INTERVAL);
			
			}
			break;
			
		case INPUT_POWER:
			
			_remoteState.powerOn = !_remoteState.powerOn;
			
#ifdef DAMPER_AUTO_CONTROL	
			if (_remoteState.powerOn == 0)
			{				
						if (set_damper_timer_flag == 2) {
							last_damper_position = _remoteState.userDamperPosition;
							damper_flag = 0;
						}
						_remoteState.userDamperPosition = THE_LIMIT;
						set_damper_timer_flag = 0;
						stopTimer(TIMER_SET_DAMPER);
						timer_go = 0;
						_deltaBreezer = 0;
						_deltaCond = 0;
						_deltaHumidifier = 0;
						_deltaTemp = 0;
			} else 
			{ // если же скорость приточки больше нуля и последнее сохранённое положение заслонки ещё не выставлено (last_damper_position = 0xFF), то выставляем последнее сохранённое значение и ставим 0xFF в переменную
				//_remoteState.userDamperPosition = last_damper_position;
				set_damper_timer_flag = 1;
				damper_flag = 1;
				timer_go = 1;
				startTimer(TIMER_SET_DAMPER, SET_DAMPER_TIME);
				//last_damper_position = 0xFF;			
			}
#endif		

			
			
			if (_remoteState.powerOn == false)
			{
				last_damper_position = _remoteState.userDamperPosition;
				_remoteState.userDamperPosition = THE_LIMIT;
				//_lcd.powerOff();
				//_lcd.powerOff();
				drawSmoothly_all(LOW);
				//setCO2ppm(1);
				//startAnimation_charHub(LOW);
				

			} else {
				_remoteState.userDamperPosition = last_damper_position;
				// if was pressed DUCT button and breezer speed was zero
				if (was_breezer_speed_zero) 
				{
					_remoteState.userBlowerSpeed = 0;
					was_breezer_speed_zero = false;
				}
				drawSmoothly_all(HIGH);
				//startAnimation_charHub(HIGH);

			}
			
			_remoteState.REFmode = _remoteState.powerOn ? 1 : 0;
			_saveSettings = true;
			
			if (can_send_button_power) {
				can_send_buttons = 1;
				can_send_button_power = 0;
				} else {
				can_send_button_power_delayed = 1;
			}
			startTimer(TIMER_BUTTON_POWER, TIMER_BUTTONS_DELAY);
			
			break;
			
		case INPUT_T_MINUS:
		if ((_breezerOnline || _condOnline) && _remoteState.powerOn == 1)
		{
			if (inputState->state == LOW)
			{
				_deltaTemp = -0.5f;
				updateTemp();
			}
			else
			{
				_deltaTemp = 0.0f;
				_saveSettings = true;
				
				if (can_send_button_temperature) {
					can_send_buttons = 1;
					can_send_button_temperature = 0;
					} else {
					can_send_button_temperature_delayed = 1;
				}
				startTimer(TIMER_BUTTON_TEMPERATURE, TIMER_BUTTONS_DELAY);
			}
			startTimer(TIMER_TEMP, UPDATE_START_INTERVAL);
		}
			break;
		
		case INPUT_T_PLUS:
		if ((_breezerOnline || _condOnline) && _remoteState.powerOn == 1)
		{
			if (inputState->state == LOW)
			{
				_deltaTemp = +0.5f;
				updateTemp();
			}
			else
			{
				_deltaTemp = 0.0f;
				_saveSettings = true;
				
				if (can_send_button_temperature) {
					can_send_buttons = 1;
					can_send_button_temperature = 0;
					} else {
					can_send_button_temperature_delayed = 1;
				}
				startTimer(TIMER_BUTTON_TEMPERATURE, TIMER_BUTTONS_DELAY);
			}
			startTimer(TIMER_TEMP, UPDATE_START_INTERVAL);
		}
			break;
			
		case INPUT_WIND_SPEED_MINUS:
		/*
			if (_remoteState.userBlowerSpeed > 0)
			{
				--_remoteState.userBlowerSpeed;
				_saveSettings = true;

			}
			*/
			if (_remoteState.powerOn == 1)
			{
				
			if (inputState->state == LOW)
			{
				_deltaBreezer = -1;
				updateBreezer();
			}
			else
			{
				_deltaBreezer = 0;
				_saveSettings = true;
				
				if (can_send_button_breezer) {
					can_send_buttons = 1;
					can_send_button_breezer = 0;
					} else {
					can_send_button_breezer_delayed = 1;
				}
				startTimer(TIMER_BUTTON_BREEZER, TIMER_BUTTONS_DELAY);
			}
			startTimer(TIMER_BREEZER_BUTTONS, UPDATE_START_INTERVAL);
			
			}
			break;
			
		case INPUT_MODE:
		if (_remoteState.powerOn == 1)
		{
		if (_condOnline)
		{
			if (_remoteState.nightOn == false && _remoteState.autoOn == false)
			{
				_remoteState.nightOn = true;
				if (_remoteState.userBlowerSpeed >= 1) _remoteState.userBlowerSpeed = 1;
				if (_remoteState.REFfanSpeed >= 1) _remoteState.REFfanSpeed = 1;
				if (_remoteState.userHumidity >= 1) _remoteState.userHumidity = 1;
				if (_breezerOnline) _lcd.setUserBlowerSpeed(_remoteState.userBlowerSpeed, _remoteState.nightOn);
				if (_condOnline) _lcd.setUserCoolerIntensity(_remoteState.REFfanSpeed, _remoteState.nightOn);
				if (_humidifierOnline) _lcd.setUserHumidity(_remoteState.userHumidity, _remoteState.nightOn);
				// запрещаем перезаписывать состояния пульта втечение нескольких секунд, чтобы случайно пакет, пришедший с кондиционера не перезаписал обратно состояния пульта
				//permitSetRemoteStates();
			}
			else if (_remoteState.nightOn == true && _remoteState.autoOn == false)
			{
				_remoteState.nightOn = false;
				_remoteState.autoOn = true;
				_lcd.setSegment(LCD_S7, LOW); // ZzzZzZzZ
				_lcd.setSegment(LCD_S3, LOW); // ZzzZzZzZ
				_lcd.setSegment(LCD_S5, LOW); // ZzzZzZzZ
				//permitSetRemoteStates();
			}
			else
			{
				_remoteState.nightOn = false;
				_remoteState.autoOn = false;
				
				//permitSetRemoteStates();				
			}
			_saveSettings = true;
			
			if (can_send_button_mode) {
				can_send_buttons = 1;
				can_send_button_mode = 0;
				} else {
				can_send_button_mode_delayed = 1;
			}
			startTimer(TIMER_BUTTON_MODE, TIMER_BUTTONS_DELAY);
		}
		}
			break;
		
		case INPUT_WIND_SPEED_PLUS:
		/*
			if (_remoteState.userBlowerSpeed < (NUM_OF_BLOWER_SPEEDS - 1))
			{
				++_remoteState.userBlowerSpeed;
				_saveSettings = true;
			}
			*/
		
		if (_remoteState.powerOn == 1)
		{
		if (inputState->state == LOW)
		{
			_deltaBreezer = +1;
			updateBreezer();
		}
		else
		{
			_deltaBreezer = 0;
			_saveSettings = true;
			
			if (can_send_button_breezer) {
				can_send_buttons = 1;
				can_send_button_breezer = 0;
			} else {
				can_send_button_breezer_delayed = 1;
			}
			startTimer(TIMER_BUTTON_BREEZER, TIMER_BUTTONS_DELAY);
		}
		startTimer(TIMER_BREEZER_BUTTONS, UPDATE_START_INTERVAL);
		}
			break;
	}
	
	switch (input)
	{
		case INPUT_COND_MINUS:
		case INPUT_COND_PLUS:
		case INPUT_H_MINUS:
		case INPUT_H_PLUS:
		case INPUT_WIND_SPEED_MINUS:
		case INPUT_WIND_SPEED_PLUS:
			if (_remoteState.autoOn)
			{
				_remoteState.autoOn = false;
				//permitSetRemoteStates();
				
				//_saveSettings = true;
			}
			if (_remoteState.nightOn)
			{
				_remoteState.nightOn = false;
				_lcd.setSegment(LCD_S7, LOW); // ZzzZzZzZ
				_lcd.setSegment(LCD_S3, LOW); // ZzzZzZzZ
				_lcd.setSegment(LCD_S5, LOW); // ZzzZzZzZ
				//permitSetRemoteStates();
				//_saveSettings = true;
			}
	}
	
	if (_saveSettings)
	{
		//ProcessParameters();
		//_lcd.setSensorCO2ppm(0);
		
		redraw();
		if (can_send_buttons)
		{
			incrCmd();
			sendState();
			resend_packet_start();
		}
		
		_saveSettings = false;
		save_settings_2 = true;
	}
}

bool CRemote::timedInputEventRun(int input)
{
	switch (input)
	{
		case INPUT_MODE:
			is_binding_device = 1;
			_lcd.setChar(CHAR_SEARCH, true);
			startTimer(TIMER_CONTIOUNE_RESET_FACTORY, TIMER_CONTIOUNE_RESET_FACTORY_DELAY);
			return true;
			
		case INPUT_POWER:
			_lcd.clear();
			showFirmwareVersion();
			delay(100);
			_ZigBee.SystemResetE18();
			wdt_enable(WDTO_15MS);
			while(1);
			
		case INPUT_DUCT:
			showFirmwareVersion();
			return true;
	}
	
	return false;
}

bool CRemote::timerEventRun(int timerIdx)
{
	switch (timerIdx)
	{
		case TIMER_BLINK:
			checkBat_ad();
			onBlink();
			return true;
			
		case TIMER_SENSORS:
			if (ds_cooler_flag == 0 && ds_breezer_flag == 0 && ds_humidifier_flag == 0 && ach_flag == 0 && _deltaBreezer == 0 && _deltaCond == 0 && _deltaTemp == 0 && _deltaHumidifier == 0)
			{
				_sensor.get();
				redraw();
			}		
			return true;
			
		case TIMER_STANDBY:
			if (save_settings_2 && _savedDamperPosition == _remoteState.userDamperPosition)
			{
				save(CONFIG_EEPROM_ADDRESS, (uint8_t*)&_remoteState, sizeof(_remoteState));
				
				_remoteState.saveSettings = true;
				incrCmd();
				sendState();
				_remoteState.saveSettings = false;
				save_settings_2 = false;
			}
			digitalWrite(REMOTE_LCD_BL, LOW);
			return false;
			
		case TIMER_TEMP:
			updateTemp();
			return true;
		
		case TIMER_COND_BUTTONS:
			updateCond();
		return true;
		
		case TIMER_BREEZER_BUTTONS:
			updateBreezer();
		return true;
		
		case TIMER_HUMIDIFER_BUTTONS:
			updateHumidifier();
		return true;
		
		case TIMER_BREEZER:
			_breezerOnline = false;
			digitalWrite(REMOTE_LCD_BL, HIGH);
			startTimer(TIMER_STANDBY, STANDBY_INTERVAL);
			if (_remoteState.powerOn) drawSmoothly_breezer(LOW);
			_lcd.setSegment(LCD_S2, LOW); // icon of breezer
			_lcd.setChar(CHAR_DUCT, LOW);
			_lcd.setUserDamperPos(CLEAR_DAMPER_POSITION);
			return false;
			
		case TIMER_HUMIDIFIER:
			_humidifierOnline = false;
			digitalWrite(REMOTE_LCD_BL, HIGH);
			startTimer(TIMER_STANDBY, STANDBY_INTERVAL);
			if (_remoteState.powerOn) drawSmoothly_humidifier(LOW);
			_lcd.setSegment(LCD_S4, LOW); // icon of humidifier
			return false;
			
		case TIMER_COND:
			_condOnline = false;
			digitalWrite(REMOTE_LCD_BL, HIGH);
			startTimer(TIMER_STANDBY, STANDBY_INTERVAL);
			startAnimation_charHub(LOW);
			if (_remoteState.powerOn) drawSmoothly_cooler(LOW);
			_lcd.setSegment(LCD_S6, LOW); // icon of conditioner
			return false;
			
		case TIMER_SET_DAMPER:
#ifdef DAMPER_AUTO_CONTROL
			if (damper_flag == 1 && (_remoteState.userBlowerSpeed > 0 && _remoteState.powerOn == 1))
			{
				_remoteState.userDamperPosition = last_damper_position;
			
				redraw();
				incrCmd();
				sendState();
				
				set_damper_timer_flag = 2;
				
					
			}	
			damper_flag = 0;
			stopTimer(TIMER_SET_DAMPER);
			#ifdef DEBUG1
			_lcd.setSensorCO2ppm(set_damper_timer_flag);
			#endif
			//timer_go = 0;
#endif
			return false;
			
		case TIMER_PERMIT_SET_REMOTE_STATES:
			permit_set_remote_states = 0; // разрешаем перезаписывать состояние пульта, приходящими от мастера командами
			return false;
			
		case TIMER_DRAW_SMOOTHLY:
			drawSmoothly();
			return true;
			
		case TIMER_ANIMATION_CHAR_HUB:
			Animation_charHub();
			return true;
			
		case TIMER_RESEND:
			
			if (resend_times_counter != 0)
			{
				sendState();
				resend_times_counter--;
			} else {
				stopTimer(TIMER_RESEND);
			}
			return true;
			
			
			
			
			
			
		// Антиспам для кнопок
		case TIMER_BUTTON_POWER:
			if (can_send_button_power_delayed) {
				sendState();
				resend_packet_start();
			}
			can_send_button_power_delayed = 0;
			can_send_button_power = 1;
		return false;
		
		case TIMER_BUTTON_DAMPER:
			if (can_send_button_damper_delayed) {
				sendState();
				resend_packet_start();
			}
			can_send_button_damper_delayed = 0;
			can_send_button_damper = 1;
		return false;
		
		case TIMER_BUTTON_FLOW:
			if (can_send_button_flow_delayed) {
				sendState();
				resend_packet_start();
			}
			can_send_button_flow_delayed = 0;
			can_send_button_flow = 1;
		return false;
		
		case TIMER_BUTTON_MODE:
			if (can_send_button_mode_delayed) {
				sendState();
				resend_packet_start();
			}
			can_send_button_mode_delayed = 0;
			can_send_button_mode = 1;
		return false;
		
		case TIMER_BUTTON_BREEZER:
			if (can_send_button_breezer_delayed) {
				sendState();
				resend_packet_start();
			}
			can_send_button_breezer_delayed = 0;
			can_send_button_breezer = 1;
		return false;
		
		case TIMER_BUTTON_HUMIDIFIER:
			if (can_send_button_humidifier_delayed) {
				sendState();
				resend_packet_start();
			}
			can_send_button_humidifier_delayed = 0;
			can_send_button_humidifier = 1;
		return false;
	
		case TIMER_BUTTON_CONDITIONER:
			if (can_send_button_conditioner_delayed) {
				sendState();
				resend_packet_start();
			}
			can_send_button_conditioner_delayed = 0;
			can_send_button_conditioner = 1;
		return false;
			
		case TIMER_BUTTON_TEMPERATURE:
			if (can_send_button_temperature_delayed) {
				sendState();
				resend_packet_start();
			}
			can_send_button_temperature_delayed = 0;
			can_send_button_temperature = 1;
		return false;
		// -------------------------------------------------
		
		case TIMER_SHOW_THAT_BREEZER_BECAME_ONLINE:
			Animation_breezerIcon();
			return true;
			
		case TIMER_SHOW_THAT_HUMIDIFIER_BECAME_ONLINE:
			Animation_humidifierIcon();
			return true;
			
		case TIMER_CONTIOUNE_RESET_FACTORY:
			if (is_binding_device)
			{
				startTimer(TIMER_SET_POLL_RATE, TIMER_SET_POLL_RATE_DELAY);
				_ZigBee.bindDevice(true);
				if (_remoteState.powerOn) drawSmoothly_all(LOW);
				else _lcd.clearIcons();
				ds_close_simultaneously = 0;
				_lcd.setChar(CHAR_HUB, LOW);
				_lcd.setChar(CHAR_DUCT, LOW);
				_lcd.setUserDamperPos(CLEAR_DAMPER_POSITION);
				_condOnline = 0;
				_breezerOnline = 0;
				_humidifierOnline = 0;
				_remoteState.nightOn = false;
				_remoteState.autoOn = false;
				stopTimer(TIMER_COND);
				stopTimer(TIMER_BREEZER);
				stopTimer(TIMER_HUMIDIFIER);
				digitalWrite(REMOTE_LCD_BL, HIGH);
				startTimer(TIMER_STANDBY, STANDBY_INTERVAL);
			}
			return false;
			
		case TIMER_SET_POLL_RATE:
			_ZigBee.SetPollRateE18(POLL_RATE);
			return false;
		
		case TIMER_HIDE_FIRMWARE_VERSION:
			_lcd.setFirmwareVersion(-1);
			break;
			
	}
		
	return false;
}

void CRemote::onIncomingData(CBuffer* buf)
{
	uint8_t header = 0;
	
	buf->read(&header, sizeof(header));
	buf->seekAbs(0);
	
	switch (header)
	{
		case HEADER_BREEZER:
			buf->read(&_breezerState, sizeof(_breezerState));
			startTimer(TIMER_BREEZER, BREEZER_TIMEOUT);
			if (!_breezerOnline)
			{
				_breezerOnline = true;
				digitalWrite(REMOTE_LCD_BL, HIGH);
				startTimer(TIMER_STANDBY, STANDBY_INTERVAL);
				_remoteState.userBlowerSpeed = _breezerState.blowerSpeed;
				_remoteState.userDamperPosition = _breezerState.damperPosition;
				was_breezer_speed_zero = false;
				_lcd.setUserDamperPos(_remoteState.userDamperPosition);
				if (_remoteState.powerOn) {
					drawSmoothly_breezer(HIGH);
					//if (_remoteState.userBlowerSpeed != 0) _lcd.setUserDamperPos(_remoteState.userDamperPosition);
					_lcd.setUserTemperature(_remoteState.userTemperature, _remoteState.nightOn);
				} else {
					startAnimation_breezerIcon();
				}
			}
			break;
			
		case HEADER_REMOTE:
			if (permit_set_remote_states == 0) // если мы не запрещаем перезаписывать состояния пульта
			{			
				bool power_on;
				power_on = _remoteState.powerOn;
				
				buf->read(&_remoteState, sizeof(_remoteState));			
				_remoteState.saveSettings = false;
				was_breezer_speed_zero = false;
				
				if (power_on != _remoteState.powerOn)
				{
					if (!_remoteState.powerOn)
					{
						drawSmoothly_all(LOW);
					} else {
						drawSmoothly_all(HIGH); 
						redraw();
					}
				} else {
					remoteStateWasGot = 1;
					redraw();
					remoteStateWasGot = 0;
				}
				
				
			}	
			return;
			
		case HEADER_BREEZER_ERR:
			buf->read(&_es, sizeof(_es));
			
			_errorCode = 0;
			
			for (int8_t i = (NUM_OF_ERRORS - 1); i >= 0; --i)
			{
				if ((_es.errorFlags & (1 << i)) != 0)
				{
					_errorCode = i + 1;
					break;
				}
			}
			
			if (_errorCode > 0)
			{
				_displayedErrorCode = _errorCode;
				printErrorCode();
			}
			return;
			
		case HEADER_COND:
			buf->read(&_condState, sizeof(_condState));
			startTimer(TIMER_COND, COND_TIMEOUT);
			if (!_condOnline)
			{
				digitalWrite(REMOTE_LCD_BL, HIGH);
				startTimer(TIMER_STANDBY, STANDBY_INTERVAL);
				startAnimation_charHub(HIGH);
				//_lcd.setChar(CHAR_HUB, true);
				_remoteState.REFfanSpeed = _condState.REFfanSpeed;
				
				_condOnline = true;
				
				if (_remoteState.powerOn) 
				{
					_lcd.setUserTemperature(_remoteState.userTemperature, _remoteState.nightOn);
					drawSmoothly_cooler(HIGH);
				}
				
				
				
				//redraw();
			}
			break;
			
			
			
			
		case HEADER_HUMIDIFIER:
			buf->read(&_humidifierState, sizeof(_humidifierState));
			startTimer(TIMER_HUMIDIFIER, HUMIDIFIER_TIMEOUT);
			if (!_humidifierOnline)
			{
				_humidifierOnline = true;
				digitalWrite(REMOTE_LCD_BL, HIGH);
				startTimer(TIMER_STANDBY, STANDBY_INTERVAL);
				_remoteState.userHumidity = _humidifierState.humidifierStage;
				if (_remoteState.powerOn) drawSmoothly_humidifier(HIGH);
				else startAnimation_humidifierIcon();
				//redraw();
			}
			
			if (fillwater != _humidifierState.fillWater) // если пришло другое значения байта fillwater, то обновляем картинку
			{
				fillwater = _humidifierState.fillWater;
				if (_remoteState.powerOn) _lcd.setChar(CHAR_HUMIDIFIER_ERR, _humidifierState.fillWater);
				
				if (_humidifierState.fillWater == 1) // если ошибка
				{
					_remoteState.userHumidity = 0; // снижаем влажность до нуля
					if (_remoteState.powerOn) _lcd.setUserHumidity(_remoteState.userHumidity, _remoteState.nightOn);
				}
				
				//_lcd.setUserHumidity(_remoteState.userHumidity, _remoteState.nightOn);
			}
			break;
	}
	
	buf->seekAbs(sizeof(header));
	uint8_t lastCmd = 0;
	buf->read(&lastCmd, sizeof(lastCmd));
	
	if (lastCmd < _remoteState.cmd)
	{
		//sendState();
	}
}

void CRemote::onLoadState()
{
	_remoteState.header = HEADER_REMOTE;
	_remoteState.saveSettings = false;
	_savedDamperPosition = _remoteState.userDamperPosition;
	_savedUserTemperature = _remoteState.userTemperature;
	
	last_damper_position = _remoteState.userDamperPosition;
}


void CRemote::setCO2ppm(int CO2ppm1)
{
	_lcd.setSensorCO2ppm(CO2ppm1);
}


void CRemote::updateTemp()
{
	if (_deltaTemp > 0.0f && _remoteState.userTemperature < MIN_TEMP)
	{
		_remoteState.userTemperature = MIN_TEMP;
	}
	else
	{
		_remoteState.userTemperature += _deltaTemp;
	}
	
	if (_remoteState.userTemperature > MAX_TEMP)
	{
		_remoteState.userTemperature = MAX_TEMP;
	}
	
	_savedUserTemperature = _remoteState.REFtempSetp = _remoteState.userTemperature;
	
	if (_remoteState.REFtempSetp < MIN_TEMP)
	{
		_remoteState.REFtempSetp = MIN_TEMP;
	}
	
	if (_deltaTemp != 0.0f)
	{
		digitalWrite(REMOTE_LCD_BL, HIGH);
		startTimer(TIMER_STANDBY, STANDBY_INTERVAL);
		
		startTimer(TIMER_TEMP, UPDATE_INTERVAL_BUTTONS);
		
		if (_remoteState.userTemperature < MIN_TEMP)
		{
			if (_remoteState.userTemperature > 0.0f)
			{
				_remoteState.userTemperature = MIN_TEMP;
				//_remoteState.userTemperature = PID_TURN_OFF_TEMP; // PID controller should turn off PTC
				//_lcd.clear(); // remove indicator
				//redraw();
				//_lcd.clearUserTemperature();
			}
		}
		else
		{
			if (_breezerOnline || _condOnline) _lcd.setUserTemperature(_remoteState.userTemperature, _remoteState.nightOn);
		}
	}
}


void CRemote::updateCond()
{
	if (_deltaCond != 0)
	{
		_remoteState.REFfanSpeed += _deltaCond;
		//_lcd.setSensorCO2ppm(1234);
		startTimer(TIMER_COND_BUTTONS, UPDATE_INTERVAL_BUTTONS);
	}
	
	if (_remoteState.REFfanSpeed > 100)
	{
		_remoteState.REFfanSpeed = 0;
	}
	
	if (_remoteState.REFfanSpeed > NUM_OF_COND_STAGES)
	{
		_remoteState.REFfanSpeed = NUM_OF_COND_STAGES;
	}
	
	
	
	
	
	if (_deltaCond != 0 && _condOnline == 1)
	{
		digitalWrite(REMOTE_LCD_BL, HIGH);
		startTimer(TIMER_STANDBY, STANDBY_INTERVAL);
		
		if (ds_cooler_flag) _lcd.setUserCoolerIntensity(_remoteState.REFfanSpeed, _remoteState.nightOn); // если идёт плавное высвечивание
		else _lcd.setUserCoolerIntensity_optimized(_remoteState.REFfanSpeed, _remoteState.nightOn);
		
		ds_cooler_flag = 0;
		
	}
}


void CRemote::updateBreezer()
{
	if (_deltaBreezer != 0)
	{
		_remoteState.userBlowerSpeed += _deltaBreezer;
		//_lcd.setSensorCO2ppm(1234);
		startTimer(TIMER_BREEZER_BUTTONS, UPDATE_INTERVAL_BUTTONS);
	}
	
	if (_remoteState.userBlowerSpeed > 100)
	{
		_remoteState.userBlowerSpeed = 0;
	}
	/*
	if (_remoteState.userBlowerSpeed > (NUM_OF_BLOWER_SPEEDS - 1))
	{
		_remoteState.userBlowerSpeed = NUM_OF_BLOWER_SPEEDS - 1;
	}
	*/
	if (_remoteState.userBlowerSpeed > (NUM_OF_BLOWER_SPEEDS ))
	{
		_remoteState.userBlowerSpeed = NUM_OF_BLOWER_SPEEDS;
	}
	
	
	
	
	if (_deltaBreezer != 0 && _breezerOnline == 1)
	{
		digitalWrite(REMOTE_LCD_BL, HIGH);
		startTimer(TIMER_STANDBY, STANDBY_INTERVAL);
		//_lcd.setSensorCO2ppm(_remoteState.userBlowerSpeed);
		if (ds_breezer_flag) _lcd.setUserBlowerSpeed(_remoteState.userBlowerSpeed, _remoteState.nightOn);
		else _lcd.setUserBlowerSpeed_optimized(_remoteState.userBlowerSpeed, _remoteState.nightOn);
		ds_breezer_flag = 0;
		
		ProcessParameters();
		//_lcd.setUserBlowerSpeed_optimized(1+_remoteState.userBlowerSpeed, _remoteState.nightOn); // шкала без 0
	}
}


void CRemote::updateHumidifier()
{
	if (_deltaHumidifier != 0)
	{
		_remoteState.userHumidity += _deltaHumidifier;
		//_lcd.setSensorCO2ppm(1234);
		startTimer(TIMER_HUMIDIFER_BUTTONS, UPDATE_INTERVAL_BUTTONS);
	}
	
	if (_remoteState.userHumidity > 100)
	{
		_remoteState.userHumidity = 0;
	}
	
	if (_remoteState.userHumidity > NUM_OF_HUMIDIFIER_STAGES)
	{
		_remoteState.userHumidity = NUM_OF_HUMIDIFIER_STAGES;
	}
	
	if (_humidifierState.fillWater == 1)
	{
		_deltaHumidifier = 0;
		_remoteState.userHumidity = 0;
	}
	
	
	
	if (_deltaHumidifier != 0 && _humidifierOnline == 1)
	{
		digitalWrite(REMOTE_LCD_BL, HIGH);
		startTimer(TIMER_STANDBY, STANDBY_INTERVAL);
		
		_lcd.setUserHumidity(_remoteState.userHumidity, _remoteState.nightOn);
		ds_humidifier_flag = 0;
	}
	
}

/*
// функция заполняет ячейки при включении и стирает ячейки при выключении
void CRemote::setCellsOn_PowerOn_or_PowerOff(bool power_state)
{
	
}
*/

void CRemote::ProcessParameters(void)
{
#ifdef DAMPER_AUTO_CONTROL
	// обрабатываем комбинацию значений приточки и заслонки 
	if (_remoteState.userBlowerSpeed == 0) // если скорость приточки равна 0
	{
		if (set_damper_timer_flag == 2) {
			last_damper_position = _remoteState.userDamperPosition;
			damper_flag = 0;
		}
		_remoteState.userDamperPosition = THE_LIMIT;
		set_damper_timer_flag = 0;
		stopTimer(TIMER_SET_DAMPER);
		timer_go = 0;
	} else if (_remoteState.userBlowerSpeed > 0) 
	{ // если же скорость приточки больше нуля и последнее сохранённое положение заслонки ещё не выставлено (last_damper_position = 0xFF), то выставляем последнее сохранённое значение и ставим 0xFF в переменную 
		//_remoteState.userDamperPosition = last_damper_position;
		if (timer_go == 0) startTimer(TIMER_SET_DAMPER, SET_DAMPER_TIME);
		timer_go = 1;
		if (set_damper_timer_flag == 0) set_damper_timer_flag = 1;
		damper_flag = 1;
		
		//last_damper_position = 0xFF;
		
	}
	
	
#endif	
}

void CRemote::permitSetRemoteStates(void)
{
	// запрещаем перезаписывать состояния пульта втечение нескольких секунд, чтобы случайно пакет, пришедший с кондиционера не перезаписал обратно состояния пульта
	permit_set_remote_states = true;
	startTimer(TIMER_PERMIT_SET_REMOTE_STATES, PERMIT_SET_REMOTE_STATES_TIME);
}



void CRemote::drawSmoothly(void)
{
	
	
	if (ds_cooler_flag == 1)
	{
		if (ds_cooler_value != _remoteState.REFfanSpeed)
		{
			ds_cooler_value++; // if == 2
			_lcd.setUserCoolerIntensity_optimized(ds_cooler_value, _remoteState.nightOn);			
		} else {
			ds_cooler_flag = 0;
		}
	} else if (ds_cooler_flag == 2)
	{
		if (ds_cooler_value != 0)
		{
			ds_cooler_value--;
			_lcd.setUserCoolerIntensity_optimized(ds_cooler_value, _remoteState.nightOn);
			} else {
				if (ds_close_simultaneously != 1)
				{
					_lcd.clearUserCoolerIntensity();
					_lcd.setChar(CHAR_AUTO, LOW);
					_lcd.setChar(CHAR_NIGHT, LOW);
				}
				ds_cooler_flag = 0;
		}
	}
	
	
	if (ds_breezer_flag == 1)
	{
		if (ds_breezer_value != _remoteState.userBlowerSpeed)
		{
			ds_breezer_value++;			
			_lcd.setUserBlowerSpeed_optimized(ds_breezer_value, _remoteState.nightOn);
			} else {
			ds_breezer_flag = 0;		
		}
	} else if (ds_breezer_flag == 2)
	{
			if (ds_breezer_value != 0)
			{
				ds_breezer_value--; 
				_lcd.setUserBlowerSpeed_optimized(ds_breezer_value, _remoteState.nightOn);
				} else {			
				if (ds_close_simultaneously != 1)
				{
					_lcd.clearUserBlowerSpeed();
					if (_breezerOnline) _lcd.setUserDamperPos(_remoteState.userDamperPosition);
					else _lcd.setUserDamperPos(CLEAR_DAMPER_POSITION);
				}
				ds_breezer_flag = 0;
			}
	}

	
	if (ds_humidifier_flag == 1)
	{
		if (ds_humidifier_value != _remoteState.userHumidity)
		{
			if (ds_humidifier_delay_counter == DS_HUMIDIFIER_DELAY) {
				ds_humidifier_value++; 	
				ds_humidifier_delay_counter = 0;	
			}
			_lcd.setUserHumidity(ds_humidifier_value, _remoteState.nightOn);
			ds_humidifier_delay_counter++;
			} else {
			ds_humidifier_flag = 0;		
		}
	} else if (ds_humidifier_flag == 2)
	{
		if (ds_humidifier_value != 0)
		{
			if (ds_humidifier_delay_counter == DS_HUMIDIFIER_DELAY) 
			{
				ds_humidifier_value--; 
				ds_humidifier_delay_counter = 0;
			}
			_lcd.setUserHumidity(ds_humidifier_value, _remoteState.nightOn);
			ds_humidifier_delay_counter++;
			} else {
				if (ds_close_simultaneously != 1)
				{
					_lcd.clearUserHumidity();
				}
				
				ds_humidifier_flag = 0;
		}
	}
	
	
	if (ds_cooler_flag == 0 && ds_breezer_flag == 0 && ds_humidifier_flag == 0) {
		if (ds_close_simultaneously == 1)
		{
			_lcd.clearUserCoolerIntensity();
			_lcd.clearUserBlowerSpeed();
			if (_breezerOnline) _lcd.setUserDamperPos(_remoteState.userDamperPosition);
			else _lcd.setUserDamperPos(CLEAR_DAMPER_POSITION);
			_lcd.clearUserHumidity();
			_lcd.clearUserTemperature();
			_lcd.setChar(CHAR_NIGHT, LOW);
			_lcd.setChar(CHAR_AUTO, LOW);
		} else if (ds_close_simultaneously == 2)
		{
			_lcd.setUserTemperature(_remoteState.userTemperature, _remoteState.nightOn);
			_lcd.setChar(CHAR_NIGHT, _remoteState.nightOn);
			_lcd.setChar(CHAR_AUTO, _remoteState.autoOn);
			_lcd.setChar(CHAR_DUCT, LOW);
			/*
			if (_remoteState.userBlowerSpeed != 0 && _breezerOnline == 1)
			{
				_lcd.setUserDamperPos(_remoteState.userDamperPosition);
			} else if (_remoteState.userBlowerSpeed == 0) // стираем заслонку с экрана если скорость приточки равна 0
			{
				_lcd.setSegment(LCD_T7, LOW);
				_lcd.setSegment(LCD_T8, LOW);
				_lcd.setSegment(LCD_T9, LOW);
			}
			*/	
		}
		
		ds_close_simultaneously = 0;
		
		if (!_condOnline && !_breezerOnline) _lcd.clearUserTemperature();
		
		stopTimer(TIMER_DRAW_SMOOTHLY); // если ничего больше не надо вырисовывать, то останавливаем таймер		
	}
	//else {
		//startTimer(TIMER_DRAW_SMOOTHLY, TIMER_DRAW_SMOOTHLY_TIME); // иначе продолжаем 
	//}
}

void CRemote::drawSmoothly_cooler(bool up_or_down)
{
	if (up_or_down == 1) { // если включили
		if (ds_cooler_flag == 0) ds_cooler_value = 0;
		ds_cooler_flag = 1;		
	} else { // если выключили
		if (ds_cooler_flag == 0) ds_cooler_value = _remoteState.REFfanSpeed;
		ds_cooler_flag = 2;		
	}
	
	
	_lcd.setSegment(LCD_S6, HIGH);
	_lcd.setSegment(LCD_S7, _remoteState.nightOn); // ZzzZzZzZ
	_lcd.setSegment(LCD_Y19, HIGH);
	
	
	startTimer(TIMER_DRAW_SMOOTHLY, TIMER_DRAW_SMOOTHLY_TIME); // вырисовывем уставку кондиционера
}

void CRemote::drawSmoothly_breezer(bool up_or_down)
{
	if (up_or_down == 1) { // если включили
		if (ds_breezer_flag == 0) ds_breezer_value = 0;
		ds_breezer_flag = 1;
		
		_lcd.setSegment(LCD_S2, HIGH);
		_lcd.setSegment(LCD_S3, _remoteState.nightOn); // ZzzZzZzZ
		_lcd.setSegment(LCD_Y0, HIGH);

		if (_remoteState.userBlowerSpeed != 0) _lcd.setUserDamperPos(_remoteState.userDamperPosition);
	} else {  // если выключили
		if (ds_breezer_flag == 0) ds_breezer_value = _remoteState.userBlowerSpeed;
		ds_breezer_flag = 2;
	}
	
	
	
	
	
	startTimer(TIMER_DRAW_SMOOTHLY, TIMER_DRAW_SMOOTHLY_TIME); // вырисовывем уставку кондиционера
}

void CRemote::drawSmoothly_humidifier(bool up_or_down)
{
	if (up_or_down == 1) { // если включили
		if (ds_humidifier_flag == 0) ds_humidifier_value = 0;
		ds_humidifier_flag = 1; 
	} else { // если выключили
		if (ds_humidifier_flag == 0) ds_humidifier_value = _remoteState.userHumidity;
		ds_humidifier_flag = 2; 
	}
	ds_humidifier_delay_counter = DS_HUMIDIFIER_DELAY;
	
	_lcd.setChar(CHAR_HUMIDIFIER_ERR, _humidifierState.fillWater);
	_lcd.setSegment(LCD_S4, HIGH);
	_lcd.setSegment(LCD_S5, _remoteState.nightOn); // ZzzZzZzZ
	_lcd.setSegment(LCD_Y18, HIGH);
	startTimer(TIMER_DRAW_SMOOTHLY, TIMER_DRAW_SMOOTHLY_TIME); // вырисовывем уставку кондиционера
}

void CRemote::drawSmoothly_all(bool up_or_down)
{
	if (_condOnline) drawSmoothly_cooler(up_or_down);
	if (_breezerOnline) drawSmoothly_breezer(up_or_down);
	if (_humidifierOnline) drawSmoothly_humidifier(up_or_down);
	
	if (_condOnline == true || _breezerOnline == true || _humidifierOnline == true) // if some device online
	{
		if (up_or_down == 0) ds_close_simultaneously = 1; // если выключаем, то скрыть скелеты уставок одновременно
		else ds_close_simultaneously = 2;
	}
}



void CRemote::Animation_charHub(void)
{
	ach_polarity = !ach_polarity;
	ach_value++;
	
	if (ach_value == TIMER_ANIMATION_CHAR_HUB_CYCLES)
	{
		ach_flag = 0;
		if (_condOnline == 1 && _remoteState.powerOn != 1) _lcd.setSegment(LCD_S6, HIGH);
		stopTimer(TIMER_ANIMATION_CHAR_HUB);
		return;
	}
	
	if (ach_value == 1) {
		_lcd.setChar(CHAR_HUB, ach_polarity);
		if (_condOnline == 1 && _remoteState.powerOn != 1) _lcd.setSegment(LCD_S6, ach_polarity);
		startTimer(TIMER_ANIMATION_CHAR_HUB, TIMER_ANIMATION_CHAR_HUB_TIME_LONG);
	}
	else {
		_lcd.setChar(CHAR_HUB, ach_polarity);
		if (_condOnline == 1 && _remoteState.powerOn != 1) _lcd.setSegment(LCD_S6, ach_polarity);
		startTimer(TIMER_ANIMATION_CHAR_HUB, TIMER_ANIMATION_CHAR_HUB_TIME_SHORT);
	}
}


void CRemote::startAnimation_charHub(bool visible)
{
	startTimer(TIMER_ANIMATION_CHAR_HUB, TIMER_ANIMATION_CHAR_HUB_TIME_FIRST);
	ach_value = 0;
	ach_polarity = visible;
	_lcd.setChar(CHAR_HUB, visible);
	if (_condOnline == 1 && _remoteState.powerOn != 1) _lcd.setSegment(LCD_S6, HIGH);
	
	if (visible) // если высчечиваем
	{
		ach_flag = 1;	
	} else { // если гасим
		ach_flag = 2;
	}
}


void CRemote::loadFirmwareVersion()
{
	/*
	if (EEPROM.read(EEPROM_ADDRESS) == 0x5a)
	{
		EEPROM.read(EEPROM_ADDRESS + 1, (uint8_t*)&_connection_id, sizeof(_connection_id));
	}
	else
	{
		_connection_id = _isHost ? 0 : 1;
	}
	*/
	

	uint16_t _myFirmwareVersion;
	_myFirmwareVersion = (uint16_t)VERSION_MAJOR << 8 | (uint16_t)VERSION_MINOR;
	
	if (EEPROM.read(FIRMWARE_VERSION_EEPROM_ADDRESS) == 0x5a)
	{
		uint16_t temp_vers = 0;
		EEPROM.read(FIRMWARE_VERSION_EEPROM_ADDRESS + 1, (uint8_t*)&temp_vers, sizeof(temp_vers));
		
		if (temp_vers == _myFirmwareVersion)
		{
			return;
		}
	}
	EEPROM.write(FIRMWARE_VERSION_EEPROM_ADDRESS, 0x5a);
	EEPROM.write(FIRMWARE_VERSION_EEPROM_ADDRESS + 1, (uint8_t*)&_myFirmwareVersion, sizeof(_myFirmwareVersion));
	
	// soft reset
	wdt_enable(WDTO_15MS);
	while(1);
}


void CRemote::showFirmwareVersion()
{
	uint16_t myFirmwareVersion_t;
	EEPROM.read(FIRMWARE_VERSION_EEPROM_ADDRESS + 1, (uint8_t*)&myFirmwareVersion_t, sizeof(myFirmwareVersion_t));
	_lcd.setFirmwareVersion(myFirmwareVersion_t);
	startTimer(TIMER_HIDE_FIRMWARE_VERSION, TIMER_HIDE_FIRMWARE_VERSION_DELAY);
}


void CRemote::showTextUpdate()
{
	_lcd.clear();
	_lcd.setDigit(4, 10);
	_lcd.setDigit(5, LCD_SYMBOL_U);
	_lcd.setDigit(6, LCD_SYMBOL_P);
	_lcd.setDigit(7, LCD_SYMBOL_D);
}


// Функция для переотправки сообщения. Необходима по причине того, что общение в ZigBee-сети происходит групповыми сообщения, у которых нет подтверждения ответа, тем
// самым пакеты могут не доходить до адреса с первой попытки.
void CRemote::resend_packet_start()
{
	resend_times_counter = TIMER_RESEND_TIMES;
	startTimer(TIMER_RESEND, TIMER_RESEND_PERIOD);
}



void CRemote::startAnimation_breezerIcon(void)
{
	startTimer(TIMER_SHOW_THAT_BREEZER_BECAME_ONLINE, TIMER_SHOW_THAT_BREEZER_BECAME_ONLINE_PERIOD);
	abi_value = 0;
	abi_polarity = 1;
	
	_lcd.setSegment(LCD_S2, HIGH); // icon of breezer
}


void CRemote::Animation_breezerIcon(void)
{
	abi_polarity = !abi_polarity;
	abi_value++;
	
	if (abi_value == TIMER_SHOW_THAT_BREEZER_BECAME_ONLINE_CYCLES || _remoteState.powerOn == 1)
	{
		if (_breezerOnline == 0) _lcd.setSegment(LCD_S2, LOW);
		stopTimer(TIMER_SHOW_THAT_BREEZER_BECAME_ONLINE);
		return;
	}
	
	_lcd.setSegment(LCD_S2, abi_polarity);
	//startTimer(TIMER_SHOW_THAT_BREEZER_BECAME_ONLINE, TIMER_SHOW_THAT_BREEZER_BECAME_ONLINE_PERIOD);
}





void CRemote::startAnimation_humidifierIcon()
{
	startTimer(TIMER_SHOW_THAT_HUMIDIFIER_BECAME_ONLINE, TIMER_SHOW_THAT_HUMIDIFIER_BECAME_ONLINE_PERIOD);
	ahi_value = 0;
	ahi_polarity = 1;
	
	_lcd.setSegment(LCD_S4, HIGH); // icon of humidifier
}

void CRemote::Animation_humidifierIcon(void)
{
	ahi_polarity = !ahi_polarity;
	ahi_value++;
	
	if (ahi_value == TIMER_SHOW_THAT_HUMIDIFIER_BECAME_ONLINE_CYCLES || _remoteState.powerOn == 1)
	{
		if (_humidifierOnline == 0) _lcd.setSegment(LCD_S4, LOW);
		stopTimer(TIMER_SHOW_THAT_HUMIDIFIER_BECAME_ONLINE);
		return;
	}
	
	_lcd.setSegment(LCD_S4, ahi_polarity);
	//startTimer(TIMER_SHOW_THAT_HUMIDIFIER_BECAME_ONLINE, TIMER_SHOW_THAT_HUMIDIFIER_BECAME_ONLINE_PERIOD);
}



void CRemote::printErrorCode()
{
	_lcd.setDigit(1, DIG_E);								/* E */
	_lcd.setDigit(2, DIG_DASH);								/* - */
	_lcd.setDigit(3, (_displayedErrorCode / 10 % 10));		/* d */
	_lcd.setDigit(4, (_displayedErrorCode % 10));			/* u */
}