#include "bookmark.hpp"

namespace bookmarker {

userver::formats::json::Value Serialize(const TBookmark& bookmark,
                                        userver::formats::serialize::To<userver::formats::json::Value>) {
    userver::formats::json::ValueBuilder item;
    item["id"] = bookmark.id;
    item["url"] = bookmark.url;
    item["title"] = bookmark.title;
    item["created_ts"] = bookmark.created_ts;
    return item.ExtractValue();
}

}  // namespace bookmarker