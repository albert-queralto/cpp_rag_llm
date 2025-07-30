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
    std::vector<chromadb::Collection> get_collections();
    chromadb::Collection get_collection(const std::string& name);
    chromadb::Collection create_collection(const std::string& name);
    chromadb::Collection update_collection(
        const std::string& oldName, 
        const std::string& newName, 
        const std::unordered_map<std::string, std::string>& newMetadata = {}
    );
    bool delete_collection(chromadb::Collection& collection);
    std::vector<chromadb::EmbeddingResource> get_embeddings(
        const chromadb::Collection& collection,
        const std::vector<std::string>& ids,
        const std::vector<std::string>& include,
        const nlohmann::json& where_document,
        const nlohmann::json& where
    );
    void add_embeddings(
        const chromadb::Collection& collection, 
        const std::vector<std::string>& ids, 
        const std::vector<std::vector<double>>& embeddings, 
        const std::vector<std::unordered_map<std::string, std::string>>& metadatas);
    std::vector<chromadb::QueryResponseResource> query(
        const chromadb::Collection& collection, 
        const std::vector<std::string>& ids, 
        const std::vector<std::vector<double>>& embeddings, 
        int limit
    );
};

#endif // CHROMA_CLIENT_HPP