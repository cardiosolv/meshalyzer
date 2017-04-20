#ifndef LINKMESSAGES_H
#define LINKMESSAGES_H

#include <Quaternion.h>
#include <Vector3D.h>
#include "Colourscale.h"

namespace LinkMessage {
  
  struct CommandMsg {
    long mtype;
    int senderPid;
    int receiverPid;
    char command[20];
	int sliderTime;
	
    struct TrackBallState{
      float scale;          
      V3f trans;            //!< translation normalized by model size
      Quaternion qSpin;     //!< incremental spin from standardview
      Quaternion qRot;      //!< rotation relative to standard view
    } trackballState;
    
    struct ColourScaleState {
      float     min, max;
      int       scale;
      int       levels;
    } colourState;
  };

  const char* const LINK = "link";
  const char* const UNLINK = "unlink";
  const char* const VIEWPORT_SYNC = "viewport sync";
  const char* const LINK_SYNC = "link sync";
  const char* const COLOUR_SYNC = "color scale sync";

}



#endif
