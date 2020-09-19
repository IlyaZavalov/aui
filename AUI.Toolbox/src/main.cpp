#include <filesystem>
#include <functional>
#include <iostream>
#include <AUI/IO/APath.h>


#include "AUI/Common/AString.h"
#include "AUI/Common/AStringVector.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/AMap.h"
#include "AUI/IO/FileInputStream.h"
#include "AUI/IO/FileOutputStream.h"
#include "AUI/Util/BuiltinFiles.h"
#include "AUI/Util/LZ.h"
#include <AUI/Platform/Entry.h>

void printHelp()
{
	std::cout << "Available commands:" << std::endl
		<< "\tpack <dir> <dst>\t\tpack a dir into the .h file";
	exit(-1);
}

AUI_ENTRY
{	
	std::cout << "Alex2772 Universal Interface toolbox" << std::endl;
	if (args.size() <= 1)
	{
		printHelp();
	}
	else
	{
	    std::cout << "Command line:" << std::endl;
	    for (int i = 0; i < args.size(); ++i) {
            std::cout << args[i] << " ";
        }
	    std::cout << std::endl;

		AStringVector actualArgs;
		for (int i = 2; i < args.size(); ++i)
		{
			actualArgs << args[i];
		}

		AMap<AString, std::function<void()>> commands = {
			{
				"pack", [&]()
				{
					if (actualArgs.size() != 2)
					{
						printHelp();
					}
					else
					{
					    try {
					        APath p(actualArgs[1]);
					        p.removeFileRecursive();
					        p.makeDirs();
                        } catch (...) {

                        }
						unsigned index = 0;
						try
						{
							for (auto& entry : APath(actualArgs[0]).listDir(LF_REGULAR_FILES | LF_RECURSIVE))
							{
							    ++index;
                                try
                                {
                                    AByteBuffer buffer;
                                    AString filePath = entry;
                                    filePath = filePath.mid(actualArgs[0].length() + 1);
                                    filePath = filePath.replaceAll("\\", '/');

                                    buffer << filePath.toStdString();
                                    auto fis = _new<FileInputStream>(entry);

                                    AByteBuffer data;

                                    char buf[0x1000];
                                    for (int r; (r = fis->read(buf, sizeof(buf))) > 0;)
                                    {
                                        data.put(buf, r);
                                    }
                                    buffer << uint32_t(data.getSize());
                                    buffer << data;

                                    AByteBuffer packed;
                                    LZ::compress(buffer, packed);


                                    uint32_t hash = std::hash<AString>()(actualArgs[0] + actualArgs[1]) ^index;

                                    auto path = APath(actualArgs[1])
                                            .file(("assets" + AString::number(hash) + ".cpp"));

                                    AString name(path);

                                    auto out = _new<FileOutputStream>(name);
                                    *out << AString(
                                            "// This file is autogenerated by AUI.Toolkit. Please do not modify.\n");
                                    *out << AString(
                                            "\n"
                                            "#include \"AUI/Common/AByteBuffer.h\"\n"
                                            "#include \"AUI/Util/BuiltinFiles.h\"\n");
                                    *out << AString("const static unsigned char AUI_PACKED_asset") << AString::number(hash) << AString("[] = {");
                                    for (uint8_t c : packed) {
                                        char buf[32];
                                        sprintf(buf, "0x%02x,", c);
                                        *out << AString(buf);
                                    }
                                    *out << AString("};\n");

                                    *out << AString("struct Assets" + AString::number(hash) + " {\n"
                                                                                              "\tAssets" + AString::number(hash) + "(){\n"
                                                                                                                                   "\t\tBuiltinFiles::load(AUI_PACKED_asset" +
                                                    AString::number(hash) +
                                                    ", sizeof(AUI_PACKED_asset" +
                                                    AString::number(hash) + "));\n\t}\n};\n"
                                                                            "static Assets" + AString::number(hash) +
                                                    " a" + AString::number(hash) + ";");
                                    out = nullptr;
                                    std::cout << filePath << " -> " << path
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
						catch (...)
						{
							std::cout << "Could not open directory: " << APath(actualArgs[0]).absolute() << std::endl;
							exit(-2);
						}
					}
				},

			}
		};
		if (auto c = commands.contains(args[1]))
		{
			c->second();
		}
		else
		{
			printHelp();
		}
	}

	return 0;
}
