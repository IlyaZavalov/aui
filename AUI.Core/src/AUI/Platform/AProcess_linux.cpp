//
// Created by alex2 on 19.12.2020.
//
#ifdef __linux
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cassert>

void AProcess::execute(const AString& command, const AString& args, const APath& workingDirectory) {
    assert(0);
}

#endif