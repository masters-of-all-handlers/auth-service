#include "view.h"

#include <fmt/format.h>

#include <userver/formats/json.hpp>
#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/assert.hpp>
#include <userver/clients/http/client.hpp>
#include <userver/clients/http/component.hpp>

#include "../../lib/auth.hpp"

class CheckHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-v1-check-handler";

    CheckHandler(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& component_context)
            : HttpHandlerBase(config, component_context),
              pg_cluster_(
                      component_context
                              .FindComponent<userver::components::Postgres>("postgres-db-1")
                              .GetCluster()){}

    std::string HandleRequestThrow(
            const userver::server::http::HttpRequest& request,
            userver::server::request::RequestContext&
    ) const override {
        auto session = GetSessionInfo(pg_cluster_, request);
        if (!session) {
            auto& response = request.GetHttpResponse();
            response.SetStatus(userver::server::http::HttpStatus::kUnauthorized);
            return {};
        }
        userver::formats::json::ValueBuilder response;
        response["user_id"] = session.value().user_id;
        return ToString(response.ExtractValue());
    }

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

void AppendCheckHandler(userver::components::ComponentList& component_list) {
    component_list.Append<CheckHandler>();
}
