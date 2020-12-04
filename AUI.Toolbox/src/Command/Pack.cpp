//
// Created by alex2772 on 11/6/20.
//

#include "Pack.h"
#include <AUI/Crypt/AHash.h>
#include <AUI/IO/APath.h>
#include <AUI/IO/FileInputStream.h>
#include <AUI/IO/FileOutputStream.h>
#include <AUI/Util/ATokenizer.h>
#include <AUI/Util/LZ.h>

void Pack::run(Toolbox& t) {
    // TODO переделать компилятор ассетов так, чтобы он принимал не папку, а только один файл. Чтобы CMake знал, как будет в итоге называться cpp файл и смог корректно к нему прилинковаться
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
        try {
            entry.parent().makeDirs();
        } catch (...) {

        }
        try
        {

            AByteBuffer buffer;
            AString inputFilePath = APath(t.args[1]).absolute();
            inputFilePath = inputFilePath.mid(APath(t.args[0]).absolute().length() + 1);
            inputFilePath = inputFilePath.replacedAll("\\", '/');

            buffer << inputFilePath.toStdString();
            auto fis = _new<FileInputStream>(t.args[1]);

            AByteBuffer data;

            char buf[0x1000];
            for (int r; (r = fis->read(buf, sizeof(buf))) > 0;)
            {
                data.put(buf, r);
            }

            data.setCurrentPos(0);

            auto fileHash = AHash::sha512(data).toHexString();

            // попробуем открыть cpp файл по этому пути. если он существует, то есть шанс, что
            // нам не придётся перезаписывать одно и то же содержимое файла.
            try {
                ATokenizer t(_new<FileInputStream>(entry));

                // пропускаем первую строку, где написано предупреждение, что этот файл
                // сгенерирован автоматически.
                t.readStringUntilUnescaped('\n');

                // попробуем прочитать хеш со строчки. он должен начинаться с комментария.
                AString desiredBeginning = "// hash: ";
                AString actualBeginning = t.readString(desiredBeginning.length());
                if (actualBeginning == desiredBeginning) {
                    // наш клиент.
                    auto targetFileHash = t.readStringUntilUnescaped('\n');
                    if (targetFileHash == fileHash) {
                        std::cout << "skipped " << inputFilePath << std::endl;
                        return;
                    }
                }
            } catch (...) {

            }


            buffer << uint32_t(data.getSize());
            buffer << data;

            AByteBuffer packed;
            LZ::compress(buffer, packed);

            auto cppObjectName = entry.filenameWithoutExtension();

            auto out = _new<FileOutputStream>(entry);
            *out << "// This file is autogenerated by AUI.Toolkit. Please do not modify.\n"
                    "// hash: "_as << fileHash << "\n"
                                                  "\n"
                                                  "#include \"AUI/Common/AByteBuffer.h\"\n"
                                                  "#include \"AUI/Util/BuiltinFiles.h\"\n"
                                                  "const static unsigned char AUI_PACKED_asset"_as << cppObjectName
                 << "[] = {"_as;
            for (uint8_t c : packed) {
                char buf[32];
                sprintf(buf, "0x%02x,", c);
                *out << AString(buf);
            }
            *out << "};\n"_as;


            *out << "struct Assets"_as << cppObjectName << " {\n"_as
                 << "\tAssets"_as + cppObjectName + "(){\n"_as
                         "\t\tBuiltinFiles::load(AUI_PACKED_asset"_as
                 << cppObjectName
                 << ", sizeof(AUI_PACKED_asset"_as
                 << cppObjectName << "));\n\t}\n};\n"
                                                 "static Assets"_as
                 << cppObjectName
                 << " a"_as << cppObjectName << ";"_as;
            out = nullptr;
            std::cout << inputFilePath << " -> " << entry.filename()
                      << std::endl;
        }
        catch (std::exception& e)
        {
            std::cout << "Warning: could not read file " << entry << ": " << e.what() << std::endl;
        }
        catch (AException& e)
        {
            std::cout << "Warning: could not read file " << entry << ": " << e.getMessage().toStdString() << std::endl;
        }
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
