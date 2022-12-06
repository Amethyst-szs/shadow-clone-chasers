#include "main.hpp"
#include "PuppetDirector.h"
#include "al/LiveActor/LiveActor.h"
#include "al/area/AreaObj.h"
#include "al/area/AreaObjGroup.h"
#include "al/camera/CameraDirector.h"
#include "al/camera/CameraPoser.h"
#include "al/nerve/Nerve.h"
#include "al/util.hpp"
#include "al/util/ControllerUtil.h"
#include "al/util/LiveActorUtil.h"
#include "al/util/NerveUtil.h"
#include "al/util/StringUtil.h"
#include "areas/AreaTester.h"
#include "cameras/CameraPoserCustom.h"
#include "debugMenu.hpp"
#include "game/GameData/GameDataFunction.h"
#include "layouts/LayoutHolder.h"
#include "logger.hpp"
#include "states/StageSceneStatePauseMenu.h"

#include "actors/PuppetActor.h"

#include "nerve/player/NrvHackCap.h"
#include "nerve/player/NrvPlayerActorHakoniwa.h"
#include "nerve/util/NerveFindUtil.h"

static bool showMenu = false;
int listCount = 0;

// ------------- Hooks -------------

ulong threadInit()
{ // hook for initializing any threads we need
    __asm("STR X21, [X19,#0x208]");

    return 0x20;
}

al::SequenceInitInfo* initInfo;
ulong constructHook()
{ // hook for constructing anything we need to globally be accesible

    __asm("STR X21, [X19,#0x208]"); // stores WorldResourceLoader into HakoniwaSequence

    __asm("MOV %[result], X20"
          : [result] "=r"(
              initInfo)); // Save our scenes init info to a gloabl ptr so we can access it later

    return 0x20;
}

void sceneInitHook(al::ActorInitInfo* info, StageScene* curScene, al::PlacementInfo const* placement, al::LayoutInitInfo const* lytInfo,
    al::ActorFactory const* factory, al::SceneMsgCtrl* sceneMsgCtrl, al::GameDataHolderBase* dataHolder)
{
    al::initActorInitInfo(info, curScene, placement, lytInfo, factory, sceneMsgCtrl, dataHolder);

    PuppetDirector& dir = getPuppetDirector();
    dir.initScene(*info, curScene->mHolder.mData->mWorldList->checkIsMainStage(GameDataFunction::getCurrentStageName(curScene)), GameDataFunction::getCurrentWorldId(curScene));
}

void initActorWithArchiveNameHook(al::LiveActor* actor, al::ActorInitInfo const& initInfo, sead::SafeStringBase<char> const& string, char const* anotherString)
{
    // newActor = al::createActorFunction<Gunetter>("Moonsnake"); // Moonsnake
    // al::initCreateActorNoPlacementInfo(newActor, initInfo);
    // newActor->makeActorAlive();

    // Complete hooked functionn
    al::initActorWithArchiveName(actor, initInfo, string, anotherString);
}

bool sceneKillHook(GameDataHolderAccessor value)
{
    PuppetDirector& dir = getPuppetDirector();
    dir.killScene();

    return GameDataFunction::isMissEndPrevStageForSceneDead(value);
}

bool hakoniwaSequenceHook(HakoniwaSequence* sequence)
{
    PuppetDirector& dir = getPuppetDirector();

    bool isFirstStep = al::isFirstStep(sequence);
    StageScene* stageScene = (StageScene*)sequence->getCurrentScene();

    al::PlayerHolder* pHolder = al::getScenePlayerHolder(stageScene);
    PlayerActorHakoniwa* p1 = (PlayerActorHakoniwa*)al::tryGetPlayerActor(pHolder, 0);

    bool isPause = stageScene->isPause();
    bool isDemo = rs::isActiveDemo(p1);
    bool isDead = PlayerFunction::isPlayerDeadStatus(p1);
    bool isInterupted = isDead || isDemo || isPause;

    if (al::isPadHoldZR(-1) && al::isPadTriggerUp(-1))
        showMenu = !showMenu;
    setDebugMenuState(true, showMenu);

    if (isFirstStep)
        dir.initFirstStep(al::getTransPtr(p1));

    if (!isInterupted && dir.mIsEnabled)
        dir.updatePuppets(stageScene, p1, NrvFind::isNerveAt(p1, nrvPlayerActorHakoniwaBind));

    // Check custom areas
    ca::customAreaUpdate(p1);

    return isFirstStep;
}

void seadPrintHook(const char* fmt, ...) // hook for replacing sead::system::print with our custom logger
{
    va_list args;
    va_start(args, fmt);

    gLogger->LOG(fmt, args);

    va_end(args);
}