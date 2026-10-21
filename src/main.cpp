#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

namespace {
    struct PlayerFrame {
        PlayerObject* player = nullptr;
        CCPoint position = CCPointZero;
        float rotation = 0.f;
        double yVelocity = 0.0;
        bool onGround = false;
        bool valid = false;

        void capture(PlayerObject* object) {
            player = object;
            valid = object != nullptr;
            if (!valid) return;

            position = object->getPosition();
            rotation = object->getRotation();
            yVelocity = object->m_yVelocity;
            onGround = object->m_isOnGround;
        }

        bool restore(PlayerObject* object) const {
            if (!valid || object == nullptr || object != player) return false;

            // Restore the state that matters most for a one-step collision rollback.
            object->setPosition(position);
            object->setRotation(rotation);
            object->m_yVelocity = yVelocity;
            object->m_isOnGround = onGround;
            object->m_isDead = false;
            return true;
        }
    };

    struct RollbackFrame {
        GJBaseGameLayer* layer = nullptr;
        PlayerFrame player1;
        PlayerFrame player2;
        bool valid = false;

        void capture(GJBaseGameLayer* gameLayer) {
            layer = gameLayer;
            player1.capture(gameLayer->m_player1);
            player2.capture(gameLayer->m_player2);
            valid = true;
        }

        bool restore(GJBaseGameLayer* gameLayer, PlayerObject* player) const {
            if (!valid || layer != gameLayer) return false;
            return player1.restore(player) || player2.restore(player);
        }
    };

    // GD runs gameplay and collision handling on the main thread. There is one active
    // PlayLayer at a time, so a single snapshot is enough and avoids retaining old layers.
    RollbackFrame s_previousFrame;
    bool s_rewoundThisUpdate = false;

    bool isEnabled() {
        return Mod::get()->getSettingValue<bool>("enabled");
    }
}

class $modify(FrameBackGameLayer, GJBaseGameLayer) {
    void update(float dt) {
        auto* playLayer = PlayLayer::get();
        const bool inPlayLayer = playLayer == this;

        s_rewoundThisUpdate = false;
        if (inPlayLayer && isEnabled()) {
            // Save the state from immediately before the game's next simulation step.
            s_previousFrame.capture(this);
        } else {
            s_previousFrame.valid = false;
        }

        GJBaseGameLayer::update(dt);

        // Some game updates continue running after destroyPlayer returns. Re-apply the
        // saved state at the end of that update so later movement in the same step cannot
        // undo the rollback.
        if (inPlayLayer && s_rewoundThisUpdate) {
            s_previousFrame.restore(this, m_player1);
            s_previousFrame.restore(this, m_player2);
        }
    }
};

// PlayLayer overrides GJBaseGameLayer::destroyPlayer, so hook the concrete override
// that the live game's virtual dispatch reaches.
class $modify(FrameBackPlayLayer, PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (isEnabled() && s_previousFrame.restore(this, player)) {
            s_rewoundThisUpdate = true;
            log::info("FrameBack restored the player to the previous update.");
            return;
        }

        PlayLayer::destroyPlayer(player, object);
    }
};
