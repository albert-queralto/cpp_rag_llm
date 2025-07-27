#include "ChromaClient.hpp"
#include <iostream>

ChromaClient::ChromaClient(const std::string& protocol, const std::string& host, const std::string& port)
    : client(protocol, host, port) {
    std::cout << "[DEBUG] ChromaClient initialized with protocol: " << protocol 
                << ", host: " << host << ", port: " << port << std::endl;
}

ChromaClient::~ChromaClient() {
    std::cout << "[DEBUG] ChromaClient destroyed." << std::endl;
}

std::string ChromaClient::get_version() {
    return client.GetVersion();
}

std::string ChromaClient::get_heartbeat() {
    return std::to_string(client.GetHeartbeat());
}

chromadb::Collection ChromaClient::create_collection(const std::string& name) {
    return client.CreateCollection(name);
}

bool ChromaClient::delete_collection(chromadb::Collection& collection) {
    try {
        client.DeleteCollection(collection);
        return true; // Return true if no exception is thrown
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to delete collection: " << e.what() << std::endl;
        return false; // Return false if an exception is thrown
    }
}

void ChromaClient::add_embeddings(const chromadb::Collection& collection, 
                                    const std::vector<std::string>& ids, 
                                    const std::vector<std::vector<double>>& embeddings, 
                                    const std::vector<std::unordered_map<std::string, std::string>>& metadatas) {
    client.AddEmbeddings(collection, ids, embeddings, metadatas);
}

std::vector<chromadb::QueryResponseResource> ChromaClient::query(const chromadb::Collection& collection, 
                                                                    const std::vector<std::string>& ids, 
                                                                    const std::vector<std::vector<double>>& embeddings, 
                                                                    int limit) {
    return client.Query(collection, ids, embeddings, limit);
}