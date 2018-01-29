// -*-c++-*-
//******************************************************************
//
// Copyright 2014 Intel Corporation.
// Copyright 2015 Eurogiciel <philippe.coval@eurogiciel.fr>
// Copyright 2016 Samsung <philippe.coval@osg.samsung.com>
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "common.h"

#include <csignal>
#include <functional>

#include "server.h"
#include "platform.h"

using namespace std;
using namespace OC;


bool IoTServer::m_over = false;

double IoTServer::m_latmax = 49;
double IoTServer::m_latmin = 48;

double IoTServer::m_lonmax = -1;
double IoTServer::m_lonmin = 1;

double IoTServer::m_lat_offset = 0.001;
double IoTServer::m_lon_offset = 0.001;


IoTServer *IoTServer::mInstance = nullptr;

IoTServer *IoTServer::getInstance()
{
    if (!IoTServer::mInstance)
    {
        mInstance = new IoTServer;
    }
    return mInstance;
}

IoTServer::IoTServer(string endpoint)
{
    LOG();
    Common::m_endpoint = endpoint;
    Common::m_latitude = 48.1033;
    Common::m_longitude = -1.6725;
    init();
    setup();
}


IoTServer::~IoTServer()
{
    LOG();
}


void IoTServer::init()
{
    LOG();

    m_platformConfig = make_shared<PlatformConfig>
                       (ServiceType::InProc, // different service ?
                        ModeType::Server, // other is Client or Both
                        "0.0.0.0", // default ip
                        0, // default random port
                        OC::QualityOfService::LowQos // qos
                       );
    OCPlatform::Configure(*m_platformConfig);
}

void IoTServer::setup()
{
    LOG();
    OCStackResult result ;
    EntityHandler handler = bind(&IoTServer::handleEntity, this, placeholders::_1);

    result = createResource(Common::m_endpoint, Common::m_type, handler, m_resourceHandle);
    if (OC_STACK_OK != result)
    {
        cerr << "error: Error on createResource" << endl;
        throw OC::InitializeException(__PRETTY_FUNCTION__, result);
    }
}

OCStackResult IoTServer::createResource(string uri, string type, EntityHandler handler,
                                        OCResourceHandle &handle)
{
    LOG();
    OCStackResult result;
    string resourceUri = uri;
    string resourceType = type;
    string resourceInterface = Common::m_interface;
    uint8_t resourceFlag = OC_DISCOVERABLE | OC_OBSERVABLE;
    try
    {
        result = OCPlatform::registerResource//
                 ( handle,
                   resourceUri, resourceType,
                   resourceInterface, //
                   handler, resourceFlag);

        if (result != OC_STACK_OK)
            cerr << "error: Could not create " << type << " resource" << endl;
        else
            cerr << "log: Successfully created " << type << " resource" << endl;
    }
    catch (OC::OCException &e)
    {
        cerr << "error: OCException " <<  e.reason().c_str() << " " << hex << e.code();
        cerr << "error: @" << __PRETTY_FUNCTION__ << endl;
        result = OC_STACK_ERROR;
    }

    return result;
}


OCStackResult IoTServer::respond(std::shared_ptr<OC::OCResourceResponse> response)
{
    OCStackResult result =  OC_STACK_ERROR;
    LOG();

    if (response)
    {
        response->setResponseResult(OC_EH_OK);
        response->setResourceRepresentation(m_representation);
        result = OCPlatform::sendResponse(response);
    }
    return result;
}


OCEntityHandlerResult IoTServer::handleEntity(shared_ptr<OCResourceRequest> request)
{
    LOG();

    OCEntityHandlerResult result = OC_EH_ERROR;
    if (request)
    {
        string requestType = request->getRequestType();
        int requestFlag = request->getRequestHandlerFlag();
        if (requestFlag & RequestHandlerFlag::RequestFlag)
        {
            auto response = std::make_shared<OC::OCResourceResponse>();
            response->setRequestHandle(request->getRequestHandle());
            response->setResourceHandle(request->getResourceHandle());
            if (requestType == "GET")
            {
                cerr << "GET request for platform Resource" << endl;
                if (response)
                {
                    response->setResponseResult(OC_EH_OK);
                    response->setResourceRepresentation(m_representation);
                    if (OCPlatform::sendResponse(response) == OC_STACK_OK)
                    {
                        result = OC_EH_OK;
                    }
                }
            }
            else
            {
                cerr << "error: unsupported " << requestType << endl;
                response->setResponseResult(OC_EH_ERROR);
                OCPlatform::sendResponse(response);
            }
        }
    }
    return result;
}

