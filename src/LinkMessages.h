#ifndef LINKMESSAGES_H
#define LINKMESSAGES_H

#include "Quaternion.h"
#include "Vector3D.h"
#include "Colourscale.h"

namespace LinkMessage {

  enum Msgtype { LINK, UNLINK, VIEWPORT_SYNC, TIME_SYNC, COLOUR_SYNC, DIFFUSE_LINK, CLIP_SYNC };

  struct TrackBallState{
    float scale;          
    V3f trans;            //!< translation normalized by model size
    Quaternion qSpin;     //!< incremental spin from standardview
    Quaternion qRot;      //!< rotation relative to standard view
  };

  struct ColourScaleState {
    float     min, max;
    int       scale;
    int       levels;
  };

  struct ClipState {
    V3f        cnorm[6]; //!< rotations from ref direction to clipping plane normal 
    GLfloat    inter[6]; //!< clipping plane intercept
    int        state[6]; //!< display state
  };

  struct CmdMsg {
    pid_t   senderPid;
    pid_t   receiverPid;
    Msgtype command;
    int     sliderTime;
    int     newlink;

    TrackBallState   trackball;
    ColourScaleState colour;
    ClipState        clip;

  };

  struct MsgBuf {
    long   mtype;
    CmdMsg cmdmsg;
  } ;

}


#endif
