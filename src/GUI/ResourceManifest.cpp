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
	anImage->mID = theName;
	mGroupMap[theGroup].mImageMap[theName] = anImage;
	return anImage;

}
std::shared_ptr<ResourceSound> ResourceManifest::AddSound(std::string theGroup, std::string theName)
{
	auto aSound = std::make_shared<ResourceSound>();
	aSound->mID = theName;
	mGroupMap[theGroup].mSoundMap[theName] = aSound;
	return aSound;
}

std::shared_ptr<ResourceFont> ResourceManifest::AddFont(std::string theGroup, std::string theName)
{
	auto aFont = std::make_shared<ResourceFont>();
	aFont->mID = theName;
	mGroupMap[theGroup].mFontMap[theName] = aFont;
	return aFont;
}

void ResourceManifest::DeleteItem(std::string theName)
{
	for (auto group : mGroupMap)
	{
		for (auto el : group.second.mImageMap)
		{
			if (el.first == theName)
			{
				group.second.mImageMap.erase(theName);
				break;
			}
		}
		for (auto el : group.second.mSoundMap)
		{
			if (el.first == theName)
			{
				group.second.mSoundMap.erase(theName);
				break;
			}
		}
		for (auto el : group.second.mFontMap)
		{
			if (el.first == theName)
			{
				group.second.mFontMap.erase(theName);
				break;
			}
		}
	}
}

void ResourceManifest::Export(std::string theXMLPath)
{
	XMLDocument aDocument;

	XMLElement* aRoot = aDocument.NewElement("ResourceManifest");
	aDocument.InsertFirstChild(aRoot);

	for (auto group : mGroupMap)
	{
		XMLElement* aGroup = aDocument.NewElement("Resources");
		aGroup->SetAttribute("id", group.first.c_str());
		aRoot->InsertFirstChild(aGroup);
		
		for (auto image : group.second.mImageMap)
		{
			XMLElement* anImage = aGroup->InsertNewChildElement("Image");
			anImage->SetAttribute("id", image.first.c_str());
			anImage->SetAttribute("path", image.second->mPath.c_str());

			if (image.second->mHasAlphaMask)
				anImage->SetAttribute("alphagrid", image.second->mAlphaGrid.c_str());
		}
		for (auto sound : group.second.mSoundMap)
		{
			XMLElement* anImage = aGroup->InsertNewChildElement("Sound");
			anImage->SetAttribute("id", sound.first.c_str());
			anImage->SetAttribute("path", sound.second->mPath.c_str());
		}
		for (auto font : group.second.mFontMap)
		{
			XMLElement* anImage = aGroup->InsertNewChildElement("Font");
			anImage->SetAttribute("id", font.first.c_str());
			anImage->SetAttribute("path", font.second->mPath.c_str());
		}
	}


	aDocument.SaveFile(theXMLPath.c_str());
}

void ResourceManifest::Import(std::string theXMLPath)
{
	XMLDocument aDocument;
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
		for (XMLElement* image = e->FirstChildElement("Image"); image != nullptr; image = image->NextSiblingElement("Image"))
		{
			std::string imageID = image->Attribute("id");
			auto res = AddImage(resourceGroup, imageID);
			res->mPath = image->Attribute("path");
			res->mHasAlphaMask = image->Attribute("alphagrid") != 0;
			if (res->mHasAlphaMask)
				res->mAlphaGrid = image->Attribute("alphagrid");
		}
		for (XMLElement* sound = e->FirstChildElement("Sound"); sound != nullptr; sound = sound->NextSiblingElement("Sound"))
		{
			std::string soundID = sound->Attribute("id");
			auto res = AddSound(resourceGroup, soundID);
			res->mPath = sound->Attribute("path");
		}
		for (XMLElement* font = e->FirstChildElement("Font"); font != nullptr; font = font->NextSiblingElement("Font"))
		{
			std::string fontID = font->Attribute("id");
			auto res = AddFont(resourceGroup, fontID);
			res->mPath = font->Attribute("path");
		}
	}
}