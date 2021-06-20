#include "audio_buffer.h"
#include "usbd_audio_core.h"

static uint16_t __um_buffer[UM_NODE_COUNT][(AUDIO_OUT_PACKET / 2) * NUMBER_OF_USB_FRAMES_IN_UM_NODE];

