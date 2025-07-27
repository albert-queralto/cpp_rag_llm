#ifndef CHROMA_CLIENT_HPP
#define CHROMA_CLIENT_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "ChromaDB/ChromaDB.h"

class ChromaClient {
private:
    chromadb::Client client;

public:
    explicit ChromaClient(const std::string& protocol, const std::string& host, const std::string& port);
    ~ChromaClient();

    std::string get_version();
    std::string get_heartbeat();
    chromadb::Collection create_collection(const std::string& name);
    bool delete_collection(chromadb::Collection& collection);
    void add_embeddings(const chromadb::Collection& collection, 
                        const std::vector<std::string>& ids, 
                        const std::vector<std::vector<double>>& embeddings, 
                        const std::vector<std::unordered_map<std::string, std::string>>& metadatas);
    std::vector<chromadb::QueryResponseResource> query(const chromadb::Collection& collection, 
                                                        const std::vector<std::string>& ids, 
                                                        const std::vector<std::vector<double>>& embeddings, 
                                                        int limit);
};

#endif // CHROMA_CLIENT_HPP