#include "stdafx.h"
#include "../main/Logger.h"
#include "NotificationKodi.h"
#include "Helper.h"

extern std::string szWWWFolder;

CNotificationKodi::CNotificationKodi() : CNotificationBase(std::string("kodi"), OPTIONS_NONE)
{
	SetupConfig(std::string("KodiEnabled"), &m_IsEnabled);
	SetupConfig(std::string("KodiIPAddress"), _IPAddress);
	SetupConfig(std::string("KodiPort"), &_Port);
	SetupConfig(std::string("KodiTimeToLive"), &_TTL);
}

CNotificationKodi::~CNotificationKodi()
{
}

const char* CNotificationKodi::IconFile(const std::string &ExtraData)
{
	std::stringstream logline;
#ifdef WIN32
	std::string	szImageFile = szWWWFolder  + "\\images\\logo.png";
#else
	std::string	szImageFile = szWWWFolder  + "/images/logo.png";
#endif

	// zero length 'extra data' must return logo

	if (!file_exist(szImageFile.c_str()))
	{
		logline << "File does not exist: " << szImageFile;
		_log.Log(LOG_ERROR, std::string(logline.str()).c_str());
		return (const char*)NULL;
	}

	logline << "Kodi Notification Image: " << szImageFile;
	_log.Log(LOG_NORM, std::string(logline.str()).c_str());
	
	return szImageFile.c_str();
}

bool CNotificationKodi::SendMessageImplementation(const std::string &Subject, const std::string &Text, const std::string &ExtraData, const int Priority, const std::string &Sound, const bool bFromNotification)
{
	std::stringstream logline;
	CAddress	_Address;
	int			_Sock;
	bool		bMulticast = (_IPAddress.substr(0,4) >= "224.") && (_IPAddress.substr(0,4) <= "239.");

	CAddress my_addr(_IPAddress.c_str(), _Port);
	_Address = my_addr;
	_Sock = -1;
	if (bMulticast) {
		_Sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		setsockopt(_Sock, IPPROTO_IP, IP_MULTICAST_TTL, &_TTL, sizeof(_TTL));
	}
	else {
		_Sock = socket(AF_INET, SOCK_DGRAM, 0);
	}
	
	if (_Sock < 0)
	{
		logline << "Error creating socket: " << _IPAddress << ":" << _Port;
		_log.Log(LOG_ERROR, std::string(logline.str()).c_str());
		return false;
	}

	_Address.Bind(_Sock);

	// Don't notify the same thing in two fields
	std::string	sSubject("Domoticz");
	if (Subject != Text) sSubject = Subject;

	logline << "Kodi Notification (" << _IPAddress << ":" << _Port << ", TTL " << _TTL << "): " << sSubject << ", " << Text;
	_log.Log(LOG_NORM, std::string(logline.str()).c_str());

	CPacketNOTIFICATION packet(sSubject.c_str(), Text.c_str(), ICON_PNG, IconFile(ExtraData));
	if (!packet.Send(_Sock, _Address)) {
		logline << "Error sending notification: " << _IPAddress << ":" << _Port;
		_log.Log(LOG_ERROR, std::string(logline.str()).c_str());
		return false;
	}

	return true;
}

bool CNotificationKodi::IsConfigured()
{
	return (!_IPAddress.empty());
}
