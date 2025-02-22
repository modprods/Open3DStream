#include "async_request.h"

namespace O3DS
{
	bool AsyncRequest::start(const char *url)
	{
		int ret;

		ret = nng_req0_open(&mSocket);
		NNG_ERROR("Creating request socket")

		ret = nng_aio_alloc(&aio, AsyncRequest::Callback, this);
		NNG_ERROR("Creating request socket aio")

		ret = nng_dial(mSocket, url, 0, 0);
		NNG_ERROR("Conecting request socket")

		nng_recv_aio(mSocket, aio);

		mState = Connector::STARTED;

		return true;
	}

	bool AsyncReply::start(const char *url)
	{
		int ret;

		ret = nng_req0_open(&mSocket);
		NNG_ERROR("Creating reply socket")

		ret = nng_aio_alloc(&aio, AsyncReply::Callback, this);
		NNG_ERROR("Creating reply socket aio")

		ret = nng_listen(mSocket, url, NULL, 0);
		NNG_ERROR("Listening on reply socket");

		nng_recv_aio(mSocket, aio);

		mState = Connector::STARTED;

		return true;
	}

	void AsyncRequestReply::Callback_()
	{
		int ret;

		ret = nng_aio_result(aio);
		if (ret != 0)
		{
			setError("Aio result on async request reply", ret);
			return;
		}

		char *buf = NULL;
		size_t sz;
		ret = nng_recv(mSocket, &buf, &sz, NNG_FLAG_ALLOC);
		if (ret != 0)
		{
			setError("Async receive request", ret);
			return;
		}

		if (mInDataFunc) mInDataFunc(mContext, (void*)buf, sz);

		nng_free(buf, sz);

		nng_recv_aio(mSocket, aio);
	}


}

