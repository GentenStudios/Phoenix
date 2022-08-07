#pragma once

#include <SDL.h>
#include <SDL_syswm.h>

#include <string>
#include <chrono>
#include <map>
#include <vector>

class Statistic
{
public:
	Statistic();
	Statistic( std::string name );
	void Start( );
	void Stop( );
	std::string GetName( ) { return mName; }

	long long GetMicrosecconds( ) { return mLast; }
	long long GetMilisecconds( ) { return mLast / 1000; }
	float GetFloatMilisecconds( ) { return ((float) mLast) / 1000.0f; }
	float GetFloatSecconds( ) { return (((float) mLast) / 1000.0f) / 1000.0f; }
private:
	std::string mName;
	std::chrono::system_clock::time_point mDelta;
	long long mLast;
};

struct StatisticRecording
{
	std::string name;
	float time;
};

class StatisticManager
{
public:
	void StartStatistic( std::string name );
	void StopStatistic( std::string name );
	float GetStatisticSecconds( std::string name );
	float GetStatisticDelta( std::string name );
	void Update( );
	long long GetTotalMicrosecconds( ) { return mTotalMicrosecconds; }
	long long GetTotalMilisecconds( ) { return mTotalMicrosecconds / 1000; }
	float GetTotalFloatMilisecconds( ) { return ((float) mTotalMicrosecconds) / 1000.0f; }

	std::vector<StatisticRecording> GetRecordings( ) { return mLastRecordings; };
private:

	std::map<std::string, Statistic> mStats;
	std::vector<StatisticRecording> mLastRecordings;

	std::chrono::system_clock::time_point mDelta;
	long long mTotalMicrosecconds;
};