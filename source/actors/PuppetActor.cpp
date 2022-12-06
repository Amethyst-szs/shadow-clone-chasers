#include "actors/PuppetActor.h"
#include "PuppetDirector.h"
#include "al/LiveActor/LiveActor.h"
#include "al/layout/BalloonMessage.h"
#include "al/layout/LayoutInitInfo.h"
#include "al/model/ModelMaterialCategory.h"
#include "al/model/PartsModel.h"
#include "al/string/StringTmp.h"
#include "al/util.hpp"
#include "al/util/LiveActorUtil.h"
#include "al/util/ResourceUtil.h"
#include "al/util/SensorUtil.h"
#include "game/GameData/GameDataFunction.h"
#include "game/Player/PlayerCostumeFunction.h"
#include "game/Player/PlayerCostumeInfo.h"
#include "logger.hpp"
#include "math/seadQuat.h"
#include "math/seadVector.h"
#include "rs/util/SensorUtil.h"
#include <cmath>
#include <cstddef>
#include <typeinfo>

static const char* subActorNames[] = {
    "顔", // Face
    "目", // Eye
    "頭", // Head
    "左手", // Left Hand
    "右手" // Right Hand
};

PuppetActor::PuppetActor(const char* name)
    : al::LiveActor(name)
{
    mModelHolder = new PlayerModelHolder(3); // Regular Model, 2D Model, 2D Mini Model
}

void PuppetActor::init(al::ActorInitInfo const& initInfo)
{
    al::initActorWithArchiveName(this, initInfo, "PuppetActor", nullptr);

    const char* bodyName = "MarioShadowChase";
    const char* capName = "MarioShadowChase";

    al::LiveActor* normalModel = new al::LiveActor("Normal");

    mCostumeInfo = initMarioModelPuppet(normalModel, initInfo, bodyName, capName, 0, nullptr);

    normalModel->mActorActionKeeper->mPadAndCamCtrl->mRumbleCount = 0; // set rumble count to zero so that no rumble actions will run

    mModelHolder->registerModel(normalModel, "Normal");

    al::LiveActor* normal2DModel = new al::LiveActor("Normal2D");

    PlayerFunction::initMarioModelActor2D(normal2DModel, initInfo, al::StringTmp<0x40>("%s2D", mCostumeInfo->mBodyInfo->costumeName).cstr(), PlayerFunction::isInvisibleCap(mCostumeInfo));

    mModelHolder->registerModel(normal2DModel, "Normal2D");

    al::setClippingInfo(normalModel, 50000.0f, 0);
    al::setClippingNearDistance(normalModel, 50000.0f);

    al::hideSilhouetteModelIfShow(normalModel);

    // "頭" = Head
    // "髪" = Hair
    // "キャップの目" = Cap Eyes
    // "[デモ用]キャップの目" = [Demo] Cap Eyes

    al::LiveActor* headModel = al::getSubActor(normalModel, "頭");
    al::getSubActor(headModel, "キャップの目")->kill();
    al::startVisAnimForAction(headModel, "CapOn");

    mModelHolder->changeModel("Normal");

    startAction("Wait");

    al::validateClipping(normalModel);
    al::validateClipping(normal2DModel);
}

void PuppetActor::initAfterPlacement() { al::LiveActor::initAfterPlacement(); }

void PuppetActor::setPuppetIndex(int index)
{
    PuppetDirector& dir = getPuppetDirector();
    mDirectorIndex = index;
    mDirectorPositionInQueue = (index + 1) * dir.mPuppetSeperator;
}

void PuppetActor::movement()
{
    al::LiveActor::movement();
}

void PuppetActor::calcAnim()
{
    al::LiveActor::calcAnim();
}

