#include "exporter.h"

namespace bro {
    namespace rest {
        // List of key / value pairs that should be either added or updated.
        thread_local static std::map<std::string, std::string> _export_cache_add;        

        // List of keys that should be completely removed from the list of published items.
        thread_local static std::vector<std::string> _export_cache_remove;

        // Shared variable with cached string containing JSON data in a render-able format.
        static std::string _export_cache_json;

        /**
            Renders all exported data is an easily visible fashion.
        */
        static int ExportDataHandler(struct mg_connection *conn, void *cbdata);
        
        void ExportManager::Add(const std::string key, const std::string value) {
            if(key == "") {
                return;
            }
            if(key[key.length() - 1] == '.') {
                return;
            }
            _export_cache_add[key] = value;
        }

        void ExportManager::Remove(const std::string key) {
            if(key == "") {
                return;
            }
            if(key[key.length() - 1] == '.') {
                return;
            }
            _export_cache_remove.push_back(key);
        }

        void ExportManager::Init(const uint16_t port) {
            char portbuf[1024];
            char dirbuf[1024];
            snprintf(portbuf, 1024, "%d", (int32_t)port);
            snprintf(dirbuf, 1024, "/tmp/bro-rest-XXXXXX");
            mkdtemp(dirbuf);
            printf("Using document root: %s\n", dirbuf);
            const char *options[] = { 
                "document_root", dirbuf,
                "listening_ports", portbuf,
                NULL
            };
             
            mg_callbacks callbacks;
            memset(&callbacks, 0, sizeof(callbacks));
            this->http_ctx = mg_start(&callbacks, 0, options);
            mg_set_request_handler(this->http_ctx, "/export", ExportDataHandler, 0);
        }

        void ExportManager::Update() {
            if(NULL == this->http_ctx) {
                return;
            }

            // printf("Acquiring context lock ...\n");
            mg_lock_context(this->http_ctx);

            // printf("Updating data ...\n");
            for(std::map<std::string, std::string>::iterator iter = _export_cache_add.begin();
                        iter != _export_cache_add.end(); ++iter) {
                export_container[iter->first] = iter->second;
            }

            for(std::vector<std::string>::iterator iter = _export_cache_remove.begin();
                        iter != _export_cache_remove.end(); ++iter) {
                if(export_container.find(*iter) != export_container.end()) {
                    export_container.erase(export_container.find(*iter));
                }
            }
            
            _export_cache_json = "{";
            std::vector<std::string> working;
            std::vector<bool> isfirst;
            for(std::map<std::string, std::string>::iterator iter = export_container.begin();
                        iter != export_container.end(); ++iter) {
                std::vector<std::string> curr;
                std::string leaf;
                std::stringstream ss(iter->first);
                std::string item;
                while (std::getline(ss, item, '.')) {
                    curr.push_back(item);
                }
                
                if(curr.size() == 0) {
                    continue;
                }

                leaf = curr[curr.size() - 1];
                isfirst.push_back(true);
                int i = 0;
                for(i = 0; i < working.size(); ++i) {
                    if(curr[i] != working[i]) {
                        break;
                    }
                }
                // shared parent
                if( (i == curr.size() - 1) && (i == working.size() - 1) ) {
                    isfirst.pop_back();
                    isfirst.push_back(false);
                    _export_cache_json += std::string(", ") + std::string("\"") + leaf + std::string("\": \"") + iter->second + std::string("\"");
                }
                // adding something elsewhere in the tree
                else {
                    for(int j = i; j < ((int)working.size() - 1); ++j) {
                        _export_cache_json += std::string("}");
                        isfirst.pop_back();
                    }
                    for(int j = i; j < ((int)curr.size()) - 1; ++j) {
                        _export_cache_json += (isfirst.back() ? std::string("") : std::string(", "));
                        _export_cache_json += std::string("\"") + std::string(curr[j]) + std::string("\": {");
                        isfirst.push_back(true);
                    }
                    _export_cache_json += (isfirst.back() ? std::string("") : std::string(", "));
                    isfirst.pop_back();
                    _export_cache_json += std::string("\"") + leaf + std::string("\": \"") + iter->second + std::string("\"");
                    isfirst.push_back(false);
                }
                working = curr;
            }
            
            for(int i = 0; i < (int)working.size(); ++i) {
                _export_cache_json += std::string("}");
                isfirst.pop_back();
            }

            mg_unlock_context(this->http_ctx);

            _export_cache_add.clear();
            _export_cache_remove.clear();
        }

        static int ExportDataHandler(struct mg_connection *conn, void *cbdata)
        {
            mg_context* context = mg_get_context(conn);
            mg_lock_context(context);

            mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n");
            mg_printf(conn, "%s\n", _export_cache_json.c_str());            

            mg_unlock_context(context);
            return 1;
        }
    }
}

