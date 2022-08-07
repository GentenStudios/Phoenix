#pragma once

#include <thread>

class FrameLimit
{
public:
	FrameLimit()
	{

		prev = std::chrono::system_clock::now();
		next = prev + std::chrono::duration<int, std::ratio<1, 60>>{1};
	}

	// https://stackoverflow.com/questions/63429337/limit-fps-in-loop-c
	template <class Clock, class Duration>
	void SleepUntil( std::chrono::time_point<Clock, Duration> tp )
	{
		using namespace std::chrono;
		std::this_thread::sleep_until( tp - 10us );
		while (tp >= Clock::now())
			;
	}

	void Limit(  )
	{
		auto now = std::chrono::system_clock::now();
		prev = now;

		SleepUntil( next );
		next += std::chrono::duration<int, std::ratio<1, 60>>{ 1 };
	}
private:

    std::chrono::duration<long long, std::ratio<1, 60>> framerate;
	std::chrono::system_clock::time_point prev;
	std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<long long, std::ratio<1L, 3000000000>>> next;
};