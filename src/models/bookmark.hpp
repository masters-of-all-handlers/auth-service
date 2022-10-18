#pragma once

#include <chrono>
#include <string>

#include <userver/formats/json/value_builder.hpp>

namespace bookmarker {

struct TBookmark {
    std::string id;
    std::string owner_id;
    std::string url;
    std::string title;
    std::optional<std::string> tag;
    std::chrono::system_clock::time_point created_ts;
};

userver::formats::json::Value Serialize(const TBookmark& data,
                                        userver::formats::serialize::To<userver::formats::json::Value>);

}  // namespace bookmarker
