#include "Game/Player.hpp"
#include "Game/GameQuaternionUnitTest.hpp"
#include "Game/App.hpp"

#include "Engine/Animation/FbxFileImporter.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"


constexpr int			MAX_VERTEXAS		 = 3;
constexpr unsigned char MIN_TEST_COLOR_VALUE = 5;
constexpr unsigned char MAX_TEST_COLOR_VALUE = 255;

extern App* g_theApp;


void GameQuaternionUnitTest::Startup()
{
	// set camera dimensions
	m_screenCamera.SetOrthographicView( SCREEN_BOTTOM_LEFT_ORTHO, SCREEN_TOP_RIGHT_ORTHO );

	// create clock
	m_GameClock = new Clock();

	CreateScene();
	AddBasisAtOrigin();

	PrintTitleText( "Quaternion Unit Test" );

	// RunUnitTests();
	QuaternionFromEulerAndBackTest();
}


//----------------------------------------------------------------------------------------------------------
void GameQuaternionUnitTest::Shutdown()
{
	delete m_player;
}


//----------------------------------------------------------------------------------------------------------
void GameQuaternionUnitTest::Update()
{
	UpdateGameState();
	UpdatePlayer();
	PrintDebugScreenMessage();
}


//----------------------------------------------------------------------------------------------------------
void GameQuaternionUnitTest::Render() const
{
	g_theRenderer->ClearScreen( m_backGroundColor );

	//-------------------------------------------------------------------------
	// world camera (for entities)
	g_theRenderer->BeginCamera( *m_player->m_worldCamera );

	RenderGridLines();

	DebugRenderWorld( *m_player->m_worldCamera );



	//-------------------------------------------------------------------------
	// screen camera (for HUD / UI)
	g_theRenderer->BeginCamera( m_screenCamera );
	// add text / UI code here

	DebugRenderScreen( m_screenCamera );

	g_theRenderer->EndCamera( m_screenCamera );
}


