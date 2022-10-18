#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include "handlers/v1/add-bookmark/view.hpp"
#include "handlers/v1/get-bookmark/view.hpp"
#include "handlers/v1/delete-bookmark/view.hpp"
#include "handlers/v1/get-bookmarks/view.hpp"
#include "handlers/v1/register//view.hpp"
#include "handlers/v1/login/view.hpp"

int main(int argc, char* argv[]) {
  auto component_list =
      userver::components::MinimalServerComponentList()
          .Append<userver::server::handlers::Ping>()
          .Append<userver::components::TestsuiteSupport>()
          .Append<userver::components::HttpClient>()
          .Append<userver::server::handlers::TestsControl>()
          .Append<userver::components::Postgres>("postgres-db-1")
          .Append<userver::clients::dns::Component>();

  bookmarker::AppendAddBookmark(component_list);
  bookmarker::AppendGetBookmark(component_list);
  bookmarker::AppendDeleteBookmark(component_list);
  bookmarker::AppendGetBookmarks(component_list);
  bookmarker::AppendRegisterUser(component_list);
  bookmarker::AppendLoginUser(component_list);

  return userver::utils::DaemonMain(argc, argv, component_list);
}
