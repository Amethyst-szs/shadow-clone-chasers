#include "PuppetDirector.h"
#include "game/SaveData/SaveDataAccessFunction.h"
#include "states/StageSceneStateCustomMenu.hpp"

StageSceneStateCustomMenu::StageSceneStateCustomMenu(const char* name, al::Scene* scene, const al::LayoutInitInfo& initInfo,
    FooterParts* footerParts, GameDataHolder* dataHolder, bool)
    : al::HostStateBase<al::Scene>(name, scene)
{

    mFooterParts = footerParts;
    mGameDataHolder = dataHolder;
    mMsgSystem = initInfo.getMessageSystem();
    mInput = new InputSeparator(mHost, true);

    // Root Menu
    mMainOptions = new SimpleLayoutMenu("CustomMenu", "OptionSelect", initInfo, 0, false);
    mMainOptionsList = new CommonVerticalList(mMainOptions, initInfo, true);

    al::setPaneString(mMainOptions, "TxtOption", u"Shadow Chaser Settings", 0);

    mMainOptionsList->initDataNoResetSelected(mCustomMenuSize);

    sead::SafeArray<sead::WFixedSafeString<0x200>, mCustomMenuSize>* mainMenuOptions
        = new sead::SafeArray<sead::WFixedSafeString<0x200>, mCustomMenuSize>();

    mainMenuOptions->mBuffer[CustomMenuOption::ENABLE].copy(u"Enable Shadow Chasers");
    mainMenuOptions->mBuffer[CustomMenuOption::DISABLE].copy(u"Disable Shadow Chasers");
    mainMenuOptions->mBuffer[CustomMenuOption::DAMAGESETTINGS].copy(u"Configure Damage Settings");
    mainMenuOptions->mBuffer[CustomMenuOption::DISTANCESETTINGS].copy(u"Configure Seperation Settings");
    mainMenuOptions->mBuffer[CustomMenuOption::STARTOFFSETSETTINGS].copy(u"Configure Offset Settings");

    mMainOptionsList->addStringData(mainMenuOptions->mBuffer, "TxtContent");

    // Damage Settings Menu

    mDamageSetMenu = new SimpleLayoutMenu("GamemodeSelectMenu", "OptionSelect", initInfo, 0, false);
    mDamageSetMenuList = new CommonVerticalList(mDamageSetMenu, initInfo, true);

    al::setPaneString(mDamageSetMenu, "TxtOption", u"What should happen on collision?", 0);

    mDamageSetMenuList->initDataNoResetSelected(mDamageSetSize);

    sead::SafeArray<sead::WFixedSafeString<0x200>, mDamageSetSize>* mDamageSetOptions
        = new sead::SafeArray<sead::WFixedSafeString<0x200>, mDamageSetSize>();

    mDamageSetOptions->mBuffer[DamageSetOption::KILL].copy(u"Instant Kill");
    mDamageSetOptions->mBuffer[DamageSetOption::DAMAGE].copy(u"Damage (Default)");
    mDamageSetOptions->mBuffer[DamageSetOption::PUSH].copy(u"Push / No Damage");

    mDamageSetMenuList->addStringData(mDamageSetOptions->mBuffer, "TxtContent");

    // Distance Settings Menu

    mDistanceSetMenu = new SimpleLayoutMenu("GamemodeSelectMenu", "OptionSelect", initInfo, 0, false);
    mDistanceSetMenuList = new CommonVerticalList(mDistanceSetMenu, initInfo, true);

    al::setPaneString(mDistanceSetMenu, "TxtOption", u"Distance between each chaser in chain", 0);

    mDistanceSetMenuList->initDataNoResetSelected(mDistanceSetSize);

    sead::SafeArray<sead::WFixedSafeString<0x200>, mDistanceSetSize>* mDistanceSetOptions
        = new sead::SafeArray<sead::WFixedSafeString<0x200>, mDistanceSetSize>();

    mDistanceSetOptions->mBuffer[DistanceSetOption::TEN].copy(u"10 Frames");
    mDistanceSetOptions->mBuffer[DistanceSetOption::TWENTY].copy(u"20 Frames");
    mDistanceSetOptions->mBuffer[DistanceSetOption::THIRTY].copy(u"30 Frames");
    mDistanceSetOptions->mBuffer[DistanceSetOption::FOURTY].copy(u"40 Frames");
    mDistanceSetOptions->mBuffer[DistanceSetOption::FIFTY].copy(u"50 Frames");
    mDistanceSetOptions->mBuffer[DistanceSetOption::SIXITYDEFAULT].copy(u"60 Frames (Default)");
    mDistanceSetOptions->mBuffer[DistanceSetOption::SEVENTY].copy(u"70 Frames");
    mDistanceSetOptions->mBuffer[DistanceSetOption::EIGHTY].copy(u"80 Frames");
    mDistanceSetOptions->mBuffer[DistanceSetOption::NINTY].copy(u"90 Frames");
    mDistanceSetOptions->mBuffer[DistanceSetOption::HUNDRED].copy(u"100 Frames");

    mDistanceSetMenuList->addStringData(mDistanceSetOptions->mBuffer, "TxtContent");

    // Offset Start Settings Menu

    mStartOffsetSetMenu = new SimpleLayoutMenu("GamemodeSelectMenu", "OptionSelect", initInfo, 0, false);
    mStartOffsetSetMenuList = new CommonVerticalList(mStartOffsetSetMenu, initInfo, true);

    al::setPaneString(mStartOffsetSetMenu, "TxtOption", u"Extra distance between the first chaser", 0);

    mStartOffsetSetMenuList->initDataNoResetSelected(mStartOffsetSetSize);

    sead::SafeArray<sead::WFixedSafeString<0x200>, mStartOffsetSetSize>* mStartOffsetSetOptions
        = new sead::SafeArray<sead::WFixedSafeString<0x200>, mStartOffsetSetSize>();

    mStartOffsetSetOptions->mBuffer[StartOffsetSetOption::OFFSETZERO].copy(u"0 Frames");
    mStartOffsetSetOptions->mBuffer[StartOffsetSetOption::OFFSETTWENTYDEFAULT].copy(u"20 Frames (Default)");
    mStartOffsetSetOptions->mBuffer[StartOffsetSetOption::OFFSETFOURTY].copy(u"40 Frames");
    mStartOffsetSetOptions->mBuffer[StartOffsetSetOption::OFFSETSIXTY].copy(u"60 Frames");

    mStartOffsetSetMenuList->addStringData(mStartOffsetSetOptions->mBuffer, "TxtContent");

    mCurrentList = mMainOptionsList;
    mCurrentMenu = mMainOptions;
}

