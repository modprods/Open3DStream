#include "device.h"
#include "o3ds/math.h"
#include "o3ds/publisher.h"
#include "o3ds/async_pair.h"

#define OPEN3D_DEVICE__CLASS	OPEN3D_DEVICE__CLASSNAME
#define OPEN3D_DEVICE__NAME	OPEN3D_DEVICE__CLASSSTR
#define OPEN3D_DEVICE__LABEL	"Open3D_Device"
#define OPEN3D_DEVICE__DESC		"Realtime Streaming"
#define OPEN3D_DEVICE__PREFIX	"DeviceTemplate"

#define FBX_VERSION_TAG		"Version"
#define FBX_VERSION_VAL		100
#define FBX_COMMPARAM_TAG	"Communication"
#define FBX_SAMPLERATE      "SampleRate"
#define FBX_SUBJECT_COUNT   "SubjectCount"
#define FBX_SUBJECT_NAME    "SubjectName"
#define FBX_SUBJECT_MODEL   "SubjectModel"
#define FBX_SUBJECT_REF     "SubjectReference"
#define FBX_NETWORK_IP      "NetworkIp"
#define FBX_NETWORK_PORT    "NetworkPort"
#define FBX_NETWORK_PROTO   "NetworkProtocol"
#define FBX_KEY             "Key"

#include <ws2tcpip.h>
#include <sstream>

