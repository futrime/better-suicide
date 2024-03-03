#include "Entry.h"

#include "Config.h"

#include <fmt/format.h>
#include <functional>
#include <ll/api/Config.h>
#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/data/KeyValueDB.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>
#include <ll/api/event/player/PlayerJoinEvent.h>
#include <ll/api/event/player/PlayerUseItemEvent.h>
#include <ll/api/form/ModalForm.h>
#include <ll/api/io/FileUtils.h>
#include <ll/api/plugin/NativePlugin.h>
#include <ll/api/plugin/PluginManagerRegistry.h>
#include <ll/api/service/Bedrock.h>
#include <mc/entity/utilities/ActorType.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/item/registry/ItemStack.h>
#include <memory>
#include <stdexcept>
#include <utility>

namespace better_suicide {

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)

std::unique_ptr<std::reference_wrapper<ll::plugin::NativePlugin>> selfPluginInstance;

Config                                config;
std::unique_ptr<ll::data::KeyValueDB> playerDb;

// Event listeners
ll::event::ListenerPtr playerJoinEventListener;
ll::event::ListenerPtr playerUseItemEventListener;

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

} // namespace

auto disable(ll::plugin::NativePlugin& self) -> bool;
auto enable(ll::plugin::NativePlugin& self) -> bool;
auto load(ll::plugin::NativePlugin& self) -> bool;
auto unload(ll::plugin::NativePlugin& self) -> bool;

extern "C" {
_declspec(dllexport) auto ll_plugin_disable(ll::plugin::NativePlugin& self) -> bool { return disable(self); }
_declspec(dllexport) auto ll_plugin_enable(ll::plugin::NativePlugin& self) -> bool { return enable(self); }
_declspec(dllexport) auto ll_plugin_load(ll::plugin::NativePlugin& self) -> bool { return load(self); }
_declspec(dllexport) auto ll_plugin_unload(ll::plugin::NativePlugin& self) -> bool { return unload(self); }
}

auto disable(ll::plugin::NativePlugin& /*self*/) -> bool {
    auto& logger = getSelfPluginInstance().getLogger();

    logger.info("disabling...");

    // Unsubscribe from events.
    auto& eventBus = ll::event::EventBus::getInstance();
    eventBus.removeListener(playerJoinEventListener);
    eventBus.removeListener(playerUseItemEventListener);

    logger.info("disabled");

    return true;
}

auto enable(ll::plugin::NativePlugin& /*self*/) -> bool {
    auto& logger = getSelfPluginInstance().getLogger();

    logger.info("enabling...");

    // Subscribe to events.
    auto& eventBus = ll::event::EventBus::getInstance();

    playerJoinEventListener = eventBus.emplaceListener<ll::event::player::PlayerJoinEvent>(
        [doGiveClockOnFirstJoin = config.doGiveClockOnFirstJoin,
         &logger,
         &playerDb = playerDb](ll::event::player::PlayerJoinEvent& event) {
            if (doGiveClockOnFirstJoin) {
                auto& player = event.self();

                const auto& uuid = player.getUuid();

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

    playerUseItemEventListener =
        eventBus.emplaceListener<ll::event::PlayerUseItemEvent>([enableClockMenu = config.enableClockMenu,
                                                                 &logger](ll::event::PlayerUseItemEvent& event) {
            if (enableClockMenu) {
                auto& player    = event.self();
                auto& itemStack = event.item();

                if (itemStack.getRawNameId() == "clock") {
                    ll::form::ModalForm form("Warning", "Are you sure you want to kill yourself?", "Yes", "No");

                    form.sendTo(player, [&logger](Player& player, ll::form::ModalForm::SelectedButton button) {
                        if (button == ll::form::ModalForm::SelectedButton::Upper) {
                            player.kill();

                            logger.info("{} killed themselves", player.getRealName());
                        }
                    });
                }
            }
        });

    // Register commands.
    auto commandRegistry = ll::service::getCommandRegistry();
    if (!commandRegistry) {
        throw std::runtime_error("failed to get command registry");
    }

    auto& command = ll::command::CommandRegistrar::getInstance()
                        .getOrCreateCommand("suicide", "Commits suicide.", CommandPermissionLevel::Any);
    command.overload().execute<[](CommandOrigin const& origin, CommandOutput& output) {
        auto* entity = origin.getEntity();
        if (entity == nullptr || !entity->isType(ActorType::Player)) {
            output.error("Only players can commit suicide");
            return;
        }

        auto* player = static_cast<Player*>(entity); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
        player->kill();

        getSelfPluginInstance().getLogger().info("{} killed themselves", player->getRealName());
    }>();

    logger.info("enabled");

    return true;
}

auto getSelfPluginInstance() -> ll::plugin::NativePlugin& {
    if (!selfPluginInstance) {
        throw std::runtime_error("selfPluginInstance is null");
    }

    return *selfPluginInstance;
}

auto load(ll::plugin::NativePlugin& self) -> bool {
    auto& logger = self.getLogger();

    logger.info("loading...");

    selfPluginInstance = std::make_unique<std::reference_wrapper<ll::plugin::NativePlugin>>(self);

    // Load or initialize configurations.
    const auto& configFilePath = self.getConfigDir() / "config.json";
    if (!ll::config::loadConfig(config, configFilePath)) {
        logger.warn("Cannot load configurations from {}", configFilePath);
        logger.info("Saving default configurations");

        if (!ll::config::saveConfig(config, configFilePath)) {
            logger.error("Cannot save default configurations to {}", configFilePath);
        }
    }

    // Initialize databases;
    const auto& playerDbPath = self.getDataDir() / "players";
    playerDb                 = std::make_unique<ll::data::KeyValueDB>(playerDbPath);

    logger.info("loaded");

    return true;
}

auto unload(ll::plugin::NativePlugin& self) -> bool {
    auto& logger = self.getLogger();

    logger.info("unloading...");

    selfPluginInstance.reset();

    playerDb.reset();

    logger.info("unloaded");

    return true;
}

} // namespace better_suicide
