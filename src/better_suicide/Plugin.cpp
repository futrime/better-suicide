#include "Plugin.h"

#include <memory>
#include <string>
#include <utility>

#include <ll/api/Config.h>
#include <ll/api/base/KeyValueDB.h>
#include <ll/api/command/DynamicCommand.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>
#include <ll/api/event/command/SetupCommandEvent.h>
#include <ll/api/event/player/PlayerJoinEvent.h>
#include <ll/api/event/player/PlayerUseItemEvent.h>
#include <ll/api/form/ModalForm.h>
#include <ll/api/plugin/Plugin.h>
#include <mc/entity/utilities/ActorType.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/item/registry/ItemStack.h>

namespace better_suicide {

Plugin::Plugin(ll::plugin::Plugin& self) : mSelf(self) {
    auto& logger = mSelf.getLogger();

    // Load or initialize configurations.
    const auto& configFilePath = self.getConfigDir() / "config.json";
    if (!ll::config::loadConfig(mConfig, configFilePath)) {
        logger.warn("Cannot load configurations from {}", configFilePath);
        logger.info("Saving default configurations");

        if (!ll::config::saveConfig(mConfig, configFilePath)) {
            logger.error("Cannot save default configurations to {}", configFilePath);
        }
    }

    // Initialize databases;
    const auto& playerDbPath = self.getDataDir() / "players";
    mPlayerDb                = std::make_unique<ll::KeyValueDB>(playerDbPath);
}

bool Plugin::enable() {
    auto& logger = mSelf.getLogger();

    // Subscribe to events.
    auto& eventBus = ll::event::EventBus::getInstance();

    mPlayerJoinEventListener = eventBus.emplaceListener<ll::event::player::PlayerJoinEvent>(
        [doGiveClockOnFirstJoin = mConfig.doGiveClockOnFirstJoin,
         &logger,
         &playerDb = mPlayerDb](ll::event::player::PlayerJoinEvent& event) {
            if (doGiveClockOnFirstJoin) {
                auto& player = event.self();
                auto& uuid   = player.getUuid();

                // Check if the player has joined before.
                if (!playerDb->get(uuid.asString())) {

                    ItemStack itemStack("clock", 1);
                    player.add(itemStack);

                    // Must refresh inventory to see the clock.
                    player.refreshInventory();

                    // Mark the player as joined.
                    if (!playerDb->set(uuid.asString(), "true")) {
                        logger.error("Cannot mark {} as joined in database", player.getRealName());
                    }

                    logger.info("First join of {}! Giving them a clock", player.getRealName());
                }
            }
        }
    );

    mPlayerUseItemEventListener = eventBus.emplaceListener<ll::event::PlayerUseItemEvent>(
        [enableClockMenu = mConfig.enableClockMenu, &logger](ll::event::PlayerUseItemEvent& event) {
            if (enableClockMenu) {
                auto& player    = event.self();
                auto& itemStack = event.item();

                logger.info("{} used {}", player.getRealName(), itemStack.getRawNameId());

                if (itemStack.getRawNameId() == "clock") {
                    ll::form::ModalForm form(
                        "Warning",
                        "Are you sure you want to kill yourself?",
                        "No",
                        "Yes",
                        [&logger](Player& player, bool isCanceled) {
                            if (!isCanceled) {
                                player.kill();

                                logger.info("{} killed themselves", player.getRealName());
                            }
                        }
                    );

                    form.sendTo(player);

                    logger.info("{} opened better-suicide menu", player.getRealName());
                }
            }
        }
    );

    mSetupCommandEventListener =
        eventBus.emplaceListener<ll::event::command::SetupCommandEvent>([&logger](ll::event::command::SetupCommandEvent&) {
            // Setup suicide command.
            auto command = DynamicCommand::createCommand("suicide", "Commits suicide.", CommandPermissionLevel::Any);
            command->addOverload();
            command->setCallback([&logger](DynamicCommand const&,
                                    CommandOrigin const& origin,
                                    CommandOutput&       output,
                                    std::unordered_map<std::string, DynamicCommand::Result>&) {
                auto* entity = origin.getEntity();
                if (entity == nullptr || !entity->isType(ActorType::Player)) {
                    output.error("Only players can commit suicide");
                    return;
                }

                auto* player = static_cast<Player*>(entity);
                player->kill();

                logger.info("{} killed themselves", player->getRealName());
            });
            DynamicCommand::setup(std::move(command));
        });

    return true;
}

bool Plugin::disable() {
    // Unsubscribe from events.
    auto& eventBus = ll::event::EventBus::getInstance();
    eventBus.removeListener(mPlayerJoinEventListener);
    eventBus.removeListener(mPlayerUseItemEventListener);
    eventBus.removeListener(mSetupCommandEventListener);

    return true;
}

} // namespace better_suicide
