#include <azure/identity/client_secret_credential.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/transport.hpp>
#include <azure/core/io/body_stream.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
class MockTransport final : public Azure::Core::Http::HttpTransport {
public:
    int calls = 0;
    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Http::Request& request, const Azure::Core::Context&) override {
        if (request.GetUrl().GetHost() != "login.microsoftonline.com")
            throw std::runtime_error("Unexpected token authority");
        ++calls;
        auto response = std::make_unique<Azure::Core::Http::RawResponse>(
            1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");
        static const std::string body = R"({"access_token":"mock-token","expires_in":3600,"token_type":"Bearer"})";
        response->SetHeader("Content-Type", "application/json");
        response->SetBodyStream(std::make_unique<Azure::Core::IO::MemoryBodyStream>(
            reinterpret_cast<const uint8_t*>(body.data()), body.size()));
        return response;
    }
};
int main() {
    auto transport = std::make_shared<MockTransport>();
    Azure::Identity::ClientSecretCredentialOptions options;
    options.AuthorityHost = "https://login.microsoftonline.com/";
    options.Transport.Transport = transport;
    Azure::Identity::ClientSecretCredential credential(
        "test-tenant", "test-client", "test-secret", options);
    Azure::Core::Credentials::TokenRequestContext request;
    request.Scopes = {"https://storage.azure.com/.default"};
    auto token = credential.GetToken(request, Azure::Core::Context());
    if (token.Token != "mock-token" || transport->calls != 1) return 1;
    auto cached = credential.GetToken(request, Azure::Core::Context());
    if (cached.Token != token.Token || transport->calls != 1) return 2;
    std::cout << "Installed Azure Identity mocked token acquisition and cache passed\n";
}
