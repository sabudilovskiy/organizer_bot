#include "long_poll.hpp"

namespace bot {

dd::channel<tgbm::api::optional<tgbm::api::Update>> long_poll_loop(tgbm::api::telegram api,
                                                                   tgbm::api::get_updates_request request,
                                                                   tgbm::duration_t timeout) {
  for (;;) {
    tgbm::api::arrayof<tgbm::api::Update> updates =
        co_await api.getUpdates(request, tgbm::deadline_after(timeout));
    for (tgbm::api::Update& item : updates) {
      if (item.update_id >= request.offset)
        request.offset = item.update_id + 1;
      co_yield std::move(item);
    }
    co_yield std::nullopt;
  }
}

dd::channel<tgbm::api::optional<tgbm::api::Update>> long_poll(tgbm::api::telegram api,
                                                              tgbm::long_poll_options options) {
  // validate

  if (options.timeout.count() > 40)
    options.timeout = std::chrono::seconds(40);

  for (std::string_view str : options.allowed_updates)
    if (!tgbm::api::allowed_updates::is_valid_update_category(str))
      throw tgbm::bad_request(fmt::format("\"{}\" is not valid update name", str));

  // fill first request

  tgbm::api::get_updates_request req;
  using namespace std::chrono;
  // telegram automatically answeres 0 updates after 50 seconds
  seconds timeout(50);
  req.timeout = options.timeout.count();
  // req.limit == 100 by default
  if (!options.allowed_updates.empty())
    req.allowed_updates = options.allowed_updates;

  // if webhook exist, long poll is not available

  (void)co_await api.deleteWebhook({.drop_pending_updates = options.drop_pending_updates},
                                   tgbm::deadline_after(5s));

  // send first request with 'allowed_updates' (telegram will remember it)

  auto upts = co_await api.getUpdates(req, tgbm::deadline_after(timeout));
  req.allowed_updates = std::nullopt;
  req.offset = 0;

  for (tgbm::api::Update& u : upts) {
    if (req.offset->value <= u.update_id)
      req.offset = u.update_id + 1;
    co_yield std::move(u);
  }
  co_yield dd::elements_of(long_poll_loop(std::move(api), std::move(req), timeout));
}
}  // namespace bot
