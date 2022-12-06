#include "PuppetDirector.h"

#include "actors/PuppetActor.h"
#include "al/LiveActor/LiveActorGroup.h"
#include "al/util.hpp"
#include "al/util/ControllerUtil.h"
#include "al/util/LiveActorUtil.h"
#include "game/GameData/GameDataFunction.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "game/Player/PlayerFunction.h"
#include "logger.hpp"
#include "math/seadVector.h"
#include "nerve/player/NrvPlayerActorHakoniwa.h"
#include "nerve/util/NerveFindUtil.h"
#include "rs/util.hpp"

PuppetDirector& getPuppetDirector()
{
    static PuppetDirector i;
    return i;
}

void PuppetDirector::initSequence()
{
    mPuppetFrames = (mPeakPuppetCount * 100) + mPuppetAdditionalMemory + 60 + 10;

    mFrames.allocBuffer(mPuppetFrames, nullptr);
    if (mFrames.size() > 0)
        mFrames.clear();

    gLogger->LOG("PuppetDirector InitSequence completed\n");
}

void PuppetDirector::initScene(const al::ActorInitInfo& initInfo, bool isMainStage, int worldID)
{
    gLogger->LOG("Initing PuppetDirector Scene Data\nIs Main Stage: %s\nWorld ID: %i\n", isMainStage ? "True" : "False", worldID);

    // Count setup
    if (isMainStage)
        mPuppetCount = mWorldPuppetCountList[worldID];
    else
        mPuppetCount = mSubAreaPuppetCount;
    mPuppetFrames = (mPuppetCount * mPuppetSeperator) + mPuppetAdditionalMemory + mPuppetStartOffset + 10;

    // Init actor group
    sead::Heap* seqHeap = sead::HeapMgr::instance()->findHeapByName("SceneHeap", 0);
    mActorGroup = new (seqHeap) al::LiveActorGroup("Puppets", mPuppetCount);

    for (int i = 0; i < mPuppetCount; i++) {
        PuppetActor* actor = new (seqHeap) PuppetActor("Actor");

        actor->init(initInfo);
        actor->setPuppetIndex(i);
        actor->makeActorAlive();
        al::setTrans(actor, { 0.f, -500000.f, 0.f });

        mActorGroup->registerActor(actor);

        gLogger->LOG("Init and Register Puppet Actor #%i\n", i);
    }
}

void PuppetDirector::initFirstStep(sead::Vector3f* spawnLocation)
{
    mPuppetQueueOffset = 0;
    mIsRunning = false;
    return;
}

void PuppetDirector::killScene()
{
    mFrames.clear();
    mPuppetQueueOffset = 0;
    mRequestDamage = false;
    mRequestKill = false;
}

void PuppetDirector::updatePuppets(StageScene* scene, PlayerActorHakoniwa* p1, bool isBind)
{
    // State check
    if (al::isPadTriggerAnyABXY(-1) || al::getLeftStick(-1)->x > 0.1f || al::getLeftStick(-1)->y > 0.1f)
        mIsRunning = true;

    if (!mIsRunning)
        return;

    calculateDamageData(scene, p1);
    calculateBindData(isBind);

    // Initalize frame
    PuppetFrame* newFrame = nullptr;

    sead::Heap* seqHeap = sead::HeapMgr::instance()->findHeapByName("SceneHeap", 0);
    newFrame = new (seqHeap) PuppetFrame();

    if (mPuppetFrames - mPuppetAdditionalMemory <= mFrames.size() && mPuppetQueueOffset == 0)
        delete mFrames.popFront();

    // Transform
    newFrame->mPosition = al::getTrans(p1);
    newFrame->mQuat = al::getQuat(p1);

    // Model state
    newFrame->mIs2D = p1->mDimKeeper->is2D;

    // Animation
    newFrame->mCurAnimStr.append(p1->mPlayerAnimator->curAnim.cstr());
    newFrame->mCurSubAnimStr.append(p1->mPlayerAnimator->curSubAnim.cstr());
    for (int i = 0; i < 6; i++)
        newFrame->mBlendWeights[i] = p1->mPlayerAnimator->getBlendWeight(i);

    mFrames.pushBack(newFrame);
}

void PuppetDirector::calculateDamageData(StageScene* scene, PlayerActorHakoniwa* p1)
{
    if (mRequestKill) {
        mRequestKill = false;
        GameDataFunction::killPlayer(scene->mHolder);
        NrvFind::setNerveAt(p1, nrvPlayerActorHakoniwaDead);
        rs::faceToCamera(p1);
        p1->mPlayerAnimator->endSubAnim();
        p1->mPlayerAnimator->startAnimDead();
    }

    if (mRequestDamage && mRequestDamageFrames == 0) {
        mRequestDamage = false;
        GameDataFunction::damagePlayer(scene->mHolder);
        NrvFind::setNerveAt(p1, nrvPlayerActorHakoniwaDamage);
        mRequestDamageFrames = 240;
    }

    if (mRequestDamageFrames > 0)
        mRequestDamageFrames--;
}

void PuppetDirector::calculateBindData(bool isBind)
{
    // Update bind additional memory data
    if (isBind && mPuppetQueueOffset < mPuppetAdditionalMemory)
        mPuppetQueueOffset++;
    else {
        if (mPuppetQueueOffset > 0) {
            mPuppetQueueOffset--;
            delete mFrames.popFront();
            delete mFrames.popFront();
        }
    }
}