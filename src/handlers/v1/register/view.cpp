#include "view.hpp"

#include <fmt/format.h>

#include <userver/crypto/hash.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/assert.hpp>

#include "../../../models/bookmark.hpp"

namespace bookmarker {

namespace {

class RegisterUser final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-register-user";

    RegisterUser(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& component_context)
        : HttpHandlerBase(config, component_context),
            pg_cluster_(
                component_context
                    .FindComponent<userver::components::Postgres>("postgres-db-1")
                    .GetCluster()) {}

    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext&
    ) const override {
        auto email = request.GetFormDataArg("email").value;
        auto password = userver::crypto::hash::Sha256(request.GetFormDataArg("password").value);

        auto result = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            "INSERT INTO bookmarker.users(email, password) VALUES($1, $2) "
            "ON CONFLICT DO NOTHING "
            "RETURNING users.id",
            email, password
        );

        userver::formats::json::ValueBuilder response;
        response["id"] = result.AsSingleRow<std::string>();

        return userver::formats::json::ToString(response.ExtractValue());
    }

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace

void AppendRegisterUser(userver::components::ComponentList& component_list) {
    component_list.Append<RegisterUser>();
}

}  // namespace bookmarker
