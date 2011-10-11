#include "TimeProvider.h"

time_t TimeProvider::m_time = 0;
bool TimeProvider::m_bDebug = false;

void TimeProvider::setup(time_t start, bool bDebug)
{
	m_time = start;
	m_bDebug = bDebug;
}

time_t TimeProvider::getTime(void)
{
	return m_time;
}

void TimeProvider::jumpTime(time_t offset)
{
	m_time += offset;
}

void TimeProvider::update(void)
{
	if (!m_bDebug)
	{
		m_time = time(0);
	}
	
}
