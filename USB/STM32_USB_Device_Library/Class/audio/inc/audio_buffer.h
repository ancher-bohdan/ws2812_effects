#ifndef __AUDIO_BUFFER_INIT___
#define __AUDIO_BUFFER_INIT___

#include <stdint.h>

#define NUMBER_OF_USB_FRAMES_IN_UM_NODE     4
#define UM_NODE_COUNT                       4

enum um_node_state
{
    UM_NODE_STATE_FREE = 0,
    UM_NODE_STATE_USB,
    UM_NODE_STATE_READY,
    UM_NODE_STATE_I2S
};

enum um_buffer_state
{
    UM_BUFFER_STATE_INIT = 0,
    UM_BUFFER_STATE_ACTIVE,
    UM_BUFFER_STATE_PLAY
};

struct um_node
{
    uint16_t *um_buf;
    struct um_node *next;
    uint16_t um_node_size;
    uint8_t um_node_offset;
    enum um_node_state um_node_state;
};

struct um_buffer_handle
{
    struct um_node *um_read;
    struct um_node *um_write;
    struct um_node *um_start;
    enum um_buffer_state um_buffer_state;
};

#endif