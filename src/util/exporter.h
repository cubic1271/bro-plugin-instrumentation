#ifndef _BRO_INSTRUMENTATION_EXPORTER
#define _BRO_INSTRUMENTATION_EXPORTER

#include <cstring>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include "http/civetweb.h"

namespace bro {
    namespace rest {

        /**
            Class that publishes a single REST object that can be consumed via vanilla HTTP.
            
            Since HTTP requests will be arriving on a different thread than that of Bro's main thread, we need
            to deal with copying values from Bro's main thread into a context from which they can be safely read
            from the HTTP handler.  To keep things reasonably efficient, individual updates are cached until a
            special function called 'Update' is called.  This method updates the values that are actually published
            by the server.

            The call to Update is relatively expensive, and so should be invoked sparingly.
        */
        class ExportManager {
        private:
            struct mg_context *http_ctx;
            std::map <std::string, std::string> export_container;

        public:
            void Init(const uint16_t port);
            void Add(const std::string key, const std::string value);
            void Remove(const std::string key);
            void Update();
        };

    }
}
#endif