void IoTServer::setValue(double latitude, double longitude)
{
//  cerr<<"log: "<< __PRETTY_FUNCTION__ << endl;
    Common::m_latitude = latitude;
    Common::m_longitude = longitude; 
}


void IoTServer::update()
{
    LOG();
    if ( false )
    {
        Common::m_latitude += m_lat_offset;
        Common::m_longitude += m_lon_offset;

        if (Common::m_latitude > m_latmax)
        {
            if (m_lat_offset > 0) { m_lat_offset = - m_lat_offset; }
        }
        else if (Common::m_latitude < m_latmin)
        {
            if ( m_lat_offset < 0 ) m_lat_offset = - m_lat_offset;
        }

        if (Common::m_longitude > m_lonmax)
        {
            if (m_lon_offset > 0) { m_lon_offset = - m_lon_offset; }
        }
        else if (Common::m_longitude < m_lonmin)
        {
            if ( m_lon_offset < 0 ) m_lon_offset = - m_lon_offset;
        }
    }
    {
        m_representation.setValue("latitude", Common::m_latitude);
        m_representation.setValue("longitude", Common::m_longitude);

        cout << "{"
             << Common::m_type <<": {"
             << "latitude:" << std::fixed << Common::m_latitude << "," 
             << "longitude:" << std::fixed << Common::m_longitude 
             << "}" << endl;
    }
    OCStackResult result = OCPlatform::notifyAllObservers(m_resourceHandle);
    if (OC_STACK_OK != result)
    {
        cerr << "warning: "<<  __PRETTY_FUNCTION__ << endl;
    }
}


void IoTServer::handle_signal(int signal)
{
    LOG();
    IoTServer::m_over = true;
}


void IoTServer::start()
{
    LOG();
    try
    {
        do
        {
            update();
            sleep(Common::m_period);
        }
        while (!IoTServer::m_over );
    } 
    catch (...)
    {
        exit(1);
    }

}


int IoTServer::main(int argc, char *argv[])
{
    LOG();
    struct sigaction sa;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = IoTServer::handle_signal;
    sigaction(SIGINT, &sa, nullptr);

    cerr << "log: Server: " << endl
         << "Press Ctrl-C to quit...." << endl
         << "Usage: server -v" << endl
         ;

    int subargc = argc;
    char **subargv = argv;
    for (int i = 1; i < argc; i++)
    {
        if (0 == strcmp("-v", argv[i]))
        {
            Common::m_logLevel++;
            argc--;
            subargv++;
        }
    }

    Platform::getInstance().setup(subargc, subargv);

    IoTServer* server = IoTServer::getInstance();
    try
    {
        if ((argc > 1) && argv[1])
        {
            Common::m_period  = atoi(argv[1]);
        }
        if ((argc > 2) && argv[2])
        {
            server->m_lat_offset = server->m_lon_offset = atof(argv[2]);
        }
        if ((argc > 3) && argv[3])
        {
            Common::m_latitude = atof(argv[3]);
        }
        if ((argc > 4) && argv[4])
        {
            Common::m_longitude = atof(argv[4]);
        }
        server->m_latmax = Common::m_latitude + 1;
        server->m_latmin = Common::m_latitude - 1;
        server->m_lonmax = Common::m_longitude + 1;
        server->m_lonmin = Common::m_longitude - 1;

        server->start();
    }
    catch (...)
    {
        exit(1);
    }
    return 0;
}


#ifdef CONFIG_SERVER_MAIN
int main(int argc, char *argv[])
{
    return IoTServer::main(argc, argv);
}
#endif
