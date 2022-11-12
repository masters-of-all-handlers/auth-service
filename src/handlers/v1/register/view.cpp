#include "view.hpp"

#include <fmt/format.h>

#include <userver/crypto/hash.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/assert.hpp>

#include "../../../utils/check_of_input_data.h"

class RegisterUser final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view
    kName = "handler-register-user";

    RegisterUser(const userver::components::ComponentConfig &config,
                 const userver::components::ComponentContext &component_context)
            : HttpHandlerBase(config, component_context),
              pg_cluster_(
                      component_context
                              .FindComponent<userver::components::Postgres>("postgres-db-1")
                              .GetCluster()) {}

    std::string HandleRequestThrow(
            const userver::server::http::HttpRequest &request,
            userver::server::request::RequestContext &
    ) const override {
        auto login = request.GetFormDataArg("login").value;
        auto check_password = request.GetFormDataArg("password").value;

        if (!IsRegistrationDataValid(login, check_password)) {
            auto &response = request.GetHttpResponse();
            response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
            return {};
        }
        auto password = userver::crypto::hash::Sha256(check_password);

        auto result = pg_cluster_->Execute(
                userver::storages::postgres::ClusterHostType::kMaster,
                "INSERT INTO uservice_dynconf.users(email, password) VALUES($1, $2) "
                "ON CONFLICT DO NOTHING "
                "RETURNING users.id",
                email, password
        );

        if (result.IsEmpty()) {
            auto &response = request.GetHttpResponse();
            response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
            return {};
        }

        userver::formats::json::ValueBuilder response;
        response["id"] = result.AsSingleRow<std::string>();

        return userver::formats::json::ToString(response.ExtractValue());
    }

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

void AppendRegisterUser(userver::components::ComponentList& component_list) {
    component_list.Append<RegisterUser>();
}

