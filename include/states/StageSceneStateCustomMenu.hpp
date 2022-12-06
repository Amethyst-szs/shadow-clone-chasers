#pragma once

#include "al/layout/LayoutInitInfo.h"
#include "al/message/IUseMessageSystem.h"
#include "al/message/MessageSystem.h"
#include "al/nerve/HostStateBase.h"
#include "al/scene/Scene.h"
#include "al/util/NerveUtil.h"
#include "game/Input/InputSeparator.h"
#include "game/Layouts/CommonVerticalList.h"
#include "game/Layouts/SimpleLayoutMenu.h"
#include "game/StageScene/StageSceneStateOption.h"
#include "rs/util/InputUtil.h"

#include "al/util.hpp"

#include "game/GameData/GameDataHolder.h"

#include "logger.hpp"

class FooterParts;

class StageSceneStateCustomMenu : public al::HostStateBase<al::Scene>, public al::IUseMessageSystem {
public:
    StageSceneStateCustomMenu(const char*, al::Scene*, const al::LayoutInitInfo&, FooterParts*,
        GameDataHolder*, bool);

    static const int mCustomMenuSize = 5;
    enum CustomMenuOption {
        ENABLE,
        DISABLE,
        DAMAGESETTINGS,
        DISTANCESETTINGS,
        STARTOFFSETSETTINGS
    };

    static const int mDamageSetSize = 3;
    enum DamageSetOption {
        KILL,
        DAMAGE,
        PUSH
    };

    static const int mDistanceSetSize = 10;
    enum DistanceSetOption {
        TEN,
        TWENTY,
        THIRTY,
        FOURTY,
        FIFTY,
        SIXITYDEFAULT,
        SEVENTY,
        EIGHTY,
        NINTY,
        HUNDRED
    };

    static const int mStartOffsetSetSize = 4;
    enum StartOffsetSetOption {
        OFFSETZERO,
        OFFSETTWENTYDEFAULT,
        OFFSETFOURTY,
        OFFSETSIXTY
    };

    virtual al::MessageSystem* getMessageSystem(void) const override;
    virtual void init(void) override;
    virtual void appear(void) override;
    virtual void kill(void) override;

    void exeMainMenu();
    void exeDamageSet();
    void exeDistanceSet();
    void exeStartOffsetSet();

    void endSubMenu();

private:
    inline void subMenuStart();
    inline void subMenuUpdate();

    al::MessageSystem* mMsgSystem = nullptr;
    FooterParts* mFooterParts = nullptr;
    GameDataHolder* mGameDataHolder = nullptr;

    InputSeparator* mInput = nullptr;
    bool mDecide = false;
    int mHoldFrames = 0;

    SimpleLayoutMenu* mCurrentMenu = nullptr;
    CommonVerticalList* mCurrentList = nullptr;
    // Root Page
    SimpleLayoutMenu* mMainOptions = nullptr;
    CommonVerticalList* mMainOptionsList = nullptr;
    // Damage Settings
    SimpleLayoutMenu* mDamageSetMenu = nullptr;
    CommonVerticalList* mDamageSetMenuList = nullptr;
    // Distance Settings
    SimpleLayoutMenu* mDistanceSetMenu = nullptr;
    CommonVerticalList* mDistanceSetMenuList = nullptr;
    // Start Offset Settings
    SimpleLayoutMenu* mStartOffsetSetMenu = nullptr;
    CommonVerticalList* mStartOffsetSetMenuList = nullptr;

    static const char16_t* mainMenuItems[];
};

namespace {
NERVE_HEADER(StageSceneStateCustomMenu, MainMenu)
NERVE_HEADER(StageSceneStateCustomMenu, DamageSet)
NERVE_HEADER(StageSceneStateCustomMenu, DistanceSet)
NERVE_HEADER(StageSceneStateCustomMenu, StartOffsetSet)
}