void PuppetActor::control()
{
    al::LiveActor* curModel = getCurrentModel();
    PuppetDirector& dir = getPuppetDirector();

    // Validity check
    if (mDirectorIndex == -1)
        gLogger->LOG("Puppet with invalid index exists");

    // Check if actor is in the written section of the queue
    if (dir.mFrames.size() > mDirectorPositionInQueue + dir.mPuppetStartOffset + dir.mPuppetQueueOffset) {
        PuppetFrame* frame = dir.mFrames.at(((dir.mFrames.size() - mDirectorPositionInQueue) - dir.mPuppetQueueOffset) - dir.mPuppetStartOffset);

        if (!frame) {
            gLogger->LOG("Puppet control update taking place out of bounds");
            return;
        }

        // Update this actor with data from director
        al::setTrans(this, frame->mPosition);
        al::setQuat(this, frame->mQuat);

        // Update model
        if (mIs2DModel != frame->mIs2D) {
            mIs2DModel = frame->mIs2D;
            m3DFrames = 0;

            if (mIs2DModel)
                changeModel("Normal2D");
            else
                changeModel("Normal");
        }

        // Animation state
        if (!al::isActionPlaying(curModel, frame->mCurSubAnimStr.cstr())) {
            startAction(frame->mCurAnimStr.cstr());
        } else if (al::isActionEnd(curModel)) {
            startAction(frame->mCurAnimStr.cstr());
        }

        if (isNeedBlending()) {
            for (size_t i = 0; i < 6; i++)
                setBlendWeight(i, frame->mBlendWeights[i]);
        }

        // Lock hat on
        al::LiveActor* headModel = al::getSubActor(mModelHolder->findModelActor("Normal"), "頭");
        al::startVisAnimForAction(headModel, "CapOn");

        // Effect emmission
        if (!mIs2DModel)
            m3DFrames++;

        if (!mIs2DModel && m3DFrames > 10)
            al::tryEmitEffect(curModel, "WorldWarpHoleBind", al::getTransPtr(this));
    }

    syncPose();
}

void PuppetActor::makeActorAlive()
{

    al::LiveActor* curModel = getCurrentModel();

    if (al::isDead(curModel)) {
        curModel->makeActorAlive();
    }

    al::LiveActor::makeActorAlive();
}

void PuppetActor::makeActorDead()
{

    al::LiveActor* curModel = getCurrentModel();

    if (!al::isDead(curModel)) {
        curModel->makeActorDead();
    }

    al::LiveActor::makeActorDead();
}

void PuppetActor::attackSensor(al::HitSensor* source, al::HitSensor* target)
{
    PuppetDirector& dir = getPuppetDirector();
    auto* sourceHost = al::getSensorHost(source);
    auto* targetHost = al::getSensorHost(target);

    if (al::isEqualString(typeid(*targetHost).name(), "19PlayerActorHakoniwa") && !dir.mIsPushPlayer) {
        if (dir.mIsInstantKill)
            dir.mRequestKill = true;
        else if (dir.mRequestDamageFrames == 0)
            dir.mRequestDamage = true;
    } else if (!dir.mIsPushPlayer) {
        al::sendMsgExplosion(target, source, nullptr);
        al::sendMsgExplosionCollide(target, source, nullptr);
        al::sendMsgKickStoneAttackCollide(target, source);
        al::sendMsgPlayerBoomerangAttackCollide(target, source);
        al::sendMsgPlayerKick(target, source);
        al::sendMsgPlayerInvincibleAttack(target, source, nullptr);
        rs::sendMsgKoopaCapPunchL(target, source);
        rs::sendMsgTankBullet(target, source);
    }

    if (!al::sendMsgPush(target, source) && dir.mIsPushPlayer) {
        rs::sendMsgPushToPlayer(target, source);
    }
}

bool PuppetActor::receiveMsg(const al::SensorMsg* msg, al::HitSensor* source,
    al::HitSensor* target)
{

    if ((al::isMsgPlayerTrampleReflect(msg) || rs::isMsgPlayerAndCapObjHipDropReflectAll(msg)) && al::isSensorName(target, "Body")) {
        rs::requestHitReactionToAttacker(msg, target, source);
        return true;
    }

    return false;
}

// this is more or less how nintendo does it with marios demo puppet
void PuppetActor::startAction(const char* actName)
{

    al::LiveActor* curModel = getCurrentModel();

    if (!actName)
        return;

    if (al::tryStartActionIfNotPlaying(curModel, actName)) {
        const char* curActName = al::getActionName(curModel);
        if (curActName) {
            if (al::isSklAnimExist(curModel, curActName)) {
                al::clearSklAnimInterpole(curModel);
            }
        }
    }

    for (size_t i = 0; i < 5; i++) {
        al::LiveActor* subActor = al::getSubActor(curModel, subActorNames[i]);
        const char* curActName = al::getActionName(curModel);
        if (subActor && curActName) {
            if (al::tryStartActionIfNotPlaying(subActor, curActName)) {
                if (al::isSklAnimExist(curModel, curActName)) {
                    al::clearSklAnimInterpole(curModel);
                }
            }
        }
    }

    al::LiveActor* faceActor = al::tryGetSubActor(curModel, "顔");

    if (faceActor) {
        al::StringTmp<0x80> faceAnim("%sFullFace", actName);
        if (al::tryStartActionIfNotPlaying(faceActor, faceAnim.cstr())) {
            if (al::isSklAnimExist(faceActor, faceAnim.cstr())) {
                al::clearSklAnimInterpole(faceActor);
            }
        }
    }
}

