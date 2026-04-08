#pragma once

#include <string>
#include <map>
#include <memory>
#include "ResourceItem.h"

class BaseResource
{
	public:
		std::string mID;
		std::string mPath;
		ResourceSubType mType;
};

class DefaultSettings : public BaseResource
{
	public:
		std::string mIDPrefix;
};

class ResourceSound : public BaseResource
{
	public:
		double mVolume = 0.0;
		int mPanning = 0;
};
class ResourceFont : public BaseResource
{
	public:
		bool mIsSystemFont = false;
		int mSize = 0;
		bool mBold = false;
		bool mItalic = false;
		bool mShadow = false;
		bool mUnderline = false;
};
class ResourceImage : public BaseResource
{
	public:
		bool mHasAlphaMask = false;
		std::string mAlphaGrid = "";
		int mRows = 1;
		int mCols = 1;
		std::string mPixelFormat;
		bool mNoAlpha = false;
		bool mPalletize = false;
		bool mMinimizeSubdivisions = false;
};

struct ResourceGroup
{
	std::vector<std::pair<std::string, std::shared_ptr<BaseResource>>> mResourceMap;
	std::shared_ptr<BaseResource> GetResource(std::string theID)
	{
		auto it = std::find_if(mResourceMap.begin(), mResourceMap.end(),
			[&](const auto& pair) { return pair.first == theID; });

		if (it != mResourceMap.end())
			return it->second;

		return std::make_shared<ResourceImage>();
	}
	std::shared_ptr<ResourceImage> GetImage(std::string theID)
	{
		auto it = std::find_if(mResourceMap.begin(), mResourceMap.end(),
			[&](const auto& pair) { return pair.first == theID; });

		if (it != mResourceMap.end())
			return std::static_pointer_cast<ResourceImage>(it->second);

		return std::make_shared<ResourceImage>();
	}
	std::shared_ptr<ResourceSound> GetSound(std::string theID)
	{
		auto it = std::find_if(mResourceMap.begin(), mResourceMap.end(),
			[&](const auto& pair) { return pair.first == theID; });

		if (it != mResourceMap.end())
			return std::static_pointer_cast<ResourceSound>(it->second);

		return std::make_shared<ResourceSound>();
	}
	std::shared_ptr<ResourceFont> GetFont(std::string theID)
	{
		auto it = std::find_if(mResourceMap.begin(), mResourceMap.end(),
			[&](const auto& pair) { return pair.first == theID; });

		if (it != mResourceMap.end())
			return std::static_pointer_cast<ResourceFont>(it->second);

		return std::make_shared<ResourceFont>();
	}
	std::shared_ptr<DefaultSettings> GetDefaultSettings(std::string theID)
	{
		auto it = std::find_if(mResourceMap.begin(), mResourceMap.end(),
			[&](const auto& pair) { return pair.first == theID; });

		if (it != mResourceMap.end())
			return std::static_pointer_cast<DefaultSettings>(it->second);

		return std::make_shared<DefaultSettings>();
	}
};

class ResourceManifest
{
public:
	ResourceManifest();
	~ResourceManifest();

	void AddGroup(std::string theName);
	std::shared_ptr<ResourceImage> AddImage(std::string theGroup, std::string theName);
	std::shared_ptr<ResourceSound> AddSound(std::string theGroup, std::string theName);
	std::shared_ptr<ResourceFont> AddFont(std::string theGroup, std::string theName);
	std::shared_ptr<DefaultSettings> AddDefaultSettings(std::string theGroup, std::string theName);
	void DeleteItem(std::string theGroup, std::string theName);
	void Export(std::string theXMLPath);
	void Import(std::string theXMLPath);
	std::shared_ptr<BaseResource> GetResource(std::string theGroup, std::string theName);
	void RemoveResource(std::string theGroup, std::string theName);

	std::map<std::string, ResourceGroup> mGroupMap;

private:

};