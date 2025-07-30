#include "ChromaClient.hpp"
#include <iostream>

ChromaClient::ChromaClient(
    const std::string& protocol, 
    const std::string& host, 
    const std::string& port
)
    : client(protocol, host, port) {
    std::cout << "[DEBUG] ChromaClient initialized with protocol: " << protocol 
                << ", host: " << host << ", port: " << port << std::endl;
}

ChromaClient::~ChromaClient() {
    std::cout << "[DEBUG] ChromaClient destroyed." << std::endl;
}

std::string ChromaClient::get_version() {
    try {
        return client.GetVersion();
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to get ChromaDB version: " << e.what() << std::endl;
        return "Error retrieving version";
    }
    
}

std::string ChromaClient::get_heartbeat() {
    try {
        return std::to_string(client.GetHeartbeat());
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to get heartbeat: " << e.what() << std::endl;
        return "Error: Unable to fetch heartbeat";
    }
}

std::vector<chromadb::Collection> ChromaClient::get_collections() {
    try {
        return client.GetCollections();
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to get collections: " << e.what() << std::endl;
        throw;
    }
}

chromadb::Collection ChromaClient::get_collection(const std::string& name) {
    try {
        return client.GetCollection(name);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to get collection: " << e.what() << std::endl;
        throw std::runtime_error(std::string("Failed to get collection: ") + e.what());
    }
}

chromadb::Collection ChromaClient::create_collection(const std::string& name) {
    try {
        return client.CreateCollection(name);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to create collection: " << e.what() << std::endl;
        throw std::runtime_error(std::string("Failed to create collection: ") + e.what());
    }
}

chromadb::Collection ChromaClient::update_collection(
    const std::string& oldName,
    const std::string& newName,
    const std::unordered_map<std::string, std::string>& newMetadata
) {
    try {
        return client.UpdateCollection(oldName, newName, newMetadata);
    } catch (const chromadb::ChromaNotFoundException& e) {
        std::cerr << "[ERROR] Collection not found: " << e.what() << std::endl;
        throw std::runtime_error("Collection does not exist: " + oldName);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to update collection: " << e.what() << std::endl;
        throw std::runtime_error(std::string("Failed to update collection: ") + e.what());
    }
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

std::vector<chromadb::EmbeddingResource> ChromaClient::get_embeddings(
    const chromadb::Collection& collection,
    const std::vector<std::string>& ids,
    const std::vector<std::string>& include,
    const nlohmann::json& where_document,
    const nlohmann::json& where
) {
    return client.GetEmbeddings(collection, ids, include, where_document, where);
}

void ChromaClient::add_embeddings(
    const chromadb::Collection& collection, 
    const std::vector<std::string>& ids, 
    const std::vector<std::vector<double>>& embeddings, 
    const std::vector<std::unordered_map<std::string, 
    std::string>>& metadatas
) {
    try {
        client.AddEmbeddings(collection, ids, embeddings, metadatas);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to add embeddings: " << e.what() << std::endl;
        throw; // Rethrow the exception to be handled by the caller
    }
}

std::vector<chromadb::QueryResponseResource> ChromaClient::query(
    const chromadb::Collection& collection, 
    const std::vector<std::string>& ids, 
    const std::vector<std::vector<double>>& embeddings, 
    int limit
) {
    try {
        return client.Query(collection, ids, embeddings, limit);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to query collection: " << e.what() << std::endl;
        throw; // Rethrow the exception to be handled by the caller
    }
}