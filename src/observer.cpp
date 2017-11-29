// -*-c++-*-
//******************************************************************
//
// Copyright 2014 Intel Corporation.
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
#include <cstdio>

#include "observer.h"

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
        IoTObserver::getInstance()->handle(headerOptions, representation, eCode, 0);
    }
    else
    {
        cerr << "errror:: in GET response:" << eCode << endl;
    }
}


IoTObserver *IoTObserver::mInstance = nullptr;


IoTObserver::IoTObserver()
{
    LOG();
    init();
}


IoTObserver::~IoTObserver()
{
    LOG();
}


IoTObserver *IoTObserver::getInstance()
{
    if (!IoTObserver::mInstance)
    {
        mInstance = new IoTObserver;
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


void IoTObserver::init()
{
    LOG();
    static OCPersistentStorage ps {override_fopen, fread, fwrite, fclose, unlink };
    m_PlatformConfig = make_shared<PlatformConfig>
                       (ServiceType::InProc, //
                        ModeType::Both, //
                        "0.0.0.0", //
                        0, //
                        OC::QualityOfService::LowQos, //
                        Common::isSecure() ? (&ps) : NULL
                       );
    OCPlatform::Configure(*m_PlatformConfig);
    m_FindCallback = bind(&IoTObserver::onFind, this, placeholders::_1);
}


void IoTObserver::start()
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


shared_ptr<Resource> IoTObserver::getResource()
{
    return m_Resource;
}


void IoTObserver::onFind(shared_ptr<OCResource> resource)
{
    LOG();
    try
    {
        if (resource)
        {
            auto resourceUri = resource->uri();
            if (Common::m_endpoint == resourceUri)
            {
                cerr << "resourceUri=" << resourceUri << endl;
                auto scheme = Common::isSecure() ? "coaps://" : "coap://";
                for (auto &resourceEndpoint : resource->getAllHosts())
                {
                    cerr << "resourceEndpoint=" << resourceEndpoint << endl;

                    if (std::string::npos != resourceEndpoint.find(scheme))
                    {
                        // Change Resource host if another host exists
                        std::cout << "\tChange host of resource endpoints" << std::endl;
                        std::cout << "\t\t" << "Current host is "
                                  << resource->setHost(resourceEndpoint) << std::endl;
                        m_Resource = make_shared<Resource>(resource);
                        QueryParamsMap test;
                        resource->observe(OC::ObserveType::Observe, test, &IoTObserver::onObserve);
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


void IoTObserver::onObserve(const HeaderOptions headerOptions,
                            const OCRepresentation &rep,
                            const int &eCode, const int &sequenceNumber)
{
    LOG();
    try
    {
        if (eCode == OC_STACK_OK && sequenceNumber != OC_OBSERVE_NO_OPTION)
        {
            if (sequenceNumber == OC_OBSERVE_REGISTER)
            {
                cerr << "log: Observe registration action is successful" << endl;
            }
            else if (sequenceNumber == OC_OBSERVE_DEREGISTER)
            {
                cerr << "log: Observe De-registration action is successful" << endl;
            }
            cerr << "log: observe: sequenceNumber=" << sequenceNumber << endl;
            IoTObserver::getInstance()->handle(headerOptions, rep, eCode, sequenceNumber);
        }
        else
        {
            if (sequenceNumber == OC_OBSERVE_NO_OPTION)
            {
                cerr << "warning: Observe registration or de-registration action is failed" << endl;
            }
            else
            {
                cerr << "error: onObserve Response error=" << eCode << endl;
                //exit(-1);
            }
        }
    }
    catch (exception &e)
    {
        cerr << "warning: Exception: " << e.what() << " in onObserve" << endl;
    }
}


// TODO: override with your business logic
void IoTObserver::handle(const HeaderOptions headerOptions, const OCRepresentation &rep,
                         const int &eCode, const int &sequenceNumber)
{
    rep.getValue("datetime", m_DateTime);
    cerr << "log: " << "datetime" << "=" << m_DateTime << endl;
    rep.getValue("countdown", m_CountDown);
    cerr << "log: " << "countdown" << "=" << m_CountDown << endl;
    cout << "clock: " << m_DateTime << ", " << m_CountDown << endl;
}


int IoTObserver::main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (0 == strcmp("-v", argv[i]))
        {
            Common::m_logLevel++;
        }
    }

    IoTObserver::getInstance()->start();

    int choice = 0;
    do
    {
        cin >> choice;
        switch (choice)
        {
            case 9:
            default:
                return 0;
        }
    }
    while (choice != 9);
    return 0;
}


#ifdef CONFIG_OBSERVER_MAIN
int main(int argc, char *argv[])
{
    return IoTObserver::main(argc, argv);
}
#endif
