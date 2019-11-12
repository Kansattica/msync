#ifndef _MASTODON_ENTITIES_HPP_
#define _MASTODON_ENTITIES_HPP_

struct mastodon_status
{
	std::string id;
	std::string url;
	std::string content_warning;
	std::string content;
	std::string visibility;
};

#endif