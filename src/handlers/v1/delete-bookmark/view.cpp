#include "view.hpp"

#include <fmt/format.h>

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/assert.hpp>

#include "../../lib/auth.hpp"

namespace bookmarker {

namespace {

class DeleteBookmark final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-v1-delete-bookmark";

    DeleteBookmark(const userver::components::ComponentConfig& config,
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
        auto session = GetSessionInfo(pg_cluster_, request);
        if (!session) {
            auto& response = request.GetHttpResponse();
            response.SetStatus(userver::server::http::HttpStatus::kUnauthorized);
            return {};
        }

        const auto& id = request.GetPathArg("id");
        auto result = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            "DELETE FROM bookmarker.bookmarks "
            "WHERE id = $1 ",
            id
        );

        userver::formats::json::ValueBuilder response;
        response["id"] = id;
        return userver::formats::json::ToString(response.ExtractValue());
    }

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace

void AppendDeleteBookmark(userver::components::ComponentList& component_list) {
    component_list.Append<DeleteBookmark>();
}

}  // namespace bookmarker
