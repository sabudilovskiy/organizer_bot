#include <cli_interface.hpp>

#include "app.hpp"
#include "config.hpp"

void print_config(const bot::Config& cfg) {
  fmt::println("Config values: ");
  pfr_extension::visit_object(cfg, []<typename Info>(const auto& value) {
    fmt::println("'{}'={}", Info::name.AsStringView(), value);
  });
}

int main(int argc, char* argv[]) {
  try {
    cli::options o = cli::parse_or_exit(argc, argv);
    auto cfg = bot::read_config_from_fs(o.config_path);
    if (o.print_config) {
      print_config(cfg);
      return 0;
    }

    bot::App app(std::move(cfg));
    app.run();
    return 0;
  } catch (std::exception& exc) {
    TGBM_LOG_CRIT("Start bot failed. Reason : {}", exc.what());
    return -1;
  }
}
