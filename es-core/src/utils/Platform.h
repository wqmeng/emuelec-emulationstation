#pragma once
#ifndef ES_CORE_PLATFORM_H
#define ES_CORE_PLATFORM_H

#include <string>

#ifdef WIN32
#include <Windows.h>
#include <intrin.h>

#define sleep Sleep
#endif

class Window;

namespace Utils
{
	namespace Platform
	{
		enum QuitMode
		{
			QUIT = 0,
			RESTART = 1,
			SHUTDOWN = 2,
			REBOOT = 3,
			FAST_SHUTDOWN = 4,
			FAST_REBOOT = 5
		};

		class ProcessStartInfo
		{
		public:
			ProcessStartInfo();
			ProcessStartInfo(const std::string& cmd);

			int run() const;

			std::string command;			
			bool waitForExit;
			bool showWindow;
			Window* window;
		};

		int quitES(QuitMode mode = QuitMode::QUIT);
		bool isFastShutdown();
		void processQuitMode();

		struct BatteryInformation
		{
			BatteryInformation()
			{
				hasBattery = false;
				level = 0;
				isCharging = false;
			}

			bool hasBattery;
			int  level;
			bool isCharging;
		};

		BatteryInformation queryBatteryInformation();

		std::string queryIPAdress();
		std::string getArchString();
        #ifdef _ENABLEEMUELEC
            std::string getShOutput(const std::string& mStr); /* < emuelec */
        #endif
	}
}

#endif // ES_CORE_PLATFORM_H
