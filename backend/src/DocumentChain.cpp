#include "DocumentChain.hpp"
#include "OllamaEmbeddingFunction.hpp"
#include "ChromaClient.hpp"
#include <fstream>
#include <sstream>

std::string load_document(const std::string& file_path) {
    std::ifstream ifs(file_path);
    return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

std::vector<std::string> split_document(const std::string& text, size_t chunk_size = 1000, size_t overlap = 100) {
    std::vector<std::string> chunks;
    size_t i = 0;
    while (i < text.size()) {
        size_t len = std::min(chunk_size, text.size() - i);
        chunks.push_back(text.substr(i, len));
        i += (chunk_size - overlap);
    }
    return chunks;
}

std::string process_document_chain(
    const std::string& file_path,
    const std::string& collection_name
) {
    std::string text = load_document(file_path);
    auto chunks = split_document(text);

    auto embedding_function = std::make_shared<OllamaEmbeddingFunction>("localhost:11434", "nomic-embed-text");
    auto embeddings = embedding_function->Generate(chunks);

    std::vector<std::string> ids;
    std::vector<std::unordered_map<std::string, std::string>> metadatas;
    for (size_t i = 0; i < chunks.size(); ++i) {
        ids.push_back("chunk_" + std::to_string(i));
        metadatas.push_back({{"source", file_path}});
    }

    ChromaClient chromaClient("http", "chromadb", "8000");
    std::shared_ptr<chromadb::Collection> collection;
    try {
        collection = std::make_shared<chromadb::Collection>(chromaClient.get_collection(collection_name));
    } catch (const chromadb::ChromaNotFoundException&) {
        collection = std::make_shared<chromadb::Collection>(chromaClient.create_collection(collection_name));
    }
    chromaClient.add_embeddings(*collection, ids, embeddings, metadatas);

    return "Document processed and stored in ChromaDB!";
}