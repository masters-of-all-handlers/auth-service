#include "view.hpp"

#include <fmt/format.h>
#include <sstream>
#include <unordered_map>

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/http/http_status.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/assert.hpp>

#include "../../../models/bookmark.hpp"
#include "../../lib/auth.hpp"

namespace bookmarker {

namespace {

struct TFilters {
    enum class ESortOrder {
        ID = 0,
        TITLE = 1,
        URL = 2,
        CREATED_TS = 3
    } order_by = ESortOrder::ID;

    size_t limit = 10;
    std::optional<std::string> tag;

    static TFilters Parse(const userver::server::http::HttpRequest& request) {
        TFilters result;

        if (request.HasArg("limit")) {
            result.limit = std::stoul(request.GetArg("limit").c_str());
        }

        if (request.HasArg("tag")) {
            result.tag = request.GetArg("tag");
        }

        if (request.HasArg("order_by")) {
            static std::unordered_map<std::string, ESortOrder> mappings{
                {"id", ESortOrder::ID},
                {"title", ESortOrder::TITLE},
                {"url", ESortOrder::URL},
                {"created_ts", ESortOrder::CREATED_TS},
            };
            result.order_by = mappings[request.GetArg("order_by")];
        }

        return result;
    }
};

std::string BuildDbRequest(const TSession& session, const TFilters& filters) {
    std::ostringstream ss;
    ss << "SELECT * FROM bookmarker.bookmarks ";
    ss << "WHERE owner_id = '" << session.user_id << "' ";
    if (filters.tag) {
        ss << " AND tag = '" << *filters.tag << "' ";
    }
    ss << "ORDER BY ";
    switch (filters.order_by) {
        case TFilters::ESortOrder::ID:
            ss << "id ";
            break;
        case TFilters::ESortOrder::TITLE:
            ss << "title ";
            break;
        case TFilters::ESortOrder::URL:
            ss << "url ";
            break;
        case TFilters::ESortOrder::CREATED_TS:
            ss << "created_ts ";
            break;
    }
    ss << "LIMIT " << filters.limit;

    return ss.str();
}

class GetBookmarks final : public userver::server::handlers::HttpHandlerBase {
public:
    static constexpr std::string_view kName = "handler-v1-get-bookmarks";

    GetBookmarks(const userver::components::ComponentConfig& config,
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

        auto filters = TFilters::Parse(request);
        auto result = pg_cluster_->Execute(
            userver::storages::postgres::ClusterHostType::kMaster,
            BuildDbRequest(*session, filters)
        );

        userver::formats::json::ValueBuilder response;
        response["items"].Resize(0);
        for (auto row : result.AsSetOf<TBookmark>(userver::storages::postgres::kRowTag)) {
            response["items"].PushBack(row);
        }

        return userver::formats::json::ToString(response.ExtractValue());
    }

private:
    userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace

void AppendGetBookmarks(userver::components::ComponentList& component_list) {
    component_list.Append<GetBookmarks>();
}

}  // namespace bookmarker
