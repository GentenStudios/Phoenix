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
