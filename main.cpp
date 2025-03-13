#include <cli_interface.hpp>

#include "app.hpp"
#include "config.hpp"

void print_config(const bot::Config& cfg) {
  fmt::println("Config values: ");
  pfr_extension::visit_object(cfg, []<typename Info>(const auto& value) {
    fmt::println("'{}'={}", Info::name.AsStringView(), value);
  });
}

inline cli::options parse_or_exit(int argc, char* argv[]) {
  cli::error_code ec;
  cli::options o = parse(argc, argv, ec);
  if (ec) {
    if (ec.what == cli::errc::unknown_option) {
      ::std::cerr << "Unknown option get. List of options: \n";
      cli::print_help_message_to([](auto s) { std::cerr << s; });
    } else {
      ::std::cerr << "an error occurred: ";
      ec.print_to([](auto s) { std::cerr << s; });
    }
    ::std::flush(std::cerr);
    ::std::exit(1);
  }
  return o;
}

int main(int argc, char* argv[]) {
  try {
    cli::options o = parse_or_exit(argc, argv);
    if (o.generate_config) {
      bot::Config::write_default_to_fs(o.config_path);
      return 0;
    }

    auto cfg = bot::Config::read_from_fs(o.config_path);
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