void PuppetActor::hairControl()
{

    al::LiveActor* curModel = getCurrentModel();

    if (mCostumeInfo->isNeedSyncBodyHair()) {
        PlayerFunction::syncBodyHairVisibility(al::getSubActor(curModel, "髪"), al::getSubActor(curModel, "頭"));
    }
    if (mCostumeInfo->isSyncFaceBeard()) {
        PlayerFunction::syncMarioFaceBeardVisibility(al::getSubActor(curModel, "顔"), al::getSubActor(curModel, "頭"));
    }
    if (mCostumeInfo->isSyncStrap()) {
        PlayerFunction::syncMarioHeadStrapVisibility(al::getSubActor(curModel, "頭"));
    }
    if (PlayerFunction::isNeedHairControl(mCostumeInfo->mBodyInfo, mCostumeInfo->mHeadInfo->costumeName)) {
        PlayerFunction::hideHairVisibility(al::getSubActor(curModel, "頭"));
    }
}

bool PuppetActor::isNeedBlending()
{
    const char* curActName = al::getActionName(getCurrentModel());
    if (curActName) {
        return al::isEqualSubString(curActName, "Move") || al::isEqualSubString(curActName, "Sand") || al::isEqualSubString(curActName, "MotorcycleRide");
    } else {
        return false;
    }
}

al::LiveActor* PuppetActor::getCurrentModel()
{
    return mModelHolder->currentModel->mLiveActor;
}

void PuppetActor::changeModel(const char* newModel)
{
    getCurrentModel()->makeActorDead();
    mModelHolder->changeModel(newModel);
    getCurrentModel()->makeActorAlive();
}

void PuppetActor::syncPose()
{

    al::LiveActor* curModel = getCurrentModel();

    curModel->mPoseKeeper->updatePoseQuat(al::getQuat(this)); // update pose using a quaternion instead of setting quaternion rotation

    al::setTrans(curModel, al::getTrans(this));
}

void PuppetActor::emitJoinEffect()
{

    al::tryDeleteEffect(this, "Disappear"); // remove previous effect (if played previously)

    al::tryEmitEffect(this, "Disappear", nullptr);
}

const char* executorName = "ＮＰＣ";

