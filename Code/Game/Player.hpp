#pragma once

#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"

class Camera;
class Clock;

class Player
{
public:
	Player();
	virtual ~Player();

	void Startup();

	void Update();
	void Render() const;

	Camera* m_worldCamera = nullptr;

	Vec3 m_position;
	Vec3 m_velocity;

	EulerAngles m_orientation;
	EulerAngles m_angularVelocity;

	Mat44 GetModelMatrix() const;

	Rgba8 m_color = Rgba8::WHITE;

	Clock* m_playerClock = nullptr;

protected:
	void UpdatePlayerMovement(float deltaseconds);
	void ClampOrientation();

	void UpdateVerticalMovement(float deltaseconds);
	void UpdateHorizontalMovement(float deltaseconds);
	void UpdateOrientation();
};