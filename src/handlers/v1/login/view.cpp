#include "view.hpp"

#include <fmt/format.h>

#include <userver/crypto/hash.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/assert.hpp>

#include "../../../models/user.hpp"

class LoginUser final : public userver::server::handlers::HttpHandlerBase {
public:
  static constexpr std::string_view kName = "handler-login-user";

  LoginUser(const userver::components::ComponentConfig &config,
            const userver::components::ComponentContext &component_context)
      : HttpHandlerBase(config, component_context),
        pg_cluster_(
            component_context
                .FindComponent<userver::components::Postgres>("postgres-db-1")
                .GetCluster()) {}

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest &request,
      userver::server::request::RequestContext &) const override {
    auto &http_response = request.GetHttpResponse();
    http_response.SetHeader("Content-Type", "application/json");
    http_response.SetHeader("Access-Control-Allow-Origin", "*");
    
    auto request_body =
        userver::formats::json::FromString(request.RequestBody());
    auto login = request_body["login"].As<std::optional<std::string>>();
    auto check_password =
        request_body["password"].As<std::optional<std::string>>();

    auto password = userver::crypto::hash::Sha256(check_password.value());
    LOG_CRITICAL() << login.value();
    LOG_CRITICAL() << check_password.value();
    // LOG_CRITICAL() << password; WTF?
    if (!login.has_value() || !check_password.has_value() ||
        login.value().empty() || check_password.value().empty()) {
      http_response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
      return {};
    }

    auto userResult = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "SELECT * FROM uservice_dynconf.users "
        "WHERE login = $1 AND password = $2",
        login.value(), password);

    if (userResult.IsEmpty()) {
      http_response.SetStatus(userver::server::http::HttpStatus::kNotFound);
      return {};
    }

    auto user =
        userResult.AsSingleRow<TUser>(userver::storages::postgres::kRowTag);
    if (password != user.password) {
      http_response.SetStatus(userver::server::http::HttpStatus::kNotFound);
      return {};
    }

    auto result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "INSERT INTO uservice_dynconf.auth_sessions(user_id) VALUES($1) "
        "ON CONFLICT DO NOTHING "
        "RETURNING auth_sessions.ticket",
        user.id);

    userver::formats::json::ValueBuilder response;
    response["ticket"] = result.AsSingleRow<std::string>();

    return userver::formats::json::ToString(response.ExtractValue());
  }

private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

void AppendLoginUser(userver::components::ComponentList &component_list) {
  component_list.Append<LoginUser>();
}