//----------------------------------------------------------------------------------------------------------
void GameQuaternionUnitTest::RunUnitTests()
{

	DebuggerPrintf( "\n-----------------------------------------------------------------------------\n" );
	DebuggerPrintf( "Running Unit Tests...\n" );

	// euler to quaternion unit tests
	std::vector<EulerAngles> eulerAngles;
	std::vector<Quaternion>	 results;
	std::vector<Quaternion>	 expectedResults;

	DebuggerPrintf( "Yaw--------------------------------------\n" );

	// yaw
	for ( float yaw = 0; yaw < 360; yaw++ )
	{
		EulerAngles euler	 = EulerAngles( yaw, 0.f, 0.f );
		Quaternion	result	 = Quaternion::MakeFromEulerAngles( euler );
		Quaternion	expected = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 0.f, 1.f ), yaw );

		eulerAngles.push_back( euler );
		results.push_back( result );
		expectedResults.push_back( expected );

		yaw += 1.f;
	}

	// test
	for ( int index = 0; index < results.size(); index++ )
	{
		EulerAngles const& e			  = eulerAngles[ index ];
		Quaternion const&  result		  = results[ index ];
		Quaternion const&  expectedResult = expectedResults[ index ];

		if ( result != expectedResult )
		{
			DebuggerPrintf( "failed y=%.2f, p=%.2f, r =%.2f; result: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}, expected: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}\n",
				e.m_yawDegrees, e.m_pitchDegrees, e.m_rollDegrees, result.x, result.y, result.z, result.w, expectedResult.x, expectedResult.y, expectedResult.z, expectedResult.w );
		}
		else
		{
			DebuggerPrintf( "passed y=%.2f, p=%.2f, r =%.2f; result: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}, expected: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}\n",
				e.m_yawDegrees, e.m_pitchDegrees, e.m_rollDegrees, result.x, result.y, result.z, result.w, expectedResult.x, expectedResult.y, expectedResult.z, expectedResult.w );
		}
	}

	eulerAngles.clear();
	results.clear();
	expectedResults.clear();

	// pitch
	DebuggerPrintf( "Pitch--------------------------------------\n" );

	for ( float pitch = 0; pitch < 360; pitch++ )
	{
		EulerAngles euler	 = EulerAngles( 0.f, pitch, 0.f );
		Quaternion	result	 = Quaternion::MakeFromEulerAngles( euler );
		Quaternion	expected = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 1.f, 0.f ), pitch );

		eulerAngles.push_back( euler );
		results.push_back( result );
		expectedResults.push_back( expected );

		pitch += 1.f;
	}

	// test
	for ( int index = 0; index < results.size(); index++ )
	{
		EulerAngles const& e			  = eulerAngles[ index ];
		Quaternion const&  result		  = results[ index ];
		Quaternion const&  expectedResult = expectedResults[ index ];

		if ( result != expectedResult )
		{
			DebuggerPrintf( "failed y=%.2f, p=%.2f, r =%.2f; result: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}, expected: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}\n",
				e.m_yawDegrees, e.m_pitchDegrees, e.m_rollDegrees, result.x, result.y, result.z, result.w, expectedResult.x, expectedResult.y, expectedResult.z, expectedResult.w );
		}
		else
		{
			DebuggerPrintf( "passed y=%.2f, p=%.2f, r =%.2f; result: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}, expected: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}\n",
				e.m_yawDegrees, e.m_pitchDegrees, e.m_rollDegrees, result.x, result.y, result.z, result.w, expectedResult.x, expectedResult.y, expectedResult.z, expectedResult.w );
		}
	}


	eulerAngles.clear();
	results.clear();
	expectedResults.clear();

	// roll
	DebuggerPrintf( "Roll--------------------------------------\n" );

	for ( float roll = 0; roll < 360; roll++ )
	{
		EulerAngles euler	 = EulerAngles( 0.f, 0.f, roll );
		Quaternion	result	 = Quaternion::MakeFromEulerAngles( euler );
		Quaternion	expected = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 1.f, 0.f, 0.f ), roll );

		eulerAngles.push_back( euler );
		results.push_back( result );
		expectedResults.push_back( expected );

		roll += 1.f;
	}

	// test
	for ( int index = 0; index < results.size(); index++ )
	{
		EulerAngles const& e			  = eulerAngles[ index ];
		Quaternion const&  result		  = results[ index ];
		Quaternion const&  expectedResult = expectedResults[ index ];

		if ( result != expectedResult )
		{
			DebuggerPrintf( "failed y=%.2f, p=%.2f, r =%.2f; result: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}, expected: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}\n",
				e.m_yawDegrees, e.m_pitchDegrees, e.m_rollDegrees, result.x, result.y, result.z, result.w, expectedResult.x, expectedResult.y, expectedResult.z, expectedResult.w );
		}
		else
		{
			DebuggerPrintf( "passed y=%.2f, p=%.2f, r =%.2f; result: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}, expected: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}\n",
				e.m_yawDegrees, e.m_pitchDegrees, e.m_rollDegrees, result.x, result.y, result.z, result.w, expectedResult.x, expectedResult.y, expectedResult.z, expectedResult.w );
		}
	}


	eulerAngles.clear();
	results.clear();
	expectedResults.clear();


	// yaw and pitch
	DebuggerPrintf( "Yaw and Pitch--------------------------------------\n" );

	for ( float yaw = 0; yaw < 360; )
	{
		for ( float pitch = 0; pitch < 360; )
		{
			EulerAngles euler	 = EulerAngles( yaw, pitch, 0.f );
			Quaternion	result	 = Quaternion::MakeFromEulerAngles( euler );
			Quaternion	expected = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 0.f, 1.f ), yaw ) * Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 1.f, 0.f ), pitch );

			eulerAngles.push_back( euler );
			results.push_back( result );
			expectedResults.push_back( expected );

			// randomly increase pitch
			float randomPitch = RandomNumberGenerator().RollRandomFloatInRange( 10.f, 20.f );
			pitch += randomPitch;
		}

		// randomly increase yaw
		float randomYaw = RandomNumberGenerator().RollRandomFloatInRange( 10.f, 20.f );
		yaw += randomYaw;
	}


	// test
	for ( int index = 0; index < results.size(); index++ )
	{
		EulerAngles const& e			  = eulerAngles[ index ];
		Quaternion const&  result		  = results[ index ];
		Quaternion const&  expectedResult = expectedResults[ index ];

		if ( result != expectedResult )
		{
			DebuggerPrintf( "failed y=%.2f, p=%.2f, r =%.2f; result: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}, expected: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}\n",
				e.m_yawDegrees, e.m_pitchDegrees, e.m_rollDegrees,
				result.x, result.y, result.z, result.w,
				expectedResult.x, expectedResult.y, expectedResult.z, expectedResult.w );
		}
		else
		{
			DebuggerPrintf( "passed y=%.2f, p=%.2f, r =%.2f; result: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}, expected: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}\n",
				e.m_yawDegrees, e.m_pitchDegrees, e.m_rollDegrees, result.x, result.y, result.z, result.w, expectedResult.x, expectedResult.y, expectedResult.z, expectedResult.w );
		}
	}




	DebuggerPrintf( "\n-----------------------------------------------------------------------------\n" );
}


