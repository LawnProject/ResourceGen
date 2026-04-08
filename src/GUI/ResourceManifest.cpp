#include "ResourceManifest.h"

#include <tinyxml2.h>
#include <wx/log.h>
using namespace tinyxml2;

ResourceManifest::ResourceManifest()
{
}

ResourceManifest::~ResourceManifest()
{
}

void ResourceManifest::AddGroup(std::string theName)
{
	mGroupMap[theName] = {};
}

std::shared_ptr<ResourceImage> ResourceManifest::AddImage(std::string theGroup, std::string theName)
{
	auto anImage = std::make_shared<ResourceImage>();
	anImage->mType = ResourceSubType::TYPE_IMAGE;
	anImage->mID = theName;
	mGroupMap[theGroup].mResourceMap.push_back({ theName, anImage });
	return anImage;

}
std::shared_ptr<ResourceSound> ResourceManifest::AddSound(std::string theGroup, std::string theName)
{
	auto aSound = std::make_shared<ResourceSound>();
	aSound->mType = ResourceSubType::TYPE_SOUND;
	aSound->mID = theName;
	mGroupMap[theGroup].mResourceMap.push_back({ theName, aSound });
	return aSound;
}

std::shared_ptr<ResourceFont> ResourceManifest::AddFont(std::string theGroup, std::string theName)
{
	auto aFont = std::make_shared<ResourceFont>();
	aFont->mType = ResourceSubType::TYPE_FONT;
	aFont->mID = theName;
	mGroupMap[theGroup].mResourceMap.push_back({ theName, aFont });
	return aFont;
}

std::shared_ptr<DefaultSettings> ResourceManifest::AddDefaultSettings(std::string theGroup, std::string theName)
{
	auto aSettings = std::make_shared<DefaultSettings>();
	aSettings->mType = ResourceSubType::TYPE_DEFAULT_SETTINGS;
	aSettings->mID = theName;
	mGroupMap[theGroup].mResourceMap.push_back({ theName, aSettings });
	return aSettings;
}

void ResourceManifest::DeleteItem(std::string theGroup, std::string theName)
{
	RemoveResource(theGroup, theName);
}

void ResourceManifest::Export(std::string theXMLPath)
{
	tinyxml2::XMLDocument aDocument; //why is this ambiguous???

	XMLElement* aRoot = aDocument.NewElement("ResourceManifest");
	aDocument.InsertFirstChild(aRoot);

	for (auto group : mGroupMap)
	{
		XMLElement* aGroup = aDocument.NewElement("Resources");
		aGroup->SetAttribute("id", group.first.c_str());
		aRoot->InsertFirstChild(aGroup);
		
		for (auto pair : group.second.mResourceMap)
		{
			auto element = pair.second;
			switch (element->mType)
			{
				case ResourceSubType::TYPE_IMAGE:
				{
					std::shared_ptr<ResourceImage> image = std::static_pointer_cast<ResourceImage>(element);
					XMLElement* anImage = aGroup->InsertNewChildElement("Image");
					anImage->SetAttribute("id", element->mID.c_str());
					anImage->SetAttribute("path", element->mPath.c_str());

					if (!image->mAlphaMask.empty())
						anImage->SetAttribute("alphaimage", image->mAlphaMask.c_str());

					if (!image->mAlphaGrid.empty())
						anImage->SetAttribute("alphagrid", image->mAlphaGrid.c_str());

					if (!image->mAlphaColor.empty())
						anImage->SetAttribute("alphacolor", image->mAlphaColor.c_str());

					if (image->mCols > 1)
						anImage->SetAttribute("cols", image->mCols);

					if (image->mRows > 1)
						anImage->SetAttribute("rows", image->mRows);

					if (image->mNoAlpha)
						anImage->SetAttribute("noalpha", "");

					if (mFrameworkVersion == FrameworkVersion::VERSION_SEXYAPPFRAMEWORK)
					{
						if (image->mMinimizeSubdivisions)
							anImage->SetAttribute("minsubdivide", "");

						if (!image->mPalletize)
							anImage->SetAttribute("nopal", "");
					}

					if (image->mNoBits)
						anImage->SetAttribute("nobits", "");

					if (image->mNoBits2D)
						anImage->SetAttribute("nobits2d", "");

					if (image->mNoBits3D)
						anImage->SetAttribute("nobits3d", "");

					if (image->mDDSurface)
						anImage->SetAttribute("ddsurface", "");

					if (image->mPixelFormat != "default")
						anImage->SetAttribute(image->mPixelFormat.c_str(), "");

					break;
				}
				case ResourceSubType::TYPE_SOUND:
				{
					std::shared_ptr<ResourceSound> image = std::static_pointer_cast<ResourceSound>(element);
					XMLElement* anImage = aGroup->InsertNewChildElement("Sound");
					anImage->SetAttribute("id", element->mID.c_str());
					anImage->SetAttribute("path", element->mPath.c_str());
					break;
				}
				case ResourceSubType::TYPE_FONT:
				{
					std::shared_ptr<ResourceFont> image = std::static_pointer_cast<ResourceFont>(element);
					XMLElement* anImage = aGroup->InsertNewChildElement("Sound");
					anImage->SetAttribute("id", element->mID.c_str());
					anImage->SetAttribute("path", element->mPath.c_str());
					break;
				}
				case ResourceSubType::TYPE_DEFAULT_SETTINGS:
				{
					std::shared_ptr<DefaultSettings> settings = std::static_pointer_cast<DefaultSettings>(element);
					XMLElement* anImage = aGroup->InsertNewChildElement("SetDefaults");
					anImage->SetAttribute("idprefix", settings->mIDPrefix.c_str());
					anImage->SetAttribute("path", element->mPath.c_str());
					break;
				}
			}

		}
	}


	aDocument.SaveFile(theXMLPath.c_str());
}

