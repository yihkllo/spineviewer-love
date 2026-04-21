#ifndef SL_TIMER_H_
#define SL_TIMER_H_

#include <Windows.h>


class CWinClock
{
public:
    CWinClock()  { QueryPerformanceFrequency(&m_freq); Restart(); }
    ~CWinClock() = default;


    float GetElapsedTime() const
    {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return static_cast<float>(
            static_cast<double>(now.QuadPart - m_startTick.QuadPart) / m_freq.QuadPart);
    }

    void Restart() { QueryPerformanceCounter(&m_startTick); }

private:
    LARGE_INTEGER m_freq{};
    LARGE_INTEGER m_startTick{};
};

#endif
