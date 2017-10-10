#ifndef LINKMESSAGES_H
#define LINKMESSAGES_H

#include <Quaternion.h>
#include <Vector3D.h>
#include "Colourscale.h"

namespace LinkMessage {

  enum Msgtype { LINK, UNLINK, VIEWPORT_SYNC, LINK_SYNC, COLOUR_SYNC, DIFFUSE_LINK };

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
    GLfloat    norm[6][3];
    GLfloat    inter[6];
    int        state[6];
    Quaternion ref;
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
