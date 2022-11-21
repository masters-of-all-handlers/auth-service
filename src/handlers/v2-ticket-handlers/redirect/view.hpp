#pragma once
#include "userver/clients/http/client.hpp"
#include "userver/components/component_config.hpp"
#include "userver/components/component_context.hpp"
#include "userver/formats/json/value.hpp"
#include "userver/server/handlers/http_handler_base.hpp"
#include "userver/server/handlers/http_handler_json_base.hpp"
#include "userver/storages/postgres/postgres_fwd.hpp"
#include <string_view>
#include <userver/components/component_list.hpp>

namespace auth_service::handlers::redirect {
class Handler final : public userver::server::handlers::HttpHandlerBase {
public:
  static constexpr std::string_view kName = "handler-redirect";

  Handler(const userver::components::ComponentConfig &config,
          const userver::components::ComponentContext &context);

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest &request,
      userver::server::request::RequestContext &context) const override final;

private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
  userver::clients::http::Client &http_client_;
};
} // namespace auth_service::handlers::configs

void AppendRedirectHandler(userver::components::ComponentList &component_list);
