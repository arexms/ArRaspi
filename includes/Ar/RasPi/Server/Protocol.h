#pragma once

#include <Ar/RasPi/Server/ProtocolConfig.h>

namespace Ar
{
    namespace RasPi
    {
        namespace Server
        {
            class Protocol
            {
            public:
                Protocol();
                ~Protocol();

                bool initialize(const ProtocolConfig &config);
                void run();
                void stop();

            protected:
                const ProtocolConfig& config() const;

            private:
                ProtocolConfig _config;
            };

            // IMPLEMENTATION
            inline const ProtocolConfig& Protocol::config() const
            {
                return _config;
            }
        }
    }
}