void StageSceneStateCustomMenu::init()
{
    initNerve(&nrvStageSceneStateCustomMenuMainMenu, 0);
}

void StageSceneStateCustomMenu::appear(void)
{
    mCurrentMenu->startAppear("Appear");
    al::NerveStateBase::appear();
}

void StageSceneStateCustomMenu::kill(void)
{
    mCurrentMenu->startEnd("End");
    al::NerveStateBase::kill();
}

al::MessageSystem* StageSceneStateCustomMenu::getMessageSystem(void) const
{
    return mMsgSystem;
}

void StageSceneStateCustomMenu::exeMainMenu()
{
    if (al::isFirstStep(this)) {
        mInput->reset();
        mCurrentList->activate();
        mCurrentList->appearCursor();
        mDecide = false;
    }

    mInput->update();
    mCurrentList->update();

    if (mCurrentList->mCurSelected == 0 && mInput->isTriggerUiUp())
        mCurrentList->jumpBottom();
    if (mCurrentList->mCurSelected == mCustomMenuSize - 1 && mInput->isTriggerUiDown())
        mCurrentList->jumpTop();

    if (mInput->isTriggerUiUp() || (mInput->isHoldUiUp() && mHoldFrames > 20))
        mCurrentList->up();

    if (mInput->isTriggerUiDown() || (mInput->isHoldUiDown() && mHoldFrames > 20))
        mCurrentList->down();

    if (mInput->isHoldUiDown() || mInput->isHoldUiUp())
        mHoldFrames++;
    else
        mHoldFrames = 0;

    if (rs::isTriggerUiCancel(mHost)) {
        kill();
    }

    if (rs::isTriggerUiDecide(mHost)) {
        al::startHitReaction(mCurrentMenu, "決定", 0);
        mCurrentList->endCursor();
        mCurrentList->decide();
        mDecide = true;
    }

    if (mDecide && mCurrentList->isDecideEnd()) {
        switch (mCurrentList->mCurSelected) {
        case CustomMenuOption::ENABLE: {
            getPuppetDirector().mIsEnabled = true;
            al::setNerve(this, &nrvStageSceneStateCustomMenuMainMenu);
            break;
        }
        case CustomMenuOption::DISABLE: {
            getPuppetDirector().mIsEnabled = false;
            al::setNerve(this, &nrvStageSceneStateCustomMenuMainMenu);
            break;
        }
        case CustomMenuOption::DAMAGESETTINGS: {
            al::setNerve(this, &nrvStageSceneStateCustomMenuDamageSet);
            break;
        }
        case CustomMenuOption::DISTANCESETTINGS: {
            al::setNerve(this, &nrvStageSceneStateCustomMenuDistanceSet);
            break;
        }
        case CustomMenuOption::STARTOFFSETSETTINGS: {
            al::setNerve(this, &nrvStageSceneStateCustomMenuStartOffsetSet);
            break;
        }
        default:
            kill();
            break;
        }
    }
}

