#ifndef _customRF_hvac_h_
#define _customRF_hvac_h_

#include "core/countdown_timer.h"
#include "foundation/connectivity.h"

#define FIRMWARE_VERSION_EEPROM_ADDRESS 0xC0
void do_impulse_for_wakeup_E18();
class CCustomRF_HVAC : public CConnectivity
{
public:
	enum Status
	{
		STATUS_OFFLINE,
		STATUS_DISCONNECTED,
		STATUS_SEARCHING,
		STATUS_CONNECTING,
		STATUS_READY
	};
	
	CCustomRF_HVAC(unsigned char type);
	
	bool isBindingAllowed() const { return _allowBinding; }
	bool isNetworkAvailable() const { return _networkAvailable; }
	int getStatus() const { return _status; }
	
	void bindDevice(bool allowBinding);
	bool restoreFactorySettings();
	void SystemResetE18(void);
	void SetPollRateE18(unsigned char my_poll_rate);
	
	void init();
	bool host();
	bool autoconnect();

protected:
	void connect();
	void searchHost();
	void load();
	void save();
	unsigned short readPan_id();
	bool setPan_id(unsigned short pan_id);
	int queryNetworkStatus();
	void responseUpdateCheck();
	void softReset();
	
	void commandEventRun(unsigned char command, unsigned short param);
	int read();
	void write(unsigned char c);
	bool updateCommandEventRun();
	
	CCountdownTimer _checkTimer, _delayTimer;
	int _connectionAttempts, _status;
	bool _allowBinding, _isHost, _networkAvailable, _pinging;
	unsigned short _oldConnection_id;
	unsigned short _myFirmwareVersion;
	unsigned char _type;
};

#endif
