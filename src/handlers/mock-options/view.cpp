#include "view.hpp"

#include <userver/clients/http/client.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/component_context.hpp>
#include <userver/logging/log.hpp>
#include <userver/utils/assert.hpp>

namespace auth_service::handlers::options {
Handler::Handler(const userver::components::ComponentConfig &config,
                 const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string Handler::HandleRequestThrow(
    const userver::server::http::HttpRequest &request,
    userver::server::request::RequestContext& ) const {
    auto &response = request.GetHttpResponse();
    response.SetHeader("Access-Control-Allow-Origin", request.getHeader().GetHeader("Origin"));
    return {};
}

} // namespace auth_service::handlers::configs

void AppendOptionsMock(userver::components::ComponentList &component_list) {
  component_list.Append<auth_service::handlers::options::Handler>();
}
