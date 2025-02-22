
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>

#include "async_subscriber.h"
namespace O3DS
{

bool AsyncSubscriber::start(const char *url)
{
	int ret;

	ret = nng_sub0_open(&mSocket);
	NNG_ERROR("Async Subscriber open socket");

	ret = nng_setopt(mSocket, NNG_OPT_SUB_SUBSCRIBE, "", 0);
	NNG_ERROR("Setting subscribe flag")

	ret = nng_dialer_create(&mDialer, mSocket, url);
	NNG_ERROR("Creating dialer")

	//ret = nng_pipe_notify(mSocket, nng_pipe_ev::NNG_PIPE_EV_ADD_POST,  AsyncSubscriber::pipeEvent, this);
	//NNG_ERROR("Setting pipe notify")

	// Async dial - pipe will be created on connection
	ret = nng_dialer_start(mDialer, NNG_FLAG_NONBLOCK);
	NNG_ERROR("Subscriber async dialier")

	ret = nng_aio_alloc(&aio, AsyncSubscriber::callback, this);
	NNG_ERROR("Subscriber async aio alloc")

	nng_recv_aio(mSocket, aio);

	mState = Connector::STARTED;

	return true;
}

void AsyncSubscriber::pipeEvent_(nng_pipe pipe, nng_pipe_ev pipe_ev)
{
//	int ret;
	if (pipe_ev == nng_pipe_ev::NNG_PIPE_EV_ADD_POST)
	{
		nng_socket s= nng_pipe_socket(pipe);

		//int ret = nng_ctx_open(&ctx, mSocket);
		//if (ret != 0) return;

		//nng_ctx_recv(ctx, aio);

		//in_pipe();
	}

	if (pipe_ev == nng_pipe_ev::NNG_PIPE_EV_REM_POST)
		return; // TODO
}


}
