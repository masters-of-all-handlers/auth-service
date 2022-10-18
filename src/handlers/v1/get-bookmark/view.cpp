#include "view.hpp"

#include <fmt/format.h>

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/assert.hpp>

#include "../../../models/bookmark.hpp"
#include "../../lib/auth.hpp"

namespace bookmarker {

namespace {

class GetBookmark final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-v1-get-bookmark";

    GetBookmark(const userver::components::ComponentConfig& config,
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

        const auto& id = request.GetPathArg("id");
        auto result = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            "SELECT * FROM bookmarker.bookmarks "
            "WHERE id = $1",
            id
        );

        if (result.IsEmpty()) {
            auto& response = request.GetHttpResponse();
            response.SetStatus(userver::server::http::HttpStatus::kNotFound);
            return {};
        }

        auto bookmark = result.AsSingleRow<TBookmark>(userver::storages::postgres::kRowTag);
        if (bookmark.owner_id != session->user_id) {
            auto& response = request.GetHttpResponse();
            response.SetStatus(userver::server::http::HttpStatus::kNotFound);
            return {};
        }

        return ToString(userver::formats::json::ValueBuilder{bookmark}.ExtractValue());
    }

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace

void AppendGetBookmark(userver::components::ComponentList& component_list) {
    component_list.Append<GetBookmark>();
}

}  // namespace bookmarker
