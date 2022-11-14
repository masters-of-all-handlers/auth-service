#include "view.hpp"

#include <fmt/format.h>

#include <userver/crypto/hash.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/assert.hpp>

#include "../../../utils/check_of_input_data.h"
#include "../../lib/auth.hpp"

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
        auto session = GetSessionInfo(pg_cluster_, request);
        if (!session) {
            auto& response = request.GetHttpResponse();
            response.SetStatus(userver::server::http::HttpStatus::kUnauthorized);
            return {};
        }
        auto request_body = userver::formats::json::FromString(request.RequestBody());
        auto login = request_body["login"].As<std::optional<std::string>>();
        auto check_password = request_body["password"].As<std::optional<std::string>>();

        if(!login.has_value() || !check_password.has_value() || login.value().empty() || check_password.value().empty() ){
            auto &response = request.GetHttpResponse();
            response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
            return {};
        }
        auto password = userver::crypto::hash::Sha256(check_password.value());
        //LOG_CRITICAL()<<password;
        auto result = pg_cluster_->Execute(
                userver::storages::postgres::ClusterHostType::kMaster,
                "INSERT INTO uservice_dynconf.users(login, password) VALUES($1, $2) "
                "ON CONFLICT DO NOTHING "
                "RETURNING users.id",
                login, password
        );

        if (result.IsEmpty()) {
            auto &response = request.GetHttpResponse();
            response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
            return {};
        }

        userver::formats::json::ValueBuilder response;
        response["user_id"] = result.AsSingleRow<std::string>();

        return userver::formats::json::ToString(response.ExtractValue());
    }

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

void AppendRegisterUser(userver::components::ComponentList& component_list) {
    component_list.Append<RegisterUser>();
}