void ResourceManifest::Import(std::string theXMLPath)
{
	mGroupMap.clear();
	tinyxml2::XMLDocument aDocument;
	aDocument.LoadFile(theXMLPath.c_str());
	XMLElement* aManifestRoot = aDocument.FirstChildElement();
	const char* aName = aManifestRoot->Name();
	if (strcmp(aName, "ResourceManifest") != 0)
	{
		wxLogError("Resource File doesn't start with <ResourceManifest>");
		return;
	}

	for (XMLElement* e = aManifestRoot->FirstChildElement("Resources"); e != nullptr; e = e->NextSiblingElement("Resources"))
	{
		std::string resourceGroup = e->Attribute("id");
		AddGroup(resourceGroup);
		for (XMLElement* child = e->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
		{
			std::string aName = child->Name();
			std::string resID = child->Attribute("id") == 0 ? "ERR_NO_ID" : child->Attribute("id");
			std::string resPath = child->Attribute("path");

			if (aName == "SetDefaults")
			{
				int currentSettingID = 0;
				std::string possibleID = wxString::Format("SETTINGS_%d", currentSettingID).ToStdString();
				while (GetResource(resourceGroup, possibleID) != nullptr)
				{
					currentSettingID++;
					possibleID = wxString::Format("SETTINGS_%d", currentSettingID).ToStdString();
				}
				auto res = AddDefaultSettings(resourceGroup, possibleID);
				res->mPath = resPath;
				res->mIDPrefix = child->Attribute("idprefix");
			}
			else if (aName == "Image")
			{
				auto res = AddImage(resourceGroup, resID);
				res->mPath = resPath;
				res->mPalletize = child->Attribute("nopal") == 0;
				res->mMinimizeSubdivisions = child->Attribute("minsubdivide") != 0;
				res->mNoAlpha = child->Attribute("noalpha") != 0;
				res->mNoBits = child->Attribute("nobits") != 0;
				res->mNoBits2D = child->Attribute("nobits2d") != 0;
				res->mNoBits3D = child->Attribute("nobits3d") != 0;
				res->mDDSurface = child->Attribute("ddsurface") != 0;

				if (child->Attribute("alphagrid") != 0)
					res->mAlphaGrid = child->Attribute("alphagrid");

				if (child->Attribute("alphaimage") != 0)
					res->mAlphaMask = child->Attribute("alphaimage");

				if (child->Attribute("alphacolor") != 0)
					res->mAlphaColor = child->Attribute("alphacolor");

				if (child->Attribute("cols") != 0)
					res->mCols = atoi(child->Attribute("cols"));

				if (child->Attribute("a8r8g8b8") != 0)
					res->mPixelFormat = "a8r8g8b8";
				else if (child->Attribute("a4r4g4b4") != 0)
					res->mPixelFormat = "a4r4g4b4";
				else
					res->mPixelFormat = "default";

				if (child->Attribute("rows") != 0)
					res->mRows = atoi(child->Attribute("rows"));
			}
			else if (aName == "Sound")
			{
				auto res = AddSound(resourceGroup, resID);
				res->mPath = resPath;
			}
			else if (aName == "Font")
			{
				auto res = AddFont(resourceGroup, resID);
				res->mPath = resPath;
			}
		}
	}
}

std::shared_ptr<BaseResource> ResourceManifest::GetResource(std::string theGroup, std::string theName)
{
	auto& map = mGroupMap[theGroup].mResourceMap;
	auto it = std::find_if(map.begin(), map.end(),
		[&](const auto& pair) { return pair.first == theName; });

	if (it != map.end())
		return it->second;

	return nullptr;
}

void ResourceManifest::RemoveResource(std::string theGroup, std::string theName)
{
	auto it = std::find_if(mGroupMap[theGroup].mResourceMap.begin(), mGroupMap[theGroup].mResourceMap.end(),
		[&](const auto& pair) { return pair.first == theName; });

	if (it != mGroupMap[theGroup].mResourceMap.end())
		mGroupMap[theGroup].mResourceMap.erase(it);
}
