
#include <network.h>
#include "protocol.h"

typedef struct vidlib_proxy_image
{
	int x, y, w, h;
	int string_mode;
	int blot_method;
	struct vidlib_proxy_image *parent;
	struct vidlib_proxy_image *child;
	struct vidlib_proxy_image *next;
	struct vidlib_proxy_image *prior;

	INDEX filegroup;
	TEXTSTR filename;
	Image image;
	INDEX render_id;
	INDEX id;

	struct vidlib_proxy_image_flags {
		BIT_FIELD bUsed : 1;
	} flags;

	P_8 buffer;
	size_t sendlen;
	size_t buf_avail;
	P_8 websock_buffer;
	size_t websock_sendlen;
	size_t websock_buf_avail;
} *PVPImage;

struct server_socket_state
{
	struct server_socket_flags {
		BIT_FIELD get_length : 1;
	} flags;
	POINTER buffer;
	int read_length;
};

typedef struct vidlib_proxy_renderer
{
	_32 w, h;
	S_32 x, y;
	_32 attributes;
	struct vidlib_proxy_renderer *above, *under;
	PLIST remote_render_id;  // this is synced with same index as l.clients
	struct vidlib_proxy_image *image;  // representation of the output surface
	struct vidlib_proxy_renderer_flags
	{
		BIT_FIELD hidden : 1;
		BIT_FIELD doing_redraw : 1;
		BIT_FIELD did_first_mouse : 1;
		BIT_FIELD pending_mouse : 1;
		BIT_FIELD consume_mouse : 1;  // set to consume mouse events... otherwise we have to dispatch them.
		BIT_FIELD closed : 1;
	} flags;
	INDEX id;
	MouseCallback mouse_callback;
	PTRSZVAL psv_mouse_callback;
	KeyProc key_callback;
	PTRSZVAL psv_key_callback;
	RedrawCallback redraw;
	PTRSZVAL psv_redraw;
	_32 _b;
	S_32 mouse_x, mouse_y;

} *PVPRENDER;

struct server_proxy_client
{
	PCLIENT pc;
	LOGICAL websock;
};
#define l vidlib_proxy_server_local
struct vidlib_proxy_local
{
	PCLIENT listener;
	PCLIENT web_listener;
	PLIST clients; // list of struct server_proxy_client
	TEXTSTR application_title;
	PLIST renderers;
	PLIST images;
	PLIST fonts;
	struct json_context *json_context;
	struct json_context *json_reply_context; // shorter list to search for input messages
	PLIST messages;   // json message formats
	PIMAGE_INTERFACE real_interface;
	_8 key_states[256];
	struct vidlib_proxy_renderer *focused;
	CRITICALSECTION message_formatter;
} l;

CTEXTSTR SACK_Vidlib_GetKeyText( int pressed, int key_index, int *used );
void SACK_Vidlib_ProcessKeyState( int pressed, int key_index, int *used );


