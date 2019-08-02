#ifndef PLAYER_VIDEO_INCLUDED
#define PLAYER_VIDEO_INCLUDED

#include "NuiApi.h"

class KinectVideoData;

class PlayerVideo
{
public:
	PlayerVideo();

	void handleDepthData(const NUI_IMAGE_FRAME& frame);
	void handleColorData(const NUI_IMAGE_FRAME& frame);
	
	bool getData(KinectVideoData& data);

private:
	unsigned long		depthFrame, colorFrame;
	
	// all image dimensions hardcoded for now, should be dynamic later
	unsigned char		color[640*480*4];
	unsigned short		depth[320*240];
	
};


#endif
