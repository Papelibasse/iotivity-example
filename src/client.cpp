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
#include <unistd.h>
#include <ctime>
#include "client.h"

using namespace std;
using namespace OC;


Resource::Resource(shared_ptr<OCResource> resource)
{
    LOG();
    m_OCResource = resource;
    m_GETCallback = bind(&Resource::onGet, this,
                         placeholders::_1, placeholders::_2, placeholders::_3);
}


Resource::~Resource()
{
}


void Resource::onGet(const HeaderOptions &headerOptions,
                     const OCRepresentation &representation, int eCode)
{
    LOG();
    if (eCode < OC_STACK_INVALID_URI)
    {
        IoTClient::getInstance()->handle(headerOptions, representation, eCode, 0);
    }
    else
    {
        cerr << "errror:: in GET response:" << eCode << endl;
    }
    IoTClient::getInstance()->input();
}


void Resource::get()
{
    LOG();
    QueryParamsMap params;
    m_OCResource->get(params, m_GETCallback);
}


IoTClient *IoTClient::mInstance = nullptr;


IoTClient::IoTClient()
{
    LOG();
    init();
}


IoTClient::~IoTClient()
{
    LOG();
}


IoTClient *IoTClient::getInstance()
{
    if (!IoTClient::mInstance)
    {
        mInstance = new IoTClient;
    }
    return mInstance;
}


static FILE *override_fopen(const char *path, const char *mode)
{
    LOG();
    static char const *const CRED_FILE_NAME = "oic_svr_db_client.dat";
    char const *const filename
        = (0 == strcmp(path, OC_SECURITY_DB_DAT_FILE_NAME))
          ? CRED_FILE_NAME : path;
    return fopen(filename, mode);
}


void IoTClient::init()
{
    LOG();
    static OCPersistentStorage ps {override_fopen, fread, fwrite, fclose, unlink };
    m_PlatformConfig = make_shared<PlatformConfig>
                       (ServiceType::InProc, //
                        ModeType::Both, //
                        "0.0.0.0", //
                        0, //
                        OC::QualityOfService::LowQos, //
                        &ps
                       );
    OCPlatform::Configure(*m_PlatformConfig);
    m_FindCallback = bind(&IoTClient::onFind, this, placeholders::_1);
}


void IoTClient::start()
{
    LOG();
    string coap_multicast_discovery = string(OC_RSRVD_WELL_KNOWN_URI);
    OCConnectivityType connectivityType(CT_ADAPTER_IP);
    try
    {
        OCPlatform::findResource("", //
                                 coap_multicast_discovery.c_str(),
                                 connectivityType,
                                 m_FindCallback,
                                 OC::QualityOfService::LowQos);
    }
    catch (OCException &e)
    {
        cerr << "error: Exception: " << e.what();
        exit(1);
    }
}


shared_ptr<Resource> IoTClient::getResource()
{
    return m_Resource;
}


void IoTClient::onFind(shared_ptr<OCResource> resource)
{
    LOG();
    try
    {
        if (resource)
        {
            print(resource);

            string resourceUri = resource->uri();
            if (Common::m_endpoint == resourceUri)
            {
                cerr << "resourceUri=" << resourceUri << endl;
                for (auto &resourceEndpoint : resource->getAllHosts())
                {
                    if (std::string::npos != resourceEndpoint.find("coaps"))
                    {
                        // Change Resource host if another host exists
                        std::cout << "\tChange host of resource endpoints" << std::endl;
                        std::cout << "\t\t" << "Current host is "
                                  << resource->setHost(resourceEndpoint) << std::endl;
                        m_Resource = make_shared<Resource>(resource);
                        if (true)   // simple client can only use get once
                        {
                            m_Resource->get();
                        }
                        break;
                    }

                }
            }
        }
    }
    catch (OCException &ex)
    {
        cerr << "error: Caught exception in discoveredResource: " << ex.reason() << endl;
    }
}


void IoTClient::print(shared_ptr<OCResource> resource)
{
    LOG();
    cerr << "log: Resource: uri: " << resource->uri() << endl;
    cerr << "log: Resource: host: " << resource->host() << endl;

    for (auto &type : resource->getResourceTypes())
    {
        cerr << "log: Resource: type: " << type << endl;
    }

    for (auto &interface : resource->getResourceInterfaces())
    {
        cerr << "log: Resource: interface: " << interface << endl;
    }
    for (auto &endpoint : resource->getAllHosts())
    {
        cerr << "log: Resource: endpoint: " << endpoint << endl;
    }
}


// TODO: override with your business logic
void IoTClient::handle(const HeaderOptions headerOptions, const OCRepresentation &rep,
                       const int &eCode, const int &sequenceNumber)
{
    rep.getValue("datetime", m_DateTime);
    cerr << "log: " << "datetime" << "=" << m_DateTime << endl;
    rep.getValue("countdown", m_CountDown);
    cerr << "log: " << "countdown" << "=" << m_CountDown << endl;
    cout << "clock: " << m_DateTime << ", " << m_CountDown << endl;
}


void IoTClient::input()
{
    cerr << endl << "menu: "
         << "  9) Quit"
         << "  *) Display this menu"
         << endl;
}


int IoTClient::main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (0 == strcmp("-v", argv[i]))
        {
            Common::m_logLevel++;
        }
    }

    IoTClient::getInstance()->start();

    int choice = 0;
    do
    {
        cin >> choice;
        switch (choice)
        {
            case 9:
                return 0;
            default:
                IoTClient::getInstance()->input();
                break;
        }
    }
    while (choice != 9);
    return 0;
}


#ifdef CONFIG_CLIENT_MAIN
int main(int argc, char *argv[])
{
    return IoTClient::main(argc, argv);
}
#endif