void StageSceneStateCustomMenu::exeDamageSet()
{
    if (al::isFirstStep(this)) {
        mCurrentList = mDamageSetMenuList;
        mCurrentMenu = mDamageSetMenu;
        subMenuStart();
    }

    subMenuUpdate();

    if (mDecide && mCurrentList->isDecideEnd()) {
        PuppetDirector& dir = getPuppetDirector();

        switch (mCurrentList->mCurSelected) {
        case DamageSetOption::KILL: {
            dir.mIsInstantKill = true;
            dir.mIsPushPlayer = false;
            break;
        }
        case DamageSetOption::DAMAGE: {
            dir.mIsInstantKill = false;
            dir.mIsPushPlayer = false;
            break;
        }
        case DamageSetOption::PUSH: {
            dir.mIsPushPlayer = true;
            break;
        }
        default:
            break;
        }
        endSubMenu();
    }
}

void StageSceneStateCustomMenu::exeDistanceSet()
{
    if (al::isFirstStep(this)) {
        mCurrentList = mDistanceSetMenuList;
        mCurrentMenu = mDistanceSetMenu;
        subMenuStart();
    }

    subMenuUpdate();

    if (mDecide && mCurrentList->isDecideEnd()) {
        PuppetDirector& dir = getPuppetDirector();
        dir.mPuppetSeperator = (mCurrentList->mCurSelected + 1) * 10;
        endSubMenu();
    }
}

void StageSceneStateCustomMenu::exeStartOffsetSet()
{
    if (al::isFirstStep(this)) {
        mCurrentList = mStartOffsetSetMenuList;
        mCurrentMenu = mStartOffsetSetMenu;
        subMenuStart();
    }

    subMenuUpdate();

    if (mDecide && mCurrentList->isDecideEnd()) {
        PuppetDirector& dir = getPuppetDirector();
        dir.mPuppetStartOffset = mCurrentList->mCurSelected * 20;
        endSubMenu();
    }
}

void StageSceneStateCustomMenu::endSubMenu()
{
    mCurrentList->deactivate();
    mCurrentMenu->kill();

    mCurrentList = mMainOptionsList;
    mCurrentMenu = mMainOptions;

    mCurrentMenu->startAppear("Appear");

    al::startHitReaction(mCurrentMenu, "リセット", 0);
    al::setNerve(this, &nrvStageSceneStateCustomMenuMainMenu);
}

void StageSceneStateCustomMenu::subMenuStart()
{
    mCurrentList->deactivate();
    mCurrentMenu->kill();
    mInput->reset();
    mCurrentList->activate();
    mCurrentList->appearCursor();
    mCurrentMenu->startAppear("Appear");
    mDecide = false;
}

void StageSceneStateCustomMenu::subMenuUpdate()
{
    mInput->update();
    mCurrentList->update();

    if (mCurrentList->mCurSelected == 0 && mInput->isTriggerUiUp())
        mCurrentList->jumpBottom();
    if (mCurrentList->mCurSelected == mCurrentList->mListPartsNum - 1 && mInput->isTriggerUiDown())
        mCurrentList->jumpTop();

    if (mInput->isTriggerUiUp() || (mInput->isHoldUiUp() && mHoldFrames > 20))
        mCurrentList->up();

    if (mInput->isTriggerUiDown() || (mInput->isHoldUiDown() && mHoldFrames > 20))
        mCurrentList->down();

    if (mInput->isHoldUiDown() || mInput->isHoldUiUp())
        mHoldFrames++;
    else
        mHoldFrames = 0;

    if (rs::isTriggerUiCancel(mHost)) {
        endSubMenu();
    }

    if (rs::isTriggerUiDecide(mHost)) {
        al::startHitReaction(mCurrentMenu, "決定", 0);
        mCurrentList->endCursor();
        mCurrentList->decide();
        mDecide = true;
    }
}

namespace {
NERVE_IMPL(StageSceneStateCustomMenu, MainMenu)
NERVE_IMPL(StageSceneStateCustomMenu, DamageSet)
NERVE_IMPL(StageSceneStateCustomMenu, DistanceSet)
NERVE_IMPL(StageSceneStateCustomMenu, StartOffsetSet)
}