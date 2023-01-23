// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by alex2772 on 11/6/20.
//

#include "Pack.h"
#include <AUI/Crypt/AHash.h>
#include <AUI/IO/APath.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/Util/ATokenizer.h>
#include <AUI/Util/LZ.h>

void Pack::run(Toolbox& t) {
    if (t.args.size() != 3)
    {
        throw IllegalArgumentsException("invalid argument count");
    }
    else
    {
        if (t.args[0].length() >= t.args[1].length()) {
            throw IllegalArgumentsException("second argument must be longer than first");
        }

        APath entry(t.args[2]);
        AString assetPath = APath(t.args[1]).absolute();
        assetPath = assetPath.substr(APath(t.args[0]).absolute().length() + 1);
        assetPath = assetPath.replacedAll("\\", '/');
        doPacking(t.args[1], assetPath, entry);
    }
}

AString Pack::getName() {
    return "pack";
}

AString Pack::getSignature() {
    return "<base_dir> <file to pack> <resulting cpp>";
}

AString Pack::getDescription() {
    return "pack a file into the .cpp file";
}

void Pack::doPacking(const AString& inputFile, const AString& assetPath, const APath& outputCpp) {
    try {
        outputCpp.parent().makeDirs();
    } catch (...) {

    }

    try
    {

        AByteBuffer buffer;

        buffer << aui::serialize_sized(assetPath.toStdString());
        auto fis = _new<AFileInputStream>(inputFile);

        AByteBuffer data;

        data << fis;

        auto fileHash = AHash::sha512(data).toHexString();

        // we will try cpp file on this path. if it exists there's chance we don't have to rewrite the same file
        // contents.
        try {
            ATokenizer t(_new<AFileInputStream>(outputCpp));

            // skip the first line where is a warning about this file is autogenerated.
            t.readStringUntilUnescaped('\n');

            // try to read file name. if it does not match to the actual file name we will write a warning.
            AString desiredBeginning = "// file: ";
            AString actualBeginning = t.readString(desiredBeginning.length());
            if (actualBeginning == desiredBeginning) {
                // our client.
                auto targetFileName = AString(t.readStringUntilUnescaped('\n'));
                if (targetFileName != assetPath) {
                    std::cout << "command line input file path " << assetPath << " does not match with the "
                                                                                 "file path stored in " << outputCpp << "; please use another file name for cpp in "
                                                                                                                    "order to prent embedded file loss.";
                    throw AException{};
                }
            }

            // try to read hash from the row. it should start from comment.
            desiredBeginning = "// hash: ";
            actualBeginning = t.readString(desiredBeginning.length());
            if (actualBeginning == desiredBeginning) {
                // our client.
                auto targetFileHash = AString(t.readStringUntilUnescaped('\n'));
                if (targetFileHash == fileHash) {
                    std::cout << "skipped " << assetPath << std::endl;
                    return;
                }
            }
        } catch (...) {

        }

        buffer << aui::serialize_sized(data);

        AByteBuffer packed;
        LZ::compress(buffer, packed);

        auto cppObjectName = outputCpp.filenameWithoutExtension();

        AFileOutputStream out(outputCpp);
        out << "// This file is autogenerated by aui.toolbox. Please do not modify.\n"
                "// file: " << assetPath << "\n"
                                               "// hash: " << fileHash << "\n"
                                                                             "\n"
                                                                             "#include \"AUI/Common/AByteBuffer.h\"\n"
                                                                             "#include \"AUI/Util/ABuiltinFiles.h\"\n"
                                                                             "const static unsigned char AUI_PACKED_asset" << cppObjectName
             << "[] = \"";
        for (uint8_t c : packed) {
            char buf[32];
            sprintf(buf, "\\x%02x", c);
            out << std::string(buf);
        }
        out << "\";\n";


        out << "struct Assets" << cppObjectName << " {\n"
             << "    Assets" << cppObjectName << "(){\n"
                     "        ABuiltinFiles::load(AUI_PACKED_asset"
             << cppObjectName
             << ", sizeof(AUI_PACKED_asset"
             << cppObjectName << "));\n    }\n};\n"
                                 "Assets"
             << cppObjectName
             << " a" << cppObjectName << ";";

        std::cout << assetPath << " -> " << outputCpp.filename()
                  << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "Warning: could not pack file " << outputCpp << ": " << e.what() << std::endl;
    }
}
