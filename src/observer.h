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

#ifndef OBSERVER_H_
#define OBSERVER_H_

#include <memory>
#include <iostream>

#include <iotivity/resource/OCApi.h>
#include <iotivity/resource/OCPlatform.h>
#include <iotivity/resource/OCResource.h>


class Resource
{
    public:
        Resource(std::shared_ptr<OC::OCResource> resource);
        virtual ~Resource();
        void get();
    protected:
        void onGet(const OC::HeaderOptions &, const OC::OCRepresentation &, int);
    protected:
        std::shared_ptr<OC::OCResource> m_OCResource;
        OC::OCRepresentation m_Representation;
        OC::GetCallback m_GETCallback;
};

class IoTObserver
{
    public:
        static int main(int argc, char *argv[]);

        static IoTObserver *getInstance();

        static void onObserve(const OC::HeaderOptions /*headerOptions*/,
                              const OC::OCRepresentation &rep,
                              const int &eCode, const int &sequenceNumber);

    public:
        std::shared_ptr<Resource> getResource();
        void start();
        /// Override with your business logic related to resource type
        void handle(const OC::HeaderOptions /*headerOptions*/,
                    const OC::OCRepresentation &rep,
                    const int &eCode, const int &sequenceNumber);

    private:
        IoTObserver();
        virtual ~IoTObserver();
        void init();
        void onFind(std::shared_ptr<OC::OCResource>);

    private:
        static IoTObserver *mInstance;
        std::shared_ptr<Resource> m_Resource;
        std::shared_ptr<OC::PlatformConfig> m_PlatformConfig;
        OC::FindCallback m_FindCallback;
        std::string m_DateTime;
        double m_CountDown;

};

#endif /* OBSERVER_H_ */
