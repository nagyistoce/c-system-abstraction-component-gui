
#include <http.h>
#include <html5.websocket.client.h>

struct web_socket_client
{
	struct web_socket_client_flags
	{
		BIT_FIELD connected : 1; // if not connected, then parse data as http, otherwise process as websock protocol.
		BIT_FIELD closed : 1; // may get link connected, and be waiting for a connected status, but may not get connected
		BIT_FIELD want_close : 1; // schedule to close

		BIT_FIELD fragment_collecting : 1;
	} flags;
	PCLIENT pc;

	CTEXTSTR host;
	CTEXTSTR address_url;
	struct url_data *url;

	web_socket_opened on_open;
	web_socket_event on_event;
	web_socket_closed on_close;
	web_socket_error on_error;
	PTRSZVAL psv_on;

	POINTER buffer;
	HTTPState pHttpState;

	size_t fragment_collection_avail;
	size_t fragment_collection_length;
	P_8 fragment_collection;

};


struct web_socket_client_local
{
   _32 timer;
	PLIST clients;
   CRITICALSECTION cs_opening;
   struct web_socket_client *opening_client;
} wsc_local;
