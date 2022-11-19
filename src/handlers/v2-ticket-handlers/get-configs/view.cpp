#include "view.hpp"
#include "./../../lib/auth.hpp"

#include "userver/formats/json/inline.hpp"
#include "userver/formats/json/value.hpp"
#include "userver/formats/yaml/value_builder.hpp"
#include <userver/components/component_context.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/formats/json.hpp>
#include <userver/clients/http/client.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/assert.hpp>

namespace auth_service::handlers::configs::get {
    Handler::Handler(const userver::components::ComponentConfig &config,
                     const userver::components::ComponentContext& context)
            : HttpHandlerBase(config, context),
              pg_cluster_(
                      context
                              .FindComponent<userver::components::Postgres>("postgres-db-1")
                              .GetCluster()),
              http_client_(
                      context
                              .FindComponent<userver::components::HttpClient>()
                              .GetHttpClient()) {}

    std::string Handler::HandleRequestThrow(const userver::server::http::HttpRequest &request,
                                            userver::server::request::RequestContext &context) const {
      auto ticket = request.GetHeader("Ticket");
      if (ticket.empty()) {
        auto& response = request.GetHttpResponse();
        response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
        return {};
      }

      auto session_info = GetSessionInfo(pg_cluster_, request);
      if (!session_info.has_value()) {
        auto &response = request.GetHttpResponse();
        response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
        return {};
      }

      std::string host(getenv("DYN_CONFIG_SERVER_ADRESS"));

      userver::clients::http::Headers headers = {};
      for (const auto& header_name : request.GetHeaderNames()) {
        headers[header_name] = request.GetHeader(header_name);
      }
      headers["user"] = session_info->user_id;

      auto server_response = http_client_.CreateRequest()
              ->get(host + "/admin/v1/variables")
              ->data(request.RequestBody())
              ->headers(headers)
              ->retry(5)
              ->timeout(std::chrono::seconds(1))
              ->perform();

      if (!server_response->IsOk()) {
        auto &response = request.GetHttpResponse();
        response.SetStatus(userver::server::http::HttpStatus(server_response->status_code()));
        return {};
      } else {
        return server_response->body();
      }
    }


} // namespace auth_service::handlers::configs::get

void AppendGetConfigsHandler(userver::components::ComponentList &component_list) {
  component_list.Append<auth_service::handlers::configs::get::Handler>();
}