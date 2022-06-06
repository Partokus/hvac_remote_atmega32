#ifndef _rl_cc2530_pa2_h_
#define _rl_cc2530_pa2_h_

#include "core/countdown_timer.h"
#include "foundation/connectivity.h"

class CRL_CC2530_PA2 : public CConnectivity
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
	
	CRL_CC2530_PA2();
	
	bool isBindingAllowed() const { return _allowBinding; }
	bool isNetworkAvailable() const { return _networkAvailable; }
	int getStatus() const { return _status; }
	
	void bindDevice(bool allowBinding);
	
	bool restoreFactorySettings();
	
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
	
	void commandEventRun(unsigned char command, unsigned short param);
	int read();
	void write(unsigned char c);
	
	CCountdownTimer _checkTimer, _delayTimer;
	int _connectionAttempts, _status;
	bool _allowBinding, _isHost, _networkAvailable, _pinging;
	unsigned short _oldConnection_id;
};

#endif
