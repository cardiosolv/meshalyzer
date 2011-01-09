#ifndef LINKMESSAGES_H
#define LINKMESSAGES_H

#include <Quaternion.h>
#include <Vector3D.h>

namespace LinkMessage {
  
  struct CommandMsg {
    long mtype;
    int senderPid;
    int receiverPid;
    char command[20];
	int sliderTime;
	
    struct TrackBallState{
      float scale; 
      V3f trans;
      V3f origin;
      Quaternion qSpin;
      Quaternion qRot;
    } trackballState;
    
  };

  const char* const LINK_COMMAND_LINK = "link";
  const char* const LINK_COMMAND_UNLINK = "unlink";
  const char* const LINK_COMMAND_VIEWPORT_SYNC = "viewport sync";
  const char* const LINK_COMMAND_LINK_SYNC = "link sync";

}



#endif