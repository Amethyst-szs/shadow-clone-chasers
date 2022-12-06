#include "PuppetDirector.h"
#include "al/byaml/ByamlIter.h"
#include "al/byaml/writer/ByamlWriter.h"
#include "al/util.hpp"

#include "saveInject.hpp"
#include "types.h"

void saveWriteHook(al::ByamlWriter* saveByml)
{
    PuppetDirector& dir = getPuppetDirector();

    saveByml->addInt("PupSeperator", dir.mPuppetSeperator);
    saveByml->addInt("PupOffset", dir.mPuppetStartOffset);
    saveByml->addBool("PupKill", dir.mIsInstantKill);
    saveByml->addBool("PupPush", dir.mIsPushPlayer);

    saveByml->pop();
}

bool saveReadHook(int* padRumbleInt, al::ByamlIter const& saveByml, char const* padRumbleKey)
{
    int PupSeperator;
    int PupOffset;
    bool PupKill;
    bool PupPush;

    PuppetDirector& dir = getPuppetDirector();

    if (al::tryGetByamlS32(&PupSeperator, saveByml, "PupSeperator"))
        dir.mPuppetSeperator = PupSeperator;
    if (al::tryGetByamlS32(&PupOffset, saveByml, "PupOffset"))
        dir.mPuppetStartOffset = PupOffset;
    if (al::tryGetByamlBool(&PupKill, saveByml, "PupKill"))
        dir.mIsInstantKill = PupKill;
    if (al::tryGetByamlBool(&PupPush, saveByml, "PupPush"))
        dir.mIsPushPlayer = PupPush;

    return al::tryGetByamlS32(padRumbleInt, saveByml, padRumbleKey);
}