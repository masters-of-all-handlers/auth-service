#include "view.hpp"
#include "./../../lib/auth.hpp"

#include "userver/formats/json/inline.hpp"
#include "userver/formats/json/value.hpp"
#include "userver/formats/yaml/value_builder.hpp"
#include <userver/clients/http/client.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/formats/json.hpp>
#include <userver/logging/log.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/assert.hpp>
#include <iostream>

namespace auth_service::handlers::redirect {
Handler::Handler(const userver::components::ComponentConfig &config,
                 const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      pg_cluster_(
          context.FindComponent<userver::components::Postgres>("postgres-db-1")
              .GetCluster()),
      http_client_(context.FindComponent<userver::components::HttpClient>()
                       .GetHttpClient()) {}

userver::clients::http::HttpMethod convert(
    const userver::server::http::HttpMethod method) {
  using sMethod = userver::server::http::HttpMethod;
  using cMethod = userver::clients::http::HttpMethod;
  switch(method) {
    case sMethod::kDelete: 
      return cMethod::kDelete;
    case sMethod::kPost: 
      return cMethod::kPost;
    case sMethod::kGet: 
      return cMethod::kGet;
    case sMethod::kPut: 
      return cMethod::kPut;
    case sMethod::kPatch: 
      return cMethod::kPatch;
    case sMethod::kOptions: 
      return cMethod::kOptions;
    case sMethod::kHead: 
      return cMethod::kHead;
    default:
      return cMethod::kGet;
  }
}

std::string Handler::HandleRequestThrow(
    const userver::server::http::HttpRequest &request,
    userver::server::request::RequestContext& _) const {
  auto &http_response = request.GetHttpResponse();
  http_response.SetHeader("Content-Type", "application/json");
  http_response.SetHeader("Access-Control-Allow-Origin", "*");
  
  auto ticket = request.GetHeader("Ticket");
  if (ticket.empty()) {
    http_response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
    return {};
  }

  auto session_info = GetSessionInfo(pg_cluster_, request);
  if (!session_info.has_value()) {
    http_response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
    return {};
  }

  std::string host(getenv("DYN_CONFIG_SERVER_ADRESS"));

  std::cout << host + request.GetRequestPath() << "\n\n\n\n\n" << std::endl;

  userver::clients::http::Headers headers = {};
  for (const auto &header_name : request.GetHeaderNames()) {
    headers[header_name] = request.GetHeader(header_name);
  }
  headers["user"] = session_info->user_id;

  if (request.GetMethod() == userver::server::http::HttpMethod::kPost || request.GetMethod() == userver::server::http::HttpMethod::kPatch) {
    auto server_response = http_client_.CreateRequest()
      ->method(convert(request.GetMethod()))
      ->url(host + request.GetRequestPath())
      ->data(request.RequestBody())
      ->headers(headers)
      ->retry(5)
      ->timeout(std::chrono::seconds(1))
      ->perform();
  } else {
    auto server_response = http_client_.CreateRequest()
      ->method(convert(request.GetMethod()))
      ->url(host + request.GetRequestPath())
      ->headers(headers)
      ->retry(5)
      ->timeout(std::chrono::seconds(1))
      ->perform();
  }

  http_response.SetStatus(
    userver::server::http::HttpStatus(server_response->status_code()));
  return server_response->body();
}

} // namespace auth_service::handlers::redirect

void AppendRedirectHandler(userver::components::ComponentList &component_list) {
  component_list.Append<auth_service::handlers::redirect::Handler>();
}
