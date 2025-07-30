#pragma once
#include <ChromaDB/Embeddings/EmbeddingFunction.h>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

class OllamaEmbeddingFunction : public chromadb::EmbeddingFunction {
public:
    OllamaEmbeddingFunction(
        const std::string& model_url = "localhost:11434",
        const std::string& model_name = "nomic-embed-text"
    )
        : chromadb::EmbeddingFunction("", model_name, model_url, "/api/embeddings") {}

    std::vector<std::vector<double>> Generate(const std::vector<std::string>& texts) override {
        std::vector<std::vector<double>> embeddings;
        for (const auto& text : texts) {
            nlohmann::json payload = {
                {"model", m_Model},
                {"prompt", text}
            };
            try {
                nlohmann::json resp = this->Request(payload);
                if (resp.contains("embedding")) {
                    embeddings.push_back(resp["embedding"].get<std::vector<double>>());
                } else {
                    embeddings.push_back({});
                }
            } catch (...) {
                embeddings.push_back({});
            }
        }
        return embeddings;
    }
};