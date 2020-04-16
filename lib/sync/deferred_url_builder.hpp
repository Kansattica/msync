#ifndef DEFERRED_URL_BUILDER
#define DEFERRED_URL_BUILDER

#include <string>
#include <string_view>

class deferred_url_builder
{
public:
	deferred_url_builder(std::string_view instance_url) : instance_url(instance_url)
	{ }

	const std::string& status_url();
	const std::string& media_url();

private:
	std::string& make_if_empty(std::string& field, std::string_view route);
	std::string_view instance_url;
	std::string cached_status_url;
	std::string cached_media_url;
};

#endif
