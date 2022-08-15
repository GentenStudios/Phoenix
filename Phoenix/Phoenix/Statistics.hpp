// BSD 3-Clause License
// 
// Copyright (c) 2022, Genten Studios
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