PlayerCostumeInfo* initMarioModelPuppet(al::LiveActor* player,
    const al::ActorInitInfo& initInfo,
    const char* bodyName, const char* capName,
    int subActorNum,
    al::AudioKeeper* audioKeeper)
{
    al::ActorResource* modelRes = al::findOrCreateActorResourceWithAnimResource(
        initInfo.mResourceHolder, al::StringTmp<0x100>("ObjectData/%s", bodyName).cstr(),
        al::StringTmp<0x100>("ObjectData/%s", "PlayerAnimation").cstr(), 0, false);

    PlayerBodyCostumeInfo* bodyInfo = PlayerCostumeFunction::createBodyCostumeInfo(modelRes->mResourceModel, bodyName);

    al::initActorSceneInfo(player, initInfo);
    al::initActorPoseTQGSV(player);
    al::initActorSRT(player, initInfo);

    al::initActorModelKeeper(player, initInfo,
        al::StringTmp<0x100>("ObjectData/%s", bodyName).cstr(), 6,
        al::StringTmp<0x100>("ObjectData/%s", "PlayerAnimation").cstr());

    al::ModelMaterialCategory::tryCreate(
        player->mModelKeeper->mModelCtrl, "Player",
        initInfo.mActorSceneInfo.mGfxSysInfo->mMaterialCategoryKeeper);

    al::initPartialSklAnim(player, 1, 1, 32);
    al::addPartialSklAnimPartsListRecursive(player, "Spine1", 0);

    al::initExecutorUpdate(player, initInfo, executorName);
    al::initExecutorDraw(player, initInfo, executorName);
    al::initExecutorModelUpdate(player, initInfo);

    al::ByamlIter iter;
    if (al::tryGetActorInitFileIter(&iter, modelRes->mResourceModel, "InitEffect", 0)) {
        const char* effectKeeperName;
        if (iter.tryGetStringByKey(&effectKeeperName, "Name")) {
            al::initActorEffectKeeper(player, initInfo, effectKeeperName);
        }
    }

    PlayerFunction::initMarioAudio(player, initInfo, modelRes->mResourceModel, false, audioKeeper);
    al::initActorActionKeeper(player, modelRes, bodyName, 0);
    al::setMaterialProgrammable(player);

    al::SubActorKeeper* actorKeeper = al::SubActorKeeper::tryCreate(player, 0, subActorNum);

    if (actorKeeper) {
        player->initSubActorKeeper(actorKeeper);
    }

    actorKeeper->init(initInfo, 0, subActorNum);

    int subModelNum = al::getSubActorNum(player);

    if (subModelNum >= 1) {
        for (int i = 0; i < subModelNum; i++) {
            al::LiveActor* subActor = al::getSubActor(player, i);
            const char* actorName = subActor->getName();

            if (!al::isEqualString(actorName, "シルエットモデル")) {
                al::initExecutorUpdate(subActor, initInfo, executorName);
                al::initExecutorDraw(subActor, initInfo, executorName);
                al::setMaterialProgrammable(subActor);
            }
        }
    }

    al::initActorClipping(player, initInfo);
    al::invalidateClipping(player);

    const char* capModelName;

    if (bodyInfo->mIsUseHeadSuffix) {
        if (al::isEqualString(bodyInfo->costumeName, capName)) {
            capModelName = "";
        } else {
            capModelName = capName;
        }
    } else {
        capModelName = "";
    }

    const char* headType;

    if (!al::isEqualSubString(capName, "Mario64")) {
        if (bodyInfo->mIsUseShortHead && al::isEqualString(capName, "MarioPeach")) {
            headType = "Short";
        } else {
            headType = "";
        }
    } else if (al::isEqualString(bodyInfo->costumeName, "Mario64")) {
        headType = "";
    } else if (al::isEqualString(bodyInfo->costumeName, "Mario64Metal")) {
        headType = "Metal";
    } else {
        headType = "Other";
    }

    PlayerHeadCostumeInfo* headInfo = initMarioHeadCostumeInfo(
        player, initInfo, "頭", capName, headType, capModelName);

    PlayerCostumeInfo* costumeInfo = new PlayerCostumeInfo();
    costumeInfo->init(bodyInfo, headInfo);

    if (costumeInfo->isNeedBodyHair()) {

        // Logger::log("Creating Body Hair Parts Model.\n");

        al::PartsModel* partsModel = new al::PartsModel("髪");

        partsModel->initPartsFixFile(
            player, initInfo,
            al::StringTmp<0x100>("%sHair%s", bodyName,
                costumeInfo->isEnableHairNoCap() ? "NoCap" : "")
                .cstr(),
            0, "Hair");

        al::initExecutorUpdate(partsModel, initInfo, executorName);
        al::initExecutorDraw(partsModel, initInfo, executorName);
        al::setMaterialProgrammable(partsModel);
        partsModel->makeActorDead();
        al::onSyncAppearSubActor(player, partsModel);
        al::onSyncClippingSubActor(player, partsModel);
        al::onSyncAlphaMaskSubActor(player, partsModel);
        al::onSyncHideSubActor(player, partsModel);
    }

    PlayerFunction::initMarioDepthModel(player, false, false);

    rs::createPlayerSklRetargettingInfo(player, sead::Vector3f::ones);

    player->makeActorDead();
    return costumeInfo;
}

PlayerHeadCostumeInfo* initMarioHeadCostumeInfo(al::LiveActor* player,
    const al::ActorInitInfo& initInfo,
    const char* headModelName, const char* capModelName,
    const char* headType, const char* headSuffix)
{

    al::PartsModel* headModel = new al::PartsModel(headModelName);

    al::StringTmp<0x80> headArcName("%sHead%s", capModelName, headType);
    al::StringTmp<0x100> arcSuffix("Head");
    if (headSuffix)
        arcSuffix.format("Head%s", headSuffix);

    headModel->initPartsFixFile(player, initInfo, headArcName.cstr(), 0, arcSuffix.cstr());
    al::setMaterialProgrammable(headModel);

    al::initExecutorUpdate(headModel, initInfo, executorName);
    al::initExecutorDraw(headModel, initInfo, executorName);

    headModel->makeActorDead();

    al::onSyncAppearSubActor(player, headModel);
    al::onSyncClippingSubActor(player, headModel);
    al::onSyncAlphaMaskSubActor(player, headModel);
    al::onSyncHideSubActor(player, headModel);

    al::PartsModel* capEyesModel = new al::PartsModel("キャップの目");
    capEyesModel->initPartsFixFile(headModel, initInfo, "CapManHeroEyes", "", 0);

    al::onSyncClippingSubActor(headModel, capEyesModel);
    al::onSyncAlphaMaskSubActor(headModel, capEyesModel);
    al::onSyncHideSubActor(headModel, capEyesModel);

    al::setMaterialProgrammable(headModel);
    headModel->makeActorDead();

    return PlayerCostumeFunction::createHeadCostumeInfo(al::getModelResource(headModel), capModelName, false);
}