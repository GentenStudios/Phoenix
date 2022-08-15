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

#include <Phoenix/Statistics.hpp>

Statistic::Statistic()
{
	mLast = 0;
}

Statistic::Statistic( std::string name) : mName( name )
{
	mLast = 0;
}

void Statistic::Start( )
{
	mDelta = std::chrono::system_clock::now( );
}

void Statistic::Stop( )
{
	std::chrono::system_clock::time_point     end       = std::chrono::system_clock::now();
	std::chrono::duration<double, std::milli> diffrence = (end - mDelta);
	mLast = std::chrono::duration_cast<std::chrono::microseconds>(diffrence).count( );
}

void StatisticManager::StartStatistic( std::string name )
{
	if ( mStats.find( name ) == mStats.end( ) )
	{
		mStats[name] = Statistic( name );
	}
	mStats[name].Start( );
}

void StatisticManager::StopStatistic( std::string name )
{
	if ( mStats.find( name ) == mStats.end( ) )
	{
		mStats[name] = Statistic( name );
	}
	mStats[name].Stop( );
}

float StatisticManager::GetStatisticSecconds( std::string name )
{
	if ( mStats.find( name ) == mStats.end( ) )
	{
		mStats[name] = Statistic( name );
	}
	return mStats[name].GetFloatSecconds( );
}

float StatisticManager::GetStatisticDelta( std::string name )
{
	if (mStats.find( name ) == mStats.end())
	{
		mStats[name] = Statistic( name );
	}
	return mStats[name].GetFloatMilisecconds();
}

void StatisticManager::Update( )
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::duration<double, std::milli> miliseconds = (now - mDelta);
	if ( miliseconds.count() >= 1000.0 )
	{
		mDelta = now;
		mLastRecordings.clear( );
		mLastRecordings.reserve( mStats.size( ) );
		auto it = mStats.begin( );
		long long totalMicrosecconds = 0;
		for ( int i = 0 ; i < mStats.size();i++  )
		{
			mLastRecordings.push_back( {it->second.GetName( ),it->second.GetFloatMilisecconds( )} );
			totalMicrosecconds += it->second.GetMicrosecconds( );
			it++;
		}
		mTotalMicrosecconds = totalMicrosecconds;
	}
}

