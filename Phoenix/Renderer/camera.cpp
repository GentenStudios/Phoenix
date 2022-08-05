#include "camera.hpp"

#include <globals.hpp>

Camera::Camera( uint32_t width, uint32_t height )
{
	SetMatrixPosition( glm::vec3() );
	SetProjection( width, height );

	Update( ); 
}

void Camera::SetProjection( uint32_t width, uint32_t height)
{
	mProjection = glm::perspective(
		glm::radians( 45.0f ),
		(float) width / (float) height,
		10.0f,
		100.0f
	);
	mProjection[1][1] *= -1.0f;

	/*float aspect = static_cast<float>(width) / static_cast<float>(height);

	float wantedHeight = CAMERA_HEIGHT;
	float calculatedWidth = aspect * wantedHeight;

	mOrthoWidth = calculatedWidth;
	mOrthoHeight = wantedHeight;

	float zoom = 1.0f;
	//float zoom = 2.0f;
	float wH = ((float)calculatedWidth / 2.0f) * zoom;
	float hH = ((float)wantedHeight / 2.0f) * zoom;
	mProjection = glm::ortho( -wH, wH, hH, -hH, -1.0f, 100.0f );
	//mProjection = glm::ortho( 0.0f, aspect, 0.0f, 1.0f, -1.0f, 100.0f );
	mOutOfDateFrustrum = true;
	//mProjection = glm::ortho( 0.0f, (float)width, 0.0f, (float) height, -1.0f, 1.0f );
	*/
}

void Camera::Move( glm::vec3 position )
{
	Move( position.x, position.y, position.z );
}

void Camera::Move( float x, float y, float z )
{
	SetMatrixPosition(-glm::vec3(x, y, z));
}

void Camera::Update( )
{
	mCamera.ProPos = mProjection * mPosition;
	UpdateFrustrum( mCamera.ProPos );
	mOutOfDateFrustrum = false;
}

bool Camera::CheckSphereFrustrum( glm::vec3 pos, float radius )
{
	for ( auto i = 0; i < 6; i++ )
	{
		if ((mCamera.planes[i].x * pos.x) + (mCamera.planes[i].y * pos.y) +
			(mCamera.planes[i].z * pos.z) + mCamera.planes[i].w <= -radius )
		{
			return false;
		}
	}
	return true;
}

void Camera::SetMatrixPosition( glm::vec3 position )
{
	//mPosition = glm::lookAt( glm::vec3( 0, 0, 1 ), glm::vec3( 0, 0, 0 ), glm::vec3( 0, 1, 0 ) );
	mPosition = glm::translate( glm::mat4( 1.0f ), position );
}

void Camera::UpdateFrustrum( glm::mat4 matrix )
{
	mCamera.planes[LEFT].x = matrix[0].w + matrix[0].x;
	mCamera.planes[LEFT].y = matrix[1].w + matrix[1].x;
	mCamera.planes[LEFT].z = matrix[2].w + matrix[2].x;
	mCamera.planes[LEFT].w = matrix[3].w + matrix[3].x;

	mCamera.planes[RIGHT].x = matrix[0].w - matrix[0].x;
	mCamera.planes[RIGHT].y = matrix[1].w - matrix[1].x;
	mCamera.planes[RIGHT].z = matrix[2].w - matrix[2].x;
	mCamera.planes[RIGHT].w = matrix[3].w - matrix[3].x;

	mCamera.planes[TOP].x = matrix[0].w - matrix[0].y;
	mCamera.planes[TOP].y = matrix[1].w - matrix[1].y;
	mCamera.planes[TOP].z = matrix[2].w - matrix[2].y;
	mCamera.planes[TOP].w = matrix[3].w - matrix[3].y;

	mCamera.planes[BOTTOM].x = matrix[0].w + matrix[0].y;
	mCamera.planes[BOTTOM].y = matrix[1].w + matrix[1].y;
	mCamera.planes[BOTTOM].z = matrix[2].w + matrix[2].y;
	mCamera.planes[BOTTOM].w = matrix[3].w + matrix[3].y;

	mCamera.planes[BACK].x = matrix[0].w + matrix[0].z;
	mCamera.planes[BACK].y = matrix[1].w + matrix[1].z;
	mCamera.planes[BACK].z = matrix[2].w + matrix[2].z;
	mCamera.planes[BACK].w = matrix[3].w + matrix[3].z;

	mCamera.planes[FRONT].x = matrix[0].w - matrix[0].z;
	mCamera.planes[FRONT].y = matrix[1].w - matrix[1].z;
	mCamera.planes[FRONT].z = matrix[2].w - matrix[2].z;
	mCamera.planes[FRONT].w = matrix[3].w - matrix[3].z;
	
	for ( auto i = 0; i < 6; i++ )
	{
		float length = sqrtf( mCamera.planes[i].x * mCamera.planes[i].x + mCamera.planes[i].y * mCamera.planes[i].y + mCamera.planes[i].z * mCamera.planes[i].z );
		mCamera.planes[i] /= length;
	}
}
