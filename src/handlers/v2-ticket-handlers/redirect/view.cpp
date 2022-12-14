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
    http_response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
    userver::formats::json::ValueBuilder response;
    response["error_message"] = "Ticket header is required";

    return userver::formats::json::ToString(response.ExtractValue());
  }

  auto session_info = GetSessionInfo(pg_cluster_, request);
  if (!session_info.has_value()) {
    http_response.SetStatus(userver::server::http::HttpStatus::kBadRequest);
    userver::formats::json::ValueBuilder response;
    response["error_message"] = "Ticket is invalid";

    return userver::formats::json::ToString(response.ExtractValue());
  }

  std::string host(getenv("DYN_CONFIG_SERVER_ADRESS"));

  userver::clients::http::Headers headers = {};
  for (const auto &header_name : request.GetHeaderNames()) {
    headers[header_name] = request.GetHeader(header_name);
  }
  headers["user"] = session_info->user_id;

  std::shared_ptr<userver::clients::http::Request> redirected_request = http_client_.CreateRequest();
  if (request.GetMethod() == userver::server::http::HttpMethod::kPost || request.GetMethod() == userver::server::http::HttpMethod::kPatch) {
    redirected_request = redirected_request->data(request.RequestBody());
  }

  std::string url = host + request.GetRequestPath() + "?";
  for (const auto& argument_name: request.ArgNames()) {
    for (const auto& argument_value : request.GetArgVector(argument_name)) {
      for (const auto& name_char : argument_name) {
        url.push_back(name_char);
      }
      url.push_back('=');
      for (const auto& value_char : argument_value) {
        url.push_back(value_char);
      }
      url.push_back('&');
    }
  }
  auto server_response = redirected_request->headers(headers)
                         ->method(convert(request.GetMethod()))
                         ->url(url)
                         ->retry(5)
                         ->timeout(std::chrono::seconds(1))
                         ->perform();

  http_response.SetStatus(
    userver::server::http::HttpStatus(server_response->status_code()));
  for (const auto& header : server_response->headers()) {
    http_response.SetHeader(header.first, header.second);
  }
  return server_response->body();
}

} // namespace auth_service::handlers::redirect

void AppendRedirectHandler(userver::components::ComponentList &component_list) {
  component_list.Append<auth_service::handlers::redirect::Handler>();
}