#ifdef _WIN32
#pragma warning( disable : 4996 ) 
int inet_pton_alt(int af, const char *src, void *dst)
{
  struct sockaddr_storage ss;
  int size = sizeof(ss);
  char src_copy[INET6_ADDRSTRLEN+1];

  ZeroMemory(&ss, sizeof(ss));
  strncpy (src_copy, src, INET6_ADDRSTRLEN+1);
  src_copy[INET6_ADDRSTRLEN] = 0;

  if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0) {
    switch(af) {
      case AF_INET: 
	      *(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
          return 1;
      case AF_INET6:
          *(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
          return 1;
    }
  }
  return 0;
}
#endif

FBDeviceImplementation	(	OPEN3D_DEVICE__CLASS	);
FBRegisterDevice		(	OPEN3D_DEVICE__NAME,
							OPEN3D_DEVICE__CLASS,
							OPEN3D_DEVICE__LABEL,
							OPEN3D_DEVICE__DESC,
							FB_DEFAULT_SDK_ICON		);	// Icon filename (default=Open Reality icon)


void dataFn(void *ctx, void *data, size_t len)
{
	if (ctx) static_cast<Open3D_Device*>(ctx)->InData(data, len);
}

bool Open3D_Device::FBCreate()
{
	//mNetworkAddress = "tcp://3.131.65.210:6001";
	mNetworkAddress = "tcp://127.0.0.1:6001";
	mNetworkPort = 3001;
	mStreaming = true;
	mProtocol = kNNGServer;
	mKey = nullptr;
	mServer = nullptr;

	FBTime	lPeriod;
	lPeriod.SetSecondDouble(1.0/60.0);
	SamplingPeriod	= lPeriod;
	mSamplingRate = lPeriod.GetFrame();
	return true;
}

void Open3D_Device::FBDestroy()
{
}

bool Open3D_Device::DeviceOperation( kDeviceOperations pOperation )
{
	switch (pOperation)
	{
		case kOpInit:	return Init();
		case kOpStart:	return Start();
		case kOpStop:	return Stop();
		case kOpReset:	return Reset();
		case kOpDone:	return Done();
	}
	return FBDevice::DeviceOperation( pOperation );
}

#pragma warning( disable : 4189 )

void Open3D_Device::AddItem(FBModel *model)
{
	// Add a model (called by the layout) 

	FBString name;

	FBNamespace *ns = model->GetOwnerNamespace();
	if(ns)
	{ 
		name = ns->Name.AsString();
	}
	else
	{
		name = model->Name.AsString();
	}

	if (model->Is(FBModelNull::TypeInfo) ||
		model->Is(FBModelRoot::TypeInfo) ||
		model->Is(FBCamera::TypeInfo) ||
		model->Is(FBModelSkeleton::TypeInfo))
	{
		Items.addSubject(name.operator char *(), (void*)model);
	}

}

bool Open3D_Device::Init()
{
    FBProgress	lProgress;

    lProgress.Caption	= "Open3D Stream";
	lProgress.Text		= "Initializing device...";

	return true;
}

bool Open3D_Device::Start()
{
	// Start the device

	FBProgress	lProgress;
	lProgress.Caption	= "Starting up device";
	lProgress.Text	    = "Opening device communications";
	Status			    = "Opening device communications";

	if (mServer)
	{
		delete mServer;
		mServer = nullptr;
	}

	for (O3DS::Subject* subject : Items)
	{
		FBModel *model = static_cast<FBModel*>(subject->mReference);
		O3DS::Mobu::TraverseSubject(subject, model);
	}

	if (mProtocol == Open3D_Device::kTCPClient || mProtocol == Open3D_Device::kTCPServer)
	{
		if (!mTcpIp.CreateSocket(mNetworkSocket, kFBTCPIP_Stream))
		{
			Status = "TCP Socket Error";
			mNetworkSocket = -1;
			return false;
		}

		if (mProtocol == Open3D_Device::kTCPClient)
		{
			if (!mTcpIp.Connect(mNetworkSocket, mNetworkAddress, mNetworkPort))
			{
				mTcpIp.CloseSocket(mNetworkSocket);
				mNetworkSocket = -1;
				Status = "Connect Error";
				return false;
			}

			Status = "TCP Client Started";
			return true;
		}

		if (mProtocol == Open3D_Device::kTCPServer)
		{
			if (!mTcpIp.Bind(mNetworkSocket, INADDR_ANY, mNetworkPort))
			{
				mTcpIp.CloseSocket(mNetworkSocket);
				mNetworkSocket = -1;
				Status = "TCP Bind Error";
				return false;
			}

			if (mTcpIp.Listen(mNetworkSocket) != 0)
			{
				mTcpIp.CloseSocket(mNetworkSocket);
				mNetworkSocket = -1;
				Status = "TCP Listen Error";
				return false;
			}
			Status = "TCP Server Started";
			return true;

		}
	}


	if (mProtocol == Open3D_Device::kUDP)
	{
		if (mTcpIp.CreateSocket(mNetworkSocket, kFBTCPIP_DGRAM))
		{
			Status = "UDP OK";
			return true;
		}

		mNetworkSocket = -1;
		Status = "UDP Error";
		return false;
	}

	if (mProtocol == Open3D_Device::kNNGServer ||
		mProtocol == Open3D_Device::kNNGClient || 
		mProtocol == Open3D_Device::kNNGPublish)
	{
		if (mProtocol == Open3D_Device::kNNGServer)  mServer = new O3DS::AsyncPairServer();
		if (mProtocol == Open3D_Device::kNNGClient)	 mServer = new O3DS::AsyncPairClient();
		if (mProtocol == Open3D_Device::kNNGPublish) mServer = new O3DS::Publisher();

		if (mServer->start(mNetworkAddress))
		{
			Status = "Running";
			return true;
		}
		else
		{
			Status = mServer->getError().c_str();
			return false;
		}
	}

	Status = "INVALID";
	return true;
}

void Open3D_Device::InData(void *data, size_t len)
{

}

bool Open3D_Device::Stop()
{
	// Stop the device

	FBProgress	lProgress;
	lProgress.Caption	= "Shutting down device";

	// Step 1: Stop streaming data
	lProgress.Text	= "Stopping device communications";
	Status			= "Stopping device communications";

	if (mNetworkSocket > 0)
	{
		if (mTcpIp.CloseSocket(mNetworkSocket))
		{
			Status = "";
		}
		else
		{
			Status = "Error Closing";
		}
	}

	if (mServer) 
	{
		delete mServer;
		mServer = nullptr;
     }

	lProgress.Caption = "";
	lProgress.Text = "";

    return true;
}

#define BUFSZ 1024*60

int32_t Open3D_Device::WriteTcp(int socket, void *data, int32_t bucketSize)
{
	// Write a header first 
	int32_t header = 0x0203;
	int written = 0;
	int total = 0;
	if (!mTcpIp.Write(socket, &header, sizeof(int32_t), &written))
	{
		Status = "Error";
		//mTcpIp.CloseSocket(mNetworkSocket);
		//mNetworkSocket = -1;
		return 0;
	}
	total += written;

	// Write the size of this bucket
	if (!mTcpIp.Write(socket, &bucketSize, sizeof(int32_t), &written))
	{
		Status = "Error";
	}
	total += written;

	// Write the bucket
	if (!mTcpIp.Write(socket, data, bucketSize, &written))
	{
		Status = "Error";
	}
	total += written;
	return total;
}

void Open3D_Device::DeviceIONotify(kDeviceIOs  pAction, FBDeviceNotifyInfo &pDeviceNotifyInfo)
{
	std::vector<char> buf;
	int written;

	uint32_t total = 0;

	// Called per frame to send data
	switch (pAction)
	{
		// Output devices
	case kIOPlayModeWrite:
	case kIOStopModeWrite:
	{
		if (mNetworkSocket != -1)
		{
			Items.update();

			FBTime MobuTime = FBSystem().LocalTime;
			int32_t bucketSize = Items.Serialize(buf, MobuTime.GetSecondDouble());
			if (bucketSize == 0)
			{
				Status = "Buffer Error";
				return;
			}

			if (mProtocol == Open3D_Device::kTCPServer)
			{
				kULong addr = 0;
				int newSocket = mTcpIp.Accept(mNetworkSocket, &addr);
				if (newSocket > 0)
					mClients.push_back(newSocket);

				for (std::vector<int>::iterator client = mClients.begin(); client != mClients.end();)
				{
					if (WriteTcp(*client, &buf[0], bucketSize)) {
						client++;
					}
					else {
						client = mClients.erase(client);
					}
				}
			}

			if (mProtocol == Open3D_Device::kTCPClient) {
				WriteTcp(mNetworkSocket, &buf[0], bucketSize);
			}

			if (mProtocol == Open3D_Device::kUDP)
			{
				struct in_addr ret;
				if (!inet_pton_alt(AF_INET, mNetworkAddress.operator char *(), &ret))		
				{
					Status = "UDP ERROR";
				}
				if (mTcpIp.WriteDatagram(mNetworkSocket, &buf[0], bucketSize, &written, ret.S_un.S_addr, mNetworkPort))
				{
					std::ostringstream oss;
					oss << written << " bytes";
					Status = oss.str().c_str();
				}
				else
				{
					Status = "UDP SEND ERROR";
				}

			}

			if (mProtocol == Open3D_Device::kNNGClient ||
				mProtocol == Open3D_Device::kNNGServer ||
				mProtocol == Open3D_Device::kNNGPublish)
			{
				if(mServer)
					mServer->write((const char*)&buf[0], bucketSize);
				std::ostringstream oss;
				oss << bucketSize << " bytes";
				Status = oss.str().c_str();
			}
		}
	}
	break;

	// Input devices
	case kIOStopModeRead:
	case kIOPlayModeRead:
	{
		AckOneSampleReceived();
		break;
	}
	}
}

bool Open3D_Device::Done()
{
	return false;
}

bool Open3D_Device::Reset()
{
    Stop();
    return Start();
}

double Open3D_Device::GetSamplingRate()
{
	FBTime t = SamplingPeriod;
	double dt = t.GetSecondDouble();
	if (dt == 0.0) return 0.0;
	return 1.0 / dt;
}

void Open3D_Device::SetSamplingRate(double rate)
{
	if (rate != 0.0)
	{
		FBTime t;
		t.SetSecondDouble(1.0 / rate);
		SamplingPeriod = t;
	}
}


bool Open3D_Device::AnimationNodeNotify(FBAnimationNode* pAnimationNode ,FBEvaluateInfo* pEvaluateInfo)
{
    return true;
}


bool Open3D_Device::DeviceEvaluationNotify( kTransportMode pMode, FBEvaluateInfo* pEvaluateInfo )
{
	return true;
}



void Open3D_Device::DeviceRecordFrame( FBDeviceNotifyInfo &pDeviceNotifyInfo )
{
}



bool Open3D_Device::FbxStore(FBFbxObject* pFbxObject,kFbxObjectStore pStoreWhat)
{
	char buf[40];

	if (pStoreWhat & kAttributes)
	{
		pFbxObject->FieldWriteI		(FBX_VERSION_TAG,FBX_VERSION_VAL);

		pFbxObject->FieldWriteBegin(FBX_COMMPARAM_TAG);
		{
			pFbxObject->FieldWriteEnd();
		}
		
		pFbxObject->FieldWriteD(FBX_SAMPLERATE, GetSamplingRate());
		pFbxObject->FieldWriteI(FBX_SUBJECT_COUNT, (int)Items.size());

		for (int i = 0; i < Items.size(); i++)
		{
			O3DS::Subject* subject = Items[i];
			FBModel *model = static_cast<FBModel*>(subject->mReference);

			sprintf_s(buf, 40, "%s%d", FBX_SUBJECT_NAME, i);
			pFbxObject->FieldWriteC(buf, subject->mName.c_str());

			sprintf_s(buf, 40, "%s%d", FBX_SUBJECT_MODEL, i);
			pFbxObject->FieldWriteC(buf, model->GetFullName());

			sprintf_s(buf, 40, "%s%d", FBX_SUBJECT_REF, i);
			pFbxObject->FieldWriteObjectReference(buf, model);

		}

		pFbxObject->FieldWriteC(FBX_NETWORK_IP, GetNetworkAddress());
		pFbxObject->FieldWriteI(FBX_NETWORK_PORT, GetNetworkPort());
		pFbxObject->FieldWriteI(FBX_NETWORK_PROTO, (int)GetProtocol());
		pFbxObject->FieldWriteC(FBX_KEY, GetKey());

	}
	return true;
}


bool Open3D_Device::FbxRetrieve(FBFbxObject* pFbxObject,kFbxObjectStore pStoreWhat)
{
	int			Version;
	FBString	name;
	char buf[40];

	if (pStoreWhat & kAttributes)
	{
		// Get version
		Version	= pFbxObject->FieldReadI(FBX_VERSION_TAG);

		// Get communications settings
		if (pFbxObject->FieldReadBegin(FBX_COMMPARAM_TAG))
		{
			pFbxObject->FieldReadEnd();
		}
#if 1
		SetSamplingRate(pFbxObject->FieldReadD(FBX_SAMPLERATE));
		int n = pFbxObject->FieldReadI(FBX_SUBJECT_COUNT);
#else
		SetSamplingRate(pFbxObject->FieldReadD(FBX_SAMPLERATE, GetSamplingRate()));
		int n = pFbxObject->FieldReadI(FBX_SUBJECT_COUNT);
#endif

		for (int i = 0; i < n; i++)
		{
			// Don't use const char* return values as value is not persistent.
			sprintf_s(buf, 40, "%s%d", FBX_SUBJECT_NAME, i);
			FBString subjectName = pFbxObject->FieldReadC(buf);

			sprintf_s(buf, 40, "%s%d", FBX_SUBJECT_MODEL, i);
			FBString model = pFbxObject->FieldReadC(buf);

			sprintf_s(buf, 40, "%s%d", FBX_SUBJECT_REF, i);
			HIObject ref = pFbxObject->FieldReadObjectReference(buf);

			FBComponent *component = FBGetFBComponent(ref);

			if (component)
			{
				FBModel *model = dynamic_cast<FBModel*>(component);
				auto s = Items.addSubject(subjectName.operator char *(), (void*)model);
				O3DS::Mobu::TraverseSubject(s, model);
			}
		}

#if 1
		SetNetworkAddress(pFbxObject->FieldReadC(FBX_NETWORK_IP));
		SetNetworkPort(pFbxObject->FieldReadI(FBX_NETWORK_PORT));
		SetProtocol(static_cast<TProtocol>(pFbxObject->FieldReadI(FBX_NETWORK_PROTO)));
		SetKey(pFbxObject->FieldReadC(FBX_KEY));
#else
		SetNetworkAddress(pFbxObject->FieldReadC(FBX_NETWORK_IP, GetNetworkAddress()));
		SetNetworkPort(pFbxObject->FieldReadI(FBX_NETWORK_PORT, GetNetworkPort()));
		SetProtocol(static_cast<TProtocol>(pFbxObject->FieldReadI(FBX_NETWORK_PROTO)));
		SetKey(pFbxObject->FieldReadC(FBX_KEY, GetKey()));
#endif		SetKey(pFbxObject->FieldReadC(FBX_KEY, GetKey()));
	}

	return true;
}
