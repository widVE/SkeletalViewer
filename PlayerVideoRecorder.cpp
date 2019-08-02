#include "stdafx.h"

#include "PlayerVideoRecorder.h"
#include "CaveMaster.h"

#include <cassert>

PlayerVideo::PlayerVideo() : depthFrame(0), colorFrame(0)
{
}

void PlayerVideo::handleColorData(const NUI_IMAGE_FRAME& frame)
{
	// this color data is outdated. note: only depth images update the frame counter
	if (frame.dwFrameNumber < colorFrame)
		return;

	colorFrame = frame.dwFrameNumber;
	
	assert(frame.eResolution == NUI_IMAGE_RESOLUTION_640x480);

	INuiFrameTexture * pTexture = frame.pFrameTexture;
    NUI_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    if ( LockedRect.Pitch != 0 )
    {
		//assert(LockedRect.size == sizeof(color));
		
		if (LockedRect.size == sizeof(color))
			memcpy(color, LockedRect.pBits, LockedRect.size);
		else
		{
			for (int i = 0; i < 640*480*4; i += 4)
			{
				color[i+0] = 255;
				color[i+1] = 0;
				color[i+2] = 255;
				color[i+3] = 255;
			}
		}
	}

    pTexture->UnlockRect( 0 );
	
}

void PlayerVideo::handleDepthData(const NUI_IMAGE_FRAME& frame)
{
	if (frame.dwFrameNumber < depthFrame)
		return;
	
	depthFrame = frame.dwFrameNumber;
		
	
	// copy the mask and depth information
	INuiFrameTexture * pTexture = frame.pFrameTexture;

	assert(frame.eResolution == NUI_IMAGE_RESOLUTION_320x240);

    NUI_LOCKED_RECT LockedRect;
	pTexture->LockRect( 0, &LockedRect, NULL, 0 );

	memcpy(depth, LockedRect.pBits, sizeof(depth));

	/*
	const USHORT* current = (const USHORT*) LockedRect.pBits;
	const USHORT* dataEnd = current + (320*240);

	
	unsigned int pixel = 0;
	while (current < dataEnd)
	{
		depth[pixel] = NuiDepthPixelToDepth(*current);

		++current;
		++pixel;
	}
	*/
    pTexture->UnlockRect( 0 );
}


bool PlayerVideo::getData(KinectVideoData& data)
{
	if (colorFrame >= depthFrame)
	{

		memcpy(data.color, color, sizeof(color));
		memcpy(data.depth, depth, sizeof(depth));
	
		data.colorFrame = colorFrame;
		data.depthFrame = depthFrame;
		return true;
	}
	else
		return false;
}

	