//----------------------------------------------------------------------------------------------------------
void GameQuaternionUnitTest::QuaternionFromEulerAndBackTest()
{
	// get quaternion from euler
	//
	// euler to quaternion unit tests
	std::vector<EulerAngles> eulerAngles;
	std::vector<Quaternion>	 resultQ;
	std::vector<Quaternion>	 expectedQ;

	// convert back to euler
	std::vector<EulerAngles> resultEuler;

	// yaw
	for ( float yaw = 0; yaw < 360; )
	{
		EulerAngles euler	 = EulerAngles( yaw, 0.f, 0.f );
		Quaternion	result	 = Quaternion::MakeFromEulerAngles( euler );
		Quaternion	expected = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 0.f, 1.f ), yaw );

		EulerAngles convertedEuler = result.GetAsEulerAngles();

		eulerAngles.push_back( euler );
		resultQ.push_back( result );
		expectedQ.push_back( expected );
		resultEuler.push_back( convertedEuler );

		yaw += 1.f;
	}



	// pitch
	for ( float pitch = 0; pitch < 360; )
	{
		EulerAngles euler	 = EulerAngles( 0.f, pitch, 0.f );
		Quaternion	result	 = Quaternion::MakeFromEulerAngles( euler );
		Quaternion	expected = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 1.f, 0.f ), pitch );

		EulerAngles convertedEuler = result.GetAsEulerAngles();

		eulerAngles.push_back( euler );
		resultQ.push_back( result );
		expectedQ.push_back( expected );
		resultEuler.push_back( convertedEuler );

		pitch += 1.f;
	}


	// roll
	for ( float roll = 0; roll < 360; )
	{
		EulerAngles euler	 = EulerAngles( 0.f, 0.f, roll );
		Quaternion	result	 = Quaternion::MakeFromEulerAngles( euler );
		Quaternion	expected = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 1.f, 0.f, 0.f ), roll );

		EulerAngles convertedEuler = result.GetAsEulerAngles();

		eulerAngles.push_back( euler );
		resultQ.push_back( result );
		expectedQ.push_back( expected );
		resultEuler.push_back( convertedEuler );

		roll += 1.f;
	}

	// yaw and pitch
	//for ( float yaw = 0; yaw < 360; )
	//{
	//	for ( float pitch = 0; pitch < 360; )
	//	{
	//		EulerAngles euler	 = EulerAngles( yaw, pitch, 0.f );
	//		Quaternion	result	 = Quaternion::MakeFromEulerAngles( euler );
	//		Quaternion	expected = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 0.f, 1.f ), yaw ) * Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 1.f, 0.f ), pitch );

	//		EulerAngles convertedEuler = result.GetAsEulerAngles(); 

	//		eulerAngles.push_back( euler );
	//		resultQ.push_back( result );
	//		expectedQ.push_back( expected );
	//		resultEuler.push_back( convertedEuler );

	//		//pitch += 1.f;
	//		// randomly increase pitch
	//		float randomPitch = RandomNumberGenerator().RollRandomFloatInRange( 10.f, 20.f );
	//		pitch += randomPitch;
	//	}

	//	//yaw += 1.f;
	//	// randomly increase yaw
	//	float randomYaw = RandomNumberGenerator().RollRandomFloatInRange( 10.f, 20.f );
	//	yaw += randomYaw;
	//}

	// yaw and roll
	//for (float yaw = 0; yaw < 360; )
	//{
	//	for (float roll = 0; roll < 360; )
	//	{
	//		EulerAngles euler	 = EulerAngles( yaw, 0.f, roll );
	//		Quaternion	result	 = Quaternion::MakeFromEulerAngles( euler );
	//		Quaternion	expected = Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 0.f, 0.f, 1.f ), yaw ) * Quaternion::MakeFromAxisOfRotationAndAngleDegrees( Vec3( 1.f, 0.f, 0.f ), roll );

	//		EulerAngles convertedEuler = result.GetAsEulerAngles();

	//		eulerAngles.push_back( euler );
	//		resultQ.push_back( result );
	//		expectedQ.push_back( expected );
	//		resultEuler.push_back( convertedEuler );

	//		// randomly increase roll
	//		float randomRoll = RandomNumberGenerator().RollRandomFloatInRange( 10.f, 20.f );
	//		roll += randomRoll;
	//	}

	//	// randomly increase yaw
	//	float randomYaw = RandomNumberGenerator().RollRandomFloatInRange( 10.f, 20.f );
	//	yaw += randomYaw;
	//}



	// display
	// test
	/*for ( int index = 0; index < resultQ.size(); index++ )
	{
		EulerAngles const& e			  = eulerAngles[ index ];
		Quaternion const&  result		  = resultQ[ index ];
		Quaternion const&  expectedResult = expectedQ[ index ];

		if ( result != expectedResult )
		{
			DebuggerPrintf( "failed y=%.2f, p=%.2f, r =%.2f; result: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}, expected: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}\n",
				e.m_yawDegrees, e.m_pitchDegrees, e.m_rollDegrees,
				result.x, result.y, result.z, result.w,
				expectedResult.x, expectedResult.y, expectedResult.z, expectedResult.w );
		}
		else
		{
			DebuggerPrintf( "passed y=%.2f, p=%.2f, r =%.2f; result: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}, expected: {x=%.5f, y=%.5f, z=%.5f, w=%.5f}\n",
				e.m_yawDegrees, e.m_pitchDegrees, e.m_rollDegrees, result.x, result.y, result.z, result.w, expectedResult.x, expectedResult.y, expectedResult.z, expectedResult.w );
		}
	}*/


	DebuggerPrintf( "\n-----------------------------------------------------------------------------\n" );
	DebuggerPrintf( "\n-----------------------------------------------------------------------------\n" );
	DebuggerPrintf( "\n-----------------------------------------------------------------------------\n" );
	DebuggerPrintf( "\n-----------------------------------------------------------------------------\n" );


	// compare euler
	for ( int index = 0; index < resultEuler.size(); index++ )
	{
		EulerAngles const& result		  = resultEuler[ index ];
		EulerAngles const& expectedResult = eulerAngles[ index ];

		if ( result.ComponentWiseEquals( expectedResult ) )
		{
			DebuggerPrintf( "passed result y=%.5f, p=%.5f, r =%.5f;    expected y=%.5f, p=%.5f, r =%.5f;\n",
				result.m_yawDegrees, result.m_pitchDegrees, result.m_rollDegrees,
				expectedResult.m_yawDegrees, expectedResult.m_pitchDegrees, expectedResult.m_rollDegrees );
		}
		else
		{
			DebuggerPrintf( "failed result y=%.5f, p=%.5f, r =%.5f;    expected y=%.5f, p=%.5f, r =%.5f;\n",
				result.m_yawDegrees, result.m_pitchDegrees, result.m_rollDegrees,
				expectedResult.m_yawDegrees, expectedResult.m_pitchDegrees, expectedResult.m_rollDegrees );
		}
	}
}
