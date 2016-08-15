// Copyright (c) 2001-2015, SCALABLE Network Technologies, Inc.  All Rights Reserved.
//                          600 Corporate Pointe
//                          Suite 1200
//                          Culver City, CA 90230
//                          info@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the QualNet
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

#ifndef _CLIENT_UUID_GENERATOR_H_
#define _CLIENT_UUID_GENERATOR_H_

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/lexical_cast.hpp>
#include "qualnet_error.h"

enum ClientTypes
{
    VOPS_CLIENT = 0,
    LEGACY_CLIENT = 1
};

class ClientIdGenerator
{
public:
    // This function will generate UUID
    static std::string generateClientId(ClientTypes type)
    {
        std::string clientId;
        if (type == VOPS_CLIENT)
        {
            clientId.append("V-");
        }
        else if (type == LEGACY_CLIENT)
        {
            clientId.append("G-");
        }
        else
        {
            ERROR_ReportErrorArgs("Invalid client type %d.\n", type);
        }
        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        clientId.append(boost::lexical_cast<std::string>(uuid));

        return clientId;
    }
};

#endif // #ifndef _CLIENT_UUID_GENERATOR_H_
