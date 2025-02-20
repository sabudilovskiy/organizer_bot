#include <tgbm/bot.hpp>
#include <tgbm/api/types/Update.hpp>
#include <tgbm/net/errors.hpp>

namespace bot {

dd::channel<tgbm::api::optional<tgbm::api::Update>> long_poll_loop(tgbm::api::telegram api,
                                                                   tgbm::api::get_updates_request request,
                                                                   tgbm::duration_t timeout);

dd::channel<tgbm::api::optional<tgbm::api::Update>> long_poll(tgbm::api::telegram api,
                                                              tgbm::long_poll_options options = {});

}  // namespace bot
