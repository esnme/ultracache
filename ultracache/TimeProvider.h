#pragma once

#include <time.h>

class TimeProvider
{
public:
	static void setup(time_t start, bool bDebug);
	static time_t getTime(void);
	static void jumpTime(time_t offset);
	static void update(void);

private:
	static time_t m_time;
	static bool m_bDebug;
};