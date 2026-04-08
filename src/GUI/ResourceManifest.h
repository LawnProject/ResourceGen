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
		double mVolume = -1.0;
		double mPanning = -1.0;
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
		std::string mAlphaMask = "";
		std::string mAlphaGrid = "";
		std::string mAlphaColor = "";
		std::string mVariant = "";
		int mRows = 1;
		int mCols = 1;
		std::string mPixelFormat = "default";
		bool mNoAlpha = false;
		bool mPalletize = true;
		bool mMinimizeSubdivisions = false;
		bool mNoBits3D = false;
		bool mNoBits2D = false;
		bool mNoBits = false;
		bool mDDSurface = false;

		//Animation stuff

		std::string mAnimationType = "none";
		int mFrameDelay = -1;
		int mBeginDelay = -1;
		int mEndDelay = -1; 
		std::string mPerFrameDelay = ""; //both of em are formatted: 1,2,3,4,5,etc
		std::string mFrameMap = "";
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

enum class FrameworkVersion
{
	VERSION_SEXYAPPFRAMEWORK,
	VERSION_RESODDEDFRAMEWORK,
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
	FrameworkVersion mFrameworkVersion = FrameworkVersion::VERSION_SEXYAPPFRAMEWORK;

private:

};