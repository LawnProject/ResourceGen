#pragma once

#include "ResourceManifest.h"
#include <fmt/format.h>

class ResourceSourceGen
{
public:
	ResourceSourceGen() {}
	~ResourceSourceGen() {}
	
	//todo: function prefix
#define FUNCTION_PREFIX_TODO "Lawn" //TODO: REPLACE

	static std::string GenerateSourceFile(const ResourceManifest& theManifest, const std::string& theFilename)
	{
		std::string aSrcOutput;

		aSrcOutput += fmt::format("#include \"{}.h\"\n", theFilename.c_str());
		aSrcOutput += "#include \"SexyAppFramework/ResourceManager.h\"\n";
		aSrcOutput += "using namespace Sexy;\n\n";
		aSrcOutput += "#pragma warning(disable:4311 4312)\n\n";
		aSrcOutput += "static bool gNeedRecalcVariableToIdMap = false;\n\n";

		// ExtractResourcesByName
		aSrcOutput += fmt::format("bool Sexy::{}ExtractResourcesByName(ResourceManager * theManager, const char* theName)\n{{\n", FUNCTION_PREFIX_TODO);
		bool aCanGenerateElse = false;
		for (auto group : theManifest.mGroupMap)
		{
			if (aCanGenerateElse)
				aSrcOutput += fmt::format("\telse if (strcmp(theName,\"{}\")==0) return {}Extract{}Resources(theManager);\n", group.first.c_str(), FUNCTION_PREFIX_TODO, group.first.c_str());
			else
				aSrcOutput += fmt::format("\tif (strcmp(theName,\"{}\")==0) return {}Extract{}Resources(theManager);\n", group.first.c_str(), FUNCTION_PREFIX_TODO, group.first.c_str());
			aCanGenerateElse = true;
		}
			
		aSrcOutput += "\treturn false;\n}\n\n";

		// GetIdByStringId

		aSrcOutput += fmt::format("Sexy::{}ResourceId Sexy::{}GetIdByStringId(const char *theStringId)\n{{\n", FUNCTION_PREFIX_TODO, FUNCTION_PREFIX_TODO);
		aSrcOutput += "\ttypedef std::map<std::string,int> MyMap;\n";
		aSrcOutput += "\tstatic MyMap aMap;\n";
		aSrcOutput += "\tif(aMap.empty())\n\t{\n";
		aSrcOutput += fmt::format("\t\tfor(int i=0; i<{}RESOURCE_ID_MAX; i++)\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += fmt::format("\t\t\taMap[{}GetStringIdById(i)] = i;\n\t}}\n\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += "\tMyMap::iterator anItr = aMap.find(theStringId);\n";
		aSrcOutput += "\tif (anItr == aMap.end())\n";
		aSrcOutput += fmt::format("\t\treturn {}RESOURCE_ID_MAX;\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += "\telse\n";
		aSrcOutput += fmt::format("\t\treturn ({}ResourceId) anItr->second;\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += "}\n\n";

		for (auto group : theManifest.mGroupMap)
		{
			aSrcOutput += fmt::format("// {} Resources\n", group.first.c_str());
			DefaultSettings* aLastSettings = nullptr;
			for (auto res : group.second.mResourceMap)
			{
				std::string aTypeStr;
				switch (res.second->mType)
				{
					case ResourceSubType::TYPE_IMAGE:
						aTypeStr = "Image*";
						break;
					case ResourceSubType::TYPE_FONT:
						aTypeStr = "Font*";
						break;
					case ResourceSubType::TYPE_SOUND:
						aTypeStr = "int";
						break;
					case ResourceSubType::TYPE_DEFAULT_SETTINGS:
						aLastSettings = group.second.GetDefaultSettings(res.second->mID).get();
						continue;
					default:
						continue;
				}
				if (aLastSettings != nullptr)
					aSrcOutput += fmt::format("{} Sexy::{};\n", aTypeStr.c_str(), (aLastSettings->mIDPrefix + res.second->mID).c_str());
				else
					aSrcOutput += fmt::format("{} Sexy::{};\n", aTypeStr.c_str(), res.second->mID.c_str());

			}
			aSrcOutput += "\n";

			aSrcOutput += fmt::format("bool Sexy::{}Extract{}Resources(ResourceManager *theManager)\n", FUNCTION_PREFIX_TODO, group.first.c_str());
			aSrcOutput += "{\n";
			aSrcOutput += "\tgNeedRecalcVariableToIdMap = true;\n\n";
			aSrcOutput += "\tResourceManager &aMgr = *theManager;\n";
			aSrcOutput += "\ttry\n";
			aSrcOutput += "\t{\n";
			aLastSettings = nullptr;
			for (auto res : group.second.mResourceMap)
			{
				std::string aMethodName;
				switch (res.second->mType)
				{
					case ResourceSubType::TYPE_IMAGE: aMethodName = "GetImageThrow"; break;
					case ResourceSubType::TYPE_SOUND: aMethodName = "GetSoundThrow"; break;
					case ResourceSubType::TYPE_FONT: aMethodName = "GetFontThrow"; break;					
					case ResourceSubType::TYPE_DEFAULT_SETTINGS:
						aLastSettings = group.second.GetDefaultSettings(res.second->mID).get();
						continue;
				}

				if (aLastSettings == nullptr)
					aSrcOutput += fmt::format("\t\t{} = aMgr.{}(\"{}\");\n", res.second->mID, aMethodName.c_str(), res.second->mID.c_str());
				else
					aSrcOutput += fmt::format("\t\t{} = aMgr.{}(\"{}\");\n", (aLastSettings->mIDPrefix + res.second->mID).c_str(), aMethodName.c_str(), (aLastSettings->mIDPrefix + res.second->mID).c_str());
			}
			aSrcOutput += "\t}\n";
			aSrcOutput += "\tcatch(ResourceManagerException&)\n";
			aSrcOutput += "\t{\n";
			aSrcOutput += "\t\treturn false;\n";
			aSrcOutput += "\t}\n";
			aSrcOutput += "\treturn true;\n";
			aSrcOutput += "}\n\n";

		}

		// gResources

		aSrcOutput += "static void* gResources[] =\n";
		aSrcOutput += "{\n";

		for (auto group : theManifest.mGroupMap)
		{
			DefaultSettings* aLastSettings = nullptr;
			for (auto res : group.second.mResourceMap)
			{
				if (res.second->mType == ResourceSubType::TYPE_DEFAULT_SETTINGS)
				{
					aLastSettings = group.second.GetDefaultSettings(res.second->mID).get();
					continue;
				}
				if (aLastSettings == nullptr)
					aSrcOutput += fmt::format("\t&{},\n", res.second->mID.c_str());
				else
					aSrcOutput += fmt::format("\t&{}{},\n", aLastSettings->mIDPrefix.c_str(), res.second->mID.c_str());

			}
		}
		aSrcOutput += "\tnullptr\n";
		aSrcOutput += "};\n\n";

		// LoadImageById

		aSrcOutput += fmt::format("Image* Sexy::{}LoadImageById(ResourceManager *theManager, int theId)\n{{\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += fmt::format("\treturn (*((Image**)gResources[theId]) = theManager->LoadImage({}GetStringIdById(theId)));\n}}\n\n", FUNCTION_PREFIX_TODO);

		// ReplaceImageById

		aSrcOutput += fmt::format("void Sexy::{}ReplaceImageById(ResourceManager *theManager, int theId, Image *theImage)\n{{\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += fmt::format("\ttheManager->ReplaceImage({}GetStringIdById(theId),theImage);\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += "\t * (Image**)gResources[theId] = theImage; \n}\n\n";

		// GetImageById

		aSrcOutput += fmt::format("Image* Sexy::{}GetImageById(int theId)\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += "{\n";
		aSrcOutput += "\treturn *(Image**)gResources[theId];\n";
		aSrcOutput += "}\n\n";

		// GetFontById

		aSrcOutput += fmt::format("Font* Sexy::{}GetFontById(int theId)\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += "{\n";
		aSrcOutput += "\treturn *(Font**)gResources[theId];\n";
		aSrcOutput += "}\n\n";

		// GetSoundById

		aSrcOutput += fmt::format("int Sexy::{}GetSoundById(int theId)\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += "{\n";
		aSrcOutput += "\treturn *(int*)gResources[theId];\n";
		aSrcOutput += "}\n\n";

		// GetImageRefById

		aSrcOutput += fmt::format("Image* Sexy::{}GetImageRefById(int theId)\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += "{\n";
		aSrcOutput += "\treturn *(Image**)gResources[theId];\n";
		aSrcOutput += "}\n\n";

		// GetFontRefById

		aSrcOutput += fmt::format("Image* Sexy::{}GetFontById(int theId)\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += "{\n";
		aSrcOutput += "\treturn *(Font**)gResources[theId];\n";
		aSrcOutput += "}\n\n";

		// GetSoundRefById

		aSrcOutput += fmt::format("Image* Sexy::{}GetSoundRefById(int theId)\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += "{\n";
		aSrcOutput += "\treturn *(int*)gResources[theId];\n";
		aSrcOutput += "}\n\n";

		// GetIdByVariable

		aSrcOutput += fmt::format("static Sexy::{}ResourceId {}GetIdByVariable(const void *theVariable)\n{{\n", FUNCTION_PREFIX_TODO, FUNCTION_PREFIX_TODO);
		aSrcOutput += "\ttypedef std::map<int,int> MyMap;";
		aSrcOutput += "\tstatic MyMap aMap;\n";
		aSrcOutput += "\tif(gNeedRecalcVariableToIdMap)\n\t{\n";
		aSrcOutput += "\t\tgNeedRecalcVariableToIdMap = false;\n";
		aSrcOutput += "\t\taMap.clear();\n";
		aSrcOutput += fmt::format("\t\tfor(int i=0; i<{}RESOURCE_ID_MAX; i++)\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += "\t\t\taMap[*(int*)gResources[i]] = i;\n\t}\n\n";
		aSrcOutput += "\tMyMap::iterator anItr = aMap.find((int)theVariable);\n";
		aSrcOutput += "\tif (anItr == aMap.end())\n";
		aSrcOutput += fmt::format("\t\treturn {}RESOURCE_ID_MAX;\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += "\telse\n";
		aSrcOutput += fmt::format("\t\treturn ({}ResourceId) anItr->second;\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += "}\n\n";

		// GetIdByImage

		aSrcOutput += fmt::format("Sexy::{}ResourceId Sexy::{}GetIdByImage(Image *theImage)\n", FUNCTION_PREFIX_TODO, FUNCTION_PREFIX_TODO);
		aSrcOutput += "{\n";
		aSrcOutput += fmt::format("\treturn {}GetIdByVariable(theImage);\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += "}\n\n";

		// GetIdByFont

		aSrcOutput += fmt::format("Sexy::{}ResourceId Sexy::{}GetIdByFont(Font *theFont)\n", FUNCTION_PREFIX_TODO, FUNCTION_PREFIX_TODO);
		aSrcOutput += "{\n";
		aSrcOutput += fmt::format("\treturn {}GetIdByVariable(theFont);\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += "}\n\n";

		// GetIdBySound

		aSrcOutput += fmt::format("Sexy::{}ResourceId Sexy::{}GetIdBySound(int theSound)\n", FUNCTION_PREFIX_TODO, FUNCTION_PREFIX_TODO);
		aSrcOutput += "{\n";
		aSrcOutput += fmt::format("\treturn {}GetIdByVariable(theSound);\n", FUNCTION_PREFIX_TODO);
		aSrcOutput += "}\n\n";

		// GetStringIdById

		aSrcOutput += fmt::format("const char* Sexy::{}GetStringIdById(int theId)\n", FUNCTION_PREFIX_TODO, FUNCTION_PREFIX_TODO);
		aSrcOutput += "{\n";
		aSrcOutput += "\tswitch(theId)\n";
		aSrcOutput += "\t{\n";

		for (auto group : theManifest.mGroupMap)
		{
			DefaultSettings* aLastSettings = nullptr;
			for (auto res : group.second.mResourceMap)
			{
				if (res.second->mType == ResourceSubType::TYPE_DEFAULT_SETTINGS)
				{
					aLastSettings = group.second.GetDefaultSettings(res.second->mID).get();
					continue;
				}
				if (aLastSettings == nullptr)
					aSrcOutput += fmt::format("\t\tcase {0}_ID: return \"{0}\";\n", res.second->mID.c_str());
				else
					aSrcOutput += fmt::format("\t\tcase {0}_ID: return \"{0}\";\n", (aLastSettings->mIDPrefix + res.second->mID).c_str());

			}
		}
		aSrcOutput += "\t\tdefault: return \"\";\n";
		aSrcOutput += "\t}\n";
		aSrcOutput += "}\n";
		aSrcOutput += "\n";


		return aSrcOutput;
	}

	static std::string GenerateHeaderFile(const ResourceManifest& theManifest, const std::string& theFilename)
	{
		std::string aHeaderOutput;

		aHeaderOutput += "#pragma once\n";
		aHeaderOutput += "namespace Sexy\n";
		aHeaderOutput += "{\n";
		aHeaderOutput += "\tclass ResourceManager;\n";
		aHeaderOutput += "\tclass Image;\n";
		aHeaderOutput += "\tclass Font;\n";

		aHeaderOutput += fmt::format("\tImage* {}LoadImageById(ResourceManager *theManager, int theId);", FUNCTION_PREFIX_TODO);
		aHeaderOutput += fmt::format("\tvoid {}ReplaceImageById(ResourceManager *theManager, int theId, Image *theImage);\n", FUNCTION_PREFIX_TODO);
		aHeaderOutput += fmt::format("\tbool {}ExtractResourcesByName(ResourceManager *theManager, const char *theName);\n\n", FUNCTION_PREFIX_TODO);
		aHeaderOutput += fmt::format("\tbool {}ExtractResourcesByName(ResourceManager *theManager, const char *theName);\n\n", FUNCTION_PREFIX_TODO);

		for (auto group : theManifest.mGroupMap)
		{
			aHeaderOutput += fmt::format("\t// {} Resources\n", group.first.c_str());
			aHeaderOutput += fmt::format("\tbool {}Extract{}Resources(ResourceManager *theMgr);\n", FUNCTION_PREFIX_TODO, group.first.c_str());

			DefaultSettings* aLastSettings = nullptr;
			for (auto res : group.second.mResourceMap)
			{
				std::string aTypeStr;
				switch (res.second->mType)
				{
				case ResourceSubType::TYPE_IMAGE:
					aTypeStr = "Image*";
					break;
				case ResourceSubType::TYPE_FONT:
					aTypeStr = "Font*";
					break;
				case ResourceSubType::TYPE_SOUND:
					aTypeStr = "int";
					break;
				case ResourceSubType::TYPE_DEFAULT_SETTINGS:
					aLastSettings = group.second.GetDefaultSettings(res.second->mID).get();
					continue;
				default:
					continue;
				}
				if (aLastSettings != nullptr)
					aHeaderOutput += fmt::format("\textern {} {};\n", aTypeStr.c_str(), (aLastSettings->mIDPrefix + res.second->mID).c_str());
				else
					aHeaderOutput += fmt::format("\textern {} {};\n", aTypeStr.c_str(), res.second->mID.c_str());
			}

			aHeaderOutput += "\n";

		}

		aHeaderOutput += fmt::format("\tenum {}ResourceId\n", FUNCTION_PREFIX_TODO);
		aHeaderOutput += "\t{\n";

		for (auto group : theManifest.mGroupMap)
		{
			DefaultSettings* aLastSettings = nullptr;
			for (auto res : group.second.mResourceMap)
			{
				if (res.second->mType == ResourceSubType::TYPE_DEFAULT_SETTINGS)
				{
					aLastSettings = group.second.GetDefaultSettings(res.second->mID).get();
					continue;
				}
				if (aLastSettings != nullptr)
					aHeaderOutput += fmt::format("\t\t{}_ID,\n", (aLastSettings->mIDPrefix + res.second->mID).c_str());
				else
					aHeaderOutput += fmt::format("\t\t{}_ID,\n", res.second->mID.c_str());

				//uhh todo: Aliases? whatever are they
			}

		}
		aHeaderOutput += fmt::format("\t\t{}RESOURCE_ID_MAX\n", FUNCTION_PREFIX_TODO);
		aHeaderOutput += "\t};\n\n";
		aHeaderOutput += fmt::format("\tImage* {}GetImageById(int theId);\n", FUNCTION_PREFIX_TODO);
		aHeaderOutput += fmt::format("\tFont* {}GetFontById(int theId);\n", FUNCTION_PREFIX_TODO);
		aHeaderOutput += fmt::format("\tint {}GetSoundById(int theId);\n\n", FUNCTION_PREFIX_TODO);
		aHeaderOutput += fmt::format("\tImage*& {}GetImageRefById(int theId);\n", FUNCTION_PREFIX_TODO);
		aHeaderOutput += fmt::format("\tFont*& {}GetFontRefById(int theId);\n", FUNCTION_PREFIX_TODO);
		aHeaderOutput += fmt::format("\tint& {}GetSoundRefById(int theId);\n\n", FUNCTION_PREFIX_TODO);

		aHeaderOutput += fmt::format("\t{}ResourceId {}GetIdByImage(Image *theImage);\n", FUNCTION_PREFIX_TODO, FUNCTION_PREFIX_TODO);
		aHeaderOutput += fmt::format("\t{}ResourceId {}GetIdByFont(Font *theFont);\n", FUNCTION_PREFIX_TODO, FUNCTION_PREFIX_TODO);
		aHeaderOutput += fmt::format("\t{}ResourceId {}GetIdBySound(int theSound);\n\n", FUNCTION_PREFIX_TODO, FUNCTION_PREFIX_TODO);
		aHeaderOutput += fmt::format("\tconst char* {}GetStringIdById(int theId);\n", FUNCTION_PREFIX_TODO);
		aHeaderOutput += fmt::format("\t{}ResourceId {}GetIdByStringId(const char *theStringId);\n\n", FUNCTION_PREFIX_TODO, FUNCTION_PREFIX_TODO);

		aHeaderOutput += "} // namespace Sexy\n";


		return aHeaderOutput;
	}

private:

};